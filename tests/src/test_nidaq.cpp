#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../include/DCS_Assert.h"

int main()
{
    DCS_START_TEST;

    DCS::Network::Init();

    auto c = DCS::Network::Client::Connect("127.0.0.1", 15777);

    DCS::Network::Client::Authenticate(c, "Prime", "alfa77");

    if(DCS::Network::Client::StartThread(c))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        unsigned char buffer[4096];

        auto size = DCS::Registry::SVParams::GetDataFromParams(buffer, SV_CALL_DCS_Threading_GetMaxHardwareConcurrency);

        auto max_threads = DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);

        LOG_DEBUG("Got max server concurrency: %u", *(DCS::u16*)max_threads.get().ptr);

        FILE* f = fopen("Data Dump.txt", "w");

        size = DCS::Registry::SetupEvent(buffer, SV_EVT_DCS_DAQ_VoltageEvent, [](DCS::u8* data, DCS::u8* userData){
                LOG_DEBUG("Got Voltage Event on client side.");

                DCS::f64* fdata = (DCS::f64*)data;

                FILE* f = (FILE*)userData;

                for(int i = 0 ; i < 1000; i++)
                {
                    std::string sdata = std::to_string(fdata[i]) + '\n';
                    fwrite(sdata.c_str(), 1, sdata.size(), f);
                }
            }, (DCS::u8*)f);
        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::EVT_SUB, buffer, size);

        DCS::DAQ::ChannelLimits l;
        l.min = -10.0;
        l.max =  10.0;

        size = DCS::Registry::SVParams::GetDataFromParams<DCS::Utils::BasicString, DCS::Utils::BasicString, DCS::DAQ::ChannelRef, DCS::DAQ::ChannelLimits>(buffer, SV_CALL_DCS_DAQ_NewAIVChannel, 
            { "Channel0Name" }, { "PXI_Slot2/ai0" }, DCS::DAQ::ChannelRef::Default, l);

        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);

        size = DCS::Registry::SVParams::GetDataFromParams(buffer, SV_CALL_DCS_DAQ_StartAIAcquisition, 1000.0);

        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size).wait();

        LOG_DEBUG("Wait to signal task end.");
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        LOG_DEBUG("Signal task end.");

        size = DCS::Registry::SVParams::GetDataFromParams(buffer, SV_CALL_DCS_DAQ_StopAIAcquisition);

        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        fclose(f);

        DCS::Network::Client::StopThread(c);
    }

    DCS::Network::Destroy();

    DCS_RETURN_TEST;
}