
// x64 系统需要编译为x64 PE
#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <commctrl.h>

typedef struct _TRAYDATA
{
    HWND hwnd;
    UINT uID;
    UINT uCallbackMessage;
    DWORD Reserved[2];
    HICON hIcon;
} TRAYDATA;

HWND FindTrayToolbarWindow()
{
    HWND hWnd = FindWindowA("Shell_TrayWnd", NULL);
    if(hWnd)
    {
        hWnd = FindWindowExA(hWnd, NULL, "TrayNotifyWnd", NULL);
        if(hWnd)
        {
            hWnd = FindWindowExA(hWnd, NULL, "SysPager", NULL);
            if(hWnd)
            {
                hWnd = FindWindowExA(hWnd, NULL, "ToolbarWindow32", NULL);
            }
        }
    }

    return hWnd;
}

int main()
{
    setlocale(LC_ALL, "chs");

    HWND hWnd_TrayToolbar = FindTrayToolbarWindow();         //托盘窗口句柄
    if (!hWnd_TrayToolbar)
        return -1;

    DWORD  DwTrayPid=0;
    GetWindowThreadProcessId(hWnd_TrayToolbar, &DwTrayPid);  //取得托盘窗口对应的进程id(explorer.exe)
    if (!DwTrayPid)
        return -1;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DwTrayPid);
    if (hProcess != INVALID_HANDLE_VALUE)
    {
        //在目标进程中申请一块内存，放TBBUTTON信息
        PVOID RemoteBuf = VirtualAllocEx(hProcess, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
        if (RemoteBuf)
        {
            TBBUTTON btnData     = {0};
            TRAYDATA trayData    = {0};
            NOTIFYICONDATAA data = {0};
            CHAR   szTip[128];

            //给托盘窗口发消息，得到托盘里图标个数
            int count = (int)SendMessage(hWnd_TrayToolbar, TB_BUTTONCOUNT, 0, 0);
            printf("tray icon count : %d\n", count);
            for(int i=0; i<count; i++)
            {
                szTip[0] = 0;
                szTip[1] = 0;

                //取得TBBUTTON结构到目标进程RemoteBuf
                BOOL b = (BOOL)SendMessage(hWnd_TrayToolbar, TB_GETBUTTON, i, (LPARAM)RemoteBuf);
                if (!b)  continue;

                //读TBBUTTON结构到本地
                b = ReadProcessMemory(hProcess, (LPCVOID)RemoteBuf, &btnData, sizeof(TBBUTTON), NULL);
                if (!b)  continue;

                if (btnData.iString > 0x10000)
                {
                    b = ReadProcessMemory(hProcess, (LPCVOID)btnData.iString, &szTip, sizeof(szTip), NULL);
                    if (!b) {szTip[0] = 0; szTip[1] = 0;}
                    else    {szTip[126] = 0; szTip[127] = 0;}
                }

                //读dwData字段指向的TRAYDATA结构
                b = ReadProcessMemory(hProcess, (LPCVOID)btnData.dwData, &trayData, sizeof(TRAYDATA), NULL);
                if (!b)  continue;

                if (trayData.hwnd)
                {
                    DWORD dwProcessId = 0;
                    GetWindowThreadProcessId(trayData.hwnd, &dwProcessId);  //通过TRAYDATA里的hwnd字段取得本图标的进程id
                    printf("\n[%d]\n%ws\n", dwProcessId, szTip);
                    if (!dwProcessId)
                    {
                        data.hWnd = trayData.hwnd;
                        data.uID  = trayData.uID;
                        Shell_NotifyIconA(NIM_DELETE, &data);
                    }
                }
            }

            VirtualFreeEx(hProcess, RemoteBuf, sizeof(TBBUTTON), MEM_COMMIT);
        }

        CloseHandle(hProcess);
    }

    getchar();
    return 0;
}