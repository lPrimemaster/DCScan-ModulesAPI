
set(NI_PPATH "National Instruments/NI-DAQ/DAQmx ANSI C Dev")
set(_pf_x86 "$ENV{ProgramFiles\(x86\)}/${NI_PPATH}")
list(APPEND NIDAQMX_PATH ${_pf_x86})

# Find installed library using CMake functions
find_library(LIBNIDAQMX_LIBRARY 
			NAMES "NIDAQmx"
            PATHS ${NIDAQMX_PATH}
            PATH_SUFFIXES "lib/msvc")
            
find_path(LIBNIDAQMX_INCLUDE_DIR 
			NAMES "NIDAQmx.h"
            PATHS ${NIDAQMX_PATH}
            PATH_SUFFIXES "include")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibNIDAQmx  DEFAULT_MSG
                                  LIBNIDAQMX_LIBRARY LIBNIDAQMX_INCLUDE_DIR)

mark_as_advanced(LIBNIDAQMX_LIBRARY LIBNIDAQMX_INCLUDE_DIR)
