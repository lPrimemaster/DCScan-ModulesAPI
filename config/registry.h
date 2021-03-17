////////////////////////////////////////
//    THIS FILE WAS AUTOGENERATED     //
//  ANY MODIFICATIONS WILL BE ERASED  //
////////////////////////////////////////
// Generated by the DCS pre-processor //
////////////////////////////////////////

#pragma once
#include "exports.h"
#include <unordered_map>
#include <functional>
#include <any>
#include "../DCS_Utils/include/DCS_ModuleUtils.h"

#include "C:\Users\Utilizador\Desktop\Source\DCScan-ModulesAPI\DCS_Core\include\DCS_ModuleCore.h"
#include "C:\Users\Utilizador\Desktop\Source\DCScan-ModulesAPI\DCS_EngineControl\include\DCS_ModuleEngineControl.h"

#define SV_CALL_NULL 0x0
#define SV_CALL_DCS_Threading_GetMaxHardwareConcurrency 0x1
#define SV_CALL_DCS_Control_IssueGenericCommand 0x2

#define SV_ARG_NULL 0x0
#define SV_ARG_DCS_Control_value_str_test 0x1
#define SV_ARG_DCS_Control_UnitTarget 0x2

#define SV_RET_VOID 0x0
#define SV_RET_DCS_u16 0x1

#define MAX_SUB 0x1
#define SV_EVT_OnTest 0x1

namespace DCS {

	/**
	 * \brief Autogenerated class responsible for registering any function calls that might be
	 * requested via tcp/ip. 
	 *
	 * An hash table is auto generated via the DCS_REGISTER_CALL token.
	 * Any function declarated with it shall be registered in the hash table and attributed an id callable
	 * via the Registry::Execute*() function family.
	 *
	 */
	class Registry {
	public:
		struct SVParams;
		struct SVReturn;

		static DCS_API const u16 Get(const char* func_signature)
		{
			u16 val = 0;
			auto it = id.find(func_signature);
			if (it != id.end())
				val = it->second;
			else
				LOG_ERROR("Function signature (%s) not found.", func_signature);
			return val;
		}

		static DCS_API const bool CheckEvent(u8 id)
		{
			if (id <= MAX_SUB)
				return subscriptions.at(id);
			return false;
		}

		static DCS_API void SetEvent(u8 id)
		{
			if (id <= MAX_SUB)
				subscriptions[id] = true;
		}

		static DCS_API void UnsetEvent(u8 id)
		{
			if (id <= MAX_SUB)
				subscriptions[id] = false;
		}

		static DCS_API SVReturn Execute(SVParams params);

	private:
		template<typename T>
		static inline T convert_from_byte(const unsigned char* data, i32 offset, i32 size)
		{
			if(offset >= size)
			{
				LOG_ERROR("Data conversion overflow.");
				return T();
			}

			return *((T*)(data + offset));
		}

		template<typename T>
		static inline void convert_to_byte(T value, unsigned char* buffer, i32 offset, i32 size)
		{
			if(offset >= size)
			{
				LOG_ERROR("Data conversion overflow.");
				return;
			}
			memcpy(&buffer[offset], (unsigned char*)&value, sizeof(T));
		}

		inline static std::unordered_map<const char*, u16> id = 
		{
			{"DCS::Threading::GetMaxHardwareConcurrency", 0x1},
			{"DCS::Control::IssueGenericCommand", 0x2}
		};

		inline static std::unordered_map<u8, bool> subscriptions = 
		{
			{SV_EVT_OnTest, false}
		};

	public:
		struct DCS_API SVParams
		{
		public:
			const u16 getFunccode() const
			{
				return fcode;
			}

			template<typename T>
			const T getArg(u64 i) const
			{
				T rv;
				try 
				{
				    rv = std::any_cast<T>(args.at(i));
				}
				catch(const std::bad_any_cast& e) 
				{
					LOG_ERROR("Bad SVParams getArg(%d) %s.", i, e.what());
				}
				return rv;
			}

			static const SVParams GetParamsFromData(const unsigned char* payload, i32 size);

			template<typename... Args>
			static i32 GetDataFromParams(unsigned char* buffer, u16 fcode, Args... args)
			{
				std::vector<std::any> p = {args...};
				i32 it = sizeof(u16);
				memcpy(buffer, &fcode, sizeof(u16));

				switch(fcode)
				{
					case SV_CALL_DCS_Control_IssueGenericCommand:
					{
						auto A0_v = std::any_cast<DCS::Control::UnitTarget>(p.at(0));
						u8   A0_t = SV_ARG_DCS_Control_UnitTarget;
						cpyArgToBuffer(buffer, (u8*)&A0_v, A0_t, sizeof(DCS::Control::UnitTarget), it);
						auto A1_v = std::any_cast<DCS::Control::value_str_test>(p.at(1));
						u8   A1_t = SV_ARG_DCS_Control_value_str_test;
						cpyArgToBuffer(buffer, (u8*)&A1_v, A1_t, sizeof(DCS::Control::value_str_test), it);
						break;
					}
					default:
						LOG_ERROR("GetDataFromParams() function code (fcode) not found.");
						LOG_ERROR("Maybe function signature naming is invalid, or function does not take any arguments.");
						break;
				}

				return it;
			}

		private:
			SVParams(u16 fc, std::vector<std::any> args) : fcode(fc), args(args) {  }

		private:
			static void cpyArgToBuffer(unsigned char* buffer, u8* value, u8 type, i32 argSize, i32& it);

		private:
			u16 fcode;
#pragma warning( push )
#pragma warning( disable : 4251 )
			std::vector<std::any> args;
#pragma warning( pop )
		};

#pragma pack(push, 1)
		struct DCS_API SVReturn
		{
			i8 type;
			u8 ptr[1024];
		};
#pragma pack(pop)
	};
}
