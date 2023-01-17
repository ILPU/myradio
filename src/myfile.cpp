#include "myfile.h"

bool myfile::open(mystring *pfile, bool pread, bool puse_seq_scan)
{
    is_read = pread;
    file = CreateFile(pfile->w_str(), (pread?GENERIC_READ:GENERIC_WRITE), (pread?FILE_SHARE_READ:FILE_SHARE_WRITE), 0, (pread?OPEN_EXISTING:CREATE_ALWAYS), (puse_seq_scan?FILE_FLAG_SEQUENTIAL_SCAN:0), 0);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    path = *pfile;
    is_memory = false;

    do_read         = &myfile::file_read;
    do_readbuffer   = &myfile::file_readbuffer;
    do_readline     = &myfile::file_readline;
    do_seek         = &myfile::file_seek;
    do_curposition  = &myfile::file_curposition;
    do_getsize      = &myfile::file_getsize;

    return true;
}

void myfile::open_memory(long long psize_capacity)
{
    fsize       = psize_capacity;
    fCapacity   = psize_capacity;
    is_memory   = true;
    do_read         = &myfile::memory_read;
    do_readbuffer   = &myfile::memory_readbuffer;
    do_readline     = &myfile::memory_readline;
    do_seek         = &myfile::memory_seek;
    do_curposition  = &myfile::memory_curposition;
    do_getsize      = &myfile::memory_getsize;
}

void myfile::close()
{
    if(file != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file);
        file = INVALID_HANDLE_VALUE;
    }
    if(fMemory)
    {
        free(fMemory);
        fMemory = NULL;
    }

    counter = 0;
    fsize = -1;
    fCapacity = 0;
    is_read = true;

    path.clear_str();
}

unsigned long myfile::read(unsigned char *buffer, unsigned long bytes)
{
    return (this->*do_read)(buffer, bytes);
}

unsigned long myfile::readbuffer(unsigned char *start, unsigned char **buffer, unsigned int size, unsigned long bytes)
{
    return (this->*do_readbuffer)(start, buffer, size, bytes);
}

unsigned long myfile::readline(unsigned char *buffer, unsigned long bytes)
{
    return (this->*do_readline)(buffer, bytes);
}

long long myfile::seek(long long position, unsigned long method)
{
    return (this->*do_seek)(position, method);
}

unsigned long myfile::curposition()
{
    return (this->*do_curposition)();
}

bool myfile::eof()
{
    long long fs = (this->*do_getsize)();
    if(!fs)
        return true;
    return (this->*do_curposition)() >= fs;
}

long long myfile::getsize()
{
    return (this->*do_getsize)();
}

unsigned long myfile::file_read(unsigned char *buffer, unsigned long bytes)
{
    if(file == INVALID_HANDLE_VALUE)
        return -1;
    if(ReadFile(file, buffer, bytes, &bytes, 0) == FALSE)
        bytes = -1;
    if(bytes > 0)
        counter += bytes;
    return bytes;
}

unsigned long myfile::file_readbuffer(unsigned char *start, unsigned char **buffer, unsigned int size, unsigned long bytes)
{
    unsigned char *end = start + size;
    unsigned int pos, space_to_end;
    unsigned long remaining_bytes = 0;
    unsigned long cnt, ret;

    if(file == INVALID_HANDLE_VALUE)
        return -1;

    if(*buffer < start || *buffer > end)
        return -1; //invalid pointer

    pos = *buffer - start;
    space_to_end = end - *buffer;

    if(bytes > size)
        return -1; //not enough memory
    if(bytes == 0)
        return 0;

    if(bytes > space_to_end)
    {
        remaining_bytes = bytes - space_to_end;
        bytes = (DWORD)space_to_end;
    }
    ret = file_read(*buffer, bytes);
    if(ret < 0)
        return ret;
    cnt = ret;
    *buffer += ret;
    if(*buffer == end)
        *buffer = start;

    if(remaining_bytes > 0)
    {
        ret = file_read(start, remaining_bytes);
        if (ret > 0)
        {
            cnt += ret;
            *buffer = start + ret;
        }
    }

    return cnt;
}

