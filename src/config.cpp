#include "config.h"
#include "utils.h"

#define INI_SECTION_NAME _T("myradio")

#define LOAD_CONF_DWORD(name, def) LoadDWORD(_T(#name), &name, def)
#define LOAD_CONF_SZ(name, def) LoadSZ(_T(#name), &name, _T(def), sizeof(name))

#define SAVE_CONF_DWORD(name) SaveDWORD(_T(#name), name)
#define SAVE_CONF_SZ(name) SaveSZ(_T(#name), name, sizeof(name))

config::config(HINSTANCE phInst)
{
    uselog = false;

    appdir.Allocate_wchar(MAX_PATH);
    GetModuleFileName(phInst, appdir.wptr, MAX_PATH);
    appdir.Read();
    appdir.Free();
    string::size_type found = appdir.str.find_last_of(_T("\\"));
    appdir.str = appdir.str.substr(0,found);

    GetSystemFolder(CSIDL_APPDATA, &config_dir);
    config_dir += _T("\\myradio_app");

    if(!DirectoryExists(&config_dir))
        CreateDirectory(config_dir.w_str(), NULL);

    config_file = config_dir;
    config_file += _T("\\myradio.ini");

    dbfile = config_dir;
    dbfile += _T("\\myradio.db");
}

config::~config()
{

}

void config::LoadConfigForm()
{
    if(!LoadDATA(_T("form_main"), &winpos, sizeof(winpos)))
    {
        POINT p;
        RECT r;
        GetCursorPos(&p);
        MonitorAt(p.x, p.y, &r);
        winpos.w_width      = 200;
        winpos.w_height     = 20;
        winpos.wleft        = r.left + (r.right - r.left - winpos.w_width) / 2;
        winpos.wtop         = r.top + (r.bottom - r.top - winpos.w_height) / 2;
        winpos.wstate       = 0; // normal state
        winpos.stayontop    = false;
        winpos.wsp_pos      = 0;
        winpos.lockpos      = false;
        winpos.style        = 0;
    }
    if(!LoadDATA(_T("colors_main"), &wincolors, sizeof(wincolors)))
    {
        wincolors.backcolor = 0xFFFFFF;
        wincolors.ramkacolor = 0x000000;
        wincolors.textcolor = 0x000000;
    }

    if(!LoadDATA(_T("form_info"), &infopos, sizeof(infopos)))
    {
        memset(&infopos, 0, sizeof(infopos));
    }
}

void config::SaveConfigForm()
{
    SaveDATA(_T("form_main"), (void*)&winpos, sizeof(winpos));
    SaveDATA(_T("colors_main"), (void*)&winpos, sizeof(winpos));
}

void config::LoadOtherSettings()
{
    LOAD_CONF_DWORD(winposlock, 0);
    LOAD_CONF_DWORD(volume, 100);
}

void config::SaveOtherSettings()
{
    SAVE_CONF_DWORD(volume);
    SAVE_CONF_DWORD(winposlock);
}

bool config::LoadSkinSettings()
{
    bool ret;
    ret = LoadSZ(_T("skinfile"), skinfile, _T(""), MAX_PATH);

    return ret;
}

bool config::LoadDWORD(wchar_t const *name, DWORD *value, DWORD def)
{
    wchar_t lpReturnedString[100];
    if(!GetPrivateProfileString(INI_SECTION_NAME, name, _T("\0"), lpReturnedString, 100, config_file.w_str()))
    {
        *value = def;
        return false;
    }

    *value = static_cast<DWORD>(_wtoi(lpReturnedString));
    return true;
}

bool config::LoadSZ(wchar_t const *name, wchar_t *value, const wchar_t *def, size_t size)
{
    wchar_t tmp[4096];
    if(!GetPrivateProfileString(INI_SECTION_NAME, name, _T("\0"), tmp, 4096, config_file.w_str()))
    {
        wcsncpy(value, def, size);
        value[size-1] = 0;
        return false;
    }
    wcsncpy(value, tmp, size);
    value[size-1] = 0;
    return true;
}

BOOL config::LoadDATA(wchar_t const *name, void* value, size_t size)
{
    return GetPrivateProfileStruct(INI_SECTION_NAME, name, value, size, config_file.w_str());
}

BOOL config::SaveDWORD(wchar_t const *name, DWORD value)
{
    wchar_t lpszString[100];
    swprintf(lpszString, _T("%i"),(int)value);
    return WritePrivateProfileString(INI_SECTION_NAME, name, lpszString, config_file.w_str());
}

BOOL config::SaveSZ(wchar_t const *name, wchar_t *value)
{
    return WritePrivateProfileString(INI_SECTION_NAME, name, value, config_file.w_str());
}

BOOL config::SaveDATA(wchar_t const *name, void* value, size_t size)
{
    return WritePrivateProfileStruct(INI_SECTION_NAME, name, value, size, config_file.w_str());
}
