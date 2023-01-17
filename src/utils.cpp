#include "utils.h"

COLORREF SkinDecodeColor(const char* pcColour, bool* puse_color)
{
    unsigned long dwColour;

    if(sscanf(pcColour, "#%lx", &dwColour) == 1)
    {
        // Swap red and blue bytes
        *puse_color = true;
        return ((dwColour&0x0000FF) << 16) | (dwColour&0x00FF00) | ((dwColour&0xFF0000) >> 16);
    }
    *puse_color = false;
    return 0;
}

bool DirectoryExists(mystring *pstr)
{
    DWORD dwAttrib = GetFileAttributes(pstr->w_str());

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void GetSystemFolder(int pfolder, mystring *pstr)
{
    LPITEMIDLIST pidl;
    if(SHGetSpecialFolderLocation(0, pfolder, &pidl) == NOERROR)
    {
        pstr->Allocate_wchar(MAX_PATH);
        SHGetPathFromIDList(pidl, pstr->wptr);
        pstr->Read();
        pstr->Free();
        CoTaskMemFree(pidl);
    }
}

BOOL CALLBACK EnumDisplayMonitorsProc(HMONITOR hMon, HDC hdc, LPRECT rc, LPARAM param)
{
    vector<RECT>* rec_val = reinterpret_cast<vector<RECT>*>(param);
    rec_val->push_back(*rc);
    return 1;
}

void ListMonitors(vector<RECT> *value)
{
    EnumDisplayMonitors(0, NULL, EnumDisplayMonitorsProc, LPARAM(value));
}

void MonitorAt(int x, int y, RECT *value)
{
    vector<RECT> monitors;
    ListMonitors(&monitors);
    for (vector<RECT>::iterator it = monitors.begin(); it != monitors.end(); it++)
        if(PtInRect(&(*it), {x, y}))
        {
            *value = static_cast<RECT>(*it);
            return;
        };
}
