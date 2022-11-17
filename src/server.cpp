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
#define MAX_PATH_LEN 256

#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec

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
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port PORT
    if (setsockopt(
            server_socket_fd, SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)))
    {
        perror("Setsockopt failed");
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
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_fd, QUEUE_SIZE) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        // Waiting for new client connections
        printf("Waiting for connections...\n");
        client_socket_fd = accept(
            server_socket_fd,
            (struct sockaddr *)&server_addr,
            (socklen_t *)&server_addr_len);
        if (client_socket_fd < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted.\n");

        // Creating a request handler for the accepted client
        pthread_t rh_tid;
        if (pthread_create(
                &rh_tid,
                NULL,
                &request_handler,
                &client_socket_fd) != 0)
        {
            perror("Pthread_create failed");
            exit(EXIT_FAILURE);
        }
        printf("Request handler created.\n");
    }

    // Closing the server socket
    close(server_socket_fd);

    printf("Server ended.\n");
    return 0;
}

void *request_handler(void *param)
{
    int client_socket_fd, bytes_read, bytes_sent;
    Buffer request, response;
    std::string relative_path;
    char path[MAX_PATH_LEN] = {0};
    DIR *dir;
    struct dirent *ent;

    client_socket_fd = *((int *)param);

    while (true)
    {
        bytes_read = recv(client_socket_fd, (void *)request, request.max_size(), 0);

        if (bytes_read < 0)
        {
            perror("Request error");
            break;
        }

        printf("Request recieved (%d byte(s)).\n", bytes_read);

        int type = request.getint();

        switch (type)
        {
        case NetworkChangeDir:
        {
            printf("Handling the NetworkChangeDir request...\n");

            response.putint(NetworkChangeDir);

            relative_path = request.getstring();
            if (chdir(relative_path.c_str()) == 0)
            {
                response.putint(0);
            }
            else
            {
                perror("Chdir failed");
                response.putint(1);
            }

            getcwd(path, MAX_PATH_LEN);
            response.putstring(path);

            break;
        }
        case NetworkListFiles:
        {
            printf("Handling the NetworkListFiles request...\n");

            int ent_count = 0;
            struct stat ent_stat;
            Buffer temp;

            response.putint(NetworkListFiles);

            if ((dir = opendir(".")) != NULL)
            {
                // List all the files and directories within directory
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
                        temp.putint(ent_stat.st_mtime);
                    }
                }
                closedir(dir);

                response.putint(ent_count);
                response += temp;
            }
            else
            {
                perror("Opendir failed");
            }

            break;
        }
        default:
        {
            printf("Invalid request type: %d\n", type);
            break;
        }
        }

        printf("Request handled.\n");

        printf("Sending the response...\n");
        bytes_sent = send(client_socket_fd, (void *)response, response.size(), MSG_NOSIGNAL);
        
        if (bytes_sent <= 0)
        {
            perror("Response error");
            break;
        }
        else
        {
            printf("Response sent (%d byte(s)).\n", bytes_sent);
        }

        request.clear();
        response.clear();
    }

    // Closing the client socket
    close(client_socket_fd);
    printf("Connection closed.\n");
    pthread_exit(NULL);
}
