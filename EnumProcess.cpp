
#include <windows.h>
#include <stdio.h>
#include <TlHelp32.h>

// void AdjustPrivilege()  
// {  
//     HANDLE hToken;  
//     if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))  
//     {  
//         TOKEN_PRIVILEGES tp;  
//         tp.PrivilegeCount = 1;  
//         tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  
//         if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid))  
//         {  
//             AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);  
//         }  
//         CloseHandle(hToken);  
//     }  
// } 

int main()
{
    WCHAR wchProcName[]={L"notepad.exe"}; //定义字符串并初始化

    HANDLE hProc;  //定义要结束进程句柄

    HANDLE handle=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//获得系统快照句柄

    PROCESSENTRY32 *info = new PROCESSENTRY32;
    info->dwSize=sizeof(PROCESSENTRY32);

    //调用一次 Process32First 函数，从快照中获取进程列表
    Process32First(handle,info);

    //重复调用 Process32Next，直到函数返回 FALSE 为止
    while(Process32Next(handle,info)!=FALSE)
    {
        info->szExeFile;     //指向进程名

        printf("Pid : %d, %ws\n", info->th32ProcessID, info->szExeFile);
//         if( wcsicmp(wchProcName,info->szExeFile) == 0 )
//         {
//             //PROCESS_TERMINATE表示为结束操作打开,FALSE=可继承,info->th32ProcessID=进程ID
//             hProc=OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID);
// 
//             //结束进程
//             TerminateProcess(hProc, 0);
//         }
    }

    CloseHandle(handle);
    getchar();
    return 0;
}

