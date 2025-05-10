# Android Trace Server

This is a native Android executable that listens on port 8080 for commands to control Android system tracing.

## Building

1. Make sure you have Android NDK installed.
2. Run the following command in the project directory:
   ```bash
   ndk-build
   ```

The built executable will be in the `libs` directory.

## Usage

The server accepts the following commands (each command should end with a newline):
- `begin` - Starts a trace section
- `end` - Ends the current trace section

You can connect to the server using any TCP client, for example:
```bash
echo "begin" | nc localhost 8080
echo "end" | nc localhost 8080
```

## Requirements

- Android API level 21 or higher
- Android NDK
- Internet permission in your Android app's manifest 