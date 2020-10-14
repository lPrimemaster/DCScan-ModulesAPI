#configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

add_definitions(-DAPI_EXPORT)

add_library(DCSModuleEngineControl SHARED
	src/placeholder.c 
	include/DCS_ModuleEngineControl.h
	#config.h.in
	) 

set_target_properties(DCSModuleEngineControl PROPERTIES
  PUBLIC_HEADER "include/DCS_ModuleEngineControl.h") #;${CMAKE_CURRENT_BINARY_DIR}/config.h")

install(TARGETS DCSModuleEngineControl
  # IMPORTANT: Add the DCSModuleEngineControl library to the "export-set"
  EXPORT DCSModulesAPITargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/DCSModuleEngineControl"
  COMPONENT dev)
