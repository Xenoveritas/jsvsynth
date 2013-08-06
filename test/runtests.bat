@ECHO OFF

REM Test harness; attempts to run test scripts and produce a final report.
REM Test scripts are run through VirtualDub (using vdub.exe).

IF EXIST setenv.bat CALL setenv.bat

IF NOT "%VDUB_HOME%"=="" GOTO VdubIsSet

IF "%PROCESSOR_ARCHITECTURE%"=="AMD64" SET VDUB_HOME=%ProgramFiles(x86)%\VirtualDub
IF NOT "%PROCESSOR_ARCHITECTURE%"=="AMD64" SET VDUB_HOME=%ProgramFiles%\VirtualDub

:VdubIsSet

IF EXIST "%VDUB_HOME%\vdub.exe" GOTO HaveVdub

ECHO Unable to locate VirtualDub "VDUB.EXE" in "%VDUB_HOME%".
ECHO.
ECHO Either set VDUB_HOME to point to where you have VirtualDub installed or
ECHO install VirtualDub into that directory.

EXIT /B 1

:HaveVdub

REM TODO: Create this file based on a parameter or something.

ECHO LoadPlugin("../Debug/jsvsynth.dll") > loadplugin.avsi

SET TEST_LOG=results.txt

ECHO Test Results > %TEST_LOG%
ECHO. >> %TEST_LOG%

SET TESTS_PASSED=0
SET TESTS_FAILED=0
SET TESTS_ERRORED=0

REM Always run sanity.avs - if it fails, the environment isn't sane, and we
REM can't continue.

CALL EXECTEST.BAT sanity.avs

IF %TESTS_PASSED%==1 GOTO EnvIsSane

ECHO Sanity checks failed! Aborting test run.
ECHO Sanity checks failed! Test run was aborted. >> %TEST_LOG%

EXIT /B 1

:EnvIsSane

FOR %%t IN (*test.avs) DO CALL EXECTEST.BAT %%t

SET /A TESTS_TOTAL=%TESTS_PASSED%+%TESTS_FAILED%+%TESTS_ERRORED%

ECHO -------- >> %TEST_LOG%
ECHO %TESTS_PASSED% passed, %TESTS_FAILED% failed, %TESTS_ERRORED% errored >> %TEST_LOG%

ECHO --------
ECHO %TESTS_PASSED% passed, %TESTS_FAILED% failed, %TESTS_ERRORED% errored
