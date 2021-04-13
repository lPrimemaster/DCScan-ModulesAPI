#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_Acquisition/include/internal.h"
#include "../include/DCS_Assert.h"

static DCS::i32 Callback(DCS::DAQ::TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData)
{
    LOG_DEBUG("Got 1000 samples...");
    return 0;
}

int main()
{
    DCS_START_TEST;

    DCS::DAQ::InternalTask t;

    DCS::DAQ::CreateTask(&t);

    // This is connector 2 ai0 equivalent
    DCS::DAQ::AddTaskChannel(&t, "PXI_Slot2/ai16", DCS::DAQ::ChannelType::Voltage, DCS::DAQ::ChannelRef::Differential);

    DCS::DAQ::SetupTask(&t, nullptr, 10000.0, Callback);

    DCS::DAQ::StartTask(&t);

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    DCS::DAQ::StopTask(&t);

    DCS::DAQ::ClearTask(&t);

    DCS_RETURN_TEST;
}