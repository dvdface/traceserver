# Android Trace Server

This is a native Android server that listens on port 8080 for commands to control Android system tracing.

## Building

1. Make sure you have Android NDK installed
2. Run the following command in the project directory:
```bash
ndk-build
```

The built library will be in the `libs` directory.

## Usage

The server accepts the following commands (each command should end with a newline):
- `begin` - Starts a trace section
- `end` - Ends the current trace section

You can connect to the server using any TCP client, for example:
```bash
echo "begin" | nc localhost 8080
echo "end" | nc localhost 8080
```

## Integration with Android App

To use this native server in your Android app:

1. Copy the built library to your app's `jniLibs` directory
2. Create a Java wrapper class:

```java
package com.example.traceserver;

public class TraceServer {
    static {
        System.loadLibrary("trace_server");
    }

    public native boolean startServer();
    public native void stopServer();
}
```

3. Start the server in your app:
```java
TraceServer server = new TraceServer();
server.startServer();
```

## Requirements

- Android API level 21 or higher
- Android NDK
- Internet permission in your Android app's manifest 