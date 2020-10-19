function(NEWTEST DNAME EXECUTABLE)
	add_executable(${EXECUTABLE} src/${EXECUTABLE}.cpp include/DCS_Assert.h)
	target_link_libraries(${EXECUTABLE} DCSModuleUtils)
	add_test(${DNAME} ${EXECUTABLE})
endfunction()

#file(COPY ${CMAKE_BINARY_DIR}/DCS_Utils/Release/DCSModuleUtils.dll 
#  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)

set(BNAME ${CMAKE_BINARY_DIR}/DCS_Utils/Release/DCSModuleUtils.dll)
set(BDEST ${CMAKE_CURRENT_BINARY_DIR}/Release)

newtest(timer test_timer)
newtest(logger test_logger)


add_custom_command(TARGET test_timer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${BNAME} ${BDEST})
