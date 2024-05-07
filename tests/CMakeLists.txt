function(NEWTEST DNAME EXECUTABLE)
	add_executable(${EXECUTABLE} src/${EXECUTABLE}.cpp include/DCS_Assert.h)
	target_link_libraries(${EXECUTABLE} DCSModuleUtils DCSModuleCore DCSModuleNetwork DCSModuleEngineControl DCSModuleAcquisition)
	add_test(${DNAME} ${EXECUTABLE})
    set_pretty_filenames(${EXECUTABLE})
endfunction()

function(NEWTEST_WITH_HEADER DNAME EXECUTABLE)
	add_executable(${EXECUTABLE} src/${EXECUTABLE}.cpp src/${EXECUTABLE}.h include/DCS_Assert.h)
	target_link_libraries(${EXECUTABLE} DCSModuleUtils DCSModuleCore DCSModuleNetwork DCSModuleEngineControl DCSModuleAcquisition)
	add_test(${DNAME} ${EXECUTABLE})
    set_pretty_filenames(${EXECUTABLE})
endfunction()


set(BDEST ${CMAKE_CURRENT_BINARY_DIR}/Release)

newtest(timer test_timer)
newtest(logger test_logger)
newtest(memory test_memory)
newtest(threading test_threading)
newtest(socket test_socket)
newtest(message test_message)
newtest(serialcom test_serialcom)
newtest(usb test_usb)
newtest(nidaq test_nidaq)
newtest(db test_db)
newtest(auth test_auth)
newtest(counter test_counter)


# set(BNAME ${CMAKE_BINARY_DIR}/DCS_Utils/Release/DCSModuleUtils.dll)
# add_custom_command(TARGET test_timer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${BNAME} ${BDEST})

# set(BNAME ${CMAKE_BINARY_DIR}/DCS_Core/Release/DCSModuleCore.dll)
# add_custom_command(TARGET test_timer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${BNAME} ${BDEST})

# set(BNAME ${CMAKE_BINARY_DIR}/DCS_Network/Release/DCSModuleNetwork.dll)
# add_custom_command(TARGET test_timer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${BNAME} ${BDEST})

# set(BNAME ${CMAKE_BINARY_DIR}/DCS_EngineControl/Release/DCSModuleEngineControl.dll)
# add_custom_command(TARGET test_timer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${BNAME} ${BDEST})

# set(BNAME ${CMAKE_BINARY_DIR}/DCS_Acquisition/Release/DCSModuleAcquisition.dll)
# add_custom_command(TARGET test_timer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${BNAME} ${BDEST})
