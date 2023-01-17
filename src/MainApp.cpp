#include "MainApp.h"
#include "common_dop.h"
#include "action_names.h"

MainApp::MainApp(HINSTANCE phInst, LPSTR lpszCmdLine, config *pconf)
{
    Inst = phInst;
    conf = pconf;
    move_form = false;
    fMouseLeaveEventSet = false;
    main_context_menu = NULL;
}

MainApp::~MainApp()
{
    if(main_context_menu)
    {
        DestroyMenu(main_context_menu);
        main_context_menu = NULL;
    }

    UnregisterClass(APP_CLASS, Inst);
}

int MainApp::Run()
{
    while (GetMessage(&messages, NULL, 0, 0))
    {
        //if(!TranslateAccelerator(hWin,, &messages))
        //{
        TranslateMessage(&messages);
        DispatchMessage(&messages);
        //}
    }
    return messages.wParam;
}

int MainApp::BuildMainApp()
{
    WNDCLASSEX wincl;

    wincl.hInstance = Inst;
    wincl.lpszClassName = APP_CLASS;
    wincl.lpfnWndProc = WndMainProc_;
    wincl.style = 0;//CS_DBLCLKS;
    wincl.cbSize = sizeof(WNDCLASSEX);

    wincl.hIcon = LoadIcon(Inst, MAKEINTRESOURCE(1));
    wincl.hIconSm = LoadIcon(Inst, MAKEINTRESOURCE(1));
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)NULL;

    if(!RegisterClassEx(&wincl))
        return 0;

    conf->LoadConfigForm();

    hWin = CreateWindowEx(
               0,
               APP_CLASS,
               APP_TITLE,
               WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX,
               conf->winpos.wleft,
               conf->winpos.wtop,
               300,
               19,
               HWND_DESKTOP,
               NULL,
               Inst,
               this
           );

    return hWin?1:0;
}

int MainApp::GetWidth()
{
    return mf_rect.right - mf_rect.left;
}

int MainApp::GetHeight()
{
    return mf_rect.bottom - mf_rect.top;
}

LRESULT MainApp::CloseWindows()
{
    RECT rcPos;
    GetWindowRect(hWin, &rcPos);
    conf->winpos.wleft = rcPos.left;
    conf->winpos.wtop  = rcPos.top;

    conf->SaveConfigForm();
    conf->SaveOtherSettings();

    if(formi)
        delete formi;

    if(formc)
        delete formc;

    if(skin)
        delete skin;

    if(paint_font)
        delete paint_font;

    DeleteDC(dc_memory);
    ReleaseDC(hWin, dc_main);

    DestroyWindow(hWin);
    return 0;
}

bool MainApp::ModifyMainExStyle(DWORD padd, DWORD pdel)
{
    if(pdel)
        if(!SetWindowLong(hWin, GWL_EXSTYLE,
                          GetWindowLong(hWin, GWL_EXSTYLE) &~ pdel)) //Удалить стили
            return false;
    if(padd)
        if(!SetWindowLong(hWin, GWL_EXSTYLE,
                          GetWindowLong(hWin, GWL_EXSTYLE) | padd)) //Добавить стили
            return false;
    return true;
}

