#configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

add_definitions(-DAPI_EXPORT)

message(STATUS "LIBRARY TYPE SET TO: ${LIB_TYPE}")
message(STATUS "LIBRARY INSTALL BIN LOCATION: ${INSTALL_LIB_DIR}")

#find_package(OpenSSL REQUIRED)

add_library(DCSModuleNetwork STATIC
  src/socket.cpp
  src/server.cpp
  src/client.cpp
  src/message.cpp

  src/cli.cpp

  ../config/registry.cpp
  ../config/exports.h
  ../config/registry.h

  include/internal.h
  include/DCS_ModuleNetwork.h
  )

set_pretty_filenames(DCSModuleNetwork)

target_link_libraries(DCSModuleNetwork DCSModuleCore DCSModuleUtils DCSModuleEngineControl DCSModuleAcquisition ws2_32)
#target_link_libraries(DCSModuleNetwork INTERFACE OpenSSL::SSL)
#target_link_libraries(DCSModuleNetwork INTERFACE OpenSSL::Crypto)

#target_include_directories(DCSModuleNetwork BEFORE INTERFACE $<BUILD_INTERFACE:${OPENSSL_PREFIX}/usr/local/include>)
#target_include_directories(DCSModuleNetwork BEFORE INTERFACE $<BUILD_INTERFACE:${OPENSSL_PREFIX}/usr/local/include>)

# set_target_properties(DCSModuleNetwork PROPERTIES
#   PUBLIC_HEADER "include/DCS_ModuleNetwork.h"
#   ) #;${CMAKE_CURRENT_BINARY_DIR}/config.h")

install(TARGETS DCSModuleNetwork
  # Add the DCSModuleNetwork library to the "export-set"
  EXPORT DCSModulesAPITargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  COMPONENT dev)

# Install modified headers
install(FILES "${CMAKE_BINARY_DIR}/gen/DCS_Network/include/DCS_ModuleNetwork.h" "${CMAKE_SOURCE_DIR}/config/rpc.h"
  DESTINATION "${INSTALL_INCLUDE_DIR}/DCS_Network/include")
