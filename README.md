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

### Command Line Options
- `-t`: Run in test mode (generate test traces)
- `-p port`: Specify port number (default: 8080)

### Example

#### Normal Mode
To start the server in normal mode on the default port (8080):
```bash
./traceserver
```

#### Test Mode
To start the server in test mode on port 9000:
```bash
./traceserver -t -p 9000
```

## Requirements

- Android API level 21 or higher
- Android NDK
- Internet permission in your Android app's manifest 