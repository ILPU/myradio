#ifndef MAINAPP_CONFIG_H
#define MAINAPP_CONFIG_H

#include "common.h"
#include "config.h"
#include "font.h"

class MainApp_Config
{
public:
    MainApp_Config(HINSTANCE *pinst, HWND *pMainWin, config *pconf);
    ~MainApp_Config();
    void Show();
private:

    HINSTANCE *hParentInst;
    HWND *hParentWin;
    config *conf;

    static LRESULT CALLBACK SettingsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif // MAINAPP_CONFIG_H
