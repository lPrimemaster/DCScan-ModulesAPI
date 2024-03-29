#configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

add_definitions(-DAPI_EXPORT)

message(STATUS "LIBRARY TYPE SET TO: ${LIB_TYPE}")
message(STATUS "LIBRARY INSTALL BIN LOCATION: ${INSTALL_LIB_DIR}")

set_pretty_filenames()

add_library(DCSModuleEngineControl ${LIB_TYPE}
  src/serial_win.cpp
  src/winusb.cpp
  src/control_service.cpp
  src/external_cmds.cpp
  ../config/exports.h
  
  include/DCS_ModuleEngineControl.h
  include/internal.h
  )


target_link_libraries(DCSModuleEngineControl DCSModuleUtils DCSModuleNetwork Setupapi Winusb)

set_target_properties(DCSModuleEngineControl PROPERTIES
  PUBLIC_HEADER "include/DCS_ModuleEngineControl.h"
  ) #;${CMAKE_CURRENT_BINARY_DIR}/config.h")

install(TARGETS DCSModuleEngineControl
  # Add the DCSModuleEngineControl library to the "export-set"
  EXPORT DCSModulesAPITargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  COMPONENT dev)

# Install modified headers
install(FILES "${CMAKE_BINARY_DIR}/gen/DCS_EngineControl/include/DCS_ModuleEngineControl.h" 
  DESTINATION "${INSTALL_INCLUDE_DIR}/DCS_EngineControl/include")