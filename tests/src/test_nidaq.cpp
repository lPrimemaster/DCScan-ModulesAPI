#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../include/DCS_Assert.h"

int main()
{
    DCS_START_TEST;

    DCS::DAQ::TaskSettings settings;

    settings.task_name = { "MyTask" };

    settings.channel_name[0] = { "PXI_Slot2/ai0" };
    settings.channel_type[0] = DCS::DAQ::ChannelType::Voltage;
    settings.channel_ref[0]  = DCS::DAQ::ChannelRef::Differential;
    settings.channel_lim[0].min = -10.0;
    settings.channel_lim[0].max =  10.0;

    DCS::Network::Init();

    auto c = DCS::Network::Client::Connect("127.0.0.1", 15777);

    if(DCS::Network::Client::StartThread(c))
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        unsigned char buffer[4096];

        auto size = DCS::Registry::SVParams::GetDataFromParams(buffer, SV_CALL_DCS_DAQ_NewTask, settings);

        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);



        size = DCS::Registry::SVParams::GetDataFromParams<DCS::Utils::BasicString>(buffer, SV_CALL_DCS_DAQ_StartNamedTask, { "MyTask_Error" });

        DCS::Network::Message::SendAsync(DCS::Network::Message::Operation::REQUEST, buffer, size);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        DCS::Network::Client::StopThread(c);
    }

    DCS::Network::Destroy();

    DCS_RETURN_TEST;
}