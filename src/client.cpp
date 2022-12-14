#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string.h>
#include <vector>
#include <deque>
#include <algorithm>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "protocol.h"
#include "client.h"
#include "buffer.h"

int sock = -1;
std::string current_path = "/";
std::deque<std::string> console_buffer;

int main(int argc, char const *argv[])
{
    add_to_console_buffer("Client started.");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0)
    {
        add_to_console_buffer("\033[1;31mSocket creation error.\033[0m");
        return -1;
    }

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, IP, &server_addr.sin_addr) <= 0)
    {
        add_to_console_buffer("\033[1;31mInvalid address or address is not supported.\033[0m");
        return -1;
    }

    char msg[256];
    sprintf(msg, "Trying to connect to %s...", IP);
    add_to_console_buffer(msg);

    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        add_to_console_buffer("\033[1;31mConnection failed.\033[0m");
        return -1;
    }
    
    add_to_console_buffer("\033[1;32mConnection established.\033[0m\n");

    init_cd_request();
    read_inputs();

    // Closing the connected socket
    close(sock);
    
    add_to_console_buffer("Client ended.");
    return 0;
}

void read_inputs()
{
    int bytes_read, bytes_sent;

    char msg[256];

    while(true)
    {
        printf("\033[1;32mclient@%s\033[0m:\033[1;34m%s\033[0m$ ", IP, current_path.c_str());
        std::string command;
        
        getline(std::cin, command);

        sprintf(msg, "\033[1;32mclient@%s\033[0m:\033[1;34m%s\033[0m$ %s", IP, current_path.c_str(), command.c_str());
        add_to_console_buffer(msg);

        if(command == "")
        {
            continue;
        }
        else if(command == "quit")
        {
            break;
        }
        else if(command == "clear")
        {
            console_buffer.clear();
            update_console();
        }
        else if(command == "ls")
        {
            Buffer request;
            request.putint(NetworkListFiles); // type
            request.putstring(current_path);  // dir path

            bytes_sent = send(sock, (void*)request, request.size(), 0);

            if(bytes_sent <= 0)
            {
                add_to_console_buffer("\033[1;31mCouldn't send request to the server.\033[0m");
                continue;
            }

            Buffer response;
            bytes_read = recv(sock, (void*)response, response.max_size(), 0);

            if(bytes_read <= 0 || response.getint() != NetworkListFiles)
            {
                add_to_console_buffer("\033[1;31mReceived invalid response from the server.\033[0m");
                continue;
            }

            int n = response.getint();

            for(int i = 0; i < n; ++i)
            {
                int is_dir = response.getint();

                if(is_dir == 0)
                {
                    std::string filename = response.getstring();
                    int size = response.getint();
                    int creation_datetime = response.getint();

                    sprintf(msg, "%-32s %9s    %-30s", filename.c_str(),
                        human_readable_size(size).c_str(),
                        human_readable_datetime(creation_datetime).c_str());

                    add_to_console_buffer(msg);
                }
                else
                {
                    std::string dirname = response.getstring();
                    sprintf(msg, "\033[1;34m%-32s\033[0m", dirname.c_str());
                    add_to_console_buffer(msg);
                }
            }
        }
        else
        {
            std::vector<std::string> splits = split_string(command);

            // If split_string doesn't return anything splits[0] is the whole command
            if(splits.size() < 1)
            {
                splits.push_back(command);
            }

            if(splits[0] == "cd")
            {
                if(splits.size() == 1)
                {
                    // If splits only has "cd" and no path, set path to the currenty directory
                    splits.push_back(".");
                }

                Buffer request;
                request.putint(NetworkChangeDir); // type
                request.putstring(splits[1]);     // relative path

                bytes_sent = send(sock, (void*)request, request.size(), 0);

                if(bytes_sent <= 0)
                {
                    add_to_console_buffer("\033[1;31mCouldn't send request to the server.\033[0m");
                    continue;
                }

                Buffer response;
                bytes_read = recv(sock, (void*)response, response.max_size(), 0);

                if(bytes_read <= 0 || response.getint() != NetworkChangeDir)
                {
                    add_to_console_buffer("\033[1;31mReceived invalid response from the server.\033[0m");
                    continue;
                }

                int error = response.getint();

                if(!error)
                {
                    std::string new_path = response.getstring();
                    current_path = new_path;
                    update_console();
                }
                else
                {
                    sprintf(msg, "\033[1;31mCouldn't go to directory \"%s\" - No such directory\033[0m", splits[1].c_str());
                    add_to_console_buffer(msg);
                }
            }
            else
            {
                sprintf(msg, "Command \"%s\" is invalid", splits[0].c_str());
                add_to_console_buffer(msg);
            }
        }
    }
}

void add_to_console_buffer(std::string str)
{
    console_buffer.push_back(str);

    while(console_buffer.size() > HEIGHT)
    {
        console_buffer.pop_front();
    }

    update_console();
}

void print_bar()
{
    for(int i = 0; i < WIDTH; ++i)
    {
        printf("-");
    }

    printf("\n");
}

void init_cd_request()
{
    Buffer request;
    Buffer response;

    request.putint(NetworkChangeDir); // type
    request.putstring(".");           // relative path

    int bytes_sent = send(sock, (void*)request, request.size(), 0);

    if(bytes_sent <= 0) return;

    int bytes_read = recv(sock, (void*)response, response.max_size(), 0);

    if(bytes_read <= 0 || response.getint() != NetworkChangeDir) return;

    int error = response.getint();
    if(error) return;

    std::string new_path = response.getstring();
    current_path = new_path;
}

void update_console()
{
    system("clear"); // idk if that works on ubuntu

    printf("Available commands:\n"
           "cd    - Change directory\n"
           "ls    - List all contents of a directory\n"
           "clear - Clear the terminal\n"
           "quit  - Exit the program\n");

    print_bar();

    for(auto i = console_buffer.begin(); i < console_buffer.end(); ++i)
    {
        printf("%s\n", i->c_str());
    }
}

std::string human_readable_size(int size)
{
    const char* suffix[] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;
    double dblBytes = size;

    if(size > 1024)
    {
        for (i = 0; (size / 1024) > 0 && i<length-1; i++, size /= 1024)
        {
            dblBytes = size / 1024.0;
        }
    }

    char output[256];
    sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
    
    return output;
}

std::string human_readable_datetime(time_t datetime)
{
    struct tm *dt;
    char buffer[30];
    dt = localtime(&datetime);
    strftime(buffer, sizeof(buffer), "%d %b %Y %H:%M:%S", dt);
    return std::string(buffer);
}

std::vector<std::string> split_string(std::string text)
{
    char space_char = ' ';
    std::vector<std::string> words;

    std::stringstream sstream(text);
    std::string word;

    while(std::getline(sstream, word, space_char))
    {
        word.erase(std::remove_if(word.begin(), word.end(), isspace), word.end());
        words.push_back(word);
    }
    return words;
}
