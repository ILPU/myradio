#include "MainApp_Config.h"

MainApp_Config::MainApp_Config(HINSTANCE *pinst, HWND *pMainWin, config *pconf)
{
    hParentInst = pinst;
    conf = pconf;
    hParentWin = pMainWin;
}

MainApp_Config::~MainApp_Config()
{
    //dtor
}

void MainApp_Config::Show()
{
    DialogBox(*hParentInst, MAKEINTRESOURCE(DLG_SETTINGS), *hParentWin, (DLGPROC)SettingsDlgProc);
}

LRESULT CALLBACK MainApp_Config::SettingsDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCITEM item;
    HWND hWndTemp;
    RECT   rc;
    HWND*  phDlg;
    int    nCurSel;

    switch (message)
    {
    case WM_INITDIALOG:
        hWndTemp = GetDlgItem(hWnd, IDC_TAB1);

        ZeroMemory(&item, sizeof(item));
        item.mask    = TCIF_TEXT;

        item.pszText = _T("Вывод");
        TabCtrl_InsertItem(hWndTemp, 0, &item);
        item.pszText = _T("Подключение");
        TabCtrl_InsertItem(hWndTemp, 1, &item);
        item.pszText = _T("Разное");
        TabCtrl_InsertItem(hWndTemp, 2, &item);


        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hWnd, wParam);
            return TRUE;
        }
        break;

    default:
        break;
    }

    return FALSE;
}
