# Changelog

Add all code changes (features, deprecations, and enhancements) under the `Unreleased` topic to track changes for
the next release. Once the changes are released,
rename `Unreleased` topic with the new version tag. Finally, create a new `Unreleased` topic for future changes.

## Unreleased

## v6.5.0

### Core: events
- New window events: `windowMinimize`, `windowRestore`, `windowMaximize`, `windowFullScreenEnter`, and `windowFullScreenExit`

### API: window
- Add `window.setBorderless(bool)` to toggle borderless mode while the Neutralinojs app is running.

### Configuration
- Add `modes.chrome.browserBinary` option to set custom browser binary path under the chrome mode. If this field is specified, the framework will try to launch Chrome from there. If it fails, the framework will initiate the Chrome binary search as usual:
```js
// cross-platform
"browserBinary": "/path/to/chrome/bin"

// platform-specific path
"browserBinaryLinux": "/usr/bin/google-chrome",
"browserBinaryDarwin": "/Applications/Google Chrome.app",
"browserBinaryWindows": "C:\\Programs\\Google Chrome\\chrome.exe"

// cross-platform (with path constants)
"browserBinary": "${NL_OSDATAPATH}/chrome/bin"
"browserBinaryWindows": "${NL_OSDOWNLOADSPATH}/chrome.exe"
```
- Add the `modes.window.useLogicalPixels: true|false` option to activate DPI-aware sizing based on the operating system's display scale factor.
- Add extra path constants support (early versions only supported `${NL_PATH}`) for the extensions command: `${NL_OSDATAPATH}`, `${NL_OSCACHEPATH}`, ... All supported path constants use this format: `${NL_OS<name>PATH}` where `<name>` is any accepted parameter (uppercased) to the `os.getPath(name)` function:
```js
"commandLinux": "${NL_OSDOWNLOADSPATH}/extensionBinary --load"
``` 

### Improvements/bugfixes
- Fix issues with filter extension handling of the file dialogs API on Linux
- Fix tray icon disappearing issue on Windows

## v6.4.0

### API: storage
- Implement `storage.clear()` and `storage.removeData(key)` functions to remove saved storage data.

### Improvements/bugfixes
- Fix the draggable region not working issue on Windows.
- Replace deprecated macOS API with suitable modern APIs in the codebase.
- Static file server enhancements.
- Improve `window.getPositon()` in macOS.
- Save the correct window size and position when a maximized/minimized window is being closed on Windows.
- Fix the window disappearing issue while restoring the window on Windows

## v6.3.0

### Single-executable mode
Earlier, the Neutralinojs framework normally loaded resources either from the resources directory or the `resources.neu` file, and didn't offer a proper way to use embedded resource files in the app binary to allow developers to create single-executable apps. Now, the framework loads resources from the binary itself on all platforms if the app was built using the `neu build --embed-resources` CLI flag. This feature deprecates the `--load-dir-res` option and introduces the new `--res-mode=<mode>` option to choose the preferred resource loading mode from `embedded` (default), `bundle`, or `directory`. 

