#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../include/DCS_Assert.h"

int main()
{
    DCS_START_TEST;

    DCS::DAQ::TaskSettings settings;

    settings.task_name = { "MyTask" };

    settings.channel_name[0] = { "PXI_Slot2/ctr0" };
    settings.channel_type    = DCS::DAQ::ChannelType::Counter;
    settings.channel_ref[0]  = DCS::DAQ::ChannelRef::Default;
    settings.channel_lim[0].min = -10.0;
    settings.channel_lim[0].max =  10.0;

    DCS::Network::Init();

    auto c = DCS::Network::Client::Connect("127.0.0.1", 15777);

    if(DCS::Network::Client::StartThread(c))
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        unsigned char buffer[4096];

        auto size = DCS::Registry::SVParams::GetDataFromParams(buffer, SV_CALL_DCS_Threading_GetMaxHardwareConcurrency);

        auto max_threads = DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);

        LOG_DEBUG("Got max server concurrency: %u", *(DCS::u16*)max_threads.get().ptr);

        size = DCS::Registry::SVParams::GetDataFromParams(buffer, SV_CALL_DCS_DAQ_NewTask, settings);

        auto task = DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);

        task.wait();

        size = DCS::Registry::SVParams::GetDataFromParams<DCS::Utils::BasicString>(buffer, SV_CALL_DCS_DAQ_StartNamedTask, { "MyTask" });

        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);


        LOG_DEBUG("Wait to signal task end.");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        LOG_DEBUG("Signal task end.");

        size = DCS::Registry::SVParams::GetDataFromParams(buffer, SV_CALL_DCS_DAQ_StopTask, *(DCS::DAQ::Task*)task.get().ptr);

        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);

        size = DCS::Registry::SVParams::GetDataFromParams(buffer, SV_CALL_DCS_DAQ_DestroyTask, *(DCS::DAQ::Task*)task.get().ptr);

        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        DCS::Network::Client::StopThread(c);
    }

    DCS::Network::Destroy();

    DCS_RETURN_TEST;
}