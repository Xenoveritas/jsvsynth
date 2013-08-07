@ECHO OFF

REM Runs a test script in VirtualDub.exe, piping STDOUT to "stdout.log" and
REM STDERR to "stderr.log".

IF "%1"=="" GOTO usage

IF EXIST setenv.bat CALL setenv.bat

IF NOT "%VDUB_HOME%"=="" GOTO VdubIsSet

IF "%PROCESSOR_ARCHITECTURE%"=="AMD64" SET VDUB_HOME=%ProgramFiles(x86)%\VirtualDub
IF NOT "%PROCESSOR_ARCHITECTURE%"=="AMD64" SET VDUB_HOME=%ProgramFiles%\VirtualDub

:VdubIsSet

IF EXIST "%VDUB_HOME%\VirtualDub.exe" GOTO HaveVdub

ECHO Unable to locate VirtualDub "VirtualDub.EXE" in "%VDUB_HOME%".
ECHO.
ECHO Either set VDUB_HOME to point to where you have VirtualDub installed or
ECHO install VirtualDub into that directory.

EXIT /B 1

:usage

ECHO Usage: %0 FILE
ECHO.
ECHO Execute FILE in VirtualDub.exe

EXIT /B 1

:HaveVdub

"%VDUB_HOME%\VirtualDub.exe" %1 > stdout.log 2> stderr.log
