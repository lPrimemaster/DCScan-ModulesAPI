#include "../include/DCS_ModuleAcquisition.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/internal.h"
#include "../../DCS_Network/include/internal.h"

#include <queue>
#include <atomic>

static std::atomic<DCS::u16> MCA_NumChannels = 2048;

DCS::u16 DCS::DAQ::GetMCANumChannels()
{
    return MCA_NumChannels.load();
}

void DCS::DAQ::SetMCANumChannels(DCS::u16 nChannels)
{
    u16 c = nChannels;
    if(nChannels > INTERNAL_ADC_MAX_CHAN)
    {
        c = MCA_NumChannels.load();
        LOG_ERROR("Attempting to set MCA channels to a value larger than INTERNAL_ADC_MAX_CHAN (%u).", INTERNAL_ADC_MAX_CHAN);
    }
    MCA_NumChannels.store(c);
    LOG_MESSAGE("MCA Channel number set to: %u", c);
}

void DCS::DAQ::DCSCountEvent()
{
    // TODO : Discriminate voltage levels here also. Ruling out unwanted X-ray counts.

    InternalVoltageData ivd = GetLastDCS_IVD();

    DCSCountEventData evt_data;
    evt_data.counts = ivd.cr.num_detected;
    evt_data.measured_angle = ivd.measured_angle;
    evt_data.timestamp = ivd.timestamp;
    
    DCS_EMIT_EVT((DCS::u8*)&evt_data, sizeof(DCSCountEventData)); // HACK : This can operate with a move ctor instead
}

void DCS::DAQ::MCACountEvent()
{
    // TODO : Handle pile-up.
    InternalVoltageData ivd = GetLastMCA_IVD();

    MCACountEventData evt_data;
    evt_data.count = ivd.cr.num_detected;
    evt_data.timestamp = ivd.timestamp;
    evt_data.total_max_bin_count = MCA_NumChannels.load();

    if(evt_data.count > 0)
    {
        evt_data.bins = new u16[evt_data.count];
        for(int i = 0 ; i < evt_data.count; i++)
        {
            // TODO : Don't hard code DAQ range... (currently: [0.0, 10.0] V)
            evt_data.bins[i] = u16((ivd.cr.maxima[i] / 10.0) * evt_data.total_max_bin_count);
        }
    }
    
    DCS_EMIT_EVT((DCS::u8*)&evt_data, sizeof(MCACountEventData)); // HACK : This can operate with a move ctor instead
}