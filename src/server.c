#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

// Global variables
int client_count = 0;
int client_sockets[MAX_CLIENTS] = {0};

// mutex declaration to avoid race conditions
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// function to send messages to everyone except the sender who sent it
void broadcast_message(const char* message, int sender_socket)
{
  // critical section begins 
  pthread_mutex_lock(&lock);
  for(int i = 0; i<MAX_CLIENTS;i++)
   {
     if(client_sockets[i]!=0 && client_sockets[i] != sender_socket) 
     {
      send(client_sockets[i],message,strlen(message),0);
     }
    }
   pthread_mutex_unlock(&lock);
   // critical section ends
}

// function which will be used by the thread 
void* handle_client(void* arg)
{
    int client_socket = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    char username[50] = {0};              // takes the username of the client
    char notification[BUFFER_SIZE + 100]; // notification when the client connects or disconnects
    
    int bytes_received = recv(client_socket,username,sizeof(username)-1,0);
    if(bytes_received<=0)
    {
      close(client_socket);
      return NULL;
    }
    username[strcspn(username,"\n")] = '\0';
    
    // broadcasting to everyone that the user joined
    snprintf(notification,sizeof(notification),"📢 %s joined the chat!\n",username);
    printf("%s",notification);
    broadcast_message(notification,client_socket);
    
    // main communication loop 
    while(1)
    {
      memset(buffer,0,BUFFER_SIZE);
      bytes_received = recv(client_socket,buffer,BUFFER_SIZE - 1,0);
      if(bytes_received==0)
      {
           printf("client disconnected\n");
           break;
      }
      else if(bytes_received<0)
      {
        perror("recv() error");
        break;
      }

      snprintf(notification,sizeof(notification),"%s: %s",username ,buffer);
      printf("%s",notification);

      broadcast_message(notification,client_socket);
    }

    close(client_socket);
    // critical section begins
    pthread_mutex_lock(&lock);
    for(int i = 0;i<MAX_CLIENTS;i++)
    {
      if(client_sockets[i]==client_socket)
      {
        client_sockets[i] = 0;
        break;
      }
    }
    client_count--;
    printf("active clients: %d\n",client_count);
    pthread_mutex_unlock(&lock);
    // critical section ends

    // broadcasting that they left
    snprintf(notification,sizeof(notification),"❌ %s left the chat.\n",username);
    printf("%s",notification);
    broadcast_message(notification,0);

    return NULL;
}

int main()
{
  // initialise the server socket
  int server_socket;
  
  // configuring the server address
  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  server_socket = socket(AF_INET,SOCK_STREAM,0);
  if(server_socket<0)
    {
      perror("socket failed");
      return 1;
    }

    int opt = 1;
    if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0)
    {
      perror("setsockopt failed");
      return 2;
    }
    
    // binding the serevr 
    if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
      perror("binding failed");
      return 3;
    }

    // listening to the port
    if(listen(server_socket,MAX_CLIENTS)<0)
    {
      perror("listen failed");
      return 4;
    }
    printf("server is listening on port %d....\n",PORT);

    // main loop and thread creation
    while(1)
    {
      // configuring client address
      struct sockaddr_in client_addr = {0};
      socklen_t client_len = sizeof(client_addr);

      int* client_socket = malloc(sizeof(int));
      if(client_socket == NULL)
      {
        perror("malloc error");
        continue;
      }

      // accepting connection
      *client_socket = accept(server_socket,(struct sockaddr*)&client_addr,&client_len);
      if(*client_socket<0)
      {
        perror("accept error");
        free(client_socket);
        continue;
      }
        
      // critical section begins
      pthread_mutex_lock(&lock);
      if(client_count>=MAX_CLIENTS)
      {
        pthread_mutex_unlock(&lock);
        char* msg ="server full, try again later.\n";
        send(*client_socket,msg,strlen(msg),0);
        close(*client_socket);
        free(client_socket);
        continue;
      }
        
      for(int i=0;i<MAX_CLIENTS;i++)
      {
        if(client_sockets[i]==0)
        {
          // FIXED: Changed client_socket to client_sockets (Added missing 's')
          client_sockets[i] = *client_socket; 
          break;
        }
      }
        client_count++;
        printf("active clients: %d\n",client_count);
        pthread_mutex_unlock(&lock);
        //critical section ends

        //thread variable creation
        pthread_t thread;

        // thread creation and attributes declaration 
        if(pthread_create(&thread,NULL,&handle_client,client_socket)!=0)
        {
          perror("thread failed");
          close(*client_socket);
          free(client_socket);
          continue;
        }
       
        pthread_detach(thread);
       }

       // mutex destruction 
       pthread_mutex_destroy(&lock);
       close(server_socket);

       return 0;
}





