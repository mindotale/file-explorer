#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    int server_socket_fd, client_socket_fd, bytes_read;
    struct sockaddr_in server_addr;

    char buffer[BUFFER_SIZE] = {0};
    char *hello_msg = "Hello from client";

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    client_socket_fd = connect(
            server_socket_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr));
    if (client_socket_fd < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    send(server_socket_fd, hello_msg, strlen(hello_msg), 0);
    printf("Hello message sent\n");
    bytes_read = recv(server_socket_fd, buffer, BUFFER_SIZE, 0);
    printf("%s\n", buffer);

    // Closing the connected socket
    close(client_socket_fd);
    return 0;
}