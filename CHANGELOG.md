# Changelog

Add all code changes (features, deprecations, and enhancements) under the `Unreleased` topic to track changes for
the next release. Once the changes are released,
rename `Unreleased` topic with the new version tag. Finally, create a new `Unreleased` topic for future changes.

## Unreleased

### API: os.getEnvs
`os.getEnv` returns a value for a given environment variable key. Developers had to use alternative methods to retrieve a list of all environment variables with values. The `os.getEnvs` returns all environment variables as a JavaScript object similar to Node's [`process.env`](https://nodejs.dev/en/api/v18/process/#processenv).

### API: filesystem
- Added `pos` and `size` options for `readFile` and `readBinaryFile` methods to set the file cursor position and buffer size respectively.

### API: storage.getKeys
Added the `storage.getKeys` function to get an array of Neutralinojs storage keys. Now, developers don't need to write their own functions to retrieve storage keys with the filesystem API.

### API: computer.getMousePosition
Returns the current mouse cursor position via a JavaScript object that has `x` and `y` props. This function is helpful for develping interactive desktop widgets on all supported platforms.

### Bugfixes/improvements
- Replaced string error codes with enums in the C++ source code.
- Some refactorings done in C++ struct definitions and return values. 

## v4.7.0

### API: System information API
Ealier, we had the `getMemoryInfo` function in the `computer` namespace to retrieve system memory statistics. Now, we have added more functions to get details about the CPU, operating system, kernel, and connected displays:

- `computer.getArch`: Returns the CPU architecture. i.e, `x64`, `arm`, etc.
- `computer.getKernelInfo`: Returns the operating system's kernel details.
- `computer.getOSInfo`: Returns the operating system details.
- `computer.getCPUInfo`: Returns the CPU details.
- `computer.getDisplays`: Returns an array of all connected displays with the resolution, frequency-like information.

### API: os dialogs
- Add the `defaultPath` option to `showSaveDialog`, `showOpenDialog`, and `showFolderDialog` functions to set the initial path/filename of system dialogs.

### DevOps
- Run the test suite on Windows GitHub Actions instance.
- Fix the `armhf` framework binary generation issue in the GitHub Actions workflow.

### Bugfixes/improvements
- Hide the automation info bar from the Chrome mode by default -- Developers can add the `--enable-automation` flag to the config file if they need the particular command-line switch.


## v4.6.0

### API: Process spawning API
We have `os.execCommand` for launching processes, but it's synchronous, meaning, the developer has to wait unti process completion to receive `pid`, `stdOut` and `stdErr`. `execCommand` is not suitable for long-running processes. The new spawning API offers API functions for handling long-running processes in a multi-threaded way.

- `os.spawnProcess(command)`: Spawns a process and returns `id` (A virtual Neutralino-scoped pid) and `pid` (Operating system-level pid).
- `os.getSpawnedProcesses()`: Returns a list of spawned processes.
- `os.updateSpawnedProcess(id, action, data)`: Sends an action event for the spawned process. Supports the following actions:
  - `stdIn`: Sends a string via the standard input stream. `data` is the input string.
  - `stdInEnd`: Closes the standard input stream.
  - `exit`: Terminates the spawned process.

### Core: events
- `spawnedProcess`: Dispatched when there is a change in the spawned process. `CustomEvent` gets triggered with the following object:
```js
{
  id: <id>,
  pid: <pid>,
  action: <action>,
  data: <data>
}
```
Available actions:
- `stdOut`: Outputs standard output data. `data` contains the standard output payload.
- `stdErr`: Outputs standard error data. `data` contains the standard error payload.
- `exit`: Notified when the process terminates. `data` contains the process exit code.


### DevOps
- Add official Linux ARM (armhf and arm64) binary build support for the standard release workflow and nightly build.

### API: filesystem
- Expose `createdAt` and `modifiedAt` JavaScript timestamps via the `filesystem.getStats` function.

### Bugfixes/improvements
- Hide the `.storage` directory (Neutralinojs storage location) on Windows.
- Support using query parameters in resources URLs. i.e., `index.html?v=2.0` is supported, but caching is not yet implemented.
- Show a user-friendly error message for Neutralinojs server initialization failures.

## v4.5.0

### Core: events
- Added `windowFocus` and `windowBlur` native events.

