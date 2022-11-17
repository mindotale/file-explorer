#ifndef H_BUFFER
#define H_BUFFER

#define BUFFER_SIZE 2048

#include <string>

class Buffer
{
public:
    Buffer();
    Buffer(char* buf, int len);
    Buffer(const Buffer &copy);
    ~Buffer();

    int getint();
    void putint(int num);

    char getchar();
    void putchar(char num);

    std::string getstring();
    void putstring(std::string str);

    void clear();
    void reset_offset();

    int size();
    int max_size();

    explicit operator char*() { return buffer; }
    explicit operator void*() { return buffer; }

    Buffer operator+  (Buffer &buf);
    void   operator+= (Buffer &buf);

private:
    char buffer[BUFFER_SIZE];
    int buf_size;
    int offset;
};

#endif // H_BUFFER
