#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 9554
#define BUFFER_SIZE 1024
#define QUEUE_SIZE 8

void* request_handler(void* param);

int main(int argc, char const *argv[])
{
    printf("Server started.\n");

    int server_socket_fd, client_socket_fd;
    struct sockaddr_in server_addr;
    int server_addr_len = sizeof(server_addr);
    int opt = 1;

    // Creating socket file descriptor
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        perror("Socket failed.");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port PORT
    if (setsockopt(
            server_socket_fd, SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)))
    {
        perror("Setsockopt failed.");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Forcefully attaching socket to the port PORT
    if (bind(
            server_socket_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        perror("Bind failed.");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_fd, QUEUE_SIZE) < 0)
    {
        perror("Listen failed.");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        printf("Waiting for connections...\n");
        client_socket_fd = accept(
            server_socket_fd,
            (struct sockaddr *)&server_addr,
            (socklen_t *)&server_addr_len);
        if (client_socket_fd < 0)
        {
            perror("Accept failed.");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted.\n");

        pthread_t tid;
		if(pthread_create(
            &tid,
            NULL, 
            &request_handler, 
            &client_socket_fd) != 0)
		{
			perror("Pthread_create failed.");
            exit(EXIT_FAILURE);
		}
        printf("Created request handler.\n");
    }

    // Closing the server socket
    close(server_socket_fd);

    printf("Server ended.\n");
    return 0;
}

void* request_handler(void* param)
{
    int  client_socket_fd, bytes_read, bytes_sent;
    char buffer[BUFFER_SIZE] = {0};
    char *response_msg = "Response from the server.";

    client_socket_fd = *((int*)param);

    while(true)
    {
        bytes_read = recv(client_socket_fd, buffer, BUFFER_SIZE, 0);
        bytes_sent = send(client_socket_fd, response_msg, strlen(response_msg), MSG_NOSIGNAL);
        if(bytes_sent <= 0)
        {
            break;
        }
    }
    
    // Closing the client socket
    close(client_socket_fd);
    pthread_exit(NULL);
}