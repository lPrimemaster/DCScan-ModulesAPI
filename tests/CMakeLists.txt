function(NEWTEST DNAME EXECUTABLE)
	add_executable(${EXECUTABLE} src/${EXECUTABLE}.cpp include/DCS_Assert.h)
	target_link_libraries(${EXECUTABLE} DCSModuleUtils)
	add_test(${DNAME} ${EXECUTABLE})
endfunction()

file(COPY ${CMAKE_BINARY_DIR}/DCS_Utils/Release/DCSModuleUtils.dll 
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)

newtest(timer test_timer)
