import pprint
import os
import fnmatch
from pathlib import Path
import re
import sys
from common import checkBlockActive

# WARNING : This file is undocumented and it is advised not to edit its contents unless you know what you are doing.
# Erroneous edits can cause a DCSModules-API compile error.
# Code is messy and hard to read. Might be refactored in the future.
# Essentially, this script allows to custom 'preprocess' the DCS_REGISTER_CALL token and auto generate the
# registry (.cpp .h) files to invoke server functions via tcp.

pp = pprint.PrettyPrinter(indent=0)

curr_dir = os.getcwd()

filenames = []
for root, dirs, files in os.walk(curr_dir):
	 for file in files:
	 	if fnmatch.fnmatch(file, '*.h') and not any(x in root for x in ['build', 'install', 'submodules', 'tests', 'config']):
	 		filenames.append(os.path.join(root, file))

print('Processing files...')
pp.pprint(filenames)
print('\n\n')


token_call = 'DCS_REGISTER_CALL'
token_evt  = 'DCS_REGISTER_EVENT'

HEADER = '''////////////////////////////////////////
//    THIS FILE WAS AUTOGENERATED     //
//  ANY MODIFICATIONS WILL BE ERASED  //
////////////////////////////////////////
// Generated by the DCS pre-processor //
////////////////////////////////////////
'''
DEC = '''

/**
 * @file
 */

/**
 * \\defgroup calls_id Remote Server Callables ID List.
 * \\brief A module containing all the API functions callable via TCP/IP IDs.
 */

/**
 * \\defgroup args_id Remote Server Arguments ID List.
 * \\brief A module containing all the TCP/IP passable argument IDs.
 */

/**
 * \\defgroup ret_id Remote Server Return Types ID List.
 * \\brief A module containing all the TCP/IP passable return type IDs.
 */

/**
 * \\defgroup evt_id Remote Server Event Types ID List.
 * \\brief A module containing all the TCP/IP subscribable events IDs.
 */

#pragma once
#include "exports.h"
#include <unordered_map>
#include <functional>
#include <any>
#include "../DCS_Utils/include/DCS_ModuleUtils.h"

$0

#define SV_CALL_NULL 0x0 ///< Indicates a non existant call [Not to use].
$1

#define SV_ARG_NULL 0x0 ///< Indicates a non existant argument [Not to use].
$2

#define SV_RET_VOID 0x0 ///< Indicates a void return type.
$3

$4

namespace DCS {

	/**
	 * \\brief Autogenerated class responsible for registering any function calls that might be
	 * requested via tcp/ip. 
	 *
	 * An hash table is auto generated via the ''' + token_call + ''' token.
	 * Any function declarated with it shall be registered in the hash table and attributed an id callable
	 * via the Registry::Execute*() function family.
	 *
	 */
	class Registry {
	public:
		struct SVParams;
		struct SVReturn;

		/**
		 * \\brief Get a function id [SV_CALL_*] by function name.
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
		 * \\brief Set up event to subscribe by ID SV_EVT_*.
		 */
		static DCS_API const i32 SetupEvent(unsigned char* buffer, u8 id, EventCallbackFunc f, u8* userData = nullptr)
		{
			memcpy(buffer, &id, sizeof(u8));

			evt_callbacks.emplace(id, f);
			evt_userData.emplace(id, userData);

			return sizeof(u8);
		}

		/**
		 * \\brief Set up event to unsubscribe by ID SV_EVT_*.
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
			$5
		};

		inline static std::unordered_map<u8, bool> subscriptions = 
		{
			$6
		};

		inline static std::unordered_map<std::string, u8> evt_named_func = 
		{
			$7
		};

		inline static std::unordered_map<u16, const char*> r_id_debug = 
		{
			$8
		};

		inline static std::unordered_map<u8, EventCallbackFunc> evt_callbacks;
		inline static std::unordered_map<u8, u8*> evt_userData;

	public:
		/**
		* \\brief Auto-generated class that allows for buffer <-> parameters conversion.
		*/
		struct DCS_API SVParams
		{
		public:
			/**
			* \\brief Retrieve the function code of the currently held parameter list.
			* \\return u16 Func code
			*/
			const u16 getFunccode() const
			{
				return fcode;
			}

			/**
			* \\brief Get the i'th positional argument.
			* Mostly internal use.
			* \\tparam T parameter type
			* \\return T parameter
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
			* \\brief Get all the positional arguments from byte buffer.
			* Mostly internal use.
			* \\param payload char buffer
			* \\param size buffer size
			* \\return SVParams parameter list
			*/
			static const SVParams GetParamsFromData(const unsigned char* payload, i32 size);

			/**
			* \\brief Fill a byte buffer with a list of arguments. use this function to populate the buffer passed to
			* DCS::Network::Message::SendSync / DCS::Network::Message::SendAsync when calling a function.
			* Make sure buffer has enough size to hold the data. Error checking is disabled for speed.
			* \\param buffer char buffer to store function code to be executed + its params
			* \\param fcode function code
			* \\param args the arguments passed to the function represented by fcode
			* \\return DCS::i32 size written to buffer
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
						$9
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
		 * \\brief Holds messages return types.
		 */
		struct DCS_API SVReturn
		{
			i8 type;
			u8 ptr[1024];
		};
#pragma pack(pop)
	};
}
'''

DEF = '''
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
		$0
		default:
			// __assume(0); // Hint the compiler to optimize a jump table even further disregarding arg_code checks
		    break;
        }
	}
	return DCS::Registry::SVParams(func_code, args);
}

DCS::Registry::SVReturn DCS::Registry::Execute(DCS::Registry::SVParams params)
{
	SVReturn ret; // A generic return type container
	ret.type = SV_RET_VOID;

	u16 fcode = params.getFunccode();

	if(fcode < MAX_CALL)
		LOG_DEBUG("Executing function code -> %d (%s)", fcode, r_id_debug[fcode]);
	switch(fcode)
	{
	case SV_CALL_NULL:
		LOG_ERROR("Function call from SVParams is illegal. Funccode not in hash table.");
		LOG_ERROR("Maybe function signature naming is wrong?");
		LOG_ERROR("Prefer SV_CALL defines to string names to avoid errors.");
		break;
	$1
	default:
		// __assume(0); // Hint the compiler to optimize a jump table even further disregarding func_code checks
	    break;
    }
	return ret;
}

void DCS::Registry::SVParams::cpyArgToBuffer(unsigned char* buffer, u8* value, u8 type, i32 argSize, i32& it)
{
	memcpy(buffer + it, &type, sizeof(u8)); it += sizeof(u8);
	memcpy(buffer + it, value, argSize); it += argSize;
}

'''

# A new, simpler, RPC that makes the client code cleaner
RPC = '''
namespace DCS
{
    namespace RPC
    {
        /**
         * \\brief Autogenerated namespace for simple RPC calls.
         *
         * Functions tagged with `DCS_REGISTER_CALL` are available here.
         * All functions called within this mode are made synchronously.
         * To call them asynchronously use the Registry direct calls.
         */
$0
    }
}
'''


def handleArgs():
    if len(sys.argv) != 2:
        print('DCSGen.py requires 1 argument to run!')  # Just inform before a crash ...
    return sys.argv[1].split(';')


def cleanFiles(compdef: list[str]):
    compdef_clean = [(c.split('=')[0] if '=' in c else c) for c in compdef]
    blst = []
    for fnm in filenames:
        lst = []
        with open(fnm, 'r') as f:
            lst = f.readlines()

        nlst = []
        p = ''

        # Clean file up
        # Remove comments and includes
        i = 0
        while i < len(lst):
            ns = lst[i].lstrip().replace('\n', '')
            i += 1

            if not ns:
                continue

            plist = checkBlockActive(lst[i - 1:], compdef_clean)
            if plist:
                lst[i - 1:] = plist
                # lst[i - 1:] = plist

            if(ns.startswith('/*') or ns.startswith('*') or ns.startswith('//') or ns.startswith('#')):
                continue

            if(not ns.endswith(';') and not (ns.endswith('}') or ns.endswith('{'))):
                p += (' ' + ns)
                continue

            nlst.append((p + ' ' + ns).lstrip(' '))
            p = ''
        blst.append({'data': nlst, 'name': fnm})
    return blst


def getTokenSymbols(all_files):
    func = []
    return_type = []
    header_def = []
    args_name = []
    evt_name = []
    evt_func = []

    for one_file in all_files:
        f_prefix = []
        oc = {}
        scope = ''
        for v in one_file['data']:
            if v.startswith('namespace') or v.startswith('class') or v.startswith('struct'):
                scope = v.split(' ')[-2]
                f_prefix.append(scope)
                oc[scope] = 1
            elif scope:
                if ('{' in v):
                    oc[scope] += v.count('{')
                if ('}' in v):
                    oc[scope] -= v.count('}')

                if (oc[scope] == 0 and len(f_prefix) > 0):
                    f_prefix.pop()
                    if len(f_prefix) > 0:
                        scope = f_prefix[-1]

            if v.startswith(token_call):
                # Get function name with scope
                func.append('::'.join(f_prefix) + '::' + v.split('(')[-2].split(' ')[-1])

                # Get function return and params
                f_raw_types = [y.strip() for y in re.findall('\\((.*?)\\)', v, re.DOTALL)[0].split(',')]
                if f_raw_types[0] != 'void':
                    return_type.append(f_raw_types[0])
                else:
                    return_type.append('')
                if len(f_raw_types) > 1:
                    args_name.append([a for a in f_raw_types[1:]])
                else:
                    args_name.append([''])

                # Get function header dep
                if one_file['name'] not in header_def:
                    header_def.append(one_file['name'])

            elif v.startswith(token_evt):
                evt_f_name = '::'.join(f_prefix) + '::' + v.split('(')[-2].split(' ')[-1]
                evt_func.append(evt_f_name)
                evt_name.append(evt_f_name.replace('::', '_'))
    return func, return_type, header_def, args_name, evt_name, evt_func


