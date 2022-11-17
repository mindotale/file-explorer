#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

#include "protocol.h"
#include "buffer.h"

#define QUEUE_SIZE 8

void *request_handler(void *param);

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
        if (pthread_create(
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

void *request_handler(void *param)
{
    int client_socket_fd, bytes_read, bytes_sent;
    std::string current_dir = "/";
    DIR *dir;
    struct dirent *ent;

    client_socket_fd = *((int *)param);

    while (true)
    {
        Buffer request;
        Buffer response;
        bytes_read = recv(client_socket_fd, (void *)request, request.max_size(), 0);

        if (bytes_read < 0)
        {
            printf("Connection lost.\n");
            break;
        }

        printf("Message read (%d byte(s))\n", bytes_read);

        int type = request.getint();

        switch (type)
        {
        case NetworkChangeDir:
        {
            printf("NetworkChangeDir request.\n");
            std::string relative_path = request.getstring();

            current_dir += relative_path;

            response.putint(NetworkChangeDir); // type
            response.putint(0);                // error
            response.putstring(current_dir);   // new path

            break;
        }
        case NetworkListFiles:
        {
            printf("NetworkListFiles request.\n");

            int ent_count = 0;
            struct stat ent_stat;
            Buffer temp;

            response.putint(NetworkListFiles);

            if ((dir = opendir(current_dir.c_str())) != NULL)
            {
                printf("Opened directory '%s'.\n", current_dir.c_str());
                // list all the files and directories within directory
                while ((ent = readdir(dir)) != NULL)
                {
                    ent_count++;
                    
                    stat(ent->d_name, &ent_stat);
                    if (S_ISDIR(ent_stat.st_mode))
                    {
                        temp.putint(1);
                        temp.putstring(ent->d_name);
                    }
                    else
                    {
                        temp.putint(0);
                        temp.putstring(ent->d_name);
                        temp.putint(ent_stat.st_size);
                        temp.putint(ent_stat.st_mtim.tv_sec);
                    }
                }
                closedir(dir);

                response.putint(ent_count);
                response += temp;
            }
            else
            {
                // could not open directory
            }

            // response.putint(NetworkListFiles); // type
            // response.putint(5);                // number of files and dirs

            // response.putint(0);              // is dir
            // response.putstring("file1.txt"); // filename
            // response.putint(59238);          // size in bytes
            // response.putint(1668625530);     // timestamp

            // response.putint(0);              // is dir
            // response.putstring("file2.txt"); // filename
            // response.putint(118476);         // size in bytes
            // response.putint(1558625530);     // timestamp

            // response.putint(1);         // is dir
            // response.putstring("dir1"); // dirname

            // response.putint(1);         // is dir
            // response.putstring("dir2"); // dirname

            // response.putint(1);         // is dir
            // response.putstring("dir2"); // dirname

            break;
        }
        default:
        {
            printf("Invalid message type: %d\n", type);
            break;
        }
        }

        bytes_sent = send(client_socket_fd, (void *)response, response.size(), MSG_NOSIGNAL);
        if (bytes_sent < 0)
        {
            printf("Connection lost.\n");
            break;
        }
    }

    // Closing the client socket
    close(client_socket_fd);
    printf("Connection closed.\n");
    pthread_exit(NULL);
}
