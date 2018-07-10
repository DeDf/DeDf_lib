
#include <Windows.h>

void DeleteSelfDir(HANDLE *hProcess, HANDLE *hThread)
{
    WCHAR szCmdOrder[MAX_PATH];
    wcscpy(szCmdOrder, L"cmd /c \"del ");

    WCHAR buf[MAX_PATH];
    GetModuleFileName(NULL, buf, MAX_PATH);
    wcscat(szCmdOrder, buf);
    wcscat(szCmdOrder, L" & rd /q ");

    WCHAR *t = wcsrchr(buf, L'\\');
    *t = L'\"';
    t[1] = 0;
    wcscat(szCmdOrder, buf);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = TRUE;
    BOOL bRet = CreateProcessW(
        NULL,
        szCmdOrder,
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE | CREATE_SUSPENDED, //ѡ��������߳�
        NULL,
        NULL,
        &si,
        &pi);

    if (!bRet)
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return;
    }

    *hProcess = pi.hProcess;
    *hThread = pi.hThread;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR szCmdLine, int iCmdShow)
{
    if (!strcmp(szCmdLine, "-uninstall"))
    {
        HANDLE hProcess;
        HANDLE hThread;
        DeleteSelfDir(&hProcess,&hThread);

        SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS);  //���ÿ���ʱ��ִ��

        //���ñ���������ִ��
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
        SetPriorityClass(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);


        if (!ResumeThread(hThread))  //������ִ����Ϻ�ʼִ�����߳�
        {
            //printf("ResumeThread %d\n", GetLastError());
        }

        CloseHandle(hProcess);
        CloseHandle(hThread);

        return 0;
    }

    return 0;
}