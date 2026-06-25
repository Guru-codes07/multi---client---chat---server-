# Architecture

## Overview

This project implements a multi-client TCP echo server using POSIX sockets and POSIX threads (`pthreads`).

The server listens for incoming TCP connections on port `8080`. Each connected client is assigned a dedicated worker thread, allowing multiple clients to communicate with the server concurrently.

To ensure thread safety, a mutex is used to protect shared resources such as the active client counter.

---

## High-Level Architecture

```text
                              +------------------+
                              |    TCP Server    |
                              |    Port: 8080    |
                              +------------------+
                                       |
     -----------------------------------------------------------------
     |              |              |              |                 |
     v              v              v              v                 v

+--------------+ +--------------+ +--------------+ +--------------+ +--------------+
|   Client 1   | |   Client 2   | |   Client 3   | |   Client 4   | |   Client 5   |
+--------------+ +--------------+ +--------------+ +--------------+ +--------------+
       |                |                |                |                |
       v                v                v                v                v

+--------------+ +--------------+ +--------------+ +--------------+ +--------------+
|   Thread 1   | |   Thread 2   | |   Thread 3   | |   Thread 4   | |   Thread 5   |
+--------------+ +--------------+ +--------------+ +--------------+ +--------------+
```

Each client connection is handled independently by its own thread.

This architecture allows multiple clients to communicate with the server at the same time without blocking one another.

---

## Server Startup Flow

```text
main()
  |
  +--> socket()
  |
  +--> setsockopt()
  |
  +--> bind()
  |
  +--> listen()
  |
  +--> accept()
  |
  +--> pthread_create()
```

### Startup Steps

1. Create a TCP socket using `socket()`.
2. Enable address reuse using `setsockopt()`.
3. Bind the socket to port `8080`.
4. Put the socket into listening mode.
5. Accept incoming client connections.
6. Create a worker thread for each connected client.

---

## Connection Flow

```text
Main Thread
     |
     +--> accept() --> Client 1 --> Thread 1
     |
     +--> accept() --> Client 2 --> Thread 2
     |
     +--> accept() --> Client 3 --> Thread 3
     |
     +--> accept() --> Client 4 --> Thread 4
     |
     +--> accept() --> Client 5 --> Thread 5
```

The main thread never communicates with clients directly.

Its sole responsibility is to accept new connections and create worker threads.

---

## Thread Lifecycle

When a client connects:

```text
Client Connects
       |
       v
accept()
       |
       v
pthread_create()
       |
       v
handle_client()
       |
       v
recv() / send()
       |
       v
Client Disconnects
       |
       v
close()
```

Each worker thread performs the following tasks:

* Receives messages from its client
* Sends responses back to the client
* Detects client disconnections
* Closes the client socket
* Updates the active client counter

---

## Client Communication Flow

```text
+----------+
|  Client  |
+----------+
      |
      | send()
      v
+----------+
|  Thread  |
+----------+
      |
      | recv()
      v
+----------+
|  Buffer  |
+----------+
      |
      | send()
      v
+----------+
|  Client  |
+----------+
```

The current implementation functions as an Echo Server.

Any message received from a client is immediately sent back to the same client.

---

## Thread Safety

The variable:

```c
client_count
```

is shared by multiple threads.

Without synchronization, race conditions can occur when multiple threads attempt to modify it simultaneously.

To prevent this issue, a mutex is used.

```text
Thread 1 ----\
              \
Thread 2 ------> Mutex ----> client_count
              /
Thread 3 ----/
```

Example:

```c
pthread_mutex_lock(&lock);

client_count++;

pthread_mutex_unlock(&lock);
```

Only one thread may enter the critical section at a time.

---

## Connection Management

The server restricts the number of active clients:

```c
#define MAX_CLIENTS 5
```

Connection policy:

```text
Client 1  ✓ Connected
Client 2  ✓ Connected
Client 3  ✓ Connected
Client 4  ✓ Connected
Client 5  ✓ Connected
Client 6  ✗ Rejected
```

If the maximum number of clients has been reached:

```text
New Client
    |
    v
Server Full
    |
    v
Connection Rejected
```

The server sends an informational message and closes the connection.

---

## Memory Management

A separate socket descriptor is dynamically allocated for every client connection.

```c
int* client_socket = malloc(sizeof(int));
```

The descriptor is passed to the worker thread.

Inside the thread:

```c
free(arg);
```

This ensures:

* Each thread receives its own socket descriptor
* No accidental sharing occurs
* Memory leaks are avoided

---

## Current Design

The current communication model is:

```text
Client
   |
   v
Server
   |
   v
Same Client
```

Example:

```text
Client: Hello Server

Server: Hello Server
```

This behavior classifies the application as a Multi-Client Echo Server.

---

## Future Architecture (Broadcast Chat Server)

A future enhancement is to transform the project into a real chat server.

### Planned Architecture

```text
                           +-----------+
                           |  Server   |
                           +-----------+
                            /   |   \
                           /    |    \
                          v     v     v

                    Client1 Client2 Client3
                          \    |    /
                           \   |   /
                            \  |  /
                             \ | /
                            Client4
                                |
                                v
                            Client5
```

### Message Broadcast Flow

```text
Client1
   |
   v
 Server
 / | | \
v  v v  v

Client2
Client3
Client4
Client5
```

When one client sends a message, the server will broadcast it to all other connected clients.

Future features include:

* Message broadcasting
* Usernames
* Private messaging
* Chat rooms
* Logging
* Graceful shutdown
* Improved error handling

---

## Key Concepts Used

* TCP/IP Networking
* Socket Programming
* POSIX Threads (`pthreads`)
* Mutex Synchronization
* Dynamic Memory Allocation
* Concurrent Programming
* Client-Server Architecture
* Linux Systems Programming
* Multi-threaded Server Design

---

## Summary

This project demonstrates the implementation of a thread-per-client TCP server in C.

The server accepts multiple client connections simultaneously, creates a dedicated thread for each client, uses mutexes for thread safety, and provides a foundation for future development into a complete multi-user chat application.

