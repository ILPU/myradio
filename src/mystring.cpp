#include "mystring.h"
#include "utils_str.h"

mystring::mystring()
{
    str = _T("");
    init();
}

mystring::mystring(const int value)
{
    wchar_t lpszString[100];
    swprintf(lpszString, _T("%i"),(int)value);
    str = wstring(lpszString);
    init();
}

mystring::mystring(const char *text)
{
    int len = strlen(text) + 1;
    if (len == 1)
    {
        str = _T("");
    }
    else
    {
        wchar_t *buffer = (wchar_t *)malloc(sizeof(wchar_t) * len);
        wcscpy(buffer, _T(""));
        ANSITowchar(buffer, text, len);
        str = wstring(buffer);
        free(buffer);
    }
    init();
}

mystring::mystring(const wchar_t *text)
{
    str = wstring(text);
    init();
}

mystring::mystring(const str_utf8_t *text)
{
    int len = strlen((const char *)text) + 1;
    if (len == 1)
    {
        str = _T("");
    }
    else
    {
        wchar_t *buffer = (wchar_t *)malloc(sizeof(wchar_t) * len);
        wcscpy(buffer, _T(""));
        UTF8Towchar(buffer, (const char *)text, len);
        str = wstring(buffer);
        free(buffer);
    }
    init();
}

mystring::mystring(const string& text)
{
    int len = text.length() + 1;
    if (len == 1)
    {
        str = _T("");
    }
    else
    {
        wchar_t *buffer = (wchar_t *)malloc(sizeof(wchar_t) * len);
        wcscpy(buffer, _T(""));
        ANSITowchar(buffer, text.c_str(), len);
        str = wstring(buffer);
        free(buffer);
    }
    init();
}

mystring::mystring(const wstring& text)
{
    str = text;
    init();
}

mystring::mystring(const mystring& text)
{
    str = text.wstring_str();
    init();
}

mystring::~mystring()
{
    clean();
    if (cptr != NULL)
        free(cptr);
    if (wptr != NULL)
        free(wptr);
}

bool mystring::wcharToUTF8(char *dest, const wchar_t *src, size_t max)
{
    if (WideCharToMultiByte(CP_UTF8, 0, src, -1, (LPSTR)dest, max, NULL, NULL) == 0)
    {
        return false;
    }

    return true;
}

bool mystring::UTF8Towchar(wchar_t *dest, const char *src, size_t max)
{
    if (MultiByteToWideChar(CP_UTF8, 0, (LPSTR)src, -1, dest, max) == 0)
    {
        return false;
    }

    return true;
}

bool mystring::wcharToANSI(char *dest, const wchar_t *src, size_t max)
{
    if (WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, src, -1, (LPSTR)dest, max, NULL, NULL) == 0)
    {
        return false;
    }

    return true;
}

bool mystring::ANSITowchar(wchar_t *dest, const char *src, size_t max)
{
    if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPSTR)src, -1, dest, max) == 0)
    {
        return false;
    }

    return true;
}

mystring& mystring::operator =(const mystring& rhs)
{
    if (this == &rhs) //self-assignment
    {
        return *this;
    }

    str = rhs.str;
    clean();
    return *this;
}

mystring& mystring::operator +=(const mystring& rhs)
{
    str += rhs.str;
    clean();
    return *this;
}

mystring& mystring::operator +(const mystring& rhs)
{
    str += rhs.str;
    clean();
    return *this;
}

mystring& mystring::operator +=(const wchar_t& rhs)
{
    str += wstring(str);
    clean();
    return *this;
}

mystring& mystring::operator +(const wchar_t& rhs)
{
    str += wstring(str);
    clean();
    return *this;
}

const bool mystring::operator ==(const mystring& rhs)
{
    if (str == rhs.str)
        return true;

    return false;
}

const bool mystring::operator !=(const mystring& rhs)
{
    return !(*this == rhs);
}

wchar_t& mystring::operator [](unsigned int i)
{
    return str[i];
}

void mystring::set_lowercase_latin()
{
    make_lowercaseW_LATIN(str);
}

void mystring::set_uppercase_latin()
{
    make_uppercaseW_LATIN(str);
}

const wchar_t* mystring::Format(const wchar_t* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    Free();
    int len = vsnwprintf(NULL, 0, fmt, va)+1; //   vscwprintf
    Allocate_wchar(len);
    vsnwprintf(wptr, len, fmt, va);

    va_end(va);
    return wptr;
}

const char* mystring::c_str()
{
    if (cstr == NULL)
    {
        int len = str.length() + 1;
        cstr = (char *)malloc(len);
        strcpy(cstr, "");
        wcharToANSI(cstr, str.c_str(), len);
    }
    return (const char*)cstr;
}

const wchar_t* mystring::w_str()
{
    return str.c_str();
}

const str_utf8_t* mystring::utf8_str()
{
    if (utf8 == NULL)
    {
        int len = str.length() + 1;
        utf8 = (str_utf8_t *)malloc(len);
        strcpy((char *)utf8, "");
        wcharToUTF8((char *)utf8, str.c_str(), len);
    }
    return (const str_utf8_t*)utf8;
}

const wstring& mystring::wstring_str() const
{
    return str;
}

void mystring::Allocate_char(size_t size)
{
    if (wptr != NULL)
    {
        free(wptr);
        wptr = NULL;
    }
    if (cptr != NULL)
        free(cptr);
    cptr = (char *)malloc(size);
    strcpy(cptr, "");
}

void mystring::Allocate_wchar(size_t size)
{
    if (wptr != NULL)
        free(wptr);
    if (cptr != NULL)
    {
        free(cptr);
        cptr = NULL;
    }
    wptr = (wchar_t *)malloc(sizeof(wchar_t) * size);
    wcscpy(wptr, _T(""));
}

void mystring::Read()
{
    if (wptr != NULL)
    {
        str = wstring(wptr);
        clean();
    }
    else if (cptr != NULL)
    {
        mystring text = cptr;
        str = text.w_str();
        clean();
    }
}

void mystring::Free()
{
    if (wptr != NULL)
    {
        free(wptr);
        wptr = NULL;
    }
    if (cptr != NULL)
    {
        free(cptr);
        cptr = NULL;
    }
}

size_t mystring::length()
{
    return str.length();
}

void mystring::init()
{
    cstr = NULL;
    utf8 = NULL;
    cptr = NULL;
    wptr = NULL;
}

void mystring::clear_str()  // my
{
    clean();
    str = _T("");
    init();
}

void mystring::clean()
{
    if (cstr != NULL)
    {
        free(cstr);
        cstr = NULL;
    }
    if (utf8 != NULL)
    {
        free(utf8);
        utf8 = NULL;
    }
}
