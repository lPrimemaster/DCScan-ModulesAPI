#include "../include/DCS_ModuleEngineControl.h"
#include "../include/internal.h"

#include <thread>
#include <atomic>

static std::thread* control_service_thread = nullptr;
static std::atomic<bool> control_service_running = false;

void DCS::Control::StartServices()
{
	if (control_service_thread == nullptr)
	{
		control_service_running.store(true);

		control_service_thread = new std::thread([=]()->void {

			// Search for handle names
			//HANDLE rotation_handle = Serial::init_handle();

			while (control_service_running.load())
			{

			}
		});
	}
	else
	{
		LOG_WARNING("Could not start control_service_thread thread. Perhaps is already running?");
	}
}
