#include "../include/DCS_ModuleAcquisition.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/internal.h"
#include "../../DCS_Network/include/internal.h"
#include "DCS_Utils/include/DCS_ModuleUtils.h"

#include <queue>
#include <thread>
#include <atomic>

static std::thread* evt_loop_dcs_thread = nullptr;
static std::thread* evt_loop_mca_thread = nullptr;
static std::thread* evt_loop_cli_thread = nullptr;

static std::atomic<int> evt_loop_sig;
static bool mca_mode = false;

#define EVT_LOOP_RUN  1
#define EVT_LOOP_STOP 0
#undef max

void DCS::DAQ::StartEventLoop(DCS::u16 mca_num_channels)
{
    mca_mode = (mca_num_channels > 0);
    evt_loop_sig.store(EVT_LOOP_RUN);
    evt_loop_dcs_thread = new std::thread(DCSCountEvent);
    if(mca_mode) evt_loop_mca_thread = new std::thread(MCACountEvent, mca_num_channels);
    evt_loop_cli_thread = new std::thread(ClinometerEvent);
}

void DCS::DAQ::StopEventLoop()
{
    evt_loop_sig.store(EVT_LOOP_STOP);
    NotifyUnblockEventLoop();
    evt_loop_dcs_thread->join();
    if(mca_mode) evt_loop_mca_thread->join();
    evt_loop_cli_thread->join();

    delete evt_loop_dcs_thread;
    if(mca_mode) delete evt_loop_mca_thread;
    delete evt_loop_cli_thread;
}


void DCS::DAQ::DCSCountEvent()
{
    // TODO : Discriminate voltage levels here also. Ruling out unwanted X-ray counts.
    while(evt_loop_sig.load())
    {
        EventData ivd = GetLastDCS_IVD();

        if(ivd.counts.num_detected == std::numeric_limits<u64>::max())
        {
            continue;
        }

        // TODO : Calculate the event additional data here
        
        DCS_EMIT_EVT((DCS::u8*)&ivd, sizeof(EventData)); // HACK : This can operate with a move ctor instead
    }
}

void DCS::DAQ::MCACountEvent(DCS::u16 mca_num_channels)
{
    while (evt_loop_sig.load())
    {
        EventData ivd = GetLastMCA_IVD();

        if (ivd.counts.num_detected == std::numeric_limits<u64>::max())
        {
            continue;
        }

        MCACountEventData evt_data;
        evt_data.count = ivd.counts.num_detected;
        evt_data.timestamp = ivd.timestamp_wall;
        evt_data.total_max_bin_count = mca_num_channels;

        if (evt_data.count > 0)
        {
            for (int i = 0; i < evt_data.count; i++)
            {
                // TODO : Don't hard code DAQ range... (currently: [0.0, 10.0] V)
                evt_data.bins[i] = u16((ivd.counts.maxima[i] / 10.0) * evt_data.total_max_bin_count);
            }
        }

        DCS_EMIT_EVT((DCS::u8*)&evt_data, sizeof(MCACountEventData)); // HACK : This can operate with a move ctor instead
    }
}

void DCS::DAQ::ClinometerEvent()
{
    while(evt_loop_sig.load())
    {
        EventData ivd = GetLastClinometer_IVD();

        if(ivd.counts.num_detected == std::numeric_limits<u64>::max())
        {
            continue;
        }

        ClinometerEventData evt_data;
        constexpr f64 max_angle = 10.0;
        constexpr f64 min_angle = -10.0;
        constexpr f64 range_angle = max_angle - min_angle;

        for(int i = 0; i < 2; i++)
        {
            evt_data.c1[i] = 0.2 * ivd.tilt_c1[i] * range_angle + min_angle;
            evt_data.c2[i] = 0.2 * ivd.tilt_c2[i] * range_angle + min_angle;
        }
        evt_data.timestamp = ivd.timestamp_wall;

        DCS_EMIT_EVT((DCS::u8*)&evt_data, sizeof(ClinometerEventData)); // HACK : This can operate with a move ctor instead
    }
}

void DCS::MControl::CurrentMeasurementProgressChangedEvent()
{
    i64 progress = GetCurrentMeasurementProgress();
    DCS_EMIT_EVT((DCS::u8*)&progress, sizeof(i64));
}

void DCS::MControl::MeasurementControlRoutineEnded()
{
    DCS_EMIT_EVT((DCS::u8*)0, 0);
}
