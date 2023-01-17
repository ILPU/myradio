#ifndef MAINAPP_INFO_H
#define MAINAPP_INFO_H

#include "common.h"
#include "config.h"
#include "font.h"
#include "skin_main.h"


class MainApp_Info
{
public:
    MainApp_Info(HINSTANCE *pinst, HWND *pMainWin, config *pconf, skin_engine *pskin);
    ~MainApp_Info();

    bool CreateInfo();
    bool CloseInfo();

    void SendCloseInfo();

    void ShowInfo();

    void cr_enter();
    void cr_leave();

    HWND GetInfoWin();

private:
    bool iscreate;

    CRITICAL_SECTION cs_create;

    HINSTANCE *hParentInst;
    HWND *hParentWin;

    HWND hInfoWin;
    skin_engine *hMainSkin;

    config *conf;

    bool fMouseLeaveEventSet;
    POINT move_mf;
    bool block_mf;
    bool move_form;

    static LRESULT CALLBACK WndMainProc_(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        MainApp_Info *_this;

        if(message == WM_NCCREATE)
        {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            _this = reinterpret_cast<MainApp_Info*>(lpcs->lpCreateParams);
            _this->hInfoWin = hWnd;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)_this);
        }
        else
        {
            _this = reinterpret_cast<MainApp_Info*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }

        if(_this)
        {
            return _this->WndMainLocalProc_(message, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    LRESULT WndMainLocalProc_(UINT message, WPARAM wParam, LPARAM lParam);
};

#endif // MAINAPP_INFO_H
