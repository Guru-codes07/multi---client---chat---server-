  // chat.c - Multi-Client Chat Server Implementation
  // Implements the client registry and messaging functions declared
  // Author: Guru Prasad Mishra
 #include "chat.h"

// global variables 

client_t *clients[MAX_CLIENTS] = {0};
int client_count = 0;
int uid_counter = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// client manager

int add_client(client_t *client)
{
    int result;

    pthread_mutex_lock(&clients_mutex);

    if (client->name[0] == '\0') {
        pthread_mutex_unlock(&clients_mutex);
        return -2;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && strcmp(clients[i]->name, client->name) == 0) {
            pthread_mutex_unlock(&clients_mutex);
            return -3;
        }
    }

    result = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            clients[i] = client;
            client_count++;
            result = 0;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    return result;
}

void remove_client(int uid)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->uid == uid) {
            free(clients[i]);
            clients[i] = NULL;
            client_count--;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// messaging :

void broadcast_message(const char *message, int sender_uid)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->uid != sender_uid) {
            if (send(clients[i]->socket_fd, message, strlen(message), 0) < 0) {
                perror("broadcast_message: send failed");
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_message_to_all(const char *message)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            if (send(clients[i]->socket_fd, message, strlen(message), 0) < 0) {
                perror("send_message_to_all: send failed");
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

int send_private_message(const char *message, const char *target_name, client_t *sender)
{
    int result = -1;
    char formatted[MSG_SIZE];

    snprintf(formatted, sizeof(formatted), "[whisper from %s]: %s",
              sender->name, message);

    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && strcmp(clients[i]->name, target_name) == 0) {
            if (send(clients[i]->socket_fd, formatted, strlen(formatted), 0) < 0) {
                perror("send_private_message: send failed");
            } else {
                result = 0;
            }
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    return result;
}

// Framing :

int recv_message(int sockfd, char *acc_buf, size_t *acc_len, char *out_msg, size_t out_size)
{
    while (1) {
        /* a full line may already be sitting in the accumulator from a
         * previous recv() that returned more than one message's worth */
        char *newline = memchr(acc_buf, '\n', *acc_len);

        if (newline != NULL) {
            size_t msg_len = (size_t)(newline - acc_buf) + 1; /* include the '\n' */
            size_t copy_len = (msg_len < out_size - 1) ? msg_len : out_size - 1;

            memcpy(out_msg, acc_buf, copy_len);
            out_msg[copy_len] = '\0';

            /* shift any leftover bytes (start of the next line) to the front */
            size_t remaining = *acc_len - msg_len;
            memmove(acc_buf, acc_buf + msg_len, remaining);
            *acc_len = remaining;

            return 1;
        }

        /* no newline yet: if the accumulator is full, force a flush so a
         * pathological unterminated line can't stall the connection forever */
        if (*acc_len >= BUFFER_SIZE - 1) {
            size_t copy_len = (*acc_len < out_size - 1) ? *acc_len : out_size - 1;

            memcpy(out_msg, acc_buf, copy_len);
            out_msg[copy_len] = '\0';
            *acc_len = 0;

            return 1;
        }

        ssize_t n = recv(sockfd, acc_buf + *acc_len, BUFFER_SIZE - 1 - *acc_len, 0);
        if (n == 0) {
            return 0;  /* orderly disconnect */
        }
        if (n < 0) {
            return -1; /* recv() error */
        }

        *acc_len += (size_t)n;
        /* loop back around and check for a newline in the newly-appended bytes */
    }
}

// Utility:

void strip_newline(char *str)
{
    str[strcspn(str, "\r\n")] = '\0';
}

void format_message(char *dest, const char *name, const char *message)
{
    snprintf(dest, MSG_SIZE, "%s: %s", name, message);
}

void list_online_users(char *dest)
{
    char line[NAME_SIZE + 4];
    dest[0] = '\0';

    pthread_mutex_lock(&clients_mutex);

    strncat(dest, "Online users:\n", BUFFER_SIZE - strlen(dest) - 1);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            snprintf(line, sizeof(line), "- %s\n", clients[i]->name);
            strncat(dest, line, BUFFER_SIZE - strlen(dest) - 1);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

int get_client_count(void)
{
    int count;

    pthread_mutex_lock(&clients_mutex);
    count = client_count;
    pthread_mutex_unlock(&clients_mutex);

    return count;
}
