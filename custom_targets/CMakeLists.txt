
# Encoder test precision client target
# Builds an executable to test engine precision
add_executable(EncoderPrecTest src/enc_prec.cpp)
target_link_libraries(EncoderPrecTest DCSModuleUtils DCSModuleCore DCSModuleNetwork DCSModuleEngineControl)

add_custom_target(CTARGETS)
add_dependencies(CTARGETS EncoderPrecTest)

# Copy the libs dll
add_custom_command(TARGET EncoderPrecTest 
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy # Copy
    ${CMAKE_BINARY_DIR}/DCS_Utils/Release/DCSModuleUtils.dll # From
    ${CMAKE_CURRENT_BINARY_DIR}/Release # To
    )

add_custom_command(TARGET EncoderPrecTest 
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy # Copy
    ${CMAKE_BINARY_DIR}/DCS_Core/Release/DCSModuleCore.dll # From
    ${CMAKE_CURRENT_BINARY_DIR}/Release # To
    )

add_custom_command(TARGET EncoderPrecTest 
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy # Copy
    ${CMAKE_BINARY_DIR}/DCS_Network/Release/DCSModuleNetwork.dll # From
    ${CMAKE_CURRENT_BINARY_DIR}/Release # To
    ) 

add_custom_command(TARGET EncoderPrecTest 
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy # Copy
    ${CMAKE_BINARY_DIR}/DCS_EngineControl/Release/DCSModuleEngineControl.dll # From
    ${CMAKE_CURRENT_BINARY_DIR}/Release # To
    )