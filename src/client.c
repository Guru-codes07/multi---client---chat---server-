#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Thread function dedicated entirely to listening to the server
void* receive_messages(void* arg) {
    int client_socket = (int)(long)arg;
    char buffer[BUFFER_SIZE];
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int byte_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (byte_received <= 0)
        {
            printf("\nServer disconnected.\n");
            exit(0); 
        }
        buffer[byte_received] = '\0';
        
        // Print whatever background message came in (notifications or text from others)
        printf("\r%s", buffer);
        printf("you: "); 
        fflush(stdout);
    }
    return NULL;
}

int main() {
    char buffer[BUFFER_SIZE];
    char username[50];
    int client_socket;

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
     {
        perror("invalid error");
         return 1;
    }
    
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) 
    {    
        perror("socket failed");
         return 2;
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
     {
        perror("connection failed");
        close(client_socket); 
         return 3;
    }
    
    // User Name handshake:
    while (1) {
        printf("Enter your username: ");
        if (fgets(username, sizeof(username), stdin) == NULL)
        {
            perror("error");
            return 0;
        }

        username[strcspn(username, "\n")] = '\0';

        char username_line[64];
        snprintf(username_line, sizeof(username_line), "%s\n", username);
        send(client_socket, username_line, strlen(username_line), 0);

        // Wait for the server to actually confirm or reject this username,
        // rather than assuming success locally. The server sends "OK\n" on
        // acceptance, or a human-readable error (e.g. "already taken") that
        // we just show the user before looping back to ask again.
        char handshake_reply[BUFFER_SIZE];
        int reply_len = recv(client_socket, handshake_reply, sizeof(handshake_reply) - 1, 0);
        if (reply_len <= 0) {
            printf("Server closed the connection during handshake.\n");
            close(client_socket);
            return 1;
        }
        handshake_reply[reply_len] = '\0';

        if (strncmp(handshake_reply, "OK", 2) == 0) {
            break; 
        }

        // Anything else is a rejection reason from the server; show it and retry
        printf("%s", handshake_reply);
    }

    printf("Welcome to the chatroom, %s!\n", username);
    printf("Type 'exit' to quit.\n");
    printf("Type '/who' to list online users, or '/w <username> <message>' to whisper.\n\n");

    // Spawn a background thread to handle incoming text asynchronously 
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, (void*)(long)client_socket);
    pthread_detach(recv_thread);

    // main loop
    while (1) {
        printf("you: ");
        fflush(stdout);
        
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;

        char trimmed_input[BUFFER_SIZE];
        strncpy(trimmed_input, buffer, sizeof(trimmed_input) - 1);
        trimmed_input[sizeof(trimmed_input) - 1] = '\0';
        trimmed_input[strcspn(trimmed_input, "\n")] = '\0';
        if (strcmp(trimmed_input, "exit") == 0) break;

        if (send(client_socket, buffer, strlen(buffer), 0) < 0) 
        {
            perror("send failed"); 
            break;
        }
    }

    close(client_socket);
    return 0;
}
