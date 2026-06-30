# Multi-Client Chat Server

A multi-client TCP chat server built in **C** using **POSIX sockets** and **POSIX threads (`pthreads`)**.

Clients connect to the server, choose a username, and communicate with each other in real time. Each client is handled by a dedicated thread, and messages are broadcast to every connected client except the sender.

---

## Features

* Multi-client support using POSIX threads
* TCP/IP socket programming
* Username handshake on connection
* Real-time broadcast messaging
* Join and leave notifications
* Thread-safe shared resources using mutexes
* Configurable maximum client limit
* Graceful client disconnection handling
* Modular project structure

---

## Technologies Used

* C
* POSIX Sockets
* POSIX Threads (`pthread`)
* Linux System Programming
* GCC
* Make

---

## Project Structure

```text
multi-client-chat-server/
│
├── src/
│   ├── server.c
│   ├── client.c
│   └── chat.h
│
├── docs/
│   ├── architecture.md
│   └── learning-notes.md
│
├── screenshots/
│
├── Makefile
└── README.md
```

---

## How It Works

1. The server starts and listens on port **8080**.
2. A client connects to the server.
3. The client sends a username.
4. The server creates a dedicated thread for that client.
5. When a client sends a message:

   * The server receives it.
   * The sender's username is prepended.
   * The message is broadcast to all other connected clients.
6. Join and leave notifications are automatically broadcast to all users.

---

## Architecture

```text
                    +----------------------+
                    |      Server          |
                    |----------------------|
                    |  Listening Socket    |
                    +----------+-----------+
                               |
          +--------------------+--------------------+
          |                    |                    |
          |                    |                    |
   +-------+-------+    +-------+-------+    +-------+-------+
   | Client Thread |    | Client Thread |    | Client Thread |
   +-------+-------+    +-------+-------+    +-------+-------+
           |                    |                    |
        Client A             Client B             Client C
```

Each connected client is handled by its own thread, allowing multiple users to communicate simultaneously.

---

## Building

Compile the project using the provided Makefile.

```bash
make
```

---

## Running

Start the server:

```bash
./server
```

In separate terminals, start one or more clients:

```bash
./client
```

---

## Example

```text
Enter your username: Alice
Welcome to the chatroom, Alice!

Bob joined the chat.

you: Hello everyone!

Bob: Hi Alice!

Charlie joined the chat.

Charlie: Nice to meet you all!

Bob left the chat.
```

---

## Concepts Demonstrated

* TCP Client-Server Architecture
* Socket Programming
* Concurrent Programming
* POSIX Threads
* Mutex Synchronization
* Dynamic Memory Management
* Thread-safe Shared Resources
* Linux Network Programming

---

## Future Improvements

* Private messaging (`/msg`)
* Chat rooms / channels
* User list command (`/list`)
* Server-side commands
* Message timestamps
* Colored terminal output
* Persistent chat logs
* Configuration file support
* IPv6 support
* TLS/SSL encryption
* Replace thread-per-client model with `select()`, `poll()`, or `epoll()` for better scalability

---

## Learning Objectives

This project was built to gain practical experience with:

* Linux systems programming
* Network programming in C
* POSIX socket APIs
* Multithreading using `pthread`
* Synchronization using mutexes
* Building concurrent server applications

---

## License

This project is intended for learning and educational purposes.


