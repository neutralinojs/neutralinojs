#include <dwmapi.h>  // DwmSetWindowAttribute()

// Returns Windows' build number.
// ntdll.dll -> RtlGetNtVersionNumbers
DWORD GetBuildNumber() {
    HMODULE hNtdll = ::GetModuleHandleW(L"ntdll.dll");
    if (hNtdll) {
        typedef void(__stdcall*fnRtlGetNtVersionNumbers)(DWORD*, DWORD*, DWORD*);
        fnRtlGetNtVersionNumbers proc = (fnRtlGetNtVersionNumbers)GetProcAddress(hNtdll, "RtlGetNtVersionNumbers");  
        if (proc != nullptr) {
            DWORD dwMajor, dwMinor, dwBuildNumber;
            proc(&dwMajor, &dwMinor, &dwBuildNumber);
            dwBuildNumber &= ~0xF0000000;
            return dwBuildNumber;
        }
    }
    return 0;
}

// Build numbers 22000 and above are Windows 11.
bool IsWindows10() {
    return GetBuildNumber() < 22000;
}

// Returns whether High Contrast theme is enabled.
// user32.dll -> SystemParametersInfoW
bool IsHighContrast() {
    HMODULE hUser32 = ::GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI * fnSystemParametersInfo)(UINT, UINT, PVOID, UINT);
        fnSystemParametersInfo proc = (fnSystemParametersInfo)GetProcAddress(hUser32, "SystemParametersInfoW");
        if (proc) {
            HIGHCONTRASTW highContrast = { sizeof(highContrast) };
            if (proc(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
                return highContrast.dwFlags & HCF_HIGHCONTRASTON;
        }
    }
	return false;
}

// Returns whether system-wide dark mode is enabled.
// uxtheme.dll -> ShouldSystemUseDarkMode
bool IsDarkModePreferred() {
    HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxtheme) {
        // typedef BOOLEAN (WINAPI * fnShouldAppsUseDarkMode)(); // ordinal 132
        typedef BOOLEAN (WINAPI * fnShouldSystemUseDarkMode)(); // ordinal 138
        // fnShouldAppsUseDarkMode proc = (fnShouldAppsUseDarkMode)(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132)));
        fnShouldSystemUseDarkMode proc = (fnShouldSystemUseDarkMode)(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(138)));
        if (proc != nullptr) {
            return proc();
        }
    }
    return false;
}

// Sets flags to enable dark title bar in Windows 10 and 11.
// dwmapi.lib -> DwmSetWindowAttribute
HRESULT TrySetWindowTheme(HWND hWnd, bool dark) {
    const BOOL isDarkMode = dark;
    
    // set DWMWA_USE_IMMERSIVE_DARK_MODE
    // This flag works since Win10 20H1 but is not documented until Windows 11
    HRESULT result = DwmSetWindowAttribute(hWnd, 20, &isDarkMode, sizeof(isDarkMode));

    if (FAILED(result)) {
        // this would be the call before Windows build 18362
        result = DwmSetWindowAttribute(hWnd, 19, &isDarkMode, sizeof(isDarkMode));
    }

    if (FAILED(result))
        return result;

    // Toggle the nonclient area active state to force a redraw (Win10 workaround)
    if (IsWindows10()) {
        HWND activeWindow = GetActiveWindow();
        SendMessage(hWnd, WM_NCACTIVATE, hWnd != activeWindow, 0);
        SendMessage(hWnd, WM_NCACTIVATE, hWnd == activeWindow, 0);
    }

    return S_OK;
}

// Sets dark mode of title bar according to system theme.
HRESULT TrySetWindowTheme(HWND hWnd) {
    if (IsDarkModePreferred() && !IsHighContrast())
        return TrySetWindowTheme(hWnd, true);
    return S_OK;
}