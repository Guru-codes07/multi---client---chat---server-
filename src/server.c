#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#define BUFFER_SIZE 1024
#define PORT 8080
#define MAX_CLIENTS 5
int client_count=0;

// using mutex to avoid race conditions
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// function which will be used by the thread 
void* handle_client(void* arg)
{
   // initialising client socket 
   int client_socket = *(int* )arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    
   // communication loop 
    while(1)
    {
       memset(buffer,0,BUFFER_SIZE);
       int byte_received = recv(client_socket,buffer,BUFFER_SIZE,0);
       if(byte_received==0)
       {
         printf("client disconnected\n");
         break;
       }
       else if(byte_received<0)
       {
         perror("recv() error");
         break;
       }
        
        printf("client[%d]=%s\n",client_socket,buffer);
        if(send(client_socket,buffer,strlen(buffer),0)<0)
        {
         perror("send error");
         break;
        }
     }
     
           close(client_socket);
          
          // critical section
           pthread_mutex_lock(&lock);
           client_count--;
          printf("active clients: %d\n",client_count);
          pthread_mutex_unlock(&lock);
          // critical section ends
         
          return NULL;
}

   int main()
{
   // initialising server socket
   int server_socket;
   
   // configuring server address
   struct sockaddr_in server_addr={0};
   server_addr.sin_family=AF_INET;
   server_addr.sin_port=htons(PORT);
   server_addr.sin_addr.s_addr=INADDR_ANY;
   
   // socket creation
   server_socket=socket(AF_INET,SOCK_STREAM,0);
   if(server_socket<0)
   {
      perror("socket error");
      return 1;
    }
    
    // reuse port
    int opt=1;
    if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0)
    {
       perror("setsockopt failed");
       return 2;
    }   
    
    // binding the socket
    if(bind(server_socket,(struct sockaddr* )&server_addr,sizeof(server_addr))<0)
    {
     perror("bind failed");
     return 3;
    }
    
    // listening for connections
    if(listen(server_socket,MAX_CLIENTS)<0)
    {
    perror("listen failed");
    return 4;
    }
    printf("server is listening on port %d.....\n",PORT);
    
    // threads creation and accepting connections from the client 
    while(1)
    {
      struct sockaddr_in client_addr={0};
      socklen_t client_len = sizeof(client_addr);
      
      int* client_socket=malloc(sizeof(int));
      if(client_socket==NULL)
      {
      perror("malloc error");
      continue;
      }
      
      *client_socket=accept(server_socket,(struct sockaddr* )&client_addr,&client_len);
      if(*client_socket<0)
      {
        perror("accept error");
        free(client_socket);
        continue;
      }
      
      // critical section
      pthread_mutex_lock(&lock);
      if(client_count>=MAX_CLIENTS)
      {
        pthread_mutex_unlock(&lock);
        printf("max clients reached\n");
        char* msg = "server full, try again.\n";
        
        send(*client_socket,msg,strlen(msg),0);
        
        close(*client_socket);
        free(client_socket);
        continue;
      }
      client_count++;
      printf("client connected. active clients: %d\n", client_count);
      pthread_mutex_unlock(&lock);
      // critical section ends

      // thread creation 
      pthread_t thread;
      if(pthread_create(&thread,NULL,&handle_client,client_socket)!=0)
      {
      perror("thread failed");
      close(*client_socket);
      free(client_socket);
      
      pthread_mutex_lock(&lock);
      client_count--;
      pthread_mutex_unlock(&lock);
      
      continue;
     }
     
     // thread detachment
     pthread_detach(thread);
     }
    
     // destroying the mutex
     pthread_mutex_destroy(&lock);
    
     //closing the server socket 
     close(server_socket);
     return 0;
}
