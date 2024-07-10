
# Windows 7 Support for Neutralinojs

## Introduction
This document outlines the steps and findings related to adding Windows 7 support to Neutralinojs.

## Process

### Installation of Neutralinojs
1. **Installed `@neutralinojs/neu` globally without any issues.**
2. **Used the `-ExecutionPolicy Bypass` flag to bypass script execution restrictions on Windows 7.**

### Creating the Project
1. **Successfully created a new Neutralinojs project using:**
   ```sh
   powershell -ExecutionPolicy Bypass -Command "neu create my-project"
   ```

### Running the Project
1. **Encountered an error code 3 when running the project with:**
   ```sh
   powershell -ExecutionPolicy Bypass -Command "neu run"
   ```

### Error Details
- **Error Code:** 3
- **Description:** Neutralinojs executable (`neutralino-win_x64.exe`) stops with error code 3.

## Handling Missing `CoIncrementMTAUsage` Function in `ole32.dll` on Windows 7

### Error Details
While running the `neutralino-win_x64.exe`, I encountered an error indicating a missing `CoIncrementMTAUsage` function in the `ole32.dll` on Windows 7. This function only exists in Windows 8.0, 8.1, and 10.

**Error Message:**
```plaintext
The procedure entry point CoIncrementMTAUsage could not be located in the dynamic link library ole32.dll.
```

### Research and Analysis

#### Function Availability
- `CoIncrementMTAUsage` only exists in Windows 8.0, 8.1, and 10.
- The `winrt::Windows::` namespace is only functional on Windows 10.

#### Qt and WRL
- Qt does not call `CoIncrementMTAUsage`.
- Using WRL (Windows Runtime Library) is not a viable solution as it doesn't work on Windows 7.

#### Including `winrt::Windows::UI::ViewManagement`
- The safest approach is to avoid including these files in the Windows 7 version.
- You can use `#ifdef` compilation settings to compile two versions of the program, one for Windows 7 and one for Windows 10. However, this is not preferred as it involves maintaining two separate versions.

### Proposed Solution: Dynamic Linking Approach

Instead of compiling two separate versions, we can dynamically load the problematic part of the code only on supported Windows versions.

#### Implementation Steps

1. **Separate the Code:**
   - Identify the parts of the code that use `CoIncrementMTAUsage`.
   - Move this code into a separate DLL.

2. **Dynamic Loading:**
   - Use `QLibrary` or `LoadLibrary` to load this DLL at runtime.
   - Check the Windows version before loading the DLL.


### Next Steps
- **Continue documenting further findings and issues encountered.**
- **Update `README.md` with additional solutions and progress.**
- **Further listing down  Win7-failing functions along the process**

```