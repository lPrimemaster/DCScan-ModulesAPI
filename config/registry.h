
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

#include "H:\Data\C++\DCScan-ModulesAPI\DCS_Network\include\DCS_ModuleNetwork.h"

#define SV_CALL_DCS_Network_Server_SendData 0x0

#define SV_ARG_int8   0x0
#define SV_ARG_float  0x1
#define SV_ARG_bool   0x2
#define SV_ARG_char   0x3
#define SV_ARG_double 0x4

namespace DCS {

	/**
	 * \brief Class responsible for registering any function calls that might be
	 * requested via tcp/ip. This hash table is auto generated via the DCS_REGISTER_CALL token.
	 * Any function declarated with it shall be registered in the hash table and called with
	 * Registry::Execute*() functions.
	 *
	 */
	class Registry {
	public:
		struct SVParams;

		static const u16 Get(const char* func_signature)
		{
			u16 val = -1;
			auto it = id.find(func_signature);
			if (it != id.end())
				val = it->second;
			else
				DCS::Utils::Logger::Error("Function signature (%s) not found.", func_signature);
			return val;
		}

		static DCS_API void Execute(SVParams params);

		static DCS_API const SVParams GetParamsFromData(const unsigned char* payload, i16 size);
	private:
		template<typename T>
		static inline T convert_from_byte(const unsigned char* data, i16 offset, i16 size)
		{
			if(offset >= size)
			{
				DCS::Utils::Logger::Error("Data conversion overflow.");
				return T();
			}

			return *((T*)(data + offset));
		}

		inline static std::unordered_map<const char*,u16> id = 
		{
			{"DCS::Network::Server::SendData", 0x0}
		};

	public:
		struct DCS_API SVParams
		{
		public:
			friend class Registry;

			const i8 getOpcode() const
			{
				return opcode;
			}

			const i16 getFunccode() const
			{
				return fcode;
			}

			template<typename T>
			const T getArg(i32 i) const
			{
				return std::any_cast<T>(args.at(i));
			}

		private:
			SVParams(i8 oc, i16 fc, std::vector<std::any> args) : opcode(oc), fcode(fc), args(args) {  }

		private:

			i8 opcode;
			i16 fcode;
#pragma warning( push )
#pragma warning( disable : 4251 )
			std::vector<std::any> args;
#pragma warning( pop )
		};
	};
}
