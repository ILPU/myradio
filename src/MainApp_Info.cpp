#include "MainApp_Info.h"
#include "common_dop.h"

MainApp_Info::MainApp_Info(HINSTANCE *pinst, HWND *pMainWin, config *pconf, skin_engine *pskin)
{
    hParentInst = pinst;
    conf = pconf;
    hInfoWin = NULL;
    hParentWin = pMainWin;
    hMainSkin = pskin;

    fMouseLeaveEventSet = false;

    InitializeCriticalSection(&cs_create);

    iscreate = false;
}

MainApp_Info::~MainApp_Info()
{
    CloseInfo();
    DeleteCriticalSection(&cs_create);
}

void MainApp_Info::cr_enter()
{
    EnterCriticalSection(&cs_create);
}

void MainApp_Info::cr_leave()
{
    LeaveCriticalSection(&cs_create);
}

void MainApp_Info::ShowInfo()
{
    cr_enter();
    if(iscreate)
    {
        ShowWindow(hInfoWin, SW_SHOW);
        UpdateWindow(hInfoWin);
        SetForegroundWindow(hInfoWin);
        //BringWindowToTop(hInfoWin);
        //SetParent(hInfoWin, *hParentWin);
    }
    //UpdateWindow(hInfoWin);
    cr_leave();
}

bool MainApp_Info::CreateInfo()
{
    if(!hInfoWin)
    {
        WNDCLASS wincl;

        wincl.hInstance = *hParentInst;
        wincl.lpszClassName = APP_INFO_CLASS;
        wincl.lpfnWndProc = WndMainProc_;
        wincl.style = 0;
        wincl.hIcon = NULL;
        wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
        wincl.lpszMenuName = NULL;
        wincl.cbClsExtra = 0;
        wincl.cbWndExtra = 0;
        wincl.hbrBackground = (HBRUSH)NULL;

        if(!RegisterClass(&wincl))
            return 0;

        hInfoWin = CreateWindowEx(
                       0,
                       APP_INFO_CLASS,
                       NULL,
                       WS_POPUP,
                       100,
                       100,
                       300,
                       300,
                       NULL,//*hParentWin,
                       NULL,
                       *hParentInst,
                       this
                   );

        cr_enter();
        iscreate = (hInfoWin != 0);
        cr_leave();
    }
    return iscreate;
}

HWND MainApp_Info::GetInfoWin()
{
    return hInfoWin;
}

void MainApp_Info::SendCloseInfo()
{
    PostMessage(hInfoWin, WM_CLOSE, 0, 0);
}

bool MainApp_Info::CloseInfo()
{
    cr_enter();
    iscreate = false;
    cr_leave();

    hMainSkin->FreeInfoHandle(hInfoWin);
    SetWindowLongPtr(hInfoWin, GWLP_USERDATA, 0);
    DestroyWindow(hInfoWin);
    hInfoWin = NULL;
    UnregisterClass(APP_INFO_CLASS, *hParentInst);
}

