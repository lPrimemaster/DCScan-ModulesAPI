#include "../include/DCS_ModuleAcquisition.h"
#include "../include/internal.h"
#include "../../DCS_Utils/include/internal.h"

#include <thread>

#include <eib7.h>

#define NUM_OF_AXIS       4
#define EIB_TCP_TIMEOUT   5000
#define TIMESTAMP_PERIOD  1000   // us
#define TRIGGER_PERIOD    500000 // us
#define MAX_SRT_DATA      200

static EIB7_HANDLE eib;
static std::thread* eib_loop = nullptr;
static std::atomic<bool> eib_loop_running;

static DCS::Utils::SMessageQueue<DCS::ENC::EncoderData> outbound_data_queue;

static EIB7_AXIS axis[NUM_OF_AXIS];
static EIB7_DataRegion regions[NUM_OF_AXIS] = {
    EIB7_DR_Encoder1,
    EIB7_DR_Encoder2,
    EIB7_DR_Encoder3,
    EIB7_DR_Encoder4
};
static DCS::i8 inited_axes = 0;

static bool CheckError(EIB7_ERR error)
{
    if(error != EIB7_NoError)
    {
        char mnemonic[32];
        char message[256];

        EIB7GetErrorInfo(error, mnemonic, 32, message, 256);

        LOG_ERROR("EIB7 Error %08x (%s): %s", error, mnemonic, message);
        return true;
    }
    return false;
}

void DCS::ENC::InitEIB7Encoder(const char* hostname, i8 axes)
{
    u32 ip;
    u32 numAxes;
    char fw_version[20];
    EIB7_DataPacketSection packet[5];

    if(CheckError(EIB7GetHostIP(hostname, &ip))) return;
    if(CheckError(EIB7Open(ip, &eib, EIB_TCP_TIMEOUT, fw_version, sizeof(fw_version)))) return;
    
    if(CheckError(EIB7GetAxis(eib, axis, NUM_OF_AXIS, &numAxes))) return;

    // Init encoder axes for 1App
    for(i32 i = 0; i < (i32)numAxes; i++)
    {
        if(!(axes & (1 << i))) continue;
        if(CheckError(EIB7InitAxis(axis[i], // TODO: Change Vpp to uApp
                EIB7_IT_Incremental_11u,
                EIB7_EC_Rotary,
                EIB7_RM_None,          /* reference marks not used */
                0,                    /* reference marks not used */
                0,                    /* reference marks not used */
                EIB7_HS_None,
                EIB7_LS_None,
                EIB7_CS_CompActive,   /* signal compensation on   */
                EIB7_BW_High,         /* signal bandwidth: high   */
                EIB7_CLK_Default,     /* not used for incremental interface */
                EIB7_RT_Long,         /* not used for incremental interface */
                EIB7_CT_Long          /* not used for incremental interface */
        ))) return;

        u32 timestampTicks;
        if(CheckError(EIB7GetTimestampTicks(eib, &timestampTicks))) return;
        u32 timestampPeriod = TIMESTAMP_PERIOD * timestampTicks;
        if(CheckError(EIB7SetTimestampPeriod(eib, timestampPeriod))) return;
        if(CheckError(EIB7SetTimestamp(axis[i], EIB7_MD_Enable))) return;
        if(CheckError(EIB7StartRef(axis[i], EIB7_RP_RefPos2))) return;

        if(CheckError(EIB7AddDataPacketSection(packet, 0, EIB7_DR_Global, EIB7_PDF_TriggerCounter))) return;
        if(CheckError(EIB7AddDataPacketSection(packet, 1, regions[i], EIB7_PDF_StatusWord | EIB7_PDF_PositionData | EIB7_PDF_Timestamp | EIB7_PDF_ReferencePos))) return;
        if(CheckError(EIB7ConfigDataPacket(eib, packet, 2))) return;

        // enable internal trigger (for now)
        u32 timerTicks;
        if(CheckError(EIB7GetTimerTriggerTicks(eib, &timerTicks))) return;
        u32 timerPeriod = TRIGGER_PERIOD * timerTicks;
        if(CheckError(EIB7SetTimerTriggerPeriod(eib, timerPeriod))) return;

        if(CheckError(EIB7AxisTriggerSource(axis[i], EIB7_AT_TrgTimer))) return;
        if(CheckError(EIB7MasterTriggerSource(eib, EIB7_AT_TrgTimer))) return;

        // enable SoftRealtime mode
        if(CheckError(EIB7SelectMode(eib, EIB7_OM_SoftRealtime))) return;
    }
    inited_axes = axes;
}

void DCS::ENC::StartEIB7SoftModeTrigger()
{
    CheckError(EIB7GlobalTriggerEnable(eib, EIB7_MD_Enable, EIB7_TS_TrgTimer));
}

