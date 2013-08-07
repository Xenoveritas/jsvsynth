@ECHO OFF

REM Run a single test case.

ECHO Running %1...
ECHO.>> %TEST_LOG%
ECHO Test %1>>%TEST_LOG%
ECHO ----------->>%TEST_LOG%
ECHO.>> %TEST_LOG%

"%VDUB_HOME%\VDub.exe" %1 >>%TEST_LOG%

SET TEST_EL=%ERRORLEVEL%

IF %TEST_EL%==5 GOTO TestFailed
IF %TEST_EL%==0 GOTO TestPassed

REM Test did something strange

ECHO Test error (error %TEST_EL%)!

ECHO [ RESULT: %1 - ERROR ]>>%TEST_LOG%
ECHO Got %TEST_EL% as error level.>>%TEST_LOG%

SET /A TESTS_ERRORED=%TESTS_ERRORED%+1

GOTO done

:TestFailed
REM Test triggered failure

ECHO Test failed!

ECHO [ RESULT: %1 - FAILED ]>>%TEST_LOG%

SET /A TESTS_FAILED=%TESTS_FAILED%+1
GOTO done

:TestPassed
REM Test completed successfully

ECHO Test OK

ECHO [ RESULT: %1 - pass ]>>%TEST_LOG%

SET /A TESTS_PASSED=%TESTS_PASSED%+1
GOTO done

:done
