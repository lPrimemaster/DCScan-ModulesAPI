////////////////////////////////////////
//    THIS FILE WAS AUTOGENERATED     //
//  ANY MODIFICATIONS WILL BE ERASED  //
////////////////////////////////////////
// Generated by the DCS pre-processor //
////////////////////////////////////////


/**
 * @file
 */

/**
 * \defgroup calls_id Remote Server Callables ID List.
 * \brief A module containing all the API functions callable via TCP/IP IDs.
 */

/**
 * \defgroup args_id Remote Server Arguments ID List.
 * \brief A module containing all the TCP/IP passable argument IDs.
 */

/**
 * \defgroup ret_id Remote Server Return Types ID List.
 * \brief A module containing all the TCP/IP passable return type IDs.
 */

/**
 * \defgroup evt_id Remote Server Event Types ID List.
 * \brief A module containing all the TCP/IP subscribable events IDs.
 */

#pragma once
#include "exports.h"
#include <unordered_map>
#include <functional>
#include <any>
#include "../DCS_Utils/include/DCS_ModuleUtils.h"

#include "H:\Data\C++\DCScan-ModulesAPI\DCS_Acquisition\include\DCS_ModuleAcquisition.h"
#include "H:\Data\C++\DCScan-ModulesAPI\DCS_Core\include\DCS_ModuleCore.h"
#include "H:\Data\C++\DCScan-ModulesAPI\DCS_EngineControl\include\DCS_ModuleEngineControl.h"

#define SV_CALL_NULL 0x0 ///< Indicates a non existant call [Not to use].
#define SV_CALL_DCS_DAQ_NewAIVChannel 0x1 ///< A call to `DCS::DAQ::NewAIVChannel` \ingroup calls_id
#define SV_CALL_DCS_DAQ_DeleteAIVChannel 0x2 ///< A call to `DCS::DAQ::DeleteAIVChannel` \ingroup calls_id
#define SV_CALL_DCS_DAQ_StartAIAcquisition 0x3 ///< A call to `DCS::DAQ::StartAIAcquisition` \ingroup calls_id
#define SV_CALL_DCS_DAQ_StopAIAcquisition 0x4 ///< A call to `DCS::DAQ::StopAIAcquisition` \ingroup calls_id
#define SV_CALL_DCS_Threading_GetMaxHardwareConcurrency 0x5 ///< A call to `DCS::Threading::GetMaxHardwareConcurrency` \ingroup calls_id
#define SV_CALL_DCS_Control_IssueGenericCommand 0x6 ///< A call to `DCS::Control::IssueGenericCommand` \ingroup calls_id
#define SV_CALL_DCS_Control_IssueGenericCommandResponse 0x7 ///< A call to `DCS::Control::IssueGenericCommandResponse` \ingroup calls_id
#define MAX_CALL 0x8

#define SV_ARG_NULL 0x0 ///< Indicates a non existant argument [Not to use].
#define SV_ARG_DCS_DAQ_ChannelLimits 0x1 ///< Refers to argument `DCS::DAQ::ChannelLimits` \ingroup args_id
#define SV_ARG_DCS_f64 0x2 ///< Refers to argument `DCS::f64` \ingroup args_id
#define SV_ARG_DCS_Utils_BasicString 0x3 ///< Refers to argument `DCS::Utils::BasicString` \ingroup args_id
#define SV_ARG_DCS_Control_UnitTarget 0x4 ///< Refers to argument `DCS::Control::UnitTarget` \ingroup args_id
#define SV_ARG_DCS_DAQ_ChannelRef 0x5 ///< Refers to argument `DCS::DAQ::ChannelRef` \ingroup args_id

#define SV_RET_VOID 0x0 ///< Indicates a void return type.
#define SV_RET_DCS_Utils_BasicString 0x1 ///< Refers to return type `DCS::Utils::BasicString` \ingroup ret_id
#define SV_RET_DCS_u16 0x2 ///< Refers to return type `DCS::u16` \ingroup ret_id

