#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define QUEUE_SIZE 8

int main(int argc, char const *argv[])
{
    printf("Server started.\n");

    int server_socket_fd, client_socket_fd, bytes_read;
    struct sockaddr_in server_addr;
    int server_addr_len = sizeof(server_addr);
    int opt = 1;

    char buffer[BUFFER_SIZE] = {0};
    char *hello_msg = "Hello from server!";

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
            SO_REUSEADDR | SO_REUSEPORT,
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

        bytes_read = recv(client_socket_fd, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);
        send(client_socket_fd, hello_msg, strlen(hello_msg), 0);
        printf("Hello message sent.\n");

        // Closing the connected socket
        close(client_socket_fd);
    }

    // Closing the server socket
    close(server_socket_fd);

    printf("Server ended.\n");
    return 0;
}