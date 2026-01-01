# Example Native Extension for Neutralinojs

This is a sample dynamic library extension that demonstrates the new dynamic linking capabilities of Neutralinojs.

## Features

This example extension provides the following functions:

- `get_system_info()` - Returns platform and OS information
- `get_timestamp()` - Returns current date/time and Unix timestamp
- `echo()` - Echo back input with metadata
- `get_config()` - Returns the configuration passed during initialization

## Building

### Windows
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux/macOS
```bash
mkdir build
cd build
cmake ..
make
```

The compiled library will be output to the extensions directory.

## Usage in Neutralinojs

Update your `neutralino.config.json`:

```json
{
  "modes": {
    "window": {
      "enableExtensions": true,
      "extensions": [
        {
          "id": "example-native",
          "library": "${NL_PATH}/extensions/example_extension.dll",
          "config": {
            "version": "1.0.0",
            "mode": "production"
          }
        }
      ]
    }
  }
}
```

## Testing from JavaScript

```javascript
// Get system info
const sysInfo = await Neutralino.extensions.dispatch('example-native', 'get_system_info', {});
console.log(sysInfo);

// Get timestamp
const time = await Neutralino.extensions.dispatch('example-native', 'get_timestamp', {});
console.log(time);

// Echo test
const echo = await Neutralino.extensions.dispatch('example-native', 'echo', {message: 'Hello!'});
console.log(echo);

// Get config
const config = await Neutralino.extensions.dispatch('example-native', 'get_config', {});
console.log(config);
```

## Expected Output

```json
// get_system_info
{
  "success": true,
  "data": {
    "platform": "Windows",
    "type": "Native DLL"
  }
}

// get_timestamp
{
  "success": true,
  "timestamp": "2026-01-01 14:30:25",
  "unix": 1767266425
}

// echo
{
  "success": true,
  "echo": {"message": "Hello!"},
  "extensionId": "example-native"
}
```
