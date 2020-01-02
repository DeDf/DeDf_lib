// x64 系统需要编译为x64 PE
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

struct TRAYDATA
{
    HWND hwnd;
    UINT uID;
    UINT uCallbackMessage;
    DWORD Reserved[2];
    HICON hIcon;
};

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
    HWND hWnd_TrayToolbar = FindTrayToolbarWindow();  //托盘窗口句柄
    if (!hWnd_TrayToolbar)
        return -1;

    DWORD  DwTrayPid=0;                               //托盘进程id
    //
    GetWindowThreadProcessId(hWnd_TrayToolbar, &DwTrayPid);              //取得托盘窗口对应的进程id(explorer.exe)
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DwTrayPid); //托盘进程句柄
    if (hProcess != INVALID_HANDLE_VALUE)
    {
        //在目标进程中申请一块内存，放TBBUTTON信息
        PVOID RemoteBuf = 
            VirtualAllocEx(hProcess, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
        if (RemoteBuf)
        {
            TRAYDATA trayData    = {0};
            TBBUTTON btnData     = {0};
            NOTIFYICONDATAA data = {0};

            int count = (int)SendMessage(hWnd_TrayToolbar, TB_BUTTONCOUNT, 0, 0);  //给托盘窗口发消息，得到托盘里图标个数
            for(int i=0; i<count; i++)
            {
                BOOL b = (BOOL)SendMessage(hWnd_TrayToolbar, TB_GETBUTTON, i, (LPARAM)RemoteBuf); //取得TBBUTTON结构到目标进程RemoteBuf
                if (!b)  continue;

                //读TBBUTTON结构到本地
                b = ReadProcessMemory(hProcess, (LPCVOID)RemoteBuf, &btnData, sizeof(TBBUTTON), NULL);
                if (!b)  continue;

                //读dwData字段指向的TRAYDATA结构
                b = ReadProcessMemory(hProcess, (LPCVOID)btnData.dwData, &trayData, sizeof(TRAYDATA), NULL);
                if (!b)  continue;

                DWORD dwProcessId = 0;
                GetWindowThreadProcessId(trayData.hwnd, &dwProcessId);     //通过TRAYDATA里的hwnd字段取得本图标的进程id
                printf("Pid : %d\n", dwProcessId);
                if (!dwProcessId)
                {
                    data.hWnd = trayData.hwnd;
                    data.uID  = trayData.uID;
                    Shell_NotifyIconA(NIM_DELETE, &data);
                }
            }

            VirtualFreeEx(hProcess, RemoteBuf, sizeof(TBBUTTON), MEM_COMMIT);
        }

        CloseHandle(hProcess);
    }

    getchar();
    return 0;
}