def make_rpc_func(fname, name, params, ret_type):
    fname_z = fname
    if len(params) > 0:
        fname += ','
    params_names = ', '.join(['p'+str(i) for i in range(len(params))])
    params_str = ', '.join([ptype for i, ptype in enumerate(params)])
    ret_statement = 'return *reinterpret_cast<'+ret_type+'*>(r.ptr);' if ret_type else 'return;'
    
    out = '''
        /**
         * \\brief A synchronous call to `$fname_z`.
         */
        inline $ret_type $name($params_str)
        {
            unsigned char buffer[1024];
            auto size_written = DCS::Registry::SVParams::GetDataFromParams(
                buffer,
                $fname
                $params_names
            );
            auto r = DCS::Network::Message::SendSync(DCS::Network::Message::Operation::REQUEST, buffer, size_written);
            $ret_statement
        }
    '''
    out = out.replace('$ret_type', ret_type if ret_type else 'void')
    out = out.replace('$ret_statement', ret_statement)
    out = out.replace('$params_str', params_str)
    out = out.replace('$params_names', params_names)
    out = out.replace('$name', name)
    out = out.replace('$fname_z', fname_z)  # NOTE: (César) : Order matters here
    out = out.replace('$fname', fname)
    return out


compdef = handleArgs()
print('Found definitions:')
pp.pprint(compdef)
cFiles = cleanFiles(compdef)
func, return_type, header_def, args_name, evt_name, evt_func = getTokenSymbols(cFiles)

# pp.pprint(cFiles)

print('Registering functions:')
pp.pprint('Signature: {0}'.format(func))
pp.pprint('Return types: {0}'.format(return_type))
pp.pprint('Arguments: {0}'.format(args_name))
pp.pprint('Headers: {0}'.format(header_def))

print('Registering events:')
pp.pprint('Events: {0}'.format(evt_name))

switch = []
switch_type = []

switch_reverse = []

hdef = []
defines = []
arg_defines = []
rtype_defines = []
evt_def = []
evt_map = []
evt_f_map = []
defines_debug_name = []
rpc_calls = []

registry = []
number = 1

for hd in header_def:
    hdef.append('#include "' + hd + '"')

for rt in list(filter(None, list(set(return_type)))):
    definition = 'SV_RET_' + rt.replace('::', '_')
    rtype_defines.append('#define ' + definition + ' ' + hex(number) + ' ///< Refers to return type `' + rt + '` \\ingroup ret_id')
    number += 1

# Concat arg_types and remove empty str ('') (if exists)
arg_defines = list(filter(None, list(set(sum(args_name, [])))))
dn = 1
arg_def_all = []
if(len(arg_defines) > 0):
    for a in arg_defines:
        arg_def = 'SV_ARG_' + a.replace('::', '_').replace(' ', '_').replace('*', '_ptr')
        arg_def_all.append('#define ' + arg_def + ' ' + hex(dn) + ' ///< Refers to argument `' + a + '` \\ingroup args_id')
        switch_type.append('case ' + arg_def + ':\n\t\t\t' + 
            'args.push_back(convert_from_byte<' + a + '>(payload, it, size));\n\t\t\t' + 
            'it += sizeof(' + a + ');\n\t\t\t' + 'break;')
        dn += 1

