# Project is migrating to cmake generator
# (c) 2018 - 2023 César Godinho
# Licensed under the MIT License

cmake_minimum_required(VERSION 3.12)

project(DCSModulesAPI VERSION 1.0.0 DESCRIPTION "DCS sub project containing all the control modules as libraries.")

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_LIBS_SOURCE_DIR "")

function(add_libTargetSubDir sd)
  set(CMAKE_LIBS_SOURCE_DIR ${CMAKE_LIBS_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${sd}" PARENT_SCOPE)
  add_subdirectory(${sd})
endfunction()

function(add_libTargetSubDirNamingOnly sd)
  set(CMAKE_LIBS_SOURCE_DIR ${CMAKE_LIBS_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${sd}" PARENT_SCOPE)
endfunction()

# Disable MSVC nasty deprecations
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Set default install dir to the install dir
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING " " FORCE)

# Do not hide any production important warnings (MSVC only)
# /W4 and /Wall are just nonsensical
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W3")
endif()

function(set_pretty_filenames)
	string(LENGTH "${CMAKE_CURRENT_SOURCE_DIR}/" SOURCE_PATH_SIZE)
	add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
endfunction()

set_pretty_filenames()

# Enable testing (DISABLE THIS FOR RELEASE)
option(BUILD_TESTS "Build tests" OFF)
option(INTERNAL_DOCS "Generate internal documentation" OFF)
option(CI_DOC_GENERATE "Generate documentation only for CI." OFF)

if(BUILD_TESTS)
  add_definitions(-DENABLE_TESTING)
endif()

if(INTERNAL_DOCS)
  set(ENABLE_INTERNAL_DOCS "YES")
else()
  set(ENABLE_INTERNAL_DOCS "NO")
endif()

# Change this to SHARED if you want to build a dynamic library (.dll) specially useful for loading from external programs/scripts.
# There is the option of using STATIC libraries as well, however they are not supported yet
set(LIB_TYPE STATIC)

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

# Add sub-directories (These are the libraries basically)
if(NOT CI_DOC_GENERATE)
  set(CONF_TARGETS DCSModuleAcquisition DCSModuleEngineControl DCSModuleUtils DCSModuleCore DCSModuleNetwork)
  add_libTargetSubDir(DCS_Network)
  add_libTargetSubDir(DCS_EngineControl)
  add_libTargetSubDir(DCS_Utils)
  add_libTargetSubDir(DCS_Core)
  add_libTargetSubDir(DCS_Acquisition)

  # Add custom targets
  add_subdirectory(custom_targets)
else()
  add_libTargetSubDirNamingOnly(DCS_Network)
  add_libTargetSubDirNamingOnly(DCS_EngineControl)
  add_libTargetSubDirNamingOnly(DCS_Utils)
  add_libTargetSubDirNamingOnly(DCS_Core)
  add_libTargetSubDirNamingOnly(DCS_Acquisition)
endif()

string (REPLACE ";" " " CMAKE_LIBS_SOURCE_DIR "${CMAKE_LIBS_SOURCE_DIR}")

# ===================== #
# Start DCSGen.py setup #
# ===================== #

# DCSGen.py needs to know what portions of the code are disabled when generating
set(TARGET_DEFINES_DCSGEN "")
foreach(t ${CONF_TARGETS})
  get_target_property(TDef ${t} COMPILE_DEFINITIONS)
  if(TDef)
    message( STATUS "Found Target with restrictions: ${t} (${TDef})")
    foreach(d ${TDef})
      list(APPEND TARGET_DEFINES_DCSGEN ${d})
    endforeach()
  endif()
endforeach()

# Tell CMake to run python from the directory where this CMakeLists file exists.
execute_process(COMMAND python scripts/DCSGen.py "${TARGET_DEFINES_DCSGEN}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# =================== #
# End DCSGen.py setup #
# =================== #

# =================== #
# Begin Doxygen setup #
# =================== #

# First we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# Check if Doxygen is installed
find_package(Doxygen)
if(BUILD_DOC)
  if (DOXYGEN_FOUND)
      # Set input and output files
      set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
      set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

      # Request to configure the file
      configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
      message(STATUS "Doxygen build started ${DOXYGEN_IN} ${DOXYGEN_OUT}")

      # Note the option ALL which allows to build the docs together with the application
      #add_custom_target( doc_doxygen ALL
      #   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      #   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      #   COMMENT "Generating API documentation with Doxygen"
      #   VERBATIM 
    #	)

    execute_process(
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      OUTPUT_QUIET
    )
  else (DOXYGEN_FOUND)
    message(WARNING "Doxygen needs to be installed to generate the doxygen documentation")
  endif (DOXYGEN_FOUND)
endif(BUILD_DOC)

# ================= #
# End Doxygen setup #
# ================= #

if(BUILD_TESTS AND NOT CI_DOC_GENERATE)
  # Add testing subdirectory
  enable_testing()
  # Do not add to doxygen target
  add_subdirectory(tests)
endif()

# ================================ #
# Start install_configure.py setup #
# ================================ #

# This could be done after build as well
execute_process(COMMAND python scripts/install_configure.py "${TARGET_DEFINES_DCSGEN}" "${CMAKE_BINARY_DIR}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# ============================== #
# End install_configure.py setup #
# ============================== #

message(STATUS "CML Sources: ${CMAKE_LIBS_SOURCE_DIR}")

if(NOT CI_DOC_GENERATE)
  # ========================== #
  # Begin Cmake package config #
  # ========================== #

  # Add all targets to the build-tree export set
  export(TARGETS ${CONF_TARGETS}
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

  install(DIRECTORY config DESTINATION "${INSTALL_INCLUDE_DIR}" FILES_MATCHING PATTERN "*.h")
endif()
