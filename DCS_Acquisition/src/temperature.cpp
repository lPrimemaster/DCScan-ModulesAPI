#include "../include/internal.h"
#include "../include/DCS_ModuleAcquisition.h"

#include <thread>

static std::thread* temp_loop = nullptr;
static std::atomic<bool> temp_loop_running;

void DCS::Temp::Init(const char* com_port)
{

}

void DCS::Temp::Terminate()
{
    
}

void DCS::Temp::StartEventLoop()
{
    if(temp_loop == nullptr)
    {
        temp_loop_running.store(true);
        temp_loop = new std::thread([&]() -> void {
            
        });
    }
}

void DCS::Temp::StopEventLoop()
{
    if(temp_loop != nullptr)
    {
        temp_loop_running.store(false);
        temp_loop->join();
        delete temp_loop;
        temp_loop = nullptr;
    }
    else
    {
        LOG_WARNING("Can't stop. Temperature data loop not running.");
    }
}

DCS::Temp::TemperatureData DCS::Temp::InspectLastTemperatureValues()
{

}
