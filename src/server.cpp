#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "protocol.h"
#include "buffer.h"

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

    client_socket_fd = *((int*)param);

    while(true)
    {
        Buffer buffer;
        Buffer response_msg;

        bytes_read = recv(client_socket_fd, (void*)buffer, buffer.max_size(), 0);

        if(bytes_read <= 0) continue;

        printf("Message read (%d byte(s))\n", bytes_read);

        int type = buffer.getint();

        switch(type)
        {
            case NetworkChangeDir:
            {
                std::string client_dir = "/";
                std::string path = buffer.getstring();

                client_dir += path;

                response_msg.putint(NetworkChangeDir); // type
                response_msg.putint(0);                // error
                response_msg.putstring(client_dir);    // new path

                break;
            }
            case NetworkListFiles:
            {
                response_msg.putint(NetworkListFiles); // type
                response_msg.putint(5);                // number of files and dirs
               
                response_msg.putint(0);                // is dir
                response_msg.putstring("file1.txt");   // filename
                response_msg.putint(59238);            // size in bytes
                response_msg.putint(1668625530);       // timestamp

                response_msg.putint(0);                // is dir
                response_msg.putstring("file2.txt");   // filename
                response_msg.putint(118476);           // size in bytes
                response_msg.putint(1558625530);       // timestamp

                response_msg.putint(1);                // is dir
                response_msg.putstring("dir1");        // dirname

                response_msg.putint(1);                // is dir
                response_msg.putstring("dir2");        // dirname

                response_msg.putint(1);                // is dir
                response_msg.putstring("dir2");        // dirname

                break;
            }
            default:
            {
                printf("Invalid message type: %d\n", type);
                break;
            }
        }


        bytes_sent = send(client_socket_fd, (void*)response_msg, response_msg.size(), MSG_NOSIGNAL);
        if(bytes_sent <= 0)
        {
            break;
        }
    }

    // Closing the client socket
    close(client_socket_fd);
    pthread_exit(NULL);
}
