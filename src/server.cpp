#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define QUEUE_SIZE 8

int main(int argc, char const* argv[])
{
    int server_socket_fd, client_socket_fd, recv_value;
    struct sockaddr_in server_addr;
    int server_addr_len = sizeof(server_addr);
    int opt = 1;

    char buffer[BUFFER_SIZE] = { 0 };
    char* hello_msg = "Hello from server";
 
    // Creating socket file descriptor
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    // Forcefully attaching socket to the port PORT
    if (setsockopt(
        server_socket_fd, SOL_SOCKET,
        SO_REUSEADDR | SO_REUSEPORT, 
        &opt,
        sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
 
    // Forcefully attaching socket to the port PORT
    if (bind(
        server_socket_fd,
        (struct sockaddr*)&server_addr,
        sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_fd, QUEUE_SIZE) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(true)
    {
        client_socket_fd = accept(
            server_socket_fd,
            (struct sockaddr*)&server_addr,
            (socklen_t*)&server_addr_len);
        if (client_socket_fd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        recv_value = recv(client_socket_fd, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);
        send(client_socket_fd, hello_msg, strlen(hello_msg), 0);
        printf("Hello message sent\n");
    
        // closing the connected socket
        close(client_socket_fd);
    }
    
    // closing the listening socket
    shutdown(server_socket_fd, SHUT_RDWR);
    return 0;
}