The Neutralinojs single-executable feature internally uses the [`postject` library](https://github.com/nodejs/postject) to embed the `resources.neu` file into platform-specific binaries via the neu CLI and the `postject` library C header file to read the embedded resources during application runtime.

### Configuration
- Support the `window.skipTaskbar` boolean option to hide the application icon from the operating system taskbar/dock. This option can also be passed from the command line via the `--window-skip-taskbar=<true|false>` option.
- Implement the `window.openInspectorOnStartup` boolean option to configure auto-opening the inspector window. This feature is also available via the `--window-open-inspector-on-startup=<true|false>` command-line flag.

### Improvements/bugfixes
- Fix WebView2 crash when Windows usernames or executable paths contain Unicode characters (e.g., äüö, Chinese characters). Replaced ANSI Windows APIs with Unicode equivalents and added proper null pointer checking for environment variable access.
- Include details about missing parameter names in the error object of `NE_RT_NATRTER`.

## v6.2.0

### API: window
- Add `Neutralino.window.print()` to display the native print dialog on all platforms. This was especially added since the macOS webview doesn't implement the `window.print()` function.
- Introduce the `window.beginDrag()` function to trigger native window dragging. The new draggable region API implementation uses this function internally. 

### API: filesystem
- Add `filesystem.getJoinedPath(...paths: string[])` to create a single path by joining multiple path strings.
- Add `filesystem.getNormalizedPath()` and `filesystem.getUnnormalizedPath()` functions, which make Windows paths look like Unix paths by replacing `\\` with `/` and revert normalized paths into Windows-specific paths respectively on the Windows platform. On non-Windows platforms, these functions return the same input strings.

### Configuration
- Implement the `window.webviewArgs` configuration option to pass additional browser arguments to the WebView2 instance on Windows:
```js
"modes": {
  "window": {
     // ....
     "webviewArgs": "--user-agent=\"Custom user agent\""
  }
}
```

### Improvements/bugfixes
- Display GUI error messages for webview initialization failures. i.e., if the WebView2 runtime is not installed on Windows and if the WebKitGTK library is not installed on GNU/Linux platforms.

## v6.1.0

### API: Native window main menu
The new `window.setMainMenu(menu)` function lets developers create a native window menu on GNU/Linux and Windows and an application menu on macOS. This function can be called multiple times with different menu objects to update menu items dynamically:

```js
const menu = [
  {id: 'file', text: 'File',
    menuItems: [
    {id: 'open', text: 'Open'},
    {text: '-'},
    {id: 'quit', text: 'Quit'},
  ]},
  {id: 'edit', text: 'Edit',
    menuItems: [
    {id: 'cut', text: 'Cut'},
    {id: 'copy', text: 'Copy'},
    {id: 'paste', text: 'Paste'},
  ]}
];

await Neutralino.window.setMainMenu(menu);
```
The framework will trigger the `mainMenuItemClicked` event with menu item data when the user clicks on a specific menu item.

On macOS, app developers can register key accelerators and pre-defined actions as follows:

```js
{id: 'edit', text: 'Edit',
  menuItems: [
  {id: 'cut', text: 'Cut', action: 'cut:', shortcut: 'x'},
  {id: 'copy', text: 'Copy', action: 'copy:', shortcut: 'c'},
  {id: 'paste', text: 'Paste', action: 'paste:', shortcut: 'v'},
]}
```

On GNU/Linux and Windows, the framework only displays the keyboard shortcut within the particular menu item and doesn't register a key accelerator yet:

```js
{id: 'copy', text: 'Copy', shortcut: 'Ctrl + C'}
```

*Note: We are planning to add key accelerator support for GNU/Linux and Windows native window menus with a global key accelerator feature in an upcoming framework version.* 

### Core: global variables
- Add `NL_LOCALE` to get the user locale name, e.g., `en_US.UTF-8`
- Add `NL_COMPDATA` to display custom data strings embedded in the binary via the BuildZri configuration. Developers can use this global variable to set the build number or other static data when they compile their own framework binary with the BuildZri script:

```json
"definitions": {
    "*": [
        "NEU_COMPILATION_DATA=\\\"build_number=${BZ_BUILDNUMBER};compiler_name=${BZ_CONPILERNAME}\\\"",
```

## v6.0.0

### API: clipboard
- Implement `clipboard.writeHTML(html)` and `clipboard.readHTML()` functions to write/read HTML strings

### API: os
- Adding `envs` key-value pair parameter to the `options` of the `os.execCommand(command, options)` function to set specific environment variables for the child process.
- Change the `os.spawnProcess(command, cwd)` to `os.spawnProcess(command, options)` to set environment variables and the current working directory via the `options` object for the spawned child process:
```js
// e.g.:
await Neutralino.os.spawnCommand('env', {
  cwd: NL_PATH,
  envs: {
    VAR1: 'var1',
    VAR2: 'var2'
  }
});
```

### API: filesystem
- Add the `timestamp` (ISO 8601) property to the `watchFile` event's data payload to identify when a specific file watcher event occurred.
- Implement `filesystem.setPermissions(path, permissions, mode)` and `filesystem.getPermissions(path)` functions to set/get file permissions in a cross-platform way:
```js
// e.g.:
await Neutralino.filesystem.setPermissions(NL_PATH + '/my-directory-1', {ownerRead: true, groupRead: true});
await Neutralino.filesystem.setPermissions(NL_PATH + '/my-directory-2', {all: true});
await Neutralino.filesystem.setPermissions(NL_PATH + '/my-directory-3', {otherAll: true}, 'REMOVE');

const permissions = await Neutralino.filesystem.getPermissions(NL_PATH);
// permissions -> {all:.., ownerRead, ownerWrite...}
```
### Core: extensions
- Extensions are now loaded internally using the `os.spawnProcess()` function without triggering process events. This modification displays extension logs within the Windows terminal and lets app developers control extensions using the existing spawn process API.

### Security
- Improve the `NL_TOKEN` generation algorithm to strengthen security using the C++ `std::mt19937` random number generator.

### Improvements/bugfixes
- Fix framework crashing when creating the `.tmp` directory under restricted file manipulation permissions.
- Fix several issues in the Windows-specific GUI notification implementation of the `os.showNotification()` function.
- Fix invalid utf8 character handling issues in several native APIs (i.e., `os.spawnProcess('./bin')` crashed if `bin` output `"ä\xA9ü"`)

## v5.6.0

### API: server
Neutralinojs doesn't support the `file://` protocol to load local resource files due to application security concerns. Because of this limitation, app developers had to read files using filesystem APIs. The new `server` namespace implements `server.mount(path, target)`, `server.unmount(path)`, and `server.getMounts()` functions to let developers load local files from the Neutralinojs static server by creating directory mappings as an alternative for the `file://` protocol.

For example, the following function call configures the Neutralinojs static server to serve resources on the `${NL_PATH}/app-res` directory:

```js
await Neutralino.server.mount('/app-res', NL_PATH + '/app-res');
```

With the above server configuration, `NL_PATH + '/app-res/stat.txt'` can be loaded to the webview via the following URL:

```
http://127.0.0.1/app-res/stat.txt
```

This local directory mounting configuration can be deactivated as follows:

```js
await Neutralino.server.unmount('/app-res');
```

### API: resources
- Fallback to native filesystem APIs when `NL_RESMODE` is `directory`.
- Implement `resources.getStats(path)` and `resources.extractDirectory(path, destination)` functions.


### API: window
- Implement the `window.snapshot(path)` function to capture the window and save it as a PNG image file.

### Improvements/bugfixes
- Fix the empty string returning issue with the `window.getTitle()` function on Windows.
- Create non-existent directories while extracting resource files using the `resources.extractFile()` function.
- Supports using large `resources.neu` files.

### DevOps
- Fix minor string formatting issues in the BuildZri automation script.
- Fix various test suite failure scenarios.

## v5.5.0

### Client library and globals injection
Neutralinojs apps usually load globals and the client library using HTTP requests via the static server. This mechanism implements a generic way to enable the native API on all supported Neutralinojs app modes. However, this strategy prevents
enabling the Neutralinojs native API if the primary web app is loaded through another server (local or remote). Now, app developers can inject globals and
the client library script into external web services using `window.injectGlobals` and `window.injectClientLibrary` configuration options on the window mode.

These options are available as CLI options as well (`--window-inject-globals` and `--window-inject-client-library`), so developers can use these options via the `window.create(url, options)` function. This code injection feature currently works with HTTP URLs only (can be used with local and remote HTTP web servers).

### Preload script support
The framework already lets developers set pre-defined global variables for each web page by using custom globals from the app configuration and activating the `window.injectGlobals` option. However, custom globals are static values, so app developers can't define dynamic values or run a custom JavaScript source using globals and `window.injectGlobals` features. This framework version implements the `window.injectScript` configuration option to inject and run custom JavaScript source file before running the primary webapp's JavaScript sources.

For example, the following setup loads an initialization script from the `preload.js` file:

```json
"window": {
  "injectScript": "/resources/js/preload.js"
}
```

Developers can use native API calls within initialization scripts if `window.injectClientLibrary` is set to `true`. This option also can be set via `--window-inject-script` and `window.create(url, options)`.

### Configuration
- Implement the `dataLocation` config option to let users set data directory for framework data storage purposes, such as saving window state, storing extracted resources, etc. If `app` (default) is used, the framework will store app data within the app directory and if `system` is used, the framework will use a platform-specific data directory path (i.e., `/home/username/.local/share/<appId>` on GNU/Linux systems) to store app data. App developers can obtain the current data directory string from the `NL_DATAPATH` global variable.
- Implement the `storageLocation` config option to let developers use system data directory for the Neutralinojs storage. If this option is 'app' (default), the framework store storage files within the app directory. If `system` is used, the framework will use the platform-specific standard data directory. In both `app` and `system` modes, the framework will use the `.storage` sub-directory for storage files.

### Improvements/bugfixes
- Search and load WebKitGtk functions dynamically from the available webkit2gtk library: `libwebkit2gtk-4.0-37` or `libwebkit2gtk-4.1-0`.
- Fix the auto-reload issue during app development.

## v5.4.0

### API: resources
- Implement `getFiles()`, `extractFile(path, dest)`, `readFile(path)`, and `readBinaryFile(path)` functions via the `resources` module for reading the files embedded in the `resources.neu` resources bundle. These functions works only if the framework loaded resources from the resource bundle -- they will throw `NE_RS_APIRQRF` if the framework loaded resources from the resources directory.

### API: window
- Implement `minimize()`, `unminimize()`, and `isMinimized()` functions to minimize and restore the native app window. 

### Improvements/bugfixes

- Fix issues with the `clipboard.writeImage()` function on Windows.
- Fix the unwanted delay with the `window.exitProcessOnClose` configuration option on Windows.
- Fix a bug with the `window.isFullScreen()` function on GNU/Linux-based platforms.
- Fix duplicate virtual PID issues with the `os.spawnProcess()` function.

## v5.3.0

### Configuration: window transparency on Windows
Window transparency support was added on the Windows version of the Neutralinojs framework. This can be activated with `modes.window.transparent` configuration property or `--window-transparent` command-line option. Unlike in other platforms, Windows native window becomes borderless (window controls will be hidden) with the activation of the transparent mode. 

### API: os
- Add the `temp` key for the supported directory list of the `os.getPath(pathKey)` function.

### API: filesystem
- Add the `filesystem.getAbsolutePath(path)` function to let developers get a full path string from a relative path string.
- Add the `filesystem.getRelativePath(path, ?base)` function to get a relative path from a path and a base path.
- Add the `filesystem.getPathParts(path)` to parse and get path segments like filename, extension, root path, etc. 

### Improvements/bugfixes
- Fix Unicode issues in the Windows version with filesystem, storage, and general modules.
- Fix standard output/error data display issues on Windows

## v5.2.0

### Configuration: Configless framework initialization
Now, developers can load the Neutralinojs framework without creating a `neutralino.config.json` file. Now, there is no mandatory configuration properties since Neutralinojs set reasonable defaults. Developers can launch the framework using the following methods without a configuration file:

```bash
# Loading a remote URL
./framework-bin --url=https://neutralino.js.org/docs

# Launches a local static web app
./framework-bin --url="/resources/" --window-title="My web app" --enable-server
```
### Configuration
- New internal CLI arguments added: `--single-page-serve`, `--enable-native-api` and `--document-root=<string>`

### Core: Static server SPA (Single Page App) serving
Earlier, Neutralinojs app developers had to use hash routing with their frontend-library-based apps since the internal static server didn't offer an inbuilt URL rewrite logic for SPAs (Single Page Apps). Now, they can use the `singlePageServe: true` option in the app configuration file to activate SPA routing. If this setting is on, the static server will serve the main `index.html` file when it receives directory requests that possibly send HTTP `404` status.

For example, the `/invoices` path serves the main `index.html` file if there is no directory named `invoices` which holds an `index.html` file. 

### API: window
- Improve the behaviour of the `window.show()` function on Windows. Now, this function flashes the window if it's already in foreground and activates the window properly if it's minimized.

### Improvements/bugfixes
- Fix path issues with the `defaultPath` option in system file dialogs on Windows. 

## v5.1.0

### Configuration: window transparency
Neutralinojs offers the inbuilt borderless mode and draggable region API to create custom window frames using HTML and CSS. Earlier, Neutralinojs used a default opaque color (usually white) for the window and webview layer, so app developers couldn't make custom CSS-styled window frames transparent or implement custom window frame shapes (i.e., rounded edges). Now, it offers the `window.transparent` boolean flag to activate window transparency. 

If the transparency mode is on, the Neutralinojs window and webview use an alpha color layer and become transparent, so developers can use the `rgba` CSS color function on `body` background to handle the transparency level of the app. 

The transparency mode can be activated using the `--window-transparent=<bool>` internal command-line option too.

*Note: This feature is not implemented for the Windows operating system yet.* 

### API: clipboard
- Implement `clipboard.readImage()` and `clipboard.writeImage(image)` functions to work with clipboard image data.
- Expose the `clipboard.clear()` function to clear system clipboard.
- Expose the `clipboard.getFormat()` function to check the system clipboard data format. This function returns `text`, `image`, and `unknown` enum values based on existing data on the clipboard.

## v5.0.0

### API: Reading, writing with standard streams
- Implement `app.readProcessInput(readAll=false)` for reading string data from the standard input stream. This function can read a single line or multiple lines at once.
- Implement `app.writeProcessOutput` and `app.writeProcessError` for sending string data to standard output and error streams accordingly.

### API: filesystem
- Enable default recursive behavior in the `filesystem.createDirectory` function.
- Add `filesystem.copy`, `filesystem.move`, and `filesystem.remove` functions.
- Add `{recursive}` options object to `filesystem.readDirectory(path, options)` activate recursive directory listing.

### API: os
- Accept stdin with the background mode of the `os.execCommand` function.

### Core: security
- Now, all app clients and extensions require a valid connect token (A part of the `NL_TOKEN`) to connect to the Neutralinojs framework WebSocket server to receive app events.
- The framework now sends the access token, connect token, port, and extension identifier to extension processes via the standard input, so other processes can't read them by scanning the process list.
- The WebSocket client connection origin is checked during the HTTP/WS handshake to make sure that only local clients connect with a specific Neutralinojs app that runs on window, browser, or Chrome mode. Any URL origin is allowed for the cloud mode.
- The above security enhancements fix two security advisories on GitHub. 

### Global variables
- Now the `NL_ARCH` global variable returns the same value that `computer.getArch` returns.

### Deprecations
- Removed `filesystem.removeFile`, `filesystem.removeDirectory`, `filesystem.copyFile`, and `filesystem.moveFile` functions.
- Extensions won't receive `--nl-port`, `--nl-token`, and `--nl-extension-id` as command-line arguments. Now the framework sends these via stdin as a JSON string. 

## v4.15.0

### Configuration: custom user agent string
Developers sometimes use the user agent string to indentify the client in server-side and client-side source codes. Now, Neutralinojs lets app developers extend the default user agent string with a custom string via the `window.extendUserAgentWith` configuration property and the `--window-extend-user-agent-with=<string>` command-line option, as shown in the following example:
```js
"window": {
  // ---
  "extendUserAgentWith": "MyAppClient"
}
```
The above configuration extends the user agent string:

```js
console.log(navigator.userAgent)    // <Default UA> MyAppClient 
```

### Configuration: custom configuration files
The Neutralinojs framework typically loads the application configuration content from the `neutralino.config.json` file, but sometimes app developers need to use a custom configuration filename during development (i.e., For separating production and development environments). Now, you can use the `--config-file=<filename>` internal command-line parameter to use a custom app configuration, as shown in the following example:

```bash
./myapp-linux_x64 --load-dir-res --config-file=neutralino-dev.config.json
```

## v4.14.1

### DevOps
- Use Windows 2019 server and macOS 11 (Big Sur) to compile framework release artifacts.
- Set minimum supported macOS version via `MACOSX_DEPLOYMENT_TARGET=10.7`

## v4.14.0

### Filesystem API
- Introduce the `filesystem.getWatchers` function to get all created watchers.
- Add binary file reading support for the `filesystem.updateOpenedFile(id, action, data)` function. This function implements `readBinary` and `readAllBinary` actions and `dataBinary` output action in the `openedFile` global event.
- Return the existing watcher identifier from `filesystem.createWatcher` if there is an existing active watcher for the given path.

### OS API
- Add a way to set current working directory for process creation functions: `os.execCommand(command, options)` now supports `cwd` via the options object and `os.spawnProcess(command, cwd)` accepts currently working directory via the second string parameter.

### Improvements/bugfixes
- Fix Unicode charactor issues in the tray menu on Windows.
- Avoid including null bytes to file reader events initiated by the `filesystem.openFile` function.
- Discard `window` method executions on non-window modes.

### DevOps
- Update external GitHub Action packages to their latest versions.
- Fix issues of the failing test cases

## v4.13.0

### Core: persistent window state

Now the framework stores the primary window state in a temporary file (JSON formatted) and loads during the startup process.
This feature stores and sets the window position (x, y coordinates), size (width and height), and maximized status. This feature is enabled by default in all platforms, but app developers can turn it off by using `false` for the `window.useSavedState` configuration attribute or `--window-use-saved-state` internal CLI argument.

Also, the window state loading status is available via the `NL_WSAVSTLOADED` global variable. The framework sets `true` for this boolean variable if the window state was loaded from the saved configuration.

### Improvements/bugfixes
- Fix several issues in the webview Windows code (i.e., Wait for the window close event, window style fixes, etc.).

## v4.12.0

### API: window
- Add `window.center` to center the application window programmatically.

### Core: configuration
- Setting the initial window position via `modes.window.x` and `modes.window.y` integer config props.
- Centering the window at startup via `modes.window.center` boolean config props.
- Add config overrides for the startup window position: `--window-center=<true|false>`, `--window-x=<int>` and `--window-y=<int>`

### Core: webview
- Statically link the Webview2 loader library on Windows, so app developers can package their apps without including the `WebView2Loader.dll` file.

### Improvements/bugfixes
- Added Unicode characters support for Neutralinojs Windows.

## v4.11.0

### API: File watchers

In some scenarios, Neutralinojs app developers need to implement file watchers in their apps. Earlier, Neutralinojs API didn't offer a native file watcher API and developers had to use less-performant workarounds, such as making recursive `filesystem.readDirectory` calls. The new file watchers API lets you create native, cross-platform, event-based file watchers for filesystem paths with the following functions:

- `filesystem.createWatcher(path)`: Creates a new file watcher for a given path and returns the watcher identifier.
- `filesystem.removeWatcher(watcherId)`: Removes a files watcher based on a file watcher identifier.

Whenever a filesystem change occurs, file watcher instances dispatch the `watchFile` event with the following data:

- `id`: Watcher identifier
- `action`: Filesystem change: `add`, `delete`, `modified`, and `moved`
- `dir`: Directory path
- `filename`: Modified file

### Improvements/bugfixes

- Fix the initial window flashing and hidden window state issues on Windows.
- Fix data corruption in binary file appending.
- Apply dark/light themes based on system preferences on Windows.

## v4.10.0

### DevOps

- Generate official macOS `arm64` (For Apple M1 and higher devices) and universal binaries with the GitHub workflow.

### Improvements/bugfixes

- Add a [JSON schema](https://json-schema.org/) definition for the `neutralino.config.json` file.
- Fix source code compilation issues on macOS `arm64` systems.

### ESM/NPM support

Neutralinojs typically stores the client library implementation in a separate file (`neutralino.js`). This approach makes Neutralinojs app development
process easier with a globally exposed JavaScript object, `Neutralino`. But, modern web developers use ES modules and they usually like to fetch dependencies from NPM. So, now, developers can load the `__neutralino_globals.js` internal script (i.e., `<script src="__neutralino_globals.js"></script>`) to load only globals. Then, they can use the client library implementation via `neutralino.mjs` with the neu CLI or from [`@neutralinojs/lib`](https://www.npmjs.com/package/@neutralinojs/lib) with a Node package manager.

## v4.9.0

### API: Custom methods

Neutralinojs offers the extensions API to write custom backend code with any programming language, but extensions come with the following drawbacks that affect apps in several scenarios:

- Extensions use a shared WebSocket for communication, so using direct C++ references (i.e., the window handler) is impossible within extensions.
- The developer is responsible for packaging their extension binaries.
- A C++-based extension is not fast as native C++-based code due to the WebSockets-based IPC.

Alternatively, a developer can download the framework C++ code, modify it, and re-compile it. But, the developer may face issues while synching upstream code modifications. So, Neutralinojs offers a separate namespace, a function template, inbuilt helper functions (i.e., to get the window handler, validation, etc.), and a developer guide to add custom APIs to the Neutralinojs framework without updating the framework core.

Example:

```js
let res = await Neutralino.custom.fetch('https://neutralino.js.org');
```

If developers make a custom API that others can use, we motivate them to contribute to the Neutralinojs framework by adding it to the main codebase.

Example:

```js
let res = await Neutralino.net.fetch('https://neutralino.js.org');
```

If the developer adds a new custom method to the framework, the client library will automatically export it to the `Neutralino.custom` namespace by using the `NL_CMETHODS` internal global variable. The `Neutralino.custom.getMethods` function returns an array of custom methods similar to `NL_CMETHODS`.

### API: File streams

The current Neutralinojs API offers non-stream-based (Promise-based but synchronous-like) functions for working with files. For example, the `filesystem.readFile` function reads the file content and retrieves data synchronously even though the WebSocket communication mechanism supports asynchronous patterns. However, the synchronous API lets developers work with files in a simple way, but they face the following issues in some scenarios:

- Reading large files is not performance-friendly (The whole file content gets loaded into the memory).
- Unable to work with dynamic file objects (i.e., Device files).
- File descriptors are not persistent within the app lifecycle as they get destroyed after native API calls.

To solve this issue, we offer an event-based file stream API with the following functions/events:

#### Functions

- `filesystem.openFile`: Creates a file stream by openning a file.
- `filesystem.updateOpenedFile`: Triggers a file `read`/`readAll` event or sets the file cursor.
- `filesystem.getOpenedFileInfo`: Returns (awaited) information about the file stream (Props: `id`, `eof`, `pos`, and `lastRead`)

#### Events

- `openedFile`: Occurs per each file read event and whenever the file stream reaches `EOF`.

## v4.8.0

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
