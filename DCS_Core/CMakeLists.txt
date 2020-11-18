#configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

add_definitions(-DAPI_EXPORT)

message(STATUS "LIBRARY TYPE SET TO: ${LIB_TYPE}")
message(STATUS "LIBRARY INSTALL BIN LOCATION: ${INSTALL_LIB_DIR}")

add_library(DCSModuleCore ${LIB_TYPE}
  src/memory.cpp
  ../config/exports.h
  include/internal.h
  include/DCS_ModuleCore.h
  #config.h.in
  )

target_link_libraries(DCSModuleCore DCSModuleUtils)

set_target_properties(DCSModuleCore PROPERTIES
  PUBLIC_HEADER "include/DCS_ModuleCore.h"
  ) #;${CMAKE_CURRENT_BINARY_DIR}/config.h")

install(TARGETS DCSModuleCore
  # IMPORTANT: Add the DCSModuleCore library to the "export-set"
  EXPORT DCSModulesAPITargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/DCSModuleCore"
  COMPONENT dev)
