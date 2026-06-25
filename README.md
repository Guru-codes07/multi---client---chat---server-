# Multi-Client TCP Server in C

A multi-threaded TCP server written in C using POSIX sockets and pthreads. This project demonstrates how to handle multiple client connections concurrently while maintaining thread safety using mutexes.

## Features

* Multi-client TCP server
* Thread-per-client architecture
* POSIX Threads (`pthread`)
* Mutex-protected shared resources
* Client connection limit enforcement
* Echo server functionality
* Dynamic memory management
* Linux-compatible

## Project Structure

```text
multi-client-chat/
│
├── README.md
├── Makefile
│
├── docs/
│   ├── architecture.md
│   └── learning-notes.md
│
├── screenshots/
│   └── README.md
│
└── src/
    ├── chat.h
    ├── server.c
    └── client.c
```

## Technologies Used

* C Programming Language
* POSIX Sockets
* POSIX Threads (pthreads)
* Linux System Calls
* GCC Compiler

## How It Works

1. The server creates a TCP socket.
2. The socket is bound to a specified port.
3. The server listens for incoming client connections.
4. When a client connects, a new thread is created.
5. Each thread handles communication with one client.
6. Messages received from a client are sent back to the same client (Echo Server).
7. A mutex protects the shared `client_count` variable from race conditions.

## Building

Compile the project using GCC:

```bash
gcc src/server.c -o server -lpthread
gcc src/client.c -o client
```

Or use the Makefile:

```bash
make
```

## Running the Server

```bash
./server
```

Expected output:

```text
server is listening on port 8080.....
```

## Running the Client

Open another terminal:

```bash
./client
```

You can run multiple client instances simultaneously.

## Example

Client:

```text
Hello Server
```

Server:

```text
client[5] = Hello Server
```

Response:

```text
Hello Server
```

## Thread Safety

The server uses a mutex to protect access to the shared client counter:

```c
pthread_mutex_lock(&lock);
client_count++;
pthread_mutex_unlock(&lock);
```

This prevents race conditions when multiple threads attempt to modify the same variable.

## Current Limitations

* Echo server only
* No usernames
* No message broadcasting
* No private messaging
* No chat rooms

## Future Improvements

* Broadcast messages to all connected clients
* Username support
* Private messaging
* Chat rooms
* Logging system
* Graceful server shutdown
* Better error handling

## Learning Objectives

This project helped me learn:

* TCP/IP Networking
* Socket Programming
* Multi-threading with pthreads
* Mutexes and Synchronization
* Dynamic Memory Management
* Concurrent Programming Concepts
* Linux Systems Programming

## Author

Guru Prasad Mishra

## License

This project is open-source and available for learning and educational purposes.

