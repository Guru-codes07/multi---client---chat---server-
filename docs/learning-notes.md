# Learning Notes

## Project Goal

The objective of this project was to learn how a multi-client TCP server works internally using C, POSIX sockets, and POSIX threads.

Before building this project, I understood basic client-server communication using sockets. The next step was learning how to support multiple clients simultaneously.

---

# Concepts Learned

## 1. TCP Socket Programming

A TCP server requires the following sequence of operations:

```text id="pw1n1u"
socket()
   |
bind()
   |
listen()
   |
accept()
```

### socket()

Creates an endpoint for communication.

```c id="m3mw1l"
socket(AF_INET, SOCK_STREAM, 0);
```

* `AF_INET` → IPv4
* `SOCK_STREAM` → TCP Protocol

---

### bind()

Associates the socket with an IP address and port.

```c id="gvtksq"
bind(server_socket,
    (struct sockaddr*)&server_addr,
    sizeof(server_addr));
```

---

### listen()

Places the socket into listening mode.

```c id="m4lvtz"
listen(server_socket, MAX_CLIENTS);
```

The server waits for incoming connections.

---

### accept()

Accepts a client connection.

```c id="qk78yn"
accept(server_socket,
      (struct sockaddr*)&client_addr,
      &client_len);
```

A new socket descriptor is returned for communicating with the client.

---

# 2. Multi-Threading

A single-threaded server can only handle one client at a time.

To support multiple clients, a separate thread is created for each connection.

```c id="v4ff5z"
pthread_create(
    &thread,
    NULL,
    handle_client,
    client_socket
);
```

This allows multiple clients to communicate with the server concurrently.

---

# 3. Thread Detachment

Initially, I learned that every thread consumes system resources.

Using:

```c id="m4ydr8"
pthread_detach(thread);
```

allows threads to clean up automatically when they terminate.

Without detaching or joining threads, resources can leak.

---

# 4. Mutexes and Race Conditions

The server maintains a shared variable:

```c id="w26f3w"
client_count
```

Multiple threads can access this variable simultaneously.

This creates a race condition.

Example:

```text id="3h8srg"
Thread A reads client_count = 2
Thread B reads client_count = 2

Thread A increments -> 3
Thread B increments -> 3

Expected value = 4
Actual value = 3
```

To prevent this issue:

```c id="bgjlwm"
pthread_mutex_lock(&lock);

client_count++;

pthread_mutex_unlock(&lock);
```

Only one thread can modify the variable at a time.

---

# 5. Critical Sections

A critical section is a region of code where shared data is accessed.

Example:

```c id="n24j7l"
pthread_mutex_lock(&lock);

client_count++;

pthread_mutex_unlock(&lock);
```

The mutex protects the critical section.

---

# 6. Dynamic Memory Allocation

A common mistake when creating threads is passing the address of a local variable.

Bad example:

```c id="m3mjlwm"
int client_socket;

pthread_create(
    &thread,
    NULL,
    handle_client,
    &client_socket
);
```

All threads may end up using the same memory location.

Instead:

```c id="ztxwl1"
int* client_socket =
    malloc(sizeof(int));
```

Each thread receives its own copy.

Inside the thread:

```c id="lwgrx7"
free(arg);
```

to avoid memory leaks.

---

# 7. Client Limits

The server supports:

```c id="xvq0b5"
#define MAX_CLIENTS 5
```

If the limit is exceeded:

```text id="ybzn0u"
Client 6
   |
   v
Connection Rejected
```

This prevents excessive resource usage.

---

# 8. Echo Server Architecture

The current implementation behaves as an echo server.

Flow:

```text id="qlfyu0"
Client
   |
   | Message
   v
Server
   |
   | Same Message
   v
Client
```

Example:

```text id="ihp19w"
Client: Hello

Server: Hello
```

---

# 9. Error Handling

Throughout the project I learned the importance of checking return values.

Example:

```c id="djlwmn"
if(socket(...) < 0)
{
    perror("socket error");
}
```

Similar checks are required for:

* socket()
* bind()
* listen()
* accept()
* recv()
* send()
* pthread_create()

---

# Challenges Faced

## Understanding Thread Arguments

One of the most confusing topics was passing the client socket descriptor to the thread safely.

The solution was:

```c id="f8afjm"
int* client_socket =
    malloc(sizeof(int));
```

instead of sharing a single variable.

---

## Understanding Mutexes

At first, mutexes seemed complicated.

After implementing:

```c id="gkxhkt"
pthread_mutex_lock()

pthread_mutex_unlock()
```

it became clear that they are simply a mechanism to ensure only one thread accesses shared data at a time.

---

## Understanding Detached Threads

I learned the difference between:

```text id="13j2ny"
pthread_join()
```

and

```text id="fuwx9z"
pthread_detach()
```

Detached threads automatically release resources when they terminate.

---

# Future Improvements

The current server is a multi-client echo server.

Planned upgrades:

* Broadcast messaging
* Usernames
* Private messaging
* Chat rooms
* Logging
* Graceful shutdown
* Configuration files
* Better client management

---

# Key Takeaways

After completing this project, I gained practical experience with:

* TCP/IP networking
* Socket programming
* POSIX threads
* Mutex synchronization
* Dynamic memory management
* Concurrent programming
* Linux systems programming
* Multi-client server architecture

This project served as a strong introduction to building real-world networked applications in C.

