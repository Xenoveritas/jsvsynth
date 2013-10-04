@ECHO OFF

REM Runs a test script in benchmark.exe. Does not pipe STDOUT or STDERR as they
REM will appear in the console.

IF EXIST setenv.bat CALL setenv.bat

IF NOT DEFINED TEST_MODE SET TEST_MODE=Debug

IF /I "%1"=="/RELEASE" (
	SET TEST_MODE=Release
	SHIFT /1
)

IF "%1"=="" GOTO usage

SET TESTRUNNER=%~dp0
SET TESTRUNNER=%TESTRUNNER%\..\%TEST_MODE%\benchmark.exe

IF EXIST "%TESTRUNNER%" GOTO HaveRunner

ECHO Unable to locate the test runner - has the entire solution been built?
ECHO.
ECHO Looking for the test runner at:
ECHO   %TESTRUNNER%
ECHO.

EXIT /B 1

:usage

ECHO Usage: %0 FILE
ECHO.
ECHO Execute FILE using the test runner

EXIT /B 1

:HaveRunner

"%TESTRUNNER%" %*

ECHO Error level %ErrorLevel%
