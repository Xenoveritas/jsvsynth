@ECHO OFF

REM This script attempts to download the various required third party utilities.

svn --version >NUL 2>NUL
IF ERRORLEVEL 1 GOTO noSVN

IF EXIST v8 GOTO haveV8

ECHO.
ECHO Attempting to checkout V8...
ECHO.

svn checkout http://v8.googlecode.com/svn/trunk v8

IF ERRORLEVEL 1 GOTO checkoutFailed

ECHO.
ECHO V8 checkout complete. Attempting to checkout GYP...
ECHO.

:haveV8
CD v8

IF EXIST build\gyp GOTO haveGyp
svn co http://gyp.googlecode.com/svn/trunk build/gyp

IF ERRORLEVEL 1 GOTO checkoutGypFailed

:haveGyp

IF EXIST third_party\cygwin GOTO haveChromiumCygwin

ECHO.
ECHO Attempting to checkout the Chromium-provided Cygwin install...
ECHO.

svn co http://src.chromium.org/svn/trunk/deps/third_party/cygwin@66844 third_party/cygwin

IF ERRORLEVEL 1 GOTO checkoutCygwinFailed

:haveChromiumCygwin

REM Move up a directory to keep our location consistent
CD ..

ECHO.
REM  --------10--------20--------30--------40--------50--------60--------70--------80
ECHO V8 and the various required bits have been checked out.
ECHO.

python --version >NUL 2>NUL

IF ERRORLEVEL 1 GOTO noPython

ECHO Python was detected (via running "python --version"). Note that the build system
ECHO requires Python 2.6+, but NOT the 3.0 branch. (So 2.7 is fine, but 3.0 will NOT
ECHO work). This script doesn't attempt to detech the version.

GOTO done

:noPython

ECHO Python was not detected and is required. You can either install it via Cygwin,
ECHO grab the Chromium provided version, or install the official Windows version.
ECHO Note that the build system requires Python 2.6+, but NOT the 3.0 branch. (So
ECHO 2.7 is fine, but 3.0 will NOT work).
ECHO.
ECHO If you want to grab the Chromium provided version, from the checked out V8
ECHO directory, run the following command (all on one line):
ECHO.
ECHO    svn co http://src.chromium.org/svn/trunk/tools/third_party/python_26@89111 third_party/python_26
ECHO.

GOTO done

:noSVN

ECHO Subversion was not located. An SVN client is required to download the third
ECHO party programs. An SVN client can be obtained through Cygwin
ECHO (http://www.cygwin.com/) or from Apache Subversion's website
ECHO (http://subversion.apache.org/).

EXIT /B 1

:checkoutFailed

ECHO.
ECHO Unable to checkout V8.

EXIT /B 1

:checkoutGypFailed

ECHO.
ECHO Unable to checkout GYP, the build tool used to build V8.

EXIT /B 1

:checkoutCygwinFailed

ECHO.
ECHO Unable to checkout the Chromium provided Cygwin, which is REQUIRED to build V8.

EXIT /B 1

:done
ECHO.
ECHO To build V8, you first need to run GYP:
ECHO.
ECHO     CD v8
ECHO     python build\gyp_v8
ECHO.
ECHO And then the actual Visual Studio build:
ECHO.
ECHO     devenv /build Debug build\All.sln
ECHO         OR
ECHO     MSBuild build\all.sln /p:Configuration=Debug