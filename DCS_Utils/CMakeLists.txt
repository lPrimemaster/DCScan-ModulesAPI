#configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

add_definitions(-DAPI_EXPORT)

message(STATUS "LIBRARY TYPE SET TO: ${LIB_TYPE}")
message(STATUS "LIBRARY INSTALL BIN LOCATION: ${INSTALL_LIB_DIR}")

add_library(DCSModuleUtils ${LIB_TYPE}
  src/timer.cpp
  src/String.cpp
  include/DCS_ModuleUtils.h
  ../config/exports.h
  #config.h.in
  )

set_target_properties(DCSModuleUtils PROPERTIES
  PUBLIC_HEADER "include/DCS_ModuleUtils.h"
  ) #;${CMAKE_CURRENT_BINARY_DIR}/config.h")

install(TARGETS DCSModuleUtils
  # IMPORTANT: Add the DCSModuleUtils library to the "export-set"
  EXPORT DCSModulesAPITargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/DCSModuleUtils"
  COMPONENT dev)
