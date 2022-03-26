# Changelog

Add all code changes (features, deprecations, and enhancements) under the `Unreleased` topic to track changes for
the next release. Once the changes are released,
rename `Unreleased` topic with the new version tag. Finally, create a new `Unreleased` topic for future changes.

## Unreleased

## DevOps
- Use [BuildZri](https://github.com/codezri/buildzri) for C++ build automation.

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
