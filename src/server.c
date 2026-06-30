#include "chat.h"
static volatile sig_atomic_t shutdown_requested = 0;

static void handle_shutdown_signal(int sig)
{
    (void)sig;
    shutdown_requested = 1;
}

// mutex initializer
static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

static void log_line(const char *msg)
{
    pthread_mutex_lock(&print_mutex);
    fputs(msg, stdout);
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);
}

// function which will be used by the thread 
void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    char notification[MSG_SIZE];
    int rc;

    // username hanshake
    char raw_name[BUFFER_SIZE];
    int add_result;

    while (1) {
        rc = recv_message(client->socket_fd, client->recv_buf, &client->recv_len, raw_name, sizeof(raw_name));
        if (rc <= 0) {
            close(client->socket_fd);
            free(client);
            return NULL;
        }
        strncpy(client->name, raw_name, NAME_SIZE - 1);
        client->name[NAME_SIZE - 1] = '\0';
        strip_newline(client->name);

        add_result = add_client(client);
        if (add_result == 0) {
            // explicit success signal so the client can tell "accepted"
            // apart from "rejected, please retry" deterministically 
            send(client->socket_fd, "OK\n", 3, 0);
            break;
        }

        const char *err_msg;
        switch (add_result) {
            case -2: err_msg = "username cannot be empty, please try again.\n"; break;
            case -3: err_msg = "that username is already taken, please choose another.\n"; break;
            case -1: err_msg = "server full, try again later.\n"; break;
            default: err_msg = "could not join, please try again.\n"; break;
        }
        send(client->socket_fd, err_msg, strlen(err_msg), 0);

        if (add_result == -1) {
            // server is genuinely full
            close(client->socket_fd);
            free(client);
            return NULL;
        }
    }

    // announce to everyone that the user joined 
    snprintf(notification, sizeof(notification), "\xF0\x9F\x93\xA2 %s joined the chat!\n", client->name);
    log_line(notification);
    broadcast_message(notification, client->uid);

    // main communication loop 
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        rc = recv_message(client->socket_fd, client->recv_buf, &client->recv_len, buffer, BUFFER_SIZE);
        if (rc == 0) {
            log_line("client disconnected\n");
            break;
        } else if (rc < 0) {
            perror("recv() error");
            break;
        }
        
        char trimmed[BUFFER_SIZE];
        strncpy(trimmed, buffer, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        strip_newline(trimmed);

        if (strncmp(trimmed, "/who", 4) == 0) {
            char userlist[BUFFER_SIZE];
            list_online_users(userlist);
            send(client->socket_fd, userlist, strlen(userlist), 0);
            continue;
        }

        if (strncmp(trimmed, "/w ", 3) == 0) 
        {
            char *target = trimmed + 3;
            char *space = strchr(target, ' ');

            if (space == NULL) {
                char *usage = "usage: /w <username> <message>\n";
                send(client->socket_fd, usage, strlen(usage), 0);
                continue;
            }

            *space = '\0';
            char *pm_text = space + 1;

            if (send_private_message(pm_text, target, client) != 0) {
                char errmsg[NAME_SIZE + 32];
                snprintf(errmsg, sizeof(errmsg), "user '%s' not found.\n", target);
                send(client->socket_fd, errmsg, strlen(errmsg), 0);
            }
            continue;
        }

        format_message(notification, client->name, buffer);
        log_line(notification);

        broadcast_message(notification, client->uid);
    }

    close(client->socket_fd);

    int leaving_uid = client->uid;
    char leaving_name[NAME_SIZE];
    strncpy(leaving_name, client->name, NAME_SIZE - 1);
    leaving_name[NAME_SIZE - 1] = '\0';

    remove_client(leaving_uid);
    char count_msg[64];
    snprintf(count_msg, sizeof(count_msg), "active clients: %d\n", get_client_count());
    log_line(count_msg);

    // announce that they left
    snprintf(notification, sizeof(notification), "\xE2\x9D\x8C %s left the chat.\n", leaving_name);
    log_line(notification);
    send_message_to_all(notification);

    return NULL;
}

int main(void)
{
    int server_socket;

    // configuring the server address 
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket failed");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 2;
    }

    struct sigaction sa = {0};
    sa.sa_handler = handle_shutdown_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // binding the server 
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("binding failed");
        return 3;
    }

    // listening on the port 
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("listen failed");
        return 4;
    }
    char startup_msg[64];
    snprintf(startup_msg, sizeof(startup_msg), "server is listening on port %d....\n", PORT);
    log_line(startup_msg);

    // main loop and thread creation 
    while (!shutdown_requested) 
    {
        struct sockaddr_in client_addr = {0};
        socklen_t client_len = sizeof(client_addr);

        int incoming_fd = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (incoming_fd < 0) {
            if (errno == EINTR) 
            {
             continue;
            }
            perror("accept error");
            continue;
        }

        if (get_client_count() >= MAX_CLIENTS) {
            char *msg = "server full, try again later.\n";
            send(incoming_fd, msg, strlen(msg), 0);
            close(incoming_fd);
            continue;
        }

        client_t *client = malloc(sizeof(client_t));
        if (client == NULL) {
            perror("malloc error");
            close(incoming_fd);
            continue;
        }
        memset(client, 0, sizeof(client_t));

        client->socket_fd = incoming_fd;
        client->address = client_addr;

        pthread_mutex_lock(&clients_mutex);
        client->uid = uid_counter++;
        pthread_mutex_unlock(&clients_mutex);

        pthread_t thread;
        if (pthread_create(&thread, NULL, &handle_client, client) != 0) {
            perror("thread failed");
            close(client->socket_fd);
            free(client);
            continue;
        }

        pthread_detach(thread);
    }

    // graceful shutdown
    log_line("\nShutdown signal received, closing server...\n");
    send_message_to_all("\n\xF0\x9F\x93\xB4 Server is shutting down. Goodbye!\n");
    close(server_socket);

    return 0;
}





