#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "common.h"
#include "mystring.h"


COLORREF SkinDecodeColor(const char* pcColour, bool* puse_color);

void GetSystemFolder(int pfolder, mystring *pstr);
bool DirectoryExists(mystring *pstr);

BOOL CALLBACK EnumDisplayMonitorsProc(HMONITOR hMon, HDC hdc, LPRECT rc, LPARAM param);
void MonitorAt(int x, int y, RECT *value);

#endif // UTILS_H_INCLUDED
