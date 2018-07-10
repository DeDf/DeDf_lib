// 1.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>

#define BOOL int
#define TRUE 1
#define FALSE 0

BOOL SystemShutdown()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    /* OpenProcessToken() 这个函数的作用是打开一个进程的访问令牌
    GetCurrentProcess() 函数的作用是得到本进程的句柄*/
    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hToken))
        return FALSE;
    // LookupPrivilegeValue() 的作用是修改进程的权限
    LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,
        &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1; // one privilege to set 赋给本进程特权
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    // AdjustTokenPrivileges（）的作用是通知Windows NT修改本进程的权利
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
        (PTOKEN_PRIVILEGES)NULL, 0);
    if (GetLastError() != ERROR_SUCCESS) //失败
        return FALSE;
    if (!ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, 0)) //参数在这里设置。强行退出WINDOWS（EWX_FORCE）。
        return FALSE;

    return TRUE; 
}

void main()
{
    SystemShutdown();
}
