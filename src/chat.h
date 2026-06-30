// chat.h - Multi-Client Chat Server Header
// Author: Guru Prasad Mishra
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

// Constants

#define PORT          8080
#define BUFFER_SIZE   2048
#define MAX_CLIENTS   5
#define NAME_SIZE     32

/* Message format: "[username]: message\n" */
#define MSG_SIZE      (BUFFER_SIZE + NAME_SIZE + 4)

//Data Structures

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
    char recv_buf[BUFFER_SIZE];   // add
    size_t recv_len;              // add
} client_t;



// Global State

extern client_t *clients[MAX_CLIENTS];
extern int client_count;
extern int uid_counter;
extern pthread_mutex_t clients_mutex;

// Client Management


int add_client(client_t *client);

void remove_client(int uid);

// Messaging

void broadcast_message(const char *message, int sender_uid);

void send_message_to_all(const char *message);

int send_private_message(const char *message, const char *target_name, client_t *sender);

int recv_message(int sockfd, char *acc_buf, size_t *acc_len, char *out_msg, size_t out_size);

// Utility
void strip_newline(char *str);

void format_message(char *dest, const char *name, const char *message);

void list_online_users(char *dest);

int get_client_count(void);

#endif /* CHAT_H */
