@ECHO OFF

REM This is an autogenerated BAT file intended to pull in third party libraries.
REM Make changes to the third_party.json file and then run:
REM     node make_thirdparty.js > third_party.bat
REM To update this file.

svn --version >NUL 2>NUL
IF ERRORLEVEL 1 GOTO noSVN

IF EXIST v8 GOTO haveV8

ECHO.
ECHO Attempting to checkout V8...
ECHO.

svn checkout http://v8.googlecode.com/svn/trunk v8

IF ERRORLEVEL 1 GOTO checkoutFailedV8

ECHO.
ECHO V8 checkout complete.
ECHO.

GOTO doneV8

:checkoutFailedV8

ECHO.
ECHO Unable to checkout V8.

GOTO exitFailure

:haveV8

ECHO.
ECHO Updating V8...
ECHO.

svn update v8

IF ERRORLEVEL 1 GOTO updateFailedV8

GOTO doneV8

:updateFailedV8

ECHO.
ECHO Unable to update V8.
GOTO exitFailure

:doneV8

IF EXIST v8/build/gyp GOTO haveGYP

ECHO.
ECHO Attempting to checkout GYP...
ECHO.

svn checkout http://gyp.googlecode.com/svn/trunk v8/build/gyp

IF ERRORLEVEL 1 GOTO checkoutFailedGYP

ECHO.
ECHO GYP checkout complete.
ECHO.

GOTO doneGYP

:checkoutFailedGYP

ECHO.
ECHO Unable to checkout GYP.

GOTO exitFailure

:haveGYP

ECHO.
ECHO Updating GYP...
ECHO.

svn update v8/build/gyp

IF ERRORLEVEL 1 GOTO updateFailedGYP

GOTO doneGYP

:updateFailedGYP

ECHO.
ECHO Unable to update GYP.
GOTO exitFailure

:doneGYP

IF EXIST v8/third_party/cygwin GOTO doneChromiumCygwin

ECHO.
ECHO Attempting to checkout Chromium-provided Cygwin install...
ECHO.

svn checkout http://src.chromium.org/svn/trunk/deps/third_party/cygwin@66844 v8/third_party/cygwin

IF ERRORLEVEL 1 GOTO checkoutFailedChromiumCygwin

ECHO.
ECHO Chromium-provided Cygwin install checkout complete.
ECHO.

GOTO doneChromiumCygwin

:checkoutFailedChromiumCygwin

ECHO.
ECHO Unable to checkout Chromium-provided Cygwin install.

GOTO exitFailure

:doneChromiumCygwin

IF EXIST v8/third_party/icu GOTO doneChromiumICU

ECHO.
ECHO Attempting to checkout Chromium-provided ICU 4.6...
ECHO.

svn checkout https://src.chromium.org/chrome/trunk/deps/third_party/icu46@214189 v8/third_party/icu

IF ERRORLEVEL 1 GOTO checkoutFailedChromiumICU

ECHO.
ECHO Chromium-provided ICU 4.6 checkout complete.
ECHO.

GOTO doneChromiumICU

:checkoutFailedChromiumICU

ECHO.
ECHO Unable to checkout Chromium-provided ICU 4.6.

GOTO exitFailure

:doneChromiumICU

python --version >NUL 2>NUL

IF ERRORLEVEL 1 GOTO noPython

ECHO Python was detected (via running "python --version"). Note that the build
ECHO system requires Python 2.6+, but NOT the 3.0 branch. (So 2.7 is fine, but 3.0
ECHO will NOT work). This script doesn't attempt to detect the version.

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
ECHO.

GOTO exitSuccess

:noSVN

ECHO Subversion was not located. An SVN client is required to download the third
ECHO party libraries. An SVN client can be obtained through Cygwin
ECHO (http://www.cygwin.com/) or from Apache Subversion's website
ECHO (http://subversion.apache.org/).

GOTO exitFailure

:exitFailure
EXIT /B 1
:exitSuccess

