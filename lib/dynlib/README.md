# Dynamic Library Extension Support

Cross-platform dynamic library loader for Neutralinojs extensions.

## Overview

This module enables loading native extensions as dynamic libraries (.dll, .so, .dylib) at runtime, providing:
- Direct OS-level API access
- Runtime flexibility
- Memory efficient operations  
- Thread-safe library management
- Safe abstraction layer

## Platform Support

- **Windows**: LoadLibrary/FreeLibrary
- **Linux**: dlopen/dlclose
- **macOS**: dlopen/dlclose

## API

See the main [README](../../lib/dynlib/README.md) for detailed usage instructions.

## License

MIT License - Same as Neutralinojs
