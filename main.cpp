#include "common.h"
#include "MainApp.h"
#include "config.h"
#include "log.h"

static HANDLE JustOneMutex;

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
    HANDLE CritSecMutex;
    int ret = 0;

    CritSecMutex = CreateMutex(NULL, true, NULL);
    if(CritSecMutex)
    {
        CloseHandle(CritSecMutex);
        JustOneMutex = CreateMutex(NULL, false, _T("MYRADIO_JUSTONE_MUTEX"));
        if(JustOneMutex)
        {
            LogFile *log;

            if(WaitForSingleObject(JustOneMutex, 0) != WAIT_TIMEOUT)
            {
                config conf(hThisInstance);
                if(conf.uselog)
                {
                    log = new LogFile(_T("myradio.log"));
                }

                MainApp app(hThisInstance, lpszArgument, &conf);
                ret = app.BuildMainApp();
                if(ret == 1)
                    ret = app.Run();
            }
            else
            {
                HWND hWndMyRadio = FindWindow(APP_CLASS, NULL);
                if(hWndMyRadio != NULL)
                {
                    if(lpszArgument[0] != '\0')
                    {
                        int i;
                        int size = 0;
                        COPYDATASTRUCT cds;
                        for (i = 0; i < __argc; i++)
                            size += strlen(__argv[i]) + 1;
                        cds.cbData = size;
                        cds.dwData = __argc;
                        cds.lpData = __argv[0];
                        SendMessage(hWndMyRadio, WM_COPYDATA, (WPARAM)hWndMyRadio, (LPARAM)&cds);
                    }
                    else
                    {
                        //SetForegroundWindow(hWndMyRadio);
                        //ShowWindow(hWndMyRadio, SW_RESTORE);
                        FLASHWINFO pf;
                        pf.cbSize = sizeof(FLASHWINFO);
                        pf.hwnd = hWndMyRadio;
                        pf.dwFlags = FLASHW_TRAY; // (or FLASHW_ALL to flash and if it is not minimized)
                        pf.uCount = 10;
                        pf.dwTimeout = 0;
                        FlashWindowEx(&pf);
                    }
                }
            }

            if (log)
                delete log;

            CloseHandle(JustOneMutex);
            JustOneMutex = 0;
        }
    }
    return ret;
}
