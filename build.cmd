:: Build file for all the DCSModules targets.
@echo off
SET result="FALSE"
IF "%~1"=="" GOTO errorNA

IF NOT "%~2"=="" echo [1mThis command only supports one argument. Multiple provided. Ignoring...[0m

IF /I "%~1" == "ALL_BUILD" (SET result="TRUE")
IF /I "%~1" == "INSTALL"   (SET result="TRUE")
IF /I "%~1" == "RUN_TESTS" (SET result="TRUE")
IF /I "%~1" == "CLEAN"     (SET result="CLEAN")

IF "%result%"==""CLEAN"" (
	cd /D "%~dp0"
	rmdir build /q/s
	echo [92mCleaned build directory.[0m
	GOTO end
)

IF "%result%"==""TRUE"" (
	echo [92mBUILD STARTED.[0m
	mkdir build
	cd /D "%~dp0/build"
	cmake ..
	cmake --build . --target %1 --config Release -- /nologo /verbosity:minimal /maxcpucount
	IF errorlevel 1 GOTO errEnd
	GOTO sucEnd
) ELSE GOTO errorWT

:errorNA
echo [91mYou must specify a target to be built.[0m
echo [94mAvailable targets = [ALL_BUILD, INSTALL, RUN_TESTS, CLEAN][0m
echo Example: 'build ALL_BUILD' will build all the library targets.
GOTO errEnd

:errorWT
echo [91mThe target specified (%~1) is invalid.[0m
echo [94mAvailable targets = [ALL_BUILD, INSTALL, RUN_TESTS, CLEAN][0m
GOTO errEnd

:errEnd
echo.
echo [91mBUILD FAILED.[0m
GOTO end

:sucEnd
echo.
echo [92mBUILD SUCCEEDED.[0m

:end
cd /D "%~dp0"