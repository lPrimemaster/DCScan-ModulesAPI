#include "../include/DCS_ModuleEngineControl.h"
#include "../include/internal.h"

#include <thread>
#include <atomic>

static std::thread *control_service_thread = nullptr;
static std::atomic<bool> control_service_running = false;

static DCS::Coms::CmdBuffer cmd_buffer;

DCS::Coms::CmdBuffer &DCS::Coms::GetCmdBuffer()
{
	return cmd_buffer;
}

void DCS::Control::StartServices()
{
	if (control_service_thread == nullptr)
	{
		control_service_running.store(true);

		control_service_thread = new std::thread([=]() -> void {
			// TODO : Change the init_handle and init_usb_handle from hardcoded port names for COM and USB.
			/*char com_port[16];
			Serial::comnumber_to_string(com_port, 3);*/

			Serial::SerialArgs serial_args;

			serial_args.baudRate = 921600;	   // 900 kbps
			serial_args.byteSize = 8;		   // 8 bit size
			serial_args.eofChar = '\r';		   // Carriage return command eof (?)
			serial_args.parity = NOPARITY;	   // No parity
			serial_args.stopBits = ONESTOPBIT; // One stop bit

			// Open both ports for communication
			HANDLE esp301_handle = Serial::init_handle("COM3", GENERIC_READ | GENERIC_WRITE, serial_args);

			USerial::USBIntHandle pmc8742_handle = USerial::init_usb_handle("104D-4000");

			char response[256];
			DWORD rbSize;

			// FIXME : This as the issue of blocking operations on one target if the other is waiting for something.
			// Create a working thread for each instead.
			while (control_service_running.load())
			{
				auto cmd = cmd_buffer.process();

				if (cmd.full_cmd.size() == 0)
					continue; // Guard against cmd_buffer notify_unblock

				switch (cmd.target)
				{
				case Control::UnitTarget::ESP301:
					Serial::write_bytes(esp301_handle, cmd.full_cmd.c_str(), (DWORD)(cmd.full_cmd.size()));
					Serial::write_bytes(esp301_handle, "\r", 2);

					if (cmd.wait_response)
					{
						Serial::read_bytes(esp301_handle, (LPTSTR)response, 256, &rbSize);
						cmd_buffer.reply(response);
					}

					break;
				case Control::UnitTarget::PMC8742:
					USerial::write_bulk_bytes(pmc8742_handle, (PUCHAR)cmd.full_cmd.c_str(), (DWORD)(cmd.full_cmd.size()));
					USerial::write_bulk_bytes(pmc8742_handle, (PUCHAR) "\r", 1);

					if (cmd.wait_response)
					{
						// TODO : Check the 8742 with the MD? query to see if motor is stopped
						rbSize = USerial::read_bulk_bytes(pmc8742_handle, (PUCHAR)response, 256);
						cmd_buffer.reply(response);
					}
					break;
				}
			}

			Serial::close_handle(esp301_handle);
			DCS::USerial::term_usb_handle(pmc8742_handle);
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
