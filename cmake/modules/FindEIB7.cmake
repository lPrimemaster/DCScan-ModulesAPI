# For native x86_64
set(EIB7_PPATH "EIB7")
set(_pf_x86 "$ENV{ProgramFiles\(x86\)}/${EIB7_PPATH}")
list(APPEND EIB7_PATH ${_pf_x86})

# list(APPEND EIB7_PATH "C:\\Users\\simpa\\Downloads\\753013-10-01-00\\753013-10-01-00\\EIB_74x\\windows")

# Find installed library using CMake functions
find_library(EIB7_LIBRARY
            NAMES "eib7"
            PATHS ${EIB7_PATH}
            PATH_SUFFIXES "lib64")
            
find_path(EIB7_INCLUDE_DIR
            NAMES "eib7"
            PATHS ${EIB7_PATH}
            PATH_SUFFIXES "include")

message("EIB7 Lib: " ${EIB7_LIBRARY})
message("EIB7 Lib: " ${EIB7_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EIB7 DEFAULT_MSG EIB7_LIBRARY EIB7_INCLUDE_DIR)

# Dont display these vars to the user
mark_as_advanced(EIB7_LIBRARY EIB7_INCLUDE_DIR)
