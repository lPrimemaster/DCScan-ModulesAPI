:: Build file for all the DCSModules targets.
:: TODO - Enable ninja generator as well.
:: Only MSBuild supported for now.
@echo off
SET result="FALSE"
IF "%~1"=="" GOTO errorNA

SET ab="FASLE"
SET rtests="FALSE"

IF /I "%~1" == "ALL_BUILD" (
	SET result="TRUE"
	SET ab="TRUE"
)
IF /I "%~1" == "INSTALL"   (SET result="TRUE")
IF /I "%~1" == "RUN_TESTS" (
	SET result="TRUE"
	SET rtests="TRUE"
)
IF /I "%~1" == "CLEAN"     (SET result="CLEAN")

IF /I "%~1" == "CTARGETS"  (SET result="TRUE")

IF "%ab%"==""FALSE"" (
	IF NOT "%~2"=="" (
		echo [1mThis command only supports one argument. Multiple provided. Ignoring...[0m
	)
)

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
	:: TODO - We dont need to regen the cmake everytime
	cmake %~2 ..
	:: Make msbuild's verbosity level quiet maybe ??
	cmake --build . --target %1 --config Release -- /nologo /verbosity:minimal /maxcpucount
	IF errorlevel 1 GOTO errEnd
	GOTO sucEnd
) ELSE GOTO errorWT

:errorNA
echo [91mYou must specify a target to be built.[0m
echo [94mAvailable targets = [ALL_BUILD, INSTALL, RUN_TESTS, CTARGETS, CLEAN][0m
echo Example: 'build ALL_BUILD' will build all the library targets.
GOTO errEnd

:errorWT
echo [91mThe target specified (%~1) is invalid.[0m
echo [94mAvailable targets = [ALL_BUILD, INSTALL, RUN_TESTS, CTARGETS, CLEAN][0m
GOTO errEnd

:errEnd
echo.
IF "%rtests%"==""TRUE"" (
	echo [91mTESTING FAILED.[0m
) ELSE echo [91mBUILD FAILED.[0m

GOTO end

:sucEnd
echo.
IF "%rtests%"==""TRUE"" (
	echo [92mTESTING SUCCEEDED.[0m
) ELSE echo [92mBUILD SUCCEEDED.[0m

:end
cd /D "%~dp0"