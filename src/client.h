#ifndef H_CLIENT
#define H_CLIENT

extern int sock;
extern std::string current_path;
extern std::deque<std::string> console_buffer;

const char *IP = "127.0.0.1";

const int WIDTH = 64;
const int HEIGHT = 30;

extern void read_inputs();

extern void add_to_console_buffer(std::string str);
extern void print_bar();
extern void update_console();

extern std::vector<std::string> split_string(std::string str);

#endif // H_CLIENT