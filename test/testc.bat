@ECHO OFF

REM Runs a test script in vdub.exe. Does not pipe STDOUT or STDERR as they will
REM appear in the console.

IF "%1"=="" GOTO usage

IF EXIST setenv.bat CALL setenv.bat

IF NOT "%VDUB_HOME%"=="" GOTO VdubIsSet

IF "%PROCESSOR_ARCHITECTURE%"=="AMD64" SET VDUB_HOME=%ProgramFiles(x86)%\VirtualDub
IF NOT "%PROCESSOR_ARCHITECTURE%"=="AMD64" SET VDUB_HOME=%ProgramFiles%\VirtualDub

:VdubIsSet

IF EXIST "%VDUB_HOME%\vdub.exe" GOTO HaveVdub

ECHO Unable to locate VirtualDub "VirtualDub.EXE" in "%VDUB_HOME%".
ECHO.
ECHO Either set VDUB_HOME to point to where you have VirtualDub installed or
ECHO install VirtualDub into that directory.

EXIT /B 1

:usage

ECHO Usage: %0 FILE
ECHO.
ECHO Execute FILE in vdub.exe

EXIT /B 1

:HaveVdub

"%VDUB_HOME%\vdub.exe" test.avs

ECHO Error level %ErrorLevel%
