# Architecture

## Overview

This project implements a **multi-client TCP chat server** using **POSIX sockets** and **POSIX threads (`pthreads`)** on Linux.

The server listens for incoming TCP connections on port **8080**. Every connected client is assigned a dedicated thread, allowing multiple clients to communicate simultaneously.

When a client connects, it sends a username to the server. Messages sent by a client are prefixed with the sender's username and broadcast to every other connected client. The server also announces when users join or leave the chat.

Shared resources such as the client list and active client count are protected using a mutex to prevent race conditions.

---

# High-Level Architecture

```text
                         +----------------------+
                         |      Server          |
                         |----------------------|
                         |  Listening Socket    |
                         +----------+-----------+
                                    |
                +-------------------+-------------------+
                |                   |                   |
         accept()             accept()            accept()
                |                   |                   |
                ▼                   ▼                   ▼
        +---------------+   +---------------+   +---------------+
        | Client Thread |   | Client Thread |   | Client Thread |
        +-------+-------+   +-------+-------+   +-------+-------+
                |                   |                   |
                ▼                   ▼                   ▼
            Client A            Client B            Client C
```

Each client communicates independently with the server through its dedicated thread.

---

# Communication Flow

```text
Client connects
       │
       ▼
TCP Connection Established
       │
       ▼
Client sends username
       │
       ▼
Server stores username
       │
       ▼
Server creates dedicated thread
       │
       ▼
Client sends message
       │
       ▼
Server receives message
       │
       ▼
Prepends username
       │
       ▼
Broadcasts message to all other connected clients
```

---

# Server Architecture

The server consists of three primary components.

## 1. Main Thread

The main thread is responsible for:

* Creating the listening socket
* Binding to the server port
* Listening for incoming connections
* Accepting new clients
* Creating a dedicated thread for each client
* Tracking connected clients

The main thread never handles chat messages directly.

---

## 2. Client Threads

Each connected client is handled by an independent thread.

Responsibilities include:

* Receiving the client's username
* Broadcasting join notifications
* Receiving chat messages
* Broadcasting messages to other clients
* Detecting client disconnection
* Broadcasting leave notifications
* Cleaning up resources

This design allows multiple clients to communicate concurrently.

---

## 3. Broadcast Module

Whenever a client sends a message, the server forwards it to every connected client except the sender.

```text
          Client A
              │
              ▼
        "Hello everyone!"
              │
              ▼
             Server
        Broadcast Message
          /           \
         /             \
        ▼               ▼
    Client B       Client C
```

---

# Thread Model

```text
                Main Thread
                     │
          Accept Client Connection
                     │
                     ▼
          Create Client Thread
                     │
        ┌────────────┴────────────┐
        │                         │
        ▼                         ▼
 Client Thread A            Client Thread B
        │                         │
        ▼                         ▼
 recv() / send()           recv() / send()
```

Each client thread operates independently, enabling simultaneous communication between multiple users.

---

# Synchronization

The following resources are shared between all client threads:

* `client_sockets[]`
* `client_count`

To prevent race conditions, a POSIX mutex protects these shared resources.

```text
Thread A
    │
    ▼
Lock Mutex
    │
Modify Shared Data
    │
Unlock Mutex

        ▲

Thread B waits until mutex becomes available.
```

The mutex is used when:

* Adding a newly connected client
* Removing a disconnected client
* Updating the active client count
* Accessing the list of connected clients during broadcasting

---

# Message Flow

```text
Client
   │
   │ send()
   ▼
Server Thread
   │
   │ recv()
   ▼
Add Username
   │
   ▼
Broadcast
   │
   ├────────► Client 1
   ├────────► Client 2
   └────────► Client 3
```

---

# Connection Lifecycle

```text
Client Starts
      │
      ▼
Create Socket
      │
      ▼
Connect to Server
      │
      ▼
Send Username
      │
      ▼
Exchange Messages
      │
      ▼
Disconnect
      │
      ▼
Server Removes Client
      │
      ▼
Broadcast Leave Notification
```

---

# Client Architecture

Each client process consists of two execution paths.

## Main Thread

Responsible for:

* Connecting to the server
* Reading user input
* Sending chat messages

## Receiver Thread

Responsible for:

* Continuously listening for incoming messages
* Displaying chat messages
* Displaying join and leave notifications

```text
                Client Process
                     │
        ┌────────────┴────────────┐
        │                         │
        ▼                         ▼
 Main Thread              Receiver Thread
(User Input)             (Server Messages)
        │                         │
        ▼                         ▼
      send()                   recv()
```

---

# Key Design Decisions

* One thread per client simplifies concurrent communication.
* A mutex protects shared server resources.
* Usernames are exchanged immediately after connection.
* Messages are broadcast to all connected clients except the sender.
* Dynamic memory allocation ensures each client thread owns its socket descriptor.
* Detached threads eliminate the need for explicit thread joining.

---

# Current Limitations

* Uses a thread-per-client model, which is suitable for small to medium numbers of clients.
* TCP is treated as a message stream without explicit message framing.
* Chat history is not persisted.
* No authentication or encryption.
* IPv4 only.
* Terminal output may overlap when multiple users send messages simultaneously.

---

# Future Enhancements

* Private messaging between users
* Chat rooms or channels
* Message timestamps
* User list command (`/list`)
* Graceful server shutdown and improved error handling
* Event-driven I/O using `select()` or `epoll()` for better scalability



---

# Summary

This project demonstrates the implementation of a concurrent TCP chat server using POSIX sockets and POSIX threads. It showcases core Linux systems programming concepts, including socket programming, multithreading, synchronization with mutexes, dynamic memory management, and concurrent client-server communication.


