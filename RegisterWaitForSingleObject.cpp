
#include <stdio.h>
#define _WIN32_WINNT  0x0502
#include <windows.h>

//bWaitFired：TRUE表示超时，FALSE表示事件对象被触发
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
    printf("主线程[ID:0x%X] Runing\n", GetCurrentThreadId());

    //创建一个事件对象
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); //自动，初始未触发状态
    if (NULL == hEvent)
        return -1;

    //模拟等待五次
    HANDLE hNewWait = NULL; //用来保存线程池对象

    //WT_EXECUTEONLYONCE：表示回调函数只被执行一次。适用于进程/线程句柄这种触发后不再重置的对象
    //RegisterWaitForSingleObject(&hNewWait, hEvent, WaitCallback, NULL, INFINITE, WT_EXECUTEDEFAULT);
    RegisterWaitForSingleObject(&hNewWait, hEvent, WaitCallback, NULL, 1000/*INFINITE*/, 0xc);

    for (int i = 0; i < 5; i++)
    {
        Sleep(100);        //改变这个时间，可以看到线程池会调用不同线程来执行回调函数
        SetEvent(hEvent);  //触发5次，让回调函数被执行5次（但并不关心是那个线程执行了该函数）
    }

    UnregisterWaitEx(hNewWait, INVALID_HANDLE_VALUE);

    CloseHandle(hEvent);
    getchar();
    return 0;
}