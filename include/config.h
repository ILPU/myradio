#ifndef CONFIG_H
#define CONFIG_H

#include "mystring.h"

struct colors_t
{
    COLORREF textcolor;
    COLORREF backcolor;
    COLORREF ramkacolor;
};

class config
{
public:
    config(HINSTANCE phInst);
    virtual ~config();

    bool uselog;

    winpos_t winpos;
    colors_t wincolors;

    winpos_t infopos;

    DWORD volume;

    DWORD winposlock;
    wchar_t skinfile[MAX_PATH];

    mystring appdir;
    mystring config_dir;
    mystring config_file;
    mystring dbfile;

    void LoadConfigForm();
    void SaveConfigForm();

    void LoadOtherSettings();
    void SaveOtherSettings();

    bool LoadSkinSettings();

    bool LoadDWORD(wchar_t const *name, DWORD *value, DWORD def);
    bool LoadSZ(wchar_t const *name, wchar_t *value, const wchar_t *def, size_t size);
    BOOL LoadDATA(wchar_t const *name, void* value, size_t size);

    BOOL SaveDWORD(wchar_t const *name, DWORD value);
    BOOL SaveSZ(wchar_t const *name, wchar_t *value);
    BOOL SaveDATA(wchar_t const *name, void* value, size_t size);
};

#endif // CONFIG_H
