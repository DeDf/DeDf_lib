#include <windows.h>
#include <tlhelp32.h>

//֪ͨ������PE�ļ�Ҫ����TLSĿ¼
#pragma comment(linker, "/INCLUDE:__tls_used")

void lookupprocess(void);
void Debugger(void);

void NTAPI tls_callback(PVOID h, DWORD reason, PVOID pv)
{
//     lookupprocess();
//     Debugger();
    MessageBoxA(NULL, "NotMain", "Test1", MB_OK);
}

//����TLS��
#pragma data_seg(".CRT$XLB")
//����ص�����
PIMAGE_TLS_CALLBACK p_thread_callback = tls_callback;
#pragma data_seg()

int main()
{
    MessageBoxA(NULL,"Main","Test1",MB_OK);
    return 0;
}

//anti-debug1���̱���
void lookupprocess()
{
    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(pe32);
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    BOOL bMore=Process32First(hProcessSnap,&pe32);
    while(bMore)
    {
        strlwr(pe32.szExeFile);
        if(!strcmp(pe32.szExeFile,"ollyice.exe"))
        {
            exit(0);
        }

        bMore=Process32Next(hProcessSnap,&pe32);
    }

    CloseHandle(hProcessSnap);
}

//anti-debug2
void Debugger(void)
{
    int result = 0;
    __asm
    {
        mov   eax, dword ptr fs:[30h]//TEBƫ��30H��
        movzx eax, byte ptr ds:[eax+2h]//ȡPEB��BeingDebug����Ϊ1�򱻵���
        mov result,eax
    }
    
        if(result)
            exit(0);
}