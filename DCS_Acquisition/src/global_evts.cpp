#include "../include/DCS_ModuleAcquisition.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/internal.h"
#include "../../DCS_Network/include/internal.h"

#include <queue>

void DCS::DAQ::PeakDetectWithAngleEvent()
{
    InternalVoltageData ivd = GetLastIVD();

    DCS::Math::CountResult cr = DCS::Math::countArrayPeak(ivd.ptr, INTERNAL_SAMP_SIZE, 0.0, 10.0, 1.0);
    
    PDetectEventData evt_data;
    evt_data.counts = cr.num_detected;
    evt_data.measured_angle = ivd.measured_angle;
    evt_data.timestamp = ivd.timestamp;

    DCS_EMIT_EVT((DCS::u8*)evt_data, sizeof(PDetectEventData));
}