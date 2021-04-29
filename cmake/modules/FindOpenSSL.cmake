# For native x86_64
set(OPENSSL_PPATH "openssl")
set(_pf_x86 "$ENV{ProgramFiles\(x86\)}/${OPENSSL_PPATH}")
list(APPEND OPENSSL_PATH ${_pf_x86})

# Find installed library using CMake functions
find_library(OPENSSL_LIBRARY
            PATHS ${OPENSSL_PATH}
            PATH_SUFFIXES "lib")
            
find_path(OPENSSL_INCLUDE_DIR
            PATHS ${OPENSSL_PATH}
            PATH_SUFFIXES "include")

message("OpenSSL Lib: " ${OPENSSL_LIBRARY})
message("OpenSSL Lib: " ${OPENSSL_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenSSL DEFAULT_MSG OPENSSL_LIBRARY OPENSSL_INCLUDE_DIR)

# Dont display these vars to the user
mark_as_advanced(OPENSSL_LIBRARY OPENSSL_INCLUDE_DIR)