LRESULT MainApp_Info::WndMainLocalProc_(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {

    case WM_CREATE:
    {
        const CREATESTRUCT* pCS = (const CREATESTRUCT*)lParam;

        conf->infopos.wleft     = pCS->x;
        conf->infopos.wtop      = pCS->y;
        conf->infopos.w_height  = pCS->cy;
        conf->infopos.w_width   = pCS->cx;

        move_form = false;
        hMainSkin->SetInfoHandle(hInfoWin);

        ShowWindow(hInfoWin, SW_SHOW);
        UpdateWindow(hInfoWin);

        return 0;
    }

    /*case WM_ACTIVATE:
    {
        if (LOWORD(wParam) == WA_ACTIVE)
            SetFocus(hInfoWin);
        return 0;
    }*/

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;
        RECT rWorkArea;

        MONITORINFO mi;
        HMONITOR hmon = MonitorFromWindow(hInfoWin, MONITOR_DEFAULTTOPRIMARY);
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(hmon, &mi);
        rWorkArea.left = mi.rcWork.left - mi.rcMonitor.left;
        rWorkArea.right = mi.rcWork.right - mi.rcMonitor.left;
        rWorkArea.top = mi.rcWork.top - mi.rcMonitor.top;
        rWorkArea.bottom = mi.rcWork.bottom - mi.rcMonitor.top;

        pMinMaxInfo->ptMinTrackSize.x = hMainSkin->get_min_playlist_w();
        pMinMaxInfo->ptMinTrackSize.y = hMainSkin->get_min_playlist_h();
        pMinMaxInfo->ptMaxPosition.x = rWorkArea.left;
        pMinMaxInfo->ptMaxPosition.y = rWorkArea.top;
        pMinMaxInfo->ptMaxSize.x = rWorkArea.right - rWorkArea.left;
        pMinMaxInfo->ptMaxSize.y = rWorkArea.bottom - rWorkArea.top;
        pMinMaxInfo->ptMaxTrackSize.x = pMinMaxInfo->ptMaxSize.x;
        pMinMaxInfo->ptMaxTrackSize.y = pMinMaxInfo->ptMaxSize.y;

        return 0;
    }
    case WM_NCHITTEST:
    {
        POINT ptMouse;

        ptMouse.x = (short)LOWORD(lParam) - conf->infopos.wleft;
        ptMouse.y = (short)HIWORD(lParam) - conf->infopos.wtop;

        // - top band
        if(ptMouse.y < SKIN_SIZEBORDER)
        {
            if (ptMouse.x < SKIN_SIZEBORDER)
                return HTTOPLEFT;
            // - right
            else if (ptMouse.x > (conf->infopos.w_width - SKIN_SIZEBORDER))
                return HTTOPRIGHT;
            else
                return HTTOP;
        }
        else if(ptMouse.y > (conf->infopos.w_height - SKIN_SIZEBORDER))
        {
            // - left
            if(ptMouse.x < SKIN_SIZEBORDER)
                return HTBOTTOMLEFT;
            // - right
            else if (ptMouse.x > (conf->infopos.w_width - SKIN_SIZEBORDER))
                return HTBOTTOMRIGHT;
            else
                return HTBOTTOM;
        }
        else if (ptMouse.x < SKIN_SIZEBORDER)
            return HTLEFT;
        else if (ptMouse.x > (conf->infopos.w_width - SKIN_SIZEBORDER))
            return HTRIGHT;
        else
            return HTCLIENT;
    }
    case WM_WINDOWPOSCHANGED:
    {
        const WINDOWPOS* pWP = (const WINDOWPOS*)lParam;

        conf->infopos.wleft = pWP->x;
        conf->infopos.wtop = pWP->y;
        conf->infopos.w_width = pWP->cx;
        conf->infopos.w_height = pWP->cy;

        if(hMainSkin)
        {
            hMainSkin->form_info_pos_changed((WINDOWPOS*)pWP);
            RedrawWindow(hInfoWin, NULL, NULL, RDW_INVALIDATE);
        }
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        bool ret = false;
        if(hMainSkin)
            ret = hMainSkin->info_mousedown(LOWORD(lParam), HIWORD(lParam), wParam);

        if(!ret && wParam == MK_LBUTTON)
        {
            if(!conf->winposlock)
            {
                move_mf.x = LOWORD(lParam);
                move_mf.y = HIWORD(lParam);
                SetCapture(hInfoWin);
                SetFocus(hInfoWin);
                move_form = true;
            }
        }
        return 0;
    }
    case WM_LBUTTONUP:
    {
        bool ret = false;
        if(hMainSkin)
            ret = hMainSkin->info_mouseup(LOWORD(lParam), HIWORD(lParam), wParam);
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
            if(hMainSkin)
                hMainSkin->info_mouseleave();

            TRACKMOUSEEVENT Track;
            Track.cbSize = sizeof(Track);
            Track.dwFlags = TME_LEAVE | TME_CANCEL;
            Track.hwndTrack = hInfoWin;
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

            SetWindowPos(hInfoWin, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        else
        {
            if(!fMouseLeaveEventSet)
            {
                TRACKMOUSEEVENT Track;
                Track.cbSize = sizeof(Track);
                Track.dwFlags = TME_LEAVE;
                Track.hwndTrack = hInfoWin;
                fMouseLeaveEventSet = TrackMouseEvent(&Track) ? true : false;
            }

            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            if(hMainSkin)
                hMainSkin->info_mousemove(pt.x, pt.y, wParam);
        }
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;

    case WM_UPDATEUISTATE:
    case WM_PRINTCLIENT:
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hInfoWin, &ps);
        if(hMainSkin)
        {
            hMainSkin->paintInfo(&ps);
        }
        EndPaint(hInfoWin, &ps);
        return 0;
    }

    case WM_SETFOCUS:
    {
        return 0;
    }

    case WM_CLOSE:
    {
        CloseInfo();
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    }

    return DefWindowProc(hInfoWin, message, wParam, lParam);
}

