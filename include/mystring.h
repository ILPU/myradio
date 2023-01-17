#ifndef MYSTRING_H
#define MYSTRING_H

#include "common.h"
#include "common_dop.h"

class mystring
{
public:
    mystring();

    mystring(const int value);

    mystring(const char *text);
    mystring(const wchar_t *text);

    mystring(const str_utf8_t *text);

    mystring(const string& text);
    mystring(const wstring& text);
    mystring(const mystring& text);

    virtual ~mystring();

    mystring& operator =(const mystring& rhs);
    mystring& operator +=(const mystring& rhs);
    mystring& operator +(const mystring& rhs);
    mystring& operator +=(const wchar_t& rhs);
    mystring& operator +(const wchar_t& rhs);
    const bool operator ==(const mystring& rhs);
    const bool operator !=(const mystring& rhs);
    wchar_t& operator [](unsigned i);

    const char* c_str();
    const wchar_t* w_str();
    const str_utf8_t* utf8_str();
    const wstring& wstring_str() const;

    void set_lowercase_latin();
    void set_uppercase_latin();

    void Allocate_wchar(size_t size);
    void Allocate_char(size_t size);
    void Read();
    void Free();
    wchar_t *wptr;
    char *cptr;

    const wchar_t* Format(const wchar_t* fmt, ...);

    size_t length();

    wstring str;

    bool wcharToUTF8(char *dest, const wchar_t *src, size_t max);
    bool UTF8Towchar(wchar_t *dest, const char *src, size_t max);
    bool wcharToANSI(char *dest, const wchar_t *src, size_t max);
    bool ANSITowchar(wchar_t *dest, const char *src, size_t max);

    void clear_str();

private:

    str_utf8_t *utf8;
    char *cstr;

    void init();
    void clean();
};

#endif // MYSTRING_H
