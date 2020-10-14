# Project is migrating to cmake generator
# (c) 2018 - 2020 César Godinho
# Licensed under the MIT License

cmake_minimum_required(VERSION 3.8)
project(DCSModulesAPI VERSION 1.0.0 DESCRIPTION "DCS sub project containing all the control modules as libraries.")

# Change this to SHARED if you want to build a dynamic library (.dll) specially useful for loading from external programs/scripts.
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
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
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
  message(WARNING "Doxygen needs to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

# ================= #
# End Doxygen setup #
# ================= #

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
#set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/DCSModulesAPI)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

# set up include-directories
include_directories(
  "${PROJECT_SOURCE_DIR}"   # to find foo/foo.h
  "${PROJECT_BINARY_DIR}")  # to find foo/config.h

# Add sub-directories
add_subdirectory(DCS_EngineControl)

# The interesting stuff goes here
# ===============================

# Add all targets to the build-tree export set
export(TARGETS DCSModuleEngineControl
  FILE "${PROJECT_BINARY_DIR}/DCSModulesAPITargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE DCSModulesAPI)

# Create the FooBarConfig.cmake and FooBarConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${INSTALL_INCLUDE_DIR}")
configure_file(DCSModulesAPIConfig.cmake.in
  "${PROJECT_BINARY_DIR}/DCSModulesAPIConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${DCSMODULESAPI_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(DCSModulesAPIConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DCSModulesAPIConfig.cmake" @ONLY)
# ... for both
configure_file(DCSModulesAPIConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/DCSModulesAPIConfigVersion.cmake" @ONLY)

# Install the FooBarConfig.cmake and FooBarConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DCSModulesAPIConfig.cmake"
  "${PROJECT_BINARY_DIR}/DCSModulesAPIConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT DCSModulesAPITargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)


# OLD CODE
#add_library(DCSModuleEngineControl ${LIB_TYPE} 
#	src/DCS_EngineControl/placeholder.c)

#set_target_properties(DCSModuleEngineControl PROPERTIES
#    VERSION ${PROJECT_VERSION}
#    SOVERSION 1)

#target_include_directories(DCSModuleEngineControl PRIVATE .)

# Install directory is not working for now idk why
#set(CMAKE_INSTALL_INCLUDEDIR "include/")
#set(CMAKE_INSTALL_LIBDIR "lib/")
#message(STATUS "CMake install prefix: ${CMAKE_INSTALL_PREFIX}")

#message(STATUS "Lib Install Dir: ${CMAKE_INSTALL_LIBDIR}")
#install(TARGETS DCSModuleEngineControl
#    DESTINATION ${CMAKE_INSTALL_LIBDIR})

#message(STATUS "Include Install Dir: ${CMAKE_INSTALL_INCLUDEDIR}")
#install(FILES src/api/DCS_ModuleEngineControl.h
#	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})