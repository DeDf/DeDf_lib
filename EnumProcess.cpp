
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
    WCHAR wchProcName[]={L"notepad.exe"}; //�����ַ�������ʼ��

    HANDLE hProc;  //����Ҫ�������̾��

    HANDLE handle=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//���ϵͳ���վ��

    PROCESSENTRY32 *info = new PROCESSENTRY32;
    info->dwSize=sizeof(PROCESSENTRY32);

    //����һ�� Process32First �������ӿ����л�ȡ�����б�
    Process32First(handle,info);

    //�ظ����� Process32Next��ֱ���������� FALSE Ϊֹ
    while(Process32Next(handle,info)!=FALSE)
    {
        info->szExeFile;     //ָ�������

        printf("Pid : %d, %ws\n", info->th32ProcessID, info->szExeFile);
//         if( wcsicmp(wchProcName,info->szExeFile) == 0 )
//         {
//             //PROCESS_TERMINATE��ʾΪ����������,FALSE=�ɼ̳�,info->th32ProcessID=����ID
//             hProc=OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID);
// 
//             //��������
//             TerminateProcess(hProc, 0);
//         }
    }

    CloseHandle(handle);
    getchar();
    return 0;
}

