
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>

struct TRAYDATA
{
    HWND hwnd;
    ULONG Reseved;  // x64 alignment
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

typedef struct _TBBUTTON64 {
    int iBitmap;
    int idCommand;
    BYTE fsState;
    BYTE fsStyle;
    BYTE bReserved[6];          // padding for alignment : X86-2Bytes, x64-6Bytes
    DWORD_PTR dwData;
    INT_PTR iString;
} TBBUTTON64;

int main()
{
    HWND hWnd_TrayToolbar = FindTrayToolbarWindow();  //托盘窗口句柄
    DWORD  DwTrayPid=0;                               //托盘进程id
    //
    GetWindowThreadProcessId(hWnd_TrayToolbar, &DwTrayPid);              //取得托盘窗口对应的进程id(explorer.exe)
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DwTrayPid); //托盘进程句柄
    if (hProcess != INVALID_HANDLE_VALUE)
    {
        //在目标进程中申请一块内存，放TBBUTTON信息
        LPVOID address = 
            VirtualAllocEx(hProcess, NULL, sizeof(TBBUTTON64), MEM_COMMIT, PAGE_READWRITE);

        if (address)
        {
            TRAYDATA trayData    = {0};
            TBBUTTON64 btnData   = {0};
            NOTIFYICONDATAA data = {0};

            int count = (int)SendMessage(hWnd_TrayToolbar, TB_BUTTONCOUNT, 0, 0); //给托盘窗口发消息，得到托盘里图标个数
            for(int i=0; i<count; i++)
            {
                SendMessage(hWnd_TrayToolbar, TB_GETBUTTON, i, (LPARAM)address);  //取得TBBUTTON结构到本地
                ReadProcessMemory(hProcess,
                    (LPCVOID)address,
                    &btnData,
                    sizeof(TBBUTTON64),
                    NULL);
                ReadProcessMemory(hProcess,   //从目标进程address处存放的是TBBUTTON
                    (LPCVOID)btnData.dwData,  //取dwData字段指向的TRAYDATA结构
                    &trayData,
                    sizeof(TRAYDATA),
                    NULL);

                DWORD dwProcessId = 0;
                GetWindowThreadProcessId(trayData.hwnd, &dwProcessId);  //通过TRAYDATA里的hwnd字段取得本图标的进程id
                printf("Pid : %d\n", dwProcessId);
                if (!dwProcessId)
                {
                    data.hWnd = trayData.hwnd;
                    data.uID  = trayData.uID;
                    Shell_NotifyIconA(NIM_DELETE, &data);
                }
            }

            VirtualFreeEx(hProcess, address, sizeof(TBBUTTON64), MEM_COMMIT);
        }

        CloseHandle(hProcess);
    }

    getchar();
    return 0;
}