@echo off

echo Webview directory: %src_dir%
echo Build directory: %build_dir%

echo Looking for vswhere.exe...
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" (
	echo ERROR: Failed to find vswhere.exe
	exit 1
)
echo Found %vswhere%

echo Looking for VC...
for /f "usebackq tokens=*" %%i in (`"%vswhere%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set vc_dir=%%i
)
if not exist "%vc_dir%\Common7\Tools\vsdevcmd.bat" (
	echo ERROR: Failed to find VC tools x86/x64
	exit 1
)
echo Found %vc_dir%

call "%vc_dir%\Common7\Tools\vsdevcmd.bat" -arch=x64 -host_arch=x64

echo Compiling Neutralinojs...

if EXIST bin\neutralino.exe (
    del /f bin\neutralino.exe
)

cl /std:c++17 ^
/I . ^
/I platform/windows/webview2 ^
/EHsc ^
main.cpp ^
settings.cpp ^
resources.cpp ^
server/neuserver.cpp ^
server/router.cpp ^
auth/authbasic.cpp ^
auth/permission.cpp ^
server/ping.cpp ^
helpers.cpp ^
lib/easylogging/easylogging++.cc ^
lib/boxer/boxer_win.cpp ^
platform/windows/platform.cpp ^
api/computer/computer.cpp ^
api/filesystem/filesystem.cpp ^
api/os/os.cpp ^
api/storage/storage.cpp ^
api/debug/debug.cpp ^
api/app/app.cpp ^
api/window/window.cpp ^
/DELPP_NO_DEFAULT_LOG_FILE ^
/link platform/windows/webview2/WebView2Loader.dll.lib "/OUT:bin\neutralino.exe"

if EXIST bin\neutralino.exe (
    echo Neutralino binary is compiled in to bin/netralino.exe
)

if NOT EXIST bin\neutralino.exe (
    echo ERR : Neutralino binary is not compiled
)
