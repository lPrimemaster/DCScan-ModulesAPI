////////////////////////////////////////
//    THIS FILE WAS AUTOGENERATED     //
//  ANY MODIFICATIONS WILL BE ERASED  //
////////////////////////////////////////
// Generated by the DCS pre-processor //
////////////////////////////////////////

#include "registry.h"

const DCS::Registry::SVParams DCS::Registry::SVParams::GetParamsFromData(const unsigned char* payload, i32 size)
{
	u16 func_code = convert_from_byte<u16>(payload, 0, size); // First byte
	std::vector<std::any> args;

	// 0000 0000 0000 0000 | 0000 0000 ...
	// 0		 1			 2         ...
	// (    FuncCode     )   (   Args  ...

	// Evaluate arguments
	for(i32 it = 2; it < size;)
	{
		u8 arg_type = convert_from_byte<u8>(payload, it++, size);

		switch(arg_type)
		{
		case SV_ARG_NULL:
			LOG_ERROR("Arg type not recognized.");
			break;
		case SV_ARG_DCS_Utils_BasicString:
			args.push_back(convert_from_byte<DCS::Utils::BasicString>(payload, it, size));
			it += sizeof(DCS::Utils::BasicString);
			break;
		case SV_ARG_DCS_DAQ_TaskSettings:
			args.push_back(convert_from_byte<DCS::DAQ::TaskSettings>(payload, it, size));
			it += sizeof(DCS::DAQ::TaskSettings);
			break;
		case SV_ARG_DCS_Control_UnitTarget:
			args.push_back(convert_from_byte<DCS::Control::UnitTarget>(payload, it, size));
			it += sizeof(DCS::Control::UnitTarget);
			break;
		case SV_ARG_DCS_DAQ_Task:
			args.push_back(convert_from_byte<DCS::DAQ::Task>(payload, it, size));
			it += sizeof(DCS::DAQ::Task);
			break;
		default:
			__assume(0); // Hint the compiler to optimize a jump table even further disregarding arg_code checks
		}
	}
	return DCS::Registry::SVParams(func_code, args);
}

DCS::Registry::SVReturn DCS::Registry::Execute(DCS::Registry::SVParams params)
{
	SVReturn ret; // A generic return type container
	ret.type = SV_RET_VOID;

    LOG_DEBUG("Executing function code -> %d", params.getFunccode());
	switch(params.getFunccode())
	{
	case SV_CALL_NULL:
		LOG_ERROR("Function call from SVParams is illegal. Funccode not in hash table.");
		LOG_ERROR("Maybe function signature naming is wrong?");
		LOG_ERROR("Prefer SV_CALL defines to string names to avoid errors.");
		break;
	case SV_CALL_DCS_DAQ_NewTask:
	{
		DCS::DAQ::Task local = DCS::DAQ::NewTask(params.getArg<DCS::DAQ::TaskSettings>(0));
		if(sizeof(DCS::DAQ::Task) > 1024) LOG_ERROR("SVReturn value < sizeof(DCS::DAQ::Task).");
		memcpy(ret.ptr, &local, sizeof(DCS::DAQ::Task));
		ret.type = SV_RET_DCS_DAQ_Task;
		break;
	}
	case SV_CALL_DCS_DAQ_StartTask:
	{
		DCS::DAQ::StartTask(params.getArg<DCS::DAQ::Task>(0));
		break;
	}
	case SV_CALL_DCS_DAQ_StartNamedTask:
	{
		DCS::DAQ::StartNamedTask(params.getArg<DCS::Utils::BasicString>(0));
		break;
	}
	case SV_CALL_DCS_DAQ_StopTask:
	{
		DCS::DAQ::StopTask(params.getArg<DCS::DAQ::Task>(0));
		break;
	}
	case SV_CALL_DCS_DAQ_StopNamedTask:
	{
		DCS::DAQ::StopNamedTask(params.getArg<DCS::Utils::BasicString>(0));
		break;
	}
	case SV_CALL_DCS_DAQ_DestroyTask:
	{
		DCS::DAQ::DestroyTask(params.getArg<DCS::DAQ::Task>(0));
		break;
	}
	case SV_CALL_DCS_DAQ_DestroyNamedTask:
	{
		DCS::DAQ::DestroyNamedTask(params.getArg<DCS::Utils::BasicString>(0));
		break;
	}
	case SV_CALL_DCS_Threading_GetMaxHardwareConcurrency:
	{
		DCS::u16 local = DCS::Threading::GetMaxHardwareConcurrency();
		if(sizeof(DCS::u16) > 1024) LOG_ERROR("SVReturn value < sizeof(DCS::u16).");
		memcpy(ret.ptr, &local, sizeof(DCS::u16));
		ret.type = SV_RET_DCS_u16;
		break;
	}
	case SV_CALL_DCS_Control_IssueGenericCommand:
	{
		DCS::Control::IssueGenericCommand(params.getArg<DCS::Control::UnitTarget>(0),
			params.getArg<DCS::Utils::BasicString>(1));
		break;
	}
	case SV_CALL_DCS_Control_IssueGenericCommandResponse:
	{
		DCS::Utils::BasicString local = DCS::Control::IssueGenericCommandResponse(params.getArg<DCS::Control::UnitTarget>(0),
			params.getArg<DCS::Utils::BasicString>(1));
		if(sizeof(DCS::Utils::BasicString) > 1024) LOG_ERROR("SVReturn value < sizeof(DCS::Utils::BasicString).");
		memcpy(ret.ptr, &local, sizeof(DCS::Utils::BasicString));
		ret.type = SV_RET_DCS_Utils_BasicString;
		break;
	}
	default:
		__assume(0); // Hint the compiler to optimize a jump table even further disregarding func_code checks
	}
	return ret;
}

void DCS::Registry::SVParams::cpyArgToBuffer(unsigned char* buffer, u8* value, u8 type, i32 argSize, i32& it)
{
	memcpy(buffer + it, &type, sizeof(u8)); it += sizeof(u8);
	memcpy(buffer + it, value, argSize); it += argSize;
}