### DevOps
- Use [BuildZri](https://github.com/codezri/buildzri) for C++ build automation.
- Implement a new workflow for nightly releases. Developers can get the nightly release binaries via the `nightly` version tag.

### Bugfixes/improvements
- Search and dynamically load app indicator libraries on Linux. This enhancement supports Neutralinojs to start without a crash when there is no app indicator library present on the system.
- Throw `NE_OS_TRAYIER` from the `os.setTray` function for initialization failures.
- Fix binary file write error on Windows with the `filesystem.writeBinaryFile` function. This was fixed by using `LF` as the line breaker on all platforms as a portable solution. If the developer needs CRLF on Windows (or CR on Darwin), the developer needs to handle it explicitly with `NL_OS`.

### Core: global variables
- Add `NL_COMMIT` to hold framework's release commit. This is helpful to find the nightly release's code snapshot.

## v4.4.0

### API: window
- Add `window.getPosition` to get the current window coordinates.

### API: filesystem
- Add `filesystem.appendFile` to append text content to a file. Thrown errors are similar to the `filesystem.writeFile` function.
- Add `filesystem.appendBinaryFile` to append binary content to a file. Thrown errors are similar to the `filesystem.writeBinaryFile` function.

### Dependencies
- Use `ayatana-appindicator3-0.1` instead of `appindicator3-0.1` (Marked as obsolete in [Debian packages](https://www.debian.org/releases/bullseye/amd64/release-notes/ch-information.en.html#noteworthy-obsolete-packages))

### Bugfixes/improvements
- Dynamically call `SetProcessDpiAwarenessContext` function to support previous Windows versions.
- Display the native method name with the `NE_RT_NATPRME` error payload.

## v4.3.0

### Core: auth
- Added `tokenSecurity` to the configuration to improve the security of `NL_TOKEN`. Accepted values are `none` and `one-time`. If `none` is set, Neutralinojs server will always expose `NL_TOKEN` for any application instance, so you can open Neutralinojs apps from browser directly. If `one-time` (recommended) is set, Neutralinojs server expose `NL_TOKEN` only once and client persists the token in `sessionStorage`, so all other clients that acces the app after the initial client will get an auth error message (Displayed to the user via HTML).

### API: window
- `window.setAlwaysOnTop(bool)` was added.
- `window.getSize` was added.

### Core: global variables
- Added `NL_EXTENABLED` which returns `true` if extensions are enabled. It is used to check extensions status faster during initialization process.

### Bugfixes
- Fixed the file dialog order issue on Windows.
- Removed tray icon when the application exits.

## v4.2.0

### API: clipboard
- Add clipboard API for all platforms. Supports reading and writing text in the system clipboard.

### Core: configuration
- Accept chrome-related CLI args: `--chrome-width`, `--chrome-height`, and `--chrome-args`.
- Accept `--neu-dev-extension` to enable devtools connections (For internal usages with CLI).
- Accept custom HTTP headers via `serverHeaders` option.

### API: global variables
- Added `NL_RESMODE` to get details about application resources source. Returns `bundle` if resources are loaded from `resources.neu`. Otherwise, returns `directory`.

### Bugfixes
- Fix chrome mode user data dir issue (Support paths with spaces).
- Fix browser mode's shutdown issue with neu CLI.

## v4.1.0

### API: window
- `window.getTitle` returns the current native window title.

### API: os
- Remove `NE_OS_ENVNOEX` from `os.getEnv` and return an empty string if environment variable is not defined.

### Core: Chrome mode
- Now you can run Neutralinojs apps as Chrome apps via the `chrome` mode.

### Core: configuration
- Getting chrome mode config from `modes.chrome`. Added `height`, `width`, and args specially for the chrome mode.

## v4.0.0

### API: os.execCommand
- Renamed `shouldRunInBackground` option to `background`.
- Supports `stdIn` as an optional input.
- Returns multiple values: `stdOut`, `stdErr`, `pid`, and `exitCode`.

### Core: API extensions
- Config file supports new `extensions` array globally or in a specific mode.
- Neutralino spawns extension processes based on `extensions` array.
- Neutralino doesn't send kill signals to extension processes. The extension developer needs to stop
  processes properly.
- `extension` setting has 4 fields: `id`, `command`, `commandLinux`,`commandDarwin`, and `commandWindows`.
- The `command`'s (platform-specific command's) value accepts `${NL_PATH}` global variable.
- Each extension process instance is called with three CLI args: `--nl-port={}`, `--nl-extension-id={}` and `--nl-token={}`
- Enable/disable extensions with `enableExtensions` config.

### Core: Static server
- Supports setting document root (with `documentRoot`) via config. Now, it's possible to launch app without a subdirectory in the URL.

### Core: Community driver processes
Now developers can use Neutralinojs as a part of their software with any programming language by spawning Neutralinojs as a child process.
Use `exportAuthInfo` to write auth details to `${NL_PATH}/.tmp/auth_info.json`. Then the parent process can pickup access details there. Note that WebSocket communication needs to be initiated via extensions API/loader.

### API: app.broadcast
- Sends an event to all app clients. This method is for extension developers.

### API: events.broadcast
- Sends an event to all clients (apps and extensions). Useful for notifying important general events.

### API: extensions
- `dispatch`: Sends an event to a specific extension.
- `broadcast`: Sends an event to all connected extensions. Useful for sending shutdown signals.
- `getStats`: Returns details about loaded extensions and connected extensions.

### API: updater
- `checkForUpdates`: Send a request to a seed URL (JSON) and fetch update details.
- `install`: Install updates based on the currently downloaded manifest.

### Events
- `appClientConnect` and `appClientDisconnect`: Occurs when a new app instance is launched and closed respectively.
- `extClientConnect` and `extClientDisconnect`: Occurs when a new extension is connected and disconnected respectively.
- `extensionReady` can be used to implement immediate extension calls. This is implemented from the client-side with `extensions.getStats` and `extClientConnect`. This event gurantees that it will be triggered regardless of the extension's start time.

### Error codes
- `NE_EX_EXTNOTC`: Thrown by `extensions.dispatch` if the target extension is not connected.
- `NE_UP_CUPDMER`: Thrown by `updater.checkForUpdates` if the JSON update manifest is invalid or applicationId is not matching.
- `NE_UP_CUPDERR`: Thrown by `updater.checkForUpdates` if the updater API cannot fetch the manifest.
- `NE_UP_UPDNOUF`: Thrown by `updater.install` when the update manifest is not loaded.
- `NE_UP_UPDINER`: Thrown by `updater.install` for update installation errors.

### Bug fixes
- Fix port 0 issue with modes. Earlier, if the developer sets port as 0 from a specific mode,
the `NL_PORT` also becomes 0.
- Fix an issue with `writeToLogFile` config option. Earlier, the log file was created even this
option is set to `false`.

### Global variables
- `NL_APPVERSION`: Value of the `version` key in the config file.
