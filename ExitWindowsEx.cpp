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
    /* OpenProcessToken() ��������������Ǵ�һ�����̵ķ�������
    GetCurrentProcess() �����������ǵõ������̵ľ��*/
    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hToken))
        return FALSE;
    // LookupPrivilegeValue() ���������޸Ľ��̵�Ȩ��
    LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,
        &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1; // one privilege to set ������������Ȩ
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    // AdjustTokenPrivileges������������֪ͨWindows NT�޸ı����̵�Ȩ��
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
        (PTOKEN_PRIVILEGES)NULL, 0);
    if (GetLastError() != ERROR_SUCCESS) //ʧ��
        return FALSE;
    if (!ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, 0)) //�������������á�ǿ���˳�WINDOWS��EWX_FORCE����
        return FALSE;

    return TRUE; 
}

void main()
{
    SystemShutdown();
}
