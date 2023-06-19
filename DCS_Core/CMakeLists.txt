#configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

add_definitions(-DAPI_EXPORT)

message(STATUS "LIBRARY TYPE SET TO: ${LIB_TYPE}")
message(STATUS "LIBRARY INSTALL BIN LOCATION: ${INSTALL_LIB_DIR}")

set_pretty_filenames()

add_library(DCSModuleCore ${LIB_TYPE}
  src/memory.cpp
  src/threading.cpp
  src/db.cpp
  src/auth.cpp
  src/pid.cpp
  src/math.cpp
  ../config/exports.h
  include/internal.h
  include/DCS_ModuleCore.h
  )


# https://slproweb.com/download/Win64OpenSSL-1_1_1s.exe
find_package(OpenSSL 1.1.1 REQUIRED)

include_directories("${OPENSSL_INCLUDE_DIR}")
target_link_libraries(DCSModuleCore DCSModuleUtils ${OPENSSL_LIBRARIES})

set_target_properties(DCSModuleCore PROPERTIES
  PUBLIC_HEADER "include/DCS_ModuleCore.h"
  ) #;${CMAKE_CURRENT_BINARY_DIR}/config.h")

install(TARGETS DCSModuleCore
  # Add the DCSModuleCore library to the "export-set"
  EXPORT DCSModulesAPITargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  COMPONENT dev)

# Install modified headers
install(FILES "${CMAKE_BINARY_DIR}/gen/DCS_Core/include/DCS_ModuleCore.h" DESTINATION "${INSTALL_INCLUDE_DIR}/DCS_Core/include")