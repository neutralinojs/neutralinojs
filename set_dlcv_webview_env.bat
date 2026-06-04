@echo off
chcp 65001 >nul
setlocal

echo ==========================================
echo  Set DLCV WebView2 Environment Variables
echo ==========================================
echo.

REM Check admin privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Please run this batch file as Administrator!
    echo.
    pause
    exit /b 1
)

set "DLCV_WEBVIEW=C:\dlcv\bin\webview\Microsoft.WebView2.FixedVersionRuntime.148.0.3967.96.x64"

echo Setting system environment variables...
echo.
echo   DLCV_WEBVIEW=%DLCV_WEBVIEW%
echo.

setx /M DLCV_WEBVIEW "%DLCV_WEBVIEW%" >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Failed to set DLCV_WEBVIEW
    pause
    exit /b 1
)

echo [OK] System environment variables set successfully!
echo.
echo Please restart your application or open a new command prompt
echo for the changes to take effect.
echo.
pause
