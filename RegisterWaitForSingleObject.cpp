
#include <stdio.h>
#define _WIN32_WINNT  0x0502
#include <windows.h>

//bWaitFired��TRUE��ʾ��ʱ��FALSE��ʾ�¼����󱻴���
void CALLBACK WaitCallback(PVOID lpParameter, BOOLEAN bWaitFired)
{
    if (!bWaitFired)
    {
        printf("[ID:0x%X] WaitCallback Success\n", GetCurrentThreadId());
    }
    else
    {
        printf("[ID:0x%X] WaitCallback Failed\n", GetCurrentThreadId());
    }
}

int main()
{
    printf("���߳�[ID:0x%X] Runing\n", GetCurrentThreadId());

    //����һ���¼�����
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); //�Զ�����ʼδ����״̬
    if (NULL == hEvent)
        return -1;

    //ģ��ȴ����
    HANDLE hNewWait = NULL; //���������̳߳ض���

    //WT_EXECUTEONLYONCE����ʾ�ص�����ֻ��ִ��һ�Ρ������ڽ���/�߳̾�����ִ����������õĶ���
    //RegisterWaitForSingleObject(&hNewWait, hEvent, WaitCallback, NULL, INFINITE, WT_EXECUTEDEFAULT);
    RegisterWaitForSingleObject(&hNewWait, hEvent, WaitCallback, NULL, 1000/*INFINITE*/, 0xc);

    for (int i = 0; i < 5; i++)
    {
        Sleep(100);        //�ı����ʱ�䣬���Կ����̳߳ػ���ò�ͬ�߳���ִ�лص�����
        SetEvent(hEvent);  //����5�Σ��ûص�������ִ��5�Σ��������������Ǹ��߳�ִ���˸ú�����
    }

    UnregisterWaitEx(hNewWait, INVALID_HANDLE_VALUE);

    CloseHandle(hEvent);
    getchar();
    return 0;
}