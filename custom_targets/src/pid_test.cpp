#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_EngineControl/include/DCS_ModuleEngineControl.h"
#include <chrono>

int main()
{
    using namespace DCS;
    // Initialize Logger
	DCS::Utils::Logger::Init(DCS::Utils::Logger::Verbosity::DEBUG);

	// Initialize control services
	DCS::Control::StartServices();

	// Initialize acquisition services
	DCS::DAQ::Init();

	// Initializes encoder services
	DCS::f64 per_rev[] = {36000.0, 36000.0, 36000.0, 36000.0};
	DCS::ENC::Init("10.80.0.99", 0b1010, per_rev);

    // Do the stuff
    DCS::Control::IssueGenericCommandResponse(DCS::Control::UnitTarget::XPSRLD4, { "GroupKill(Group4)" });
    DCS::Control::IssueGenericCommandResponse(DCS::Control::UnitTarget::XPSRLD4, { "GroupInitialize(Group4)" });
    DCS::Control::IssueGenericCommandResponse(DCS::Control::UnitTarget::XPSRLD4, { "GroupHomeSearch(Group4)" });
    // DCS::Control::IssueGenericCommandResponse(DCS::Control::UnitTarget::XPSRLD4, { "GroupMoveAbsolute(Group4.Pos, 45.0)" });

    DCS::Control::SetPIDControlVariables(DCS::Control::UnitTarget::XPSRLD4, { "Group4" }, 2, -0.9, 0.0, 0.0);

    char quit;
    do
    {
        f64 target;
        std::cout << "Give me a target: ";
        std::cin >> target;
        
        DCS::Control::MoveAbsolutePID(DCS::Control::UnitTarget::XPSRLD4, { "Group4" }, target);

        std::cin >> quit;
    } while (quit != 'y');

    

	DCS::ENC::Terminate();

	DCS::DAQ::Terminate();

	DCS::Control::StopServices();

	DCS::Utils::Logger::Destroy();

	return 0;
}
