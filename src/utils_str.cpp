#include "utils_str.h"

struct lowercase_func_lat
{
    void operator()(::string::value_type &v)
    {
        v = tolower(v);
    }
};

struct uppercase_func_lat
{
    void operator()(string::value_type &v)
    {
        v = toupper(v);
    }
};

void make_lowercase_LATIN(string &s)
{
    for_each(s.begin(), s.end(), lowercase_func_lat());
}

void make_uppercase_LATIN(string &s)
{
    for_each(s.begin(), s.end(), uppercase_func_lat());
}

struct lowercasew_func_lat
{
    void operator()(wstring::value_type &v)
    {
        v = towlower(v);
    }
};

struct uppercasew_func_lat
{
    void operator()(wstring::value_type &v)
    {
        v = towupper(v);
    }
};

void make_lowercaseW_LATIN(wstring &s)
{
    for_each(s.begin(), s.end(), lowercasew_func_lat());
}

void make_uppercaseW_LATIN(wstring &s)
{
    for_each(s.begin(), s.end(), uppercasew_func_lat());
}

wchar_t *strdup_new(const wchar_t *str)
{
    wchar_t *n;

    n = new wchar_t[wcslen(str) + 1];
    wcscpy(n, str);

    return n;
}
