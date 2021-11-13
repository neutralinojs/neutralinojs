# Changelog

Add all code changes (features, deprecations, and enhancements) under the `Unreleased` topic to track changes for
the next release. Once the changes are released,
rename `Unreleased` topic with the new version tag. Finally, create a new `Unreleased` topic for future changes.

## Unreleased

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

### Core: Community driver processes

Now developers can use Neutralinojs as a part of their software with any programming language by spawning Neutralinojs as a child process.
Use `exportAuthInfo` to write auth details to `{NL_PATH}.tmp/auth_info.json`. Then the parent process can pickup access details there. Note that WebSocket communication needs to be initiated via extensions API/loader.


### API: app.broadcast
- Sends an event to all app clients. This method is for extension developers.

### API: events.broadcast
- Sends an event to all clients (apps and extensions). Useful for notifying important general events.

### API: extensions
- `dispatch`: Sends an event to a specific extension.
- `broadcast`: Sends an event to all connected extensions. Useful for sending shutdown signals.
- `getStats`: Returns details about loaded extensions and connected extensions.

### Error codes

- `NE_EX_EXTNOTC`: Thrown by `extensions.dispatch` if the target extension is not connected.

### Bug fixes

- Fix port 0 issue with modes. Earlier, if the developer sets port as 0 from a specific mode,
the `NL_PORT` also becomes 0.
- Fix an issue with `writeToLogFile` config option. Earlier, the log file was created even this
option is set to `false`.
