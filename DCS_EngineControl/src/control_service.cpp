#include "../include/DCS_ModuleEngineControl.h"
#include "../include/internal.h"

#include <thread>
#include <atomic>

static std::thread* control_service_thread = nullptr;
static std::atomic<bool> control_service_running = false;

static DCS::Coms::CmdBuffer cmd_buffer;

static DCS::Coms::COMDevicePrivateProperties com_device_properties;

DCS::Coms::CmdBuffer& DCS::Coms::GetCmdBuffer()
{
	return cmd_buffer;
}

void DCS::Control::StartServices()
{
	if (control_service_thread == nullptr)
	{
		control_service_running.store(true);

		control_service_thread = new std::thread([=]()->void {

			// Search for handle names
			char comNames[10240];
			i32 cn_sz = Serial::enumerate_ports(comNames, 10240);

			// TODO : Find COM from name - required
			com_device_properties.comport_esp301  = 0;
			com_device_properties.comport_pmc8742 = 1;

			char com_ports[2][16];
			Serial::comnumber_to_string(com_ports[0], com_device_properties.comport_esp301);
			Serial::comnumber_to_string(com_ports[1], com_device_properties.comport_pmc8742);

			// Both eps301 and pmc8742 use the same connection features
			com_device_properties.serial_args.baudRate = 921600;		// 900 kbps
			com_device_properties.serial_args.byteSize = 8;				// 8 bit size
			com_device_properties.serial_args.eofChar = '\r';			// Carriage return command eof (?)
			com_device_properties.serial_args.parity = NOPARITY;		// No parity
			com_device_properties.serial_args.stopBits = ONESTOPBIT;	// One stop bit

			// Open both ports for communication
			HANDLE esp301_handle  = Serial::init_handle(com_ports[0], GENERIC_READ | GENERIC_WRITE, com_device_properties.serial_args);
			HANDLE pmc8742_handle = Serial::init_handle(com_ports[1], GENERIC_READ | GENERIC_WRITE, com_device_properties.serial_args);

			char response[256];
			DWORD rbSize;

			// TODO : This as the issue of blocking operations on one target if the other is waiting for something.
			// Create a working thread for each instead.
			while (control_service_running.load())
			{
				auto cmd = cmd_buffer.process();

				if (cmd.full_cmd.size() == 0) continue; // Guard against cmd_buffer notify_unblock

				switch (cmd.target)
				{
				case Control::UnitTarget::ESP301:
					Serial::write_bytes(esp301_handle, cmd.full_cmd.c_str(), (DWORD)(cmd.full_cmd.size()));
					Serial::write_bytes(esp301_handle, "\r", 2);

					if (cmd.wait_response)
					{
						Serial::read_bytes(esp301_handle, response, 256, &rbSize);
						cmd_buffer.reply(response);
					}

					break;
				case Control::UnitTarget::PMC8742:
					Serial::write_bytes(pmc8742_handle, cmd.full_cmd.c_str(), (DWORD)(cmd.full_cmd.size()));
					Serial::write_bytes(pmc8742_handle, "\r", 2);

					if (cmd.wait_response)
					{
						Serial::read_bytes(pmc8742_handle, response, 256, &rbSize);
						cmd_buffer.reply(response);
					}
					break;
				}
			}

			Serial::close_handle(esp301_handle);
			Serial::close_handle(pmc8742_handle);
		});
		if (control_service_thread == nullptr)
		{
			LOG_ERROR("Could not allocate control_service_thread thread.");
		}
	}
	else
	{
		LOG_WARNING("Could not start control_service_thread thread. Perhaps is already running?");
	}
}

void DCS::Control::StopServices()
{
	control_service_running.store(false);
	cmd_buffer.notify_unblock();
	control_service_thread->join();
	delete control_service_thread;
}