unsigned long myfile::file_readline(unsigned char *buffer, unsigned long bytes)
{
    if(file == INVALID_HANDLE_VALUE)
        return -1;

    unsigned char *ptr = buffer;
    unsigned long cnt = 0, bytr;
    char buff[2];

    while(cnt < bytes-1)
    {
        if(ReadFile(file, buff, 1, &bytr, 0) == FALSE)
            break;
        if(buff[0] == '\0' || buff[0] == '\r' || buff[0] == '\n')
            break;
        *ptr++ = buff[0];
        cnt++;
    }
    *ptr++ = 0;
    if (cnt > 0)
        counter += cnt;
    return cnt;
}

long long myfile::file_seek(long long position, unsigned long method)
{
    if(file == INVALID_HANDLE_VALUE)
        return -1;
    long HiPtr = position >> 32;
    long long ret = SetFilePointer(file, int(position), &HiPtr, method);
    if ((ret == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
    {
        return -1;
    }
    return ret | ((long long)HiPtr << 32);
}

unsigned long myfile::file_curposition()
{
    return file_seek(0, FILE_CURRENT);
}

long long myfile::file_getsize()
{
    if(file == INVALID_HANDLE_VALUE)
        return -1;
    if(fsize == -1)
    {
        unsigned long SizeHigh;
        long long ret;
        ret = GetFileSize(file, &SizeHigh);
        fsize = ret | (long long)SizeHigh << 32;
    }
    return fsize;
}

long long myfile::memory_getsize()
{
    return fsize;
}

void myfile::memory_set_size(long long NewSize)
{
    long long NewCapacity;
    if(fCapacity < NewSize)
    {
        NewCapacity = NewSize;
        if(!fMemory)
        {
            if(NewSize)
                fMemory = malloc(NewCapacity);
        }
        else
            fMemory = realloc(fMemory, NewCapacity);
        fCapacity = NewCapacity;
    }
    else if(NewSize = 0 && fsize > 0)
    {
        if(fMemory)
        {
            free(fMemory);
            fMemory = NULL;
            fCapacity = 0;
        }
    }
    fsize = NewSize;
    if(counter > fsize)
        counter = fsize;
}

unsigned long myfile::memory_curposition()
{
    return memory_seek(0, FILE_CURRENT);
}

long long myfile::memory_seek(long long position, unsigned long method)
{
    long long NewPos;
    switch(method)
    {
    case FILE_BEGIN:
        NewPos = position;
        break;
    case FILE_CURRENT:
        NewPos = counter + position;
        break;
    default: // FILE_END
        NewPos = fsize + position;
        break;
    }
    if(NewPos > fsize)
        memory_set_size(NewPos);
    counter = NewPos;
    return NewPos;
}

unsigned long myfile::memory_read(unsigned char *buffer, unsigned long bytes)
{
    unsigned long C = bytes;
    if(C + counter > fsize)
        C = fsize - counter;
    memcpy(buffer, fMemory + counter, C);
    counter += C;
    return C;
}

unsigned long myfile::memory_readbuffer(unsigned char *start, unsigned char **buffer, unsigned int size, unsigned long bytes)
{
    unsigned char *end = start + size;
    unsigned int pos, space_to_end;
    unsigned long remaining_bytes = 0;
    unsigned long cnt, ret;

    if(*buffer < start || *buffer > end)
        return -1; //invalid pointer

    pos = *buffer - start;
    space_to_end = end - *buffer;

    if(bytes > size)
        return -1; //not enough memory
    if(bytes == 0)
        return 0;

    if(bytes > space_to_end)
    {
        remaining_bytes = bytes - space_to_end;
        bytes = (DWORD)space_to_end;
    }

    ret = memory_read(*buffer, bytes);
    if(ret < 0)
        return ret;
    cnt = ret;
    *buffer += ret;
    if (*buffer == end)
        *buffer = start;
    if(remaining_bytes > 0)
    {
        ret = memory_read(start, remaining_bytes);
        if(ret > 0)
        {
            cnt += ret;
            *buffer = start + ret;
        }
    }

    return cnt;
}

unsigned long myfile::memory_readline(unsigned char *buffer, unsigned long bytes)
{
    if(!fMemory)
        return 0;

    unsigned char *ptr = buffer;
    unsigned long cnt = 0;
    char buff[1];

    while(cnt < bytes-1)
    {
        if(memory_read((unsigned char*)buff, 1) < 0)
            break;
        if(buff[0] == '\0' || buff[0] == '\r' || buff[0] == '\n')
            break;
        *ptr++ = buff[0];
        cnt++;
    }
    *ptr++ = 0;

    return cnt;
}
