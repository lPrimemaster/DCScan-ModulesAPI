#include "../include/DCS_ModuleEngineControl.h"
// Socket for communication with XPS-RLD4 
#include "../../DCS_Network/include/internal.h"

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

void DCS::Control::StartServices(const char* esp301_com, const char* pmc8742_usb)
{
	if (control_service_thread == nullptr)
	{
		control_service_running.store(true);

		control_service_thread = new std::thread([=]() -> void {
			Serial::SerialArgs serial_args;

			serial_args.baudRate = 921600;	   // 900 kbps
			serial_args.byteSize = 8;		   // 8 bit size
			serial_args.eofChar = '\r';		   // Carriage return command eof (?)
			serial_args.parity = NOPARITY;	   // No parity
			serial_args.stopBits = ONESTOPBIT; // One stop bit

			// Using a ESP301 controller
			HANDLE esp301_handle = Serial::init_handle(esp301_com, GENERIC_READ | GENERIC_WRITE, serial_args);

			// Using a XPS-RLD4 controller
			SOCKET xps_rld4 = Network::CreateClientSocket("192.168.254.254", 5001);
			if(!Network::ValidateSocket(xps_rld4))
			{
				LOG_ERROR("Failed to open XPS-RLD4 at: %s:%d", "192.168.254.254", 5001);
			}
			else
			{
				LOG_DEBUG("Success connecting to XPS-RLD4 at: %s:%d", "192.168.254.254", 5001);
			}


			USerial::USBIntHandle pmc8742_handle = USerial::init_usb_handle(pmc8742_usb);

			char response[256];
			DWORD rbSize;

			// FIXME : This has the issue of blocking operations on one target if the other is waiting for something.
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
				case Control::UnitTarget::XPSRLD4:
					Network::SendData(xps_rld4, (u8*)cmd.full_cmd.c_str(), (i32)cmd.full_cmd.size());

					if(cmd.wait_response)
					{
						// Just make sure the buffer is large enough
						// i32 recv_sz = 1;
						// while(recv_sz > 0)
						i32 recv_sz = Network::ReceiveData(xps_rld4, (u8*)response, 255);
						if(recv_sz > 255) 
						{
							LOG_CRITICAL("XPS-RLD4 response buffer to small for data.");
							LOG_CRITICAL("%d bytes received out of a maximum of %d.", recv_sz, 255);
						}
						response[recv_sz] = '\0';
						cmd_buffer.reply(response);
					}
					break;
				}
			}

			Serial::close_handle(esp301_handle);
			Network::CloseSocketConnection(xps_rld4);
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
