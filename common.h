#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#pragma once

#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif


#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define STRICT
#define NOCOMM
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#undef __STRICT_ANSI__

#include <windows.h>

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#include <commctrl.h>
#include <Shlobj.h>

#include <math.h>

#include <algorithm>
#include <vector>
#include <map>
#include <string>

using namespace std;

struct winpos_t
{
    DWORD w_width,w_height;
    DWORD wleft,wtop;
    DWORD wstate;
    DWORD wsp_pos;
    DWORD style;
    bool stayontop;
    bool lockpos;
};

#endif // COMMON_H_INCLUDED
