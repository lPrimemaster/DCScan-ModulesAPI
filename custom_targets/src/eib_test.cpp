#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_Acquisition/include/internal.h"
#include <chrono>

int main()
{
    using namespace DCS::ENC;
    InitEIB7Encoder("10.80.0.99", 0b0001);

    StartEIB7SoftModeTrigger();

    EIB7SoftModeLoopStart(5000.0);
    
    getchar();

    EIB7SoftModeLoopStop();

    StopEIB7SoftModeTrigger();

    DeleteEIB7Encoder();
}
