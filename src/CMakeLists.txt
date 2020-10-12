# Project is migrating to cmake generator
# (c) 2018 - 2020 César Godinho
# Licensed under the MIT License

cmake_minimum_required(VERSION 3.8)
project(DCSModulesAPI VERSION 1.0.0 DESCRIPTION "DCS sub project containing all the control modules as libraries.")

# Change this to shared if you want to build a dynamic library (.dll) specially useful for loading from external programs/scripts.
set(LIB_TYPE STATIC)

# =================== #
# Begin Doxygen setup #
# =================== #

# First we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# Check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # Set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message(STATUS "Doxygen build started ${DOXYGEN_IN} ${DOXYGEN_OUT}")

    # Note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message(WARNING "Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

# ================= #
# End Doxygen setup #
# ================= #

add_library(DCSModuleEngineControl ${LIB_TYPE} 
	DCS_EngineControl/placeholder.c)

set_target_properties(DCSModuleEngineControl PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1)

target_include_directories(DCSModuleEngineControl PRIVATE .)

# Install directory is not working for now idk why
set(CMAKE_INSTALL_INCLUDEDIR "include/")
set(CMAKE_INSTALL_LIBDIR "lib/")
message(STATUS "CMake install prefix: ${CMAKE_INSTALL_PREFIX}")

message(STATUS "Lib Install Dir: ${CMAKE_INSTALL_LIBDIR}")
install(TARGETS DCSModuleEngineControl
    DESTINATION ${CMAKE_INSTALL_LIBDIR})

message(STATUS "Include Install Dir: ${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES src/api/DCS_ModuleEngineControl.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})