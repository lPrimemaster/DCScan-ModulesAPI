
#configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

add_definitions(-DAPI_EXPORT)

message(STATUS "LIBRARY TYPE SET TO: ${LIB_TYPE}")
message(STATUS "LIBRARY INSTALL BIN LOCATION: ${INSTALL_LIB_DIR}")

set_pretty_filenames()

add_library(DCSModuleAcquisition ${LIB_TYPE}
  src/daq.cpp
  src/external_cmds.cpp
  src/global_evts.cpp
  src/encoder.cpp

  ../config/exports.h
  include/DCS_ModuleAcquisition.h
  include/internal.h
  )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(LibNIDAQmx REQUIRED)

find_package(EIB7 REQUIRED)

include_directories("${LIBNIDAQMX_INCLUDE_DIR}" "${EIB7_INCLUDE_DIR}")

target_link_libraries(DCSModuleAcquisition DCSModuleUtils DCSModuleNetwork "${LIBNIDAQMX_LIBRARY}" "${EIB7_LIBRARY}")

set_target_properties(DCSModuleAcquisition PROPERTIES
  PUBLIC_HEADER "include/DCS_ModuleAcquisition.h"
  ) #;${CMAKE_CURRENT_BINARY_DIR}/config.h")

install(TARGETS DCSModuleAcquisition
  # Add the DCSModuleAcquisition library to the "export-set"
  EXPORT DCSModulesAPITargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/DCS_Acquisition/include"
  COMPONENT dev)
