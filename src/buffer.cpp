#include "buffer.h"

#include <iostream>

Buffer::Buffer()
{
    buf_size = 0;
    offset = 0;
}

Buffer::Buffer(char* buf, int len)
{
    for(int i = 0; i < len; ++i)
    {
        buffer[i] = buf[i];
    }

    buf_size = len;
    offset = 0;
}

Buffer::Buffer(const Buffer &copy)
{
    buf_size = copy.buf_size;
    offset = copy.offset;

    for(int i = 0; i < buf_size; ++i)
    {
        buffer[i] = copy.buffer[i];
    }
}

Buffer::~Buffer()
{

}

int Buffer::getint()
{
    int num = 0;
    num = *((int*)&buffer[offset]);

    offset += sizeof(int);

    return num;
}

void Buffer::putint(int num)
{
    buffer[buf_size]   = (num>>0) &0x000000FF;
    buffer[buf_size+1] = (num>>8) &0x000000FF;
    buffer[buf_size+2] = (num>>16)&0x000000FF;
    buffer[buf_size+3] = (num>>24)&0x000000FF;

    buf_size += sizeof(int);
}

char Buffer::getchar()
{
    char num = 0;
    num = *((char*)&buffer[offset]);

    offset += sizeof(char);

    return num;
}

void Buffer::putchar(char num)
{
    buffer[buf_size] = num;

    buf_size += sizeof(char);
}

void Buffer::putstring(std::string str)
{
    int i = 0;

    while(str[i] != '\0')
    {
        buffer[buf_size] = str[i];
        buf_size++;
        i++;
    }

    buffer[buf_size] = '\0';
    buf_size++;
}

std::string Buffer::getstring()
{
    std::string str;

    while(buffer[offset] != '\0')
    {
        str += buffer[offset];
        offset++;
    }

    offset++;

    return str;
}

void Buffer::clear()
{
    buf_size = 0;
    offset = 0;
}

void Buffer::reset_offset()
{
    offset = 0;
}

int Buffer::size()
{
    return buf_size;
}

int Buffer::max_size()
{
    return BUFFER_SIZE;
}

Buffer Buffer::operator+(Buffer &buf)
{
    Buffer result;

    this->reset_offset();
    buf.reset_offset();

    for(int i = 0; i < this->size(); ++i)
    {
        result.putchar(this->getchar());
    }

    for(int i = 0; i < buf.size(); ++i)
    {
        result.putchar(buf.getchar());
    }

    this->reset_offset();
    buf.reset_offset();

    return result;
}

void Buffer::operator+=(Buffer &buf)
{
    buf.reset_offset();

    for(int i = 0; i < buf.size(); ++i)
    {
        this->putchar(buf.getchar());
    }

    buf.reset_offset();
}
