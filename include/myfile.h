#ifndef MYFILE_H
#define MYFILE_H

#include "common.h"
#include "mystring.h"

class myfile;

typedef	unsigned long (myfile::*DO_READ_FUNC)(unsigned char *buffer, unsigned long bytes);
typedef	unsigned long (myfile::*DO_READBUFFER_FUNC)(unsigned char *start, unsigned char **buffer, unsigned int size, unsigned long bytes);
typedef	unsigned long (myfile::*DO_READLINE_FUNC)(unsigned char *buffer, unsigned long bytes);
typedef	long long (myfile::*DO_SEEK_FUNC)(long long position, unsigned long method);
typedef	unsigned long (myfile::*DO_CURPOSITION_FUNC)();
typedef	long long (myfile::*DO_GETFILESIZE_FUNC)();
// (this->*synth_1to1_mono_func)
class myfile
{
public:
    myfile()
    {
        counter = 0;
        fsize = -1;
        is_read = true;
        file = INVALID_HANDLE_VALUE;
        fCapacity = 0;
        fMemory = NULL;
        is_memory = false;
    };
    ~myfile()
    {
        close();
    };

    bool open(mystring *pfile, bool pread, bool puse_seq_scan = false);
    void open_memory(long long psize_capacity);

    void close();

    unsigned long read(unsigned char *buffer, unsigned long bytes);
    unsigned long readbuffer(unsigned char *start, unsigned char **buffer, unsigned int size, unsigned long bytes);
    unsigned long readline(unsigned char *buffer, unsigned long bytes);
    long long seek(long long position, unsigned long method);
    unsigned long curposition();
    long long getsize();

    bool eof();
    bool is_read;

    void *fMemory;
    bool is_memory;

    mystring path;

private:

    HANDLE file;
    long long counter;
    long long fsize;

    long long fCapacity;

    DO_READ_FUNC do_read;
    DO_READBUFFER_FUNC do_readbuffer;
    DO_READLINE_FUNC do_readline;
    DO_SEEK_FUNC do_seek;
    DO_CURPOSITION_FUNC do_curposition;
    DO_GETFILESIZE_FUNC do_getsize;

    unsigned long file_read(unsigned char *buffer, unsigned long bytes);
    unsigned long file_readbuffer(unsigned char *start, unsigned char **buffer, unsigned int size, unsigned long bytes);
    unsigned long file_readline(unsigned char *buffer, unsigned long bytes);
    long long file_seek(long long position, unsigned long method);
    unsigned long file_curposition();
    long long file_getsize();

    void memory_set_size(long long NewSize);

    unsigned long memory_read(unsigned char *buffer, unsigned long bytes);
    unsigned long memory_readbuffer(unsigned char *start, unsigned char **buffer, unsigned int size, unsigned long bytes);
    unsigned long memory_readline(unsigned char *buffer, unsigned long bytes);
    long long memory_seek(long long position, unsigned long method);
    unsigned long memory_curposition();
    long long memory_getsize();

};

#endif // MYFILE_H
