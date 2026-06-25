/*
 * chat.h - Multi-Client Chat Server Header
 * 
 * Shared definitions, data structures, and function declarations
 * used by both the server and client for the chat application.
 *
 * Author: Guru Prasad Mishra
 */

#ifndef CHAT_H
#define CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

/* ======================== Constants ======================== */

#define PORT          8080
#define BUFFER_SIZE   2048
#define MAX_CLIENTS   5
#define NAME_SIZE     32

/* Message format: "[username]: message\n" */
#define MSG_SIZE      (BUFFER_SIZE + NAME_SIZE + 4)

/* ======================== Data Structures ======================== */

/*
 * client_t - Represents a connected client
 *
 * Members:
 *   socket_fd  - The client's socket file descriptor
 *   address    - The client's network address
 *   name       - The client's display name / username
 *   uid        - Unique ID assigned to each client
 */
typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    char name[NAME_SIZE];
    int uid;
} client_t;

/* ======================== Global State ======================== */

/*
 * These globals are defined in chat.c and used by the server.
 * The client array and count are protected by the clients_mutex.
 */
extern client_t *clients[MAX_CLIENTS];
extern int client_count;
extern int uid_counter;
extern pthread_mutex_t clients_mutex;

/* ======================== Client Management ======================== */

/*
 * add_client - Add a client to the global clients array
 * @param client: Pointer to the client_t struct to add
 *
 * Thread-safe: acquires clients_mutex internally.
 * Returns 0 on success, -1 if the server is full.
 */
int add_client(client_t *client);

/*
 * remove_client - Remove a client from the global clients array by UID
 * @param uid: The unique ID of the client to remove
 *
 * Thread-safe: acquires clients_mutex internally.
 */
void remove_client(int uid);

/* ======================== Messaging ======================== */

/*
 * broadcast_message - Send a message to all connected clients except the sender
 * @param message:    The message string to broadcast
 * @param sender_uid: The UID of the sender (excluded from broadcast)
 *
 * Thread-safe: acquires clients_mutex internally.
 */
void broadcast_message(const char *message, int sender_uid);

/*
 * send_message_to_all - Send a message to ALL connected clients (including sender)
 * @param message: The message string to send
 *
 * Thread-safe: acquires clients_mutex internally.
 * Useful for server announcements (join/leave notifications).
 */
void send_message_to_all(const char *message);

/*
 * send_private_message - Send a private message to a specific client by name
 * @param message:     The message content
 * @param target_name: The username of the recipient
 * @param sender:      Pointer to the sender's client_t struct
 *
 * Thread-safe: acquires clients_mutex internally.
 * Returns 0 on success, -1 if the target user was not found.
 */
int send_private_message(const char *message, const char *target_name, client_t *sender);

/* ======================== Utility ======================== */

/*
 * strip_newline - Remove trailing newline characters from a string
 * @param str: The string to modify in place
 */
void strip_newline(char *str);

/*
 * format_message - Format a chat message with username prefix
 * @param dest:   Destination buffer (must be at least MSG_SIZE bytes)
 * @param name:   The sender's username
 * @param message: The message content
 */
void format_message(char *dest, const char *name, const char *message);

/*
 * list_online_users - Build a string listing all online users
 * @param dest: Destination buffer (must be at least BUFFER_SIZE bytes)
 *
 * Thread-safe: acquires clients_mutex internally.
 */
void list_online_users(char *dest);

/*
 * get_client_count - Get the current number of connected clients
 *
 * Thread-safe: acquires clients_mutex internally.
 * Returns the current client count.
 */
int get_client_count(void);

#endif /* CHAT_H */
