#ifndef MAINAPP_H
#define MAINAPP_H

#include "common.h"
#include "config.h"
#include "font.h"
#include "skin_main.h"

#include "MainApp_Info.h"
#include "MainApp_Config.h"

class MainApp
{
public:
    MainApp(HINSTANCE phInst, LPSTR lpszCmdLine, config *pconf);
    virtual ~MainApp();

    int BuildMainApp();
    int Run();

protected:

    static LRESULT CALLBACK WndMainProc_(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        MainApp *_this;

        if(message == WM_NCCREATE)
        {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            _this = reinterpret_cast<MainApp*>(lpcs->lpCreateParams);
            _this->hWin = hWnd;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(_this));
        }
        else
        {
            _this = reinterpret_cast<MainApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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

    bool ModifyMainExStyle(DWORD padd, DWORD pdel);

    void TaskBarShow_Hide(bool pShow);
    void Minimized_UnMinimAllForm();
    LRESULT CloseWindows();

    void SetSize(int pw, int ph);

    int GetWidth();
    int GetHeight();

private:
    HINSTANCE Inst;
    MSG messages;

    skin_engine *skin;

    HWND hWin;

    HDC dc_memory;
    HDC dc_main;

    RECT mf_rect;

    font *paint_font;

    HMENU main_context_menu;

    bool fMouseLeaveEventSet;
    POINT move_mf;
    bool block_mf;
    bool move_form;

    config *conf;

    MainApp_Info *formi;
    MainApp_Config *formc;

    //void procMouseEnterLeave(UINT message, WPARAM wParam, LPARAM lParam);

};

#endif // MAINAPP_H