number = 1
for sig in func:
    definition = 'SV_CALL_' + sig.replace('::', '_')
    defines.append('#define ' + definition + ' ' + hex(number) + ' ///< A call to `' + sig + '` \\ingroup calls_id')

    fargs_casted = []
    fargs_rpc = []
    anumber = 0

    if args_name[number-1][0]:
        for a in args_name[number-1]:
            fargs_casted.append('params.getArg<' + a + '>(' + str(anumber) + ')')
            fargs_rpc.append(a + ' p'+str(anumber))
            anumber += 1

    rpc_calls.append(make_rpc_func(definition, '_'.join(sig.split('::')[-2:]), fargs_rpc, return_type[number-1]))

    rt_size = 'sizeof(' + return_type[number-1] + ')'

    switch.append('case ' + definition + ':\n\t{\n\t\t' + 
        ('' if not return_type[number-1] else return_type[number-1] + ' local = ') +
        sig + '(' + ',\n\t\t\t'.join(fargs_casted) + ');' + 
        ('' if not return_type[number-1] else ('\n\t\tif(' + rt_size + 
        ' > 1024) LOG_ERROR("SVReturn value < ' + rt_size + '.");\n\t\t' +
        'memcpy(ret.ptr, &local, ' + rt_size + ');\n\t\tret.type = SV_RET_' + 
        return_type[number-1].replace('::', '_') + ';')) + '\n\t\tbreak;\n\t}')

# i32 it = 0;
# auto A0_v = std::any_cast<p0Type>(p.at(0));
# u8   A0_t = p0Type_define;
# cpyArgToBuffer(buffer, (u8*)&A0_v, A0_t, sizeof(p0Type), it);

    anumber = 0
    switch_reverse_args = []
    if args_name[number-1][0]:
        for a in args_name[number-1]:
            switch_reverse_args.append('auto A' + str(anumber) + 
                '_v = std::any_cast<' + a + '>(p.at(' + str(anumber) + '));\n\t\t\t\t\t\t' + 
                '\tu8   A' + str(anumber) + '_t = ' + 'SV_ARG_' + a.replace('::', '_') + ';\n\t\t\t\t\t\t' +
                '\tcpyArgToBuffer(buffer, (u8*)&A' + str(anumber) + '_v, A' + str(anumber) + '_t, sizeof(' + a + '), it);')
            anumber += 1

        switch_reverse.append('case ' + definition + ':\n\t\t\t\t\t\t{\n\t\t\t\t\t\t\t' + 
            '\n\t\t\t\t\t\t\t'.join(switch_reverse_args) + '\n\t\t\t\t\t\t\tbreak;\n\t\t\t\t\t\t}')

    registry.append('{"' + sig + '", ' + hex(number) + '}')
    defines_debug_name.append('{' + hex(number) + ', "' + definition + '"}')
    number += 1

defines.append('#define MAX_CALL ' + hex(number))

number = 1
for evt in evt_name:
    defi = 'SV_EVT_' + evt
    evt_def.append('#define '+ defi + ' ' + hex(number) + ' ///< A event refering to `' + evt_func[number-1] + '` \\ingroup evt_id')
    evt_map.append('{' + defi + ', false}')
    evt_f_map.append('{"' + evt_func[number-1].split('::')[-1] + '", ' + defi + '}')
    number += 1

with open(curr_dir + '/config/registry.h', 'w') as f:
    f.write(HEADER)

    DEC = DEC.replace('$0', '\n'.join(hdef)) 					  # Place necessary func call headers
    DEC = DEC.replace('$1', '\n'.join(defines)) 				  # Place func call codes definitions
    DEC = DEC.replace('$2', '\n'.join(arg_def_all)) 			  # Place func arg registry codes definitions
    DEC = DEC.replace('$3', '\n'.join(rtype_defines)) 			  # Place func return type registry codes definitions
    DEC = DEC.replace('$4', '#define MAX_SUB ' + 
        hex(len(evt_def)) + '\n' + '\n'.join(evt_def))  		  # Place evt codes definitions
    DEC = DEC.replace('$5', ',\n\t\t\t'.join(registry)) 		  # Register function signatures in unordered_map
    DEC = DEC.replace('$6', ',\n\t\t\t'.join(evt_map))  		  # Register events
    DEC = DEC.replace('$7', ',\n\t\t\t'.join(evt_f_map))		  # Register event functions
    DEC = DEC.replace('$8', ',\n\t\t\t'.join(defines_debug_name)) # Register function debug names
    DEC = DEC.replace('$9', '\n\t\t\t\t\t\t'.join(switch_reverse))

    f.write(DEC)

with open(curr_dir + '/config/registry.cpp', 'w') as f:
    f.write(HEADER)

    DEF = DEF.replace('$0', '\n\t\t'.join(switch_type)) # Add registered types to parameter cast switch
    DEF = DEF.replace('$1', '\n\t'.join(switch))		# Add registered functions to func call switch

    f.write(DEF)

with open(curr_dir + '/config/rpc.h', 'w') as f:
    f.write(HEADER)

    f.write('#pragma once\n')
    f.write('#include "DCS_ModuleNetwork.h"\n')

    RPC = RPC.replace('$0', '\n'.join(rpc_calls)) # Place all of the RPC calls

    f.write(RPC)
