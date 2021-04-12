#include <fstream>
#include <iterator>
#include <sstream>

#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../../DCS_EngineControl/include/DCS_ModuleEngineControl.h"

using namespace DCS::Utils;
using namespace DCS::Network;

static void RunToWaitPoll(int rep, float acc_min = 1.0f, float acc_max = 200.0f, bool open_loop = false)
{
	LOG_DEBUG("Starting RotDev full loop test battery [Bi-directional]...");
	LOG_DEBUG("Setting:");
	LOG_DEBUG("Max acc     = 2500");
	LOG_DEBUG("Max vel     = 40");
	LOG_DEBUG("Prec        = 6");
	LOG_DEBUG("DC OpenLoop = %s", open_loop ? "ON" : "OFF");

	unsigned char buffer[1024];
	auto size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
		SV_CALL_DCS_Control_IssueGenericCommandResponse,
		DCS::Control::UnitTarget::ESP301,
		DCS::Utils::BasicString{ "2MO;2OR1;2WS;2AU2500;2VU40;2VA40;2FP6;2AU?" }
	);

	auto zbv = Message::SendSync(Message::Operation::REQUEST, buffer, size_written);

	auto zb_ret = *(DCS::Utils::BasicString*)zbv.ptr;
	LOG_DEBUG("2AU: %s", zb_ret.buffer);


	// DC Open loop   -> 5H
	// DC Closed loop -> 301H

	if(open_loop)
		size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
			SV_CALL_DCS_Control_IssueGenericCommandResponse,
			DCS::Control::UnitTarget::ESP301,
			DCS::Utils::BasicString{ "2ZB5H;WT1000;2ZB?" }
		);
	else
		size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
			SV_CALL_DCS_Control_IssueGenericCommandResponse,
			DCS::Control::UnitTarget::ESP301,
			DCS::Utils::BasicString{ "2ZB301H;WT1000;2ZB?" }
		);

	auto wfm = Message::SendSync(Message::Operation::REQUEST, buffer, size_written);
	auto wfm_ret = *(DCS::Utils::BasicString*)wfm.ptr;
	LOG_DEBUG("2ZB: %s", wfm_ret.buffer);

	std::vector<std::vector<float>> data = { std::vector<float>(),
											 std::vector<float>(),
											 std::vector<float>(),
											 std::vector<float>(),
											 std::vector<float>() };
	float acc_step = (acc_max - acc_min) / rep;

	for (int i = 0; i < rep; i++)
	{
		float acc = acc_min + acc_step * i;
		std::string acc_str = "2PA0;2WS;2AC" + std::to_string(acc) + ";2AG" + std::to_string(acc) + ";2PR360;2WS;2TP?";

		DCS::Utils::BasicString reqstr;

		memcpy(reqstr.buffer, acc_str.c_str(), acc_str.size());

		size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
			SV_CALL_DCS_Control_IssueGenericCommandResponse,
			DCS::Control::UnitTarget::ESP301,
			reqstr
		);

		for (int j = 0; j < 100; j++)
		{
			auto ret = Message::SendSync(Message::Operation::REQUEST, buffer, size_written);
			float rpos = (float)atof((*(DCS::Utils::BasicString*)ret.ptr).buffer);
			LOG_DEBUG("Acc: %f || Dev: %f", acc, rpos - 360.0f);
			data.at(i).push_back(rpos);
			std::system("pause");
		}
	}

	if(rep != 0)
	{
		size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
			SV_CALL_DCS_Control_IssueGenericCommand,
			DCS::Control::UnitTarget::ESP301,
			DCS::Utils::BasicString{ "2MF" }
			//DCS::Utils::BasicString{ "2MO;2PA0.0;" }
		);
		Message::SendSync(Message::Operation::REQUEST, buffer, size_written);

		for (int j = 0; j < data.size(); j++)
		{
			std::ofstream output_file(std::to_string(j) + "_ACC_360_DEV.dat");
			std::ostream_iterator<float> output_iterator(output_file, "\n");
			std::copy(data[j].begin(), data[j].end(), output_iterator);
		}
	}
}

static void RunRelativeIncrement(float start, float step = 0.0005f, float acc = 25.0f, char stop = 's')
{
	unsigned char buffer[1024];



	char move[512];
	sprintf(move, "2PA%f;2WS", start);

	std::string acc_str = "2AC" + std::to_string(acc) + ";2AG" + std::to_string(acc) + ";" + move;
	DCS::Utils::BasicString reqstr;
	memcpy(reqstr.buffer, acc_str.c_str(), acc_str.size());

	auto size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
		SV_CALL_DCS_Control_IssueGenericCommand,
		DCS::Control::UnitTarget::ESP301,
		reqstr
	);
	Message::SendAsync(Message::Operation::REQUEST, buffer, size_written);


	memset(move, 0, 512);
	sprintf(move, "2PR%f;2WS;2TP?", step);

	memset(reqstr.buffer, 0, 512);
	memcpy(reqstr.buffer, move, strlen(move));

	LOG_DEBUG("-> %s", reqstr.buffer);

	size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
		SV_CALL_DCS_Control_IssueGenericCommandResponse,
		DCS::Control::UnitTarget::ESP301,
		reqstr
	);

	char s;

	std::cin.get(s);
	while(s != stop)
	{
		auto ret = Message::SendSync(Message::Operation::REQUEST, buffer, size_written);
		float rpos = (float)atof((*(DCS::Utils::BasicString*)ret.ptr).buffer);
		LOG_DEBUG("Acc: %f || Dev: %f", acc, rpos - start);
		std::cin.get(s);
	}
}

int main()
{
    Logger::Init(Logger::Verbosity::DEBUG);

	Init();

	Socket c = Client::Connect("127.0.0.1", 15777);
	bool valid = Client::StartThread(c);

	if (valid)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		unsigned char buffer[1024];
		auto size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
			SV_CALL_DCS_Threading_GetMaxHardwareConcurrency
		);

		auto max_threads_srv_b = Message::SendSync(Message::Operation::REQUEST, buffer, size_written);
		auto max_threads_srv = *(DCS::u16*)max_threads_srv_b.ptr;

		LOG_DEBUG("Got server max thread concurrency: %d", max_threads_srv);

		size_written = DCS::Registry::SetupEvent(buffer, SV_EVT_DCS_Network_Message_FibSeqEvt, [](DCS::u8* data) {
			LOG_DEBUG("FibEvent returned: %llu", *(DCS::u64*)data);
		});

		//Message::SendAsync(Message::Operation::EVT_SUB, buffer, size_written);

		//RunToWaitPoll(0, 25.0f, 200.0f, true); // Zero reps -> Initialize only
		RunToWaitPoll(1, 25.0f, 200.0f, true);

		LOG_DEBUG("Run relative increment started!");

		RunRelativeIncrement(0.0f, 1.0f);

		std::this_thread::sleep_for(std::chrono::seconds(5));

		size_written = DCS::Registry::RemoveEvent(buffer, SV_EVT_DCS_Network_Message_FibSeqEvt);

		//Message::SendAsync(Message::Operation::EVT_UNSUB, buffer, size_written);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		Client::StopThread(c);
	}

	Destroy();
}