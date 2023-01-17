#ifndef COMMON_DOP_H_INCLUDED
#define COMMON_DOP_H_INCLUDED

#include "resource.h"

#define APP_CLASS _T("MYRADIO_APP_MAIN")
#define APP_TITLE _T("MyRadio v0.2")

#define APP_INFO_CLASS _T("MYRADIO_APP_INFO")

typedef signed char str_utf8_t;

#define MES_CREATE_APP WM_APP + 0x1505

#define mymes(str) MessageBox(0, str, _T(""), 0)
#define mymesa(str) MessageBoxA(0, str, "", 0)

#define SKIN_SIZEBORDER 0x4

template<class T> inline T Max(T a, T b)
{
    return (a > b) ? a : b;
}

template<class T> inline T Min(T a, T b)
{
    return (a < b) ? a : b;
}


#endif // COMMON_DOP_H_INCLUDED
