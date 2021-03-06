
# Encoder test precision client target
# Builds an executable to test engine precision
add_executable(EncoderPrecTest src/enc_prec.cpp)
target_link_libraries(EncoderPrecTest DCSModuleUtils DCSModuleCore DCSModuleNetwork DCSModuleEngineControl)

# DAQ NI Functionality tester for simple programs mostly to test daqmx
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
find_package(LibNIDAQmx REQUIRED)
add_executable(DAQLocalTest src/local_daq_test.cpp)
include_directories("${LIBNIDAQMX_INCLUDE_DIR}")
target_link_libraries(DAQLocalTest DCSModuleUtils "${LIBNIDAQMX_LIBRARY}")

add_custom_target(CTARGETS)
add_dependencies(CTARGETS EncoderPrecTest DAQLocalTest)

# Copy the libs dll
# add_custom_command(TARGET EncoderPrecTest 
#     POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy # Copy
#     ${CMAKE_BINARY_DIR}/DCS_Utils/Release/DCSModuleUtils.dll # From
#     ${CMAKE_CURRENT_BINARY_DIR}/Release # To
#     )

# add_custom_command(TARGET EncoderPrecTest 
#     POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy # Copy
#     ${CMAKE_BINARY_DIR}/DCS_Core/Release/DCSModuleCore.dll # From
#     ${CMAKE_CURRENT_BINARY_DIR}/Release # To
#     )

# add_custom_command(TARGET EncoderPrecTest 
#     POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy # Copy
#     ${CMAKE_BINARY_DIR}/DCS_Network/Release/DCSModuleNetwork.dll # From
#     ${CMAKE_CURRENT_BINARY_DIR}/Release # To
#     ) 

# add_custom_command(TARGET EncoderPrecTest 
#     POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy # Copy
#     ${CMAKE_BINARY_DIR}/DCS_EngineControl/Release/DCSModuleEngineControl.dll # From
#     ${CMAKE_CURRENT_BINARY_DIR}/Release # To
#     )