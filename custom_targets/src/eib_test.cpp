#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_Acquisition/include/internal.h"
#include <chrono>

int main()
{
#ifndef NO_ENCODER_AVAILABLE
    using namespace DCS::ENC;

    // InitEIB7Encoder("10.80.0.99", 0b1010);

    // StartEIB7SoftModeTrigger();

    DCS::f64 f[] = {36000.0, 36000.0, 36000.0, 36000.0};
    // EIB7SoftModeLoopStart(f);

    Init("10.80.0.99", 0b1010, f);

    while(true)
    {
        LOG_DEBUG("%d", InspectLastEncoderValues().numAxis);
        LOG_DEBUG("%lf", InspectLastEncoderValues().axis[1].calpos);
        if(getchar() == 's') break;
    }

    // EIB7SoftModeLoopStop();

    // StopEIB7SoftModeTrigger();

    // DeleteEIB7Encoder();

    Terminate();
#endif // NO_ENCODER_AVAILABLE
    return 0;
}