#define MAX_SUB 0x3
#define SV_EVT_DCS_DAQ_PeakDetectWithAngleEvent 0x1 ///< A event refering to `DCS::DAQ::PeakDetectWithAngleEvent` \ingroup evt_id
#define SV_EVT_DCS_DAQ_VoltageEvent 0x2 ///< A event refering to `DCS::DAQ::VoltageEvent` \ingroup evt_id
#define SV_EVT_DCS_Network_Message_FibSeqEvt 0x3 ///< A event refering to `DCS::Network::Message::FibSeqEvt` \ingroup evt_id

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

        /**
         * \brief Get a function id [SV_CALL_*] by function name.
         * Syntax: ["ns::func"]
         * Example: "DCS::Threading::GetMaxHardwareConcurrency" -> Returns: SV_CALL_DCS_Threading_GetMaxHardwareConcurrency
         */
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

        typedef void (*EventCallbackFunc)(u8* data, u8* userData);

        /**
         * \brief Set up event to subscribe by ID SV_EVT_*.
         */
        static DCS_API const i32 SetupEvent(unsigned char* buffer, u8 id, EventCallbackFunc f, u8* userData = nullptr)
        {
            memcpy(buffer, &id, sizeof(u8));

            evt_callbacks.emplace(id, f);
			evt_userData.emplace(id, userData);

            return sizeof(u8);
        }

        /**
         * \brief Set up event to unsubscribe by ID SV_EVT_*.
         */
        static DCS_API const i32 RemoveEvent(unsigned char* buffer, u8 id)
        {
            memcpy(buffer, &id, sizeof(u8));

			if (id <= MAX_SUB)
			{
            	evt_callbacks.erase(id);
				evt_userData.erase(id);
			}

            return sizeof(u8);
        }

		// HACK : GetEventCallback might fail in index referencing.
        static DCS_API const EventCallbackFunc GetEventCallback(u8 id)
        {
            if (id <= MAX_SUB)
                return evt_callbacks.at(id);
            LOG_ERROR("Event id -> %d. No callback found.", id);
            return nullptr;
        }

		static DCS_API const bool CheckEvent(u8 id)
		{
			if (id <= MAX_SUB)
				return subscriptions.at(id);
			return false;
		}

		static DCS_API const u8 GetEvent(const std::string func)
		{
			return evt_named_func.at(func);
		}

		// HACK : GetEventUserData might fail in index referencing.
		static DCS_API u8* GetEventUserData(u8 id)
		{
			if (id <= MAX_SUB)
				return evt_userData.at(id);
			return nullptr;
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
			{"DCS::DAQ::NewAIVChannel", 0x1},
			{"DCS::DAQ::DeleteAIVChannel", 0x2},
			{"DCS::DAQ::StartAIAcquisition", 0x3},
			{"DCS::DAQ::StopAIAcquisition", 0x4},
			{"DCS::Threading::GetMaxHardwareConcurrency", 0x5},
			{"DCS::Control::IssueGenericCommand", 0x6},
			{"DCS::Control::IssueGenericCommandResponse", 0x7}
		};

		inline static std::unordered_map<u8, bool> subscriptions = 
		{
			{SV_EVT_DCS_DAQ_PeakDetectWithAngleEvent, false},
			{SV_EVT_DCS_DAQ_VoltageEvent, false},
			{SV_EVT_DCS_Network_Message_FibSeqEvt, false}
		};

		inline static std::unordered_map<std::string, u8> evt_named_func = 
		{
			{"DCS::DAQ::PeakDetectWithAngleEvent", SV_EVT_DCS_DAQ_PeakDetectWithAngleEvent},
			{"DCS::DAQ::VoltageEvent", SV_EVT_DCS_DAQ_VoltageEvent},
			{"DCS::Network::Message::FibSeqEvt", SV_EVT_DCS_Network_Message_FibSeqEvt}
		};

		inline static std::unordered_map<u16, const char*> r_id_debug = 
		{
			{0x1, "SV_CALL_DCS_DAQ_NewAIVChannel"},
			{0x2, "SV_CALL_DCS_DAQ_DeleteAIVChannel"},
			{0x3, "SV_CALL_DCS_DAQ_StartAIAcquisition"},
			{0x4, "SV_CALL_DCS_DAQ_StopAIAcquisition"},
			{0x5, "SV_CALL_DCS_Threading_GetMaxHardwareConcurrency"},
			{0x6, "SV_CALL_DCS_Control_IssueGenericCommand"},
			{0x7, "SV_CALL_DCS_Control_IssueGenericCommandResponse"}
		};

        inline static std::unordered_map<u8, EventCallbackFunc> evt_callbacks;
		inline static std::unordered_map<u8, u8*> evt_userData;

	public:
		/**
		* \brief Auto-generated class that allows for buffer <-> parameters conversion.
		*/
		struct DCS_API SVParams
		{
		public:
			/**
			* \brief Retrieve the function code of the currently held parameter list.
			* \return u16 Func code
			*/
			const u16 getFunccode() const
			{
				return fcode;
			}

			/**
			* \brief Get the i'th positional argument.
			* Mostly internal use.
			* \tparam T parameter type
			* \return T parameter
			*/
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

			/**
			* \brief Get all the positional arguments from byte buffer.
			* Mostly internal use.
			* \param payload char buffer
			* \param size buffer size
			* \return SVParams parameter list
			*/
			static const SVParams GetParamsFromData(const unsigned char* payload, i32 size);

			/**
			* \brief Fill a byte buffer with a list of arguments. use this function to populate the buffer passed to
			* DCS::Network::Message::SendSync / DCS::Network::Message::SendAsync when calling a function.
			* Make sure buffer has enough size to hold the data. Error checking is disabled for speed.
			* \param buffer char buffer to store function code to be executed + its params
			* \param fcode function code
			* \param args the arguments passed to the function represented by fcode
			* \return DCS::i32 size written to buffer
			*/
			template<typename... Args>
			static i32 GetDataFromParams(unsigned char* buffer, u16 fcode, Args... args)
			{
				std::vector<std::any> p = {args...};
				i32 it = sizeof(u16);
				memcpy(buffer, &fcode, sizeof(u16));

                if(p.size() > 0)
				{
				    switch(fcode)
				    {
					    case SV_CALL_DCS_DAQ_NewAIVChannel:
						{
							auto A0_v = std::any_cast<DCS::Utils::BasicString>(p.at(0));
							u8   A0_t = SV_ARG_DCS_Utils_BasicString;
							cpyArgToBuffer(buffer, (u8*)&A0_v, A0_t, sizeof(DCS::Utils::BasicString), it);
							auto A1_v = std::any_cast<DCS::Utils::BasicString>(p.at(1));
							u8   A1_t = SV_ARG_DCS_Utils_BasicString;
							cpyArgToBuffer(buffer, (u8*)&A1_v, A1_t, sizeof(DCS::Utils::BasicString), it);
							auto A2_v = std::any_cast<DCS::DAQ::ChannelRef>(p.at(2));
							u8   A2_t = SV_ARG_DCS_DAQ_ChannelRef;
							cpyArgToBuffer(buffer, (u8*)&A2_v, A2_t, sizeof(DCS::DAQ::ChannelRef), it);
							auto A3_v = std::any_cast<DCS::DAQ::ChannelLimits>(p.at(3));
							u8   A3_t = SV_ARG_DCS_DAQ_ChannelLimits;
							cpyArgToBuffer(buffer, (u8*)&A3_v, A3_t, sizeof(DCS::DAQ::ChannelLimits), it);
							break;
						}
						case SV_CALL_DCS_DAQ_DeleteAIVChannel:
						{
							auto A0_v = std::any_cast<DCS::Utils::BasicString>(p.at(0));
							u8   A0_t = SV_ARG_DCS_Utils_BasicString;
							cpyArgToBuffer(buffer, (u8*)&A0_v, A0_t, sizeof(DCS::Utils::BasicString), it);
							break;
						}
						case SV_CALL_DCS_DAQ_StartAIAcquisition:
						{
							auto A0_v = std::any_cast<DCS::f64>(p.at(0));
							u8   A0_t = SV_ARG_DCS_f64;
							cpyArgToBuffer(buffer, (u8*)&A0_v, A0_t, sizeof(DCS::f64), it);
							break;
						}
						case SV_CALL_DCS_Control_IssueGenericCommand:
						{
							auto A0_v = std::any_cast<DCS::Control::UnitTarget>(p.at(0));
							u8   A0_t = SV_ARG_DCS_Control_UnitTarget;
							cpyArgToBuffer(buffer, (u8*)&A0_v, A0_t, sizeof(DCS::Control::UnitTarget), it);
							auto A1_v = std::any_cast<DCS::Utils::BasicString>(p.at(1));
							u8   A1_t = SV_ARG_DCS_Utils_BasicString;
							cpyArgToBuffer(buffer, (u8*)&A1_v, A1_t, sizeof(DCS::Utils::BasicString), it);
							break;
						}
						case SV_CALL_DCS_Control_IssueGenericCommandResponse:
						{
							auto A0_v = std::any_cast<DCS::Control::UnitTarget>(p.at(0));
							u8   A0_t = SV_ARG_DCS_Control_UnitTarget;
							cpyArgToBuffer(buffer, (u8*)&A0_v, A0_t, sizeof(DCS::Control::UnitTarget), it);
							auto A1_v = std::any_cast<DCS::Utils::BasicString>(p.at(1));
							u8   A1_t = SV_ARG_DCS_Utils_BasicString;
							cpyArgToBuffer(buffer, (u8*)&A1_v, A1_t, sizeof(DCS::Utils::BasicString), it);
							break;
						}
					    default:
						    LOG_ERROR("GetDataFromParams() function code (fcode) not found.");
						    LOG_ERROR("Maybe function signature naming is invalid, or function does not take any arguments.");
						    break;
				    }
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
		/**
		 * \brief Holds messages return types.
		 */
		struct DCS_API SVReturn
		{
			i8 type;
			u8 ptr[1024];
		};
#pragma pack(pop)
	};
}
