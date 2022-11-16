#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9554
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    printf("Client started.\n");

    int server_socket_fd, bytes_read, bytes_sent, op_code;
    struct sockaddr_in server_addr;
    const char *ip_str = "127.0.0.1";

    char buffer[BUFFER_SIZE] = {0};
    char *request_msg = "Request from the client.";

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        printf("Socket creation error.\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_str, &server_addr.sin_addr) <= 0)
    {
        printf("Invalid address or address is not supported.\n");
        return -1;
    }

    printf("Trying to %s...\n", ip_str);
    if (connect(
            server_socket_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        printf("Connection failed.\n");
        return -1;
    }
    printf("Connection established.\n");

    printf(
        "0 - Exit\n"
        "1 - Help\n"
        "2 - Send message\n"
        "3 - Recieve message\n");
    while (true)
    {
        printf("> ");
        scanf("%d", &op_code);

        if (op_code == 0)
        {
            break;
        }

        switch (op_code)
        {
        case 1:
            printf(
                "0 - Exit\n"
                "1 - Help\n"
                "2 - Send message\n"
                "3 - Recieve message\n");
            break;
        case 2:
            printf("Sending a message...\n");
            bytes_sent = send(server_socket_fd, request_msg, strlen(request_msg), 0);
            if (bytes_sent < 0)
            {
                printf("Error.\n");
            }
            else
            {
                printf("Message sent (%d byte(s)).\n", bytes_sent);
            }
            break;
        case 3:
            printf("Reading a message...\n");
            bytes_read = recv(server_socket_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
            if (bytes_read < 0)
            {
                printf("Error.\n");
            }
            else
            {
                printf("Message read (%d byte(s)):\n%s\n", bytes_read, buffer);
            }
            buffer[0] = '\0';
            break;
        default:
            printf("Unknown operation code.");
            break;
        }
    }

    // Closing the connected socket
    close(server_socket_fd);
    
    printf("Client endend.\n");
    return 0;
}