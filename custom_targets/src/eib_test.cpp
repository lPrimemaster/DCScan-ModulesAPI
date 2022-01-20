#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_Acquisition/include/internal.h"
#include <chrono>

int main()
{
    using namespace DCS::ENC;
    InitEIB7Encoder("10.80.0.99", 0b0001);

    StartEIB7SoftModeTrigger();

    DCS::f64 f[] = {0, 0, 0, 5000.0};
    EIB7SoftModeLoopStart(f);
    
    getchar();

    EIB7SoftModeLoopStop();

    StopEIB7SoftModeTrigger();

    DeleteEIB7Encoder();
}