void DCS::ENC::EIB7SoftModeLoopStart(DCS::f64 sigperiods)
{
    if(eib_loop == nullptr)
    {
        eib_loop_running.store(true);
        eib_loop = new std::thread([&, sigperiods]() -> void {
            u8 udp_data[MAX_SRT_DATA];
            u32 entries;
            void* field;
            u32 sz;
            EncoderAxisData eadata;
            EncoderData edata;

            while(eib_loop_running.load())
            {
                // Read FIFO with UDP
                EIB7_ERR error = EIB7ReadFIFOData(eib, udp_data, 1, &entries, 0);
                if(error == EIB7_FIFOOverflow)
                {
                    LOG_WARNING("EIB7 FIFO queue overflow. Clearing.");
                    EIB7ClearFIFO(eib);
                }

                if(entries > 0)
                {
                    edata.numAxis = 0;
                    // TODO: Change this to numAxes. Don't waste cycles.
                    for(i32 i = 0; i < NUM_OF_AXIS; i++)
                    {
                        if(!(inited_axes & (1 << i))) continue;
                        edata.numAxis++;
                        // read trigger counter
                        CheckError(EIB7GetDataFieldPtr(eib, udp_data, 
                                    EIB7_DR_Global, 
                                    EIB7_PDF_TriggerCounter, 
                                    &field, &sz));
                        eadata.triggerCounter = *(u16*)field;

                        // read timestamp
                        CheckError(EIB7GetDataFieldPtr(eib, udp_data, 
                                    regions[i], 
                                    EIB7_PDF_Timestamp, 
                                    &field, &sz));
                        eadata.timestamp = *(u16*)field;

                        // read position
                        CheckError(EIB7GetDataFieldPtr(eib, udp_data, 
                                    regions[i], 
                                    EIB7_PDF_PositionData, 
                                    &field, &sz));
                        eadata.position = *(i64*)field;

                        // read status
                        CheckError(EIB7GetDataFieldPtr(eib, udp_data, 
                                    regions[i], 
                                    EIB7_PDF_StatusWord, 
                                    &field, &sz));
                        eadata.status = *(u16*)field;

                        // read ref
                        CheckError(EIB7GetDataFieldPtr(eib, udp_data, 
                                    regions[i], 
                                    EIB7_PDF_ReferencePos, 
                                    &field, &sz));
                        i64* posVal = (i64*)field;
                        eadata.ref[0] = posVal[0];
                        eadata.ref[1] = posVal[1];

                        CheckError(EIB7IncrPosToDouble(eadata.position, &eadata.calpos));
                        eadata.calpos *= 360.0 / sigperiods;

                        eadata.axis = (i8)i + 1;

                        edata.axis[i] = eadata;
                    }

                    if(edata.numAxis > 0)
                        outbound_data_queue.push(edata);
                }
                else
                {
                    static constexpr i32 spleeptime = TRIGGER_PERIOD / 10;
                    std::this_thread::sleep_for(std::chrono::microseconds(spleeptime));
                }
            }
        });
    }
    else
    {
        LOG_ERROR("Could not start EIB7 data loop.");
    }
}

void DCS::ENC::EIB7SoftModeLoopStop()
{
    if(eib_loop != nullptr)
    {
        eib_loop_running.store(false);
        eib_loop->join();
        delete eib_loop;
        eib_loop = nullptr;
    }
    else
    {
        LOG_WARNING("Cant stop. EIB7 data loop not running.");
    }
}

void DCS::ENC::StopEIB7SoftModeTrigger()
{
    CheckError(EIB7GlobalTriggerEnable(eib, EIB7_MD_Disable, EIB7_TS_All));
}

void DCS::ENC::DeleteEIB7Encoder()
{
    // disable trigger
    CheckError(EIB7GlobalTriggerEnable(eib, EIB7_MD_Disable, EIB7_TS_All));

    // disable SoftRealtime mode
    CheckError(EIB7SelectMode(eib, EIB7_OM_Polling));

    // close connection to EIB
    EIB7Close(eib);
}

DCS::ENC::EncoderData DCS::ENC::InspectLastEncoderValues()
{
    EncoderData data;

    if(outbound_data_queue.size() > 0)
    {
        data = outbound_data_queue.peekBack();
    }
    else
    {
        data.numAxis = 0;
    }

    return data;
}


void DCS::ENC::Init(const char* ip, i8 axis)
{
    InitEIB7Encoder(ip, axis);

    StartEIB7SoftModeTrigger();

    // FIXME: Transform sigperiods in param
    EIB7SoftModeLoopStart(5000.0);
}

void DCS::ENC::Terminate()
{
    EIB7SoftModeLoopStop();

    StopEIB7SoftModeTrigger();

    DeleteEIB7Encoder();
}
