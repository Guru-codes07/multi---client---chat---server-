#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#define PORT 8080
#define BUFFER_SIZE 1024
int main()
{
    char buffer[BUFFER_SIZE];
    // initialising client socket
    int client_socket;

    // configuring server address
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    // ip address as text to binary so that sockets can actually use it
    if(inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr)<=0)
    {
        perror("invalid error");
        return 1;
    }
    
    // creating socket
    client_socket = socket(AF_INET,SOCK_STREAM,0);
    if(client_socket<0)
    {
        perror("socket failed");
        return 2;
    }

    // connect with server 
    if(connect(client_socket,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)
    {
        perror("connection failed");
        close(client_socket);
        return 3;
    }
    
    printf("connected to the server.\n");
    printf("type 'exit' to quit.\n");

    while(1)
    {
        memset(buffer,0,BUFFER_SIZE);
        printf("you: ");
        if(fgets(buffer,BUFFER_SIZE,stdin)==NULL)
        {
            break;
        }
        if(strncmp(buffer,"exit",4)==0)
        {
            break;
        }

        // send messages
        if(send(client_socket,buffer,strlen(buffer),0)<0)
        {
            perror("send failed");
            break;
        }

        // clearing the garbage values again 
        memset(buffer,0,BUFFER_SIZE);
       
        // receive messages
        int byte_received;
        byte_received = recv(client_socket,buffer,BUFFER_SIZE-1,0);
        if(byte_received<=0)
        {
            printf("server disconnected.\n");
            break;
        }      
        buffer[byte_received]='\0';
        printf("server: %s",buffer);
    }

        // closing the socket 
        close(client_socket);
         
    return 0;
}
