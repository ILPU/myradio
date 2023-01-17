#ifndef UTILS_STR_H
#define UTILS_STR_H

#include "common.h"

/*template <typename Func>
void inplaceW(wstring &string, Func func)
{
if(string.length() > MAXDWORD)
{
    return;//throw uniconv_error("String too long");
}
func(&string[0], string.length());
}


void toUpper(wstring &string)
{
return inplaceW(string, CharUpperBuffW);
}

void toLower(wstring &string)
{
return inplaceW(string, CharLowerBuffW);
}*/

void make_lowercase_LATIN(string &s);
void make_uppercase_LATIN(string &s);

void make_lowercaseW_LATIN(wstring &s);
void make_uppercaseW_LATIN(wstring &s);

wchar_t *strdup_new(const wchar_t *str);


#endif // UTILS_STR_H