void MainApp::TaskBarShow_Hide(bool pShow)
{
    if(!pShow)
        ModifyMainExStyle(WS_EX_TOOLWINDOW, WS_EX_APPWINDOW);
    else
        ModifyMainExStyle(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
}

void MainApp::Minimized_UnMinimAllForm()
{
    if(IsWindowVisible(hWin))
    {
        SendMessage(hWin, WM_SYSCOMMAND, SC_MINIMIZE, 0);
    }
    else
    {
        ShowWindow(hWin, SW_SHOW); //  SW_SHOWNA
        SetForegroundWindow(hWin);
        SetFocus(hWin);
        SendMessage(hWin, WM_SYSCOMMAND, SC_RESTORE, 0);
    }
}

void MainApp::SetSize(int pw, int ph)
{
    if(hWin)
        SetWindowPos(hWin, 0, 0, 0, pw, ph, SWP_NOREDRAW | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT MainApp::WndMainLocalProc_(UINT message, WPARAM wParam, LPARAM lParam)
{
    //procMouseEnterLeave(message, wParam, lParam);
    switch(message)
    {

    case WM_CREATE:
    {
        InitCommonControls();
        GetClientRect(hWin, &mf_rect);

        dc_main = GetDC(hWin);
        dc_memory = CreateCompatibleDC(dc_main);

        paint_font = new font(hWin);
        skin = new skin_engine(hWin, dc_main, dc_memory, Inst, &conf->appdir);

        bool loadskin = false;
        if(conf->LoadSkinSettings())
        {
            //skin->load_skinfile(_T(""));
        }
        else
        {
            formi = new MainApp_Info(&Inst, &hWin, conf, skin);
            formi->CreateInfo();

            formc = new MainApp_Config(&Inst, &hWin, conf);

            //loadskin = skin->load_skinfile(_T("test.zip"));
            loadskin = skin->load_defaultskin();
        }

        if(!loadskin)
        {
            mymes(_T("ERROR IN SKINFILE"));
            if(skin)
                delete skin;
            if(paint_font)
                delete paint_font;
            DeleteDC(dc_memory);
            ReleaseDC(hWin, dc_main);
            return -1;
        }

        //paint_CreateObject(&dc_main, &mf_paint_hdcMem, &mf_paint_hbmMem, &mf_paint_hbmOld);
        //RedrawWindow(hWin, NULL, 0, RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);


        ShowWindow(hWin, SW_SHOW);
        UpdateWindow(hWin);
        //SetForegroundWindow(hWin);
        SetFocus(hWin);

        PostMessage(hWin, MES_CREATE_APP, 0, 0);
        return 0;
    }

    case MES_CREATE_APP:
    {
        main_context_menu = CreatePopupMenu();
        AppendMenu(main_context_menu, MF_STRING, ACT_SHOW_CONFIG, _T("Настройки"));
        AppendMenu(main_context_menu, MF_SEPARATOR | MF_DISABLED | MF_GRAYED, ( UINT_PTR )-1, NULL);
        AppendMenu(main_context_menu, MF_STRING, ACT_EXIT, _T("Выход"));

        //<--- place her load and check output Device !!
        conf->LoadOtherSettings();
        skin->set_sc_volume(conf->volume);

        //formi->ShowInfo();
    }
    break;

    case WM_COMMAND:
    {
        switch(LOWORD(wParam))
        {
        case ACT_SHOW_CONFIG:
            formc->Show();
            break;
        case ACT_EXIT:
            PostMessage(hWin, WM_CLOSE, 0, 0);
            break;
        }
    }
    break;

    case WM_RBUTTONDOWN:
    {
        POINT pt;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);

        if(skin)
        {
            if(!skin->main_controlAtPos(pt.x, pt.y))
            {
                ClientToScreen(hWin, &pt);
                TrackPopupMenu(
                    main_context_menu,
                    TPM_LEFTALIGN |
                    TPM_TOPALIGN |
                    TPM_RIGHTBUTTON,
                    pt.x,
                    pt.y,
                    0,
                    hWin,
                    NULL);
            }
        }

        return 0;
    }

    case WM_LBUTTONDOWN:
        //case WM_MBUTTONDOWN:
        //case WM_RBUTTONDOWN:
    {
        bool ret = false;
        if(skin)
            ret = skin->main_mousedown(LOWORD(lParam), HIWORD(lParam), wParam);

        SetWindowPos(hWin, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
        //SetWindowPos(formi->GetInfoWin(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

        if(!ret && wParam == MK_LBUTTON)
        {
            if(!conf->winposlock)
            {
                move_mf.x = LOWORD(lParam);
                move_mf.y = HIWORD(lParam);
                BringWindowToTop(hWin);
                SetFocus(hWin);
                SetCapture(hWin);
                move_form = true;
            }
        }
        return 0;
    }

    case WM_LBUTTONUP:
        //case WM_MBUTTONUP:
    {
        bool ret = false;
        if(skin)
            ret = skin->main_mouseup(LOWORD(lParam), HIWORD(lParam), wParam);
        if(!ret)
        {
            ReleaseCapture();
            move_form = false;
        }
        return 0;
    }

    case WM_MOUSELEAVE:
    {
        if(fMouseLeaveEventSet)
        {
            if(skin)
                skin->main_mouseleave();

            TRACKMOUSEEVENT Track;
            Track.cbSize = sizeof(Track);
            Track.dwFlags = TME_LEAVE | TME_CANCEL;
            Track.hwndTrack = hWin;
            TrackMouseEvent(&Track);
        }
        fMouseLeaveEventSet = false;
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        if(move_form)
        {
            POINT pt;
            GetCursorPos(&pt);

            int x,y;
            x = pt.x - move_mf.x;
            y = pt.y - move_mf.y;

            SetWindowPos(hWin, NULL, x, y, 0, 0,  SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        else
        {
            if(!fMouseLeaveEventSet)
            {
                TRACKMOUSEEVENT Track;
                Track.cbSize = sizeof(Track);
                Track.dwFlags = TME_LEAVE;
                Track.hwndTrack = hWin;
                fMouseLeaveEventSet = TrackMouseEvent(&Track) ? true : false;
            }

            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            if(skin)
                skin->main_mousemove(pt.x, pt.y, wParam);
        }
        return 0;
    }

    case WM_SETFOCUS:
    {
        //SetFocus(hWin);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_WINDOWPOSCHANGED:
    {
        const WINDOWPOS* pWP = (const WINDOWPOS*)lParam;
        skin->form_main_pos_changed((WINDOWPOS*)pWP);
        return 0;
    }
    case WM_UPDATEUISTATE:
    case WM_PRINTCLIENT:
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWin, &ps);
        if(skin)
        {
            RECT rClient;
            GetClientRect(hWin, &rClient);
            skin->paintMain(&rClient, &ps);
        }
        EndPaint(hWin, &ps);
        return 0;
    }

    case WM_CLOSE:
        return CloseWindows();

    case WM_QUERYENDSESSION:
    {
        CloseWindows();
        return 1;
    }

    case SKIN_APP_MES:
    {
        switch(wParam)
        {
        case SKIN_BUTTON_MOUSE_UP:
        {
            control_type ct;
            ct = (control_type)lParam;
            switch(ct)
            {
            case btn_exit:
                PostMessage(hWin, WM_CLOSE, 0, 0);
                break;
            case btn_min:
                Minimized_UnMinimAllForm();
                break;
            }
        }
        break;

        case SKIN_NEW_FORM_SIZE:
            SetSize(skin->pWidth, skin->pHeight);
            break;

        case SKIN_SCROLL_NEW_POS:
        {
            control_type ct;
            ct = (control_type)lParam;
            switch(ct)
            {
            case sc_vol:
                conf->volume = skin->get_sc_volume();
                break;
            }
        }
        break;

        }
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWin, message, wParam, lParam);
}
