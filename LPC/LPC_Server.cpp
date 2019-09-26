
#include "LPC.h"
#include <stdio.h>

BOOL g_Stop;
volatile LONG g_dwCurrentClient;

DWORD WINAPI CommunicationThread(LPVOID lParam)
{
    NTSTATUS status = 0;
    HANDLE hAcceptPort = (HANDLE)lParam;
    TRANSFERED_MESSAGE RecvMsg;
    TRANSFERED_MESSAGE SendMsg = { 0 };
    InitializeMessageHeader(&SendMsg.Header, sizeof(TRANSFERED_MESSAGE), 0);
    
    InterlockedAdd(&g_dwCurrentClient, 1);

    while (!g_Stop)
    {
        printf("\r\n");
        ZeroMemory(&RecvMsg, sizeof(TRANSFERED_MESSAGE));
        status = NtRequestWaitReplyPort(hAcceptPort, &SendMsg.Header, &RecvMsg.Header);
        if (status)
        {
            printf("NtRequestWaitReplyPort:%d\r\n", GetLastError());
            NtClose(hAcceptPort);
            break;
        }

        printf("MessageType:%d\r\nClientID<%d,%d>\r\nTotalLength:%d\r\nMessage:%ws\r\n",
            RecvMsg.Header.u2.s2.Type,
            (ULONG_PTR)RecvMsg.Header.ClientId.UniqueProcess,
            (ULONG_PTR)RecvMsg.Header.ClientId.UniqueThread,
            RecvMsg.Header.u1.s1.TotalLength,
            RecvMsg.MessageText);
    }
    
    InterlockedAdd(&g_dwCurrentClient, -1);
    return 0;
}

NTSTATUS StartServer(WCHAR *pwchServerPortName)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    HANDLE hPort = NULL;

    SECURITY_DESCRIPTOR SecurityDesc = { 0 };
    if (!InitializeSecurityDescriptor(&SecurityDesc, SECURITY_DESCRIPTOR_REVISION))
    {
        printf("InitializeSecurityDescriptor:%d\r\n", GetLastError());
        return status;
    }

    if (!SetSecurityDescriptorDacl(&SecurityDesc, TRUE, NULL, FALSE))
    {
        printf("SetSecurityDescriptorDacl:%d\r\n", GetLastError());
        return status;
    }

    UNICODE_STRING usServerPortName;
    RtlInitUnicodeString(&usServerPortName, pwchServerPortName);

    LSA_OBJECT_ATTRIBUTES ObjectAttribute;
    InitializeObjectAttributes(&ObjectAttribute, &usServerPortName, 0, NULL, &SecurityDesc);

    status = NtCreatePort(&hPort, &ObjectAttribute, NULL, sizeof(TRANSFERED_MESSAGE), 0);
    if (status)
        return status;

    BOOL        bOk = TRUE;
    HANDLE      hAcceptPort = NULL;
    TRANSFERED_MESSAGE	LPCRequertMessage;

    while (1)
    {
        // 清空RecvMsg 监听等待连接
        RtlZeroMemory(&LPCRequertMessage, sizeof(TRANSFERED_MESSAGE));
        status = NtListenPort(hPort, &LPCRequertMessage.Header);
        if (status)
        {
            printf("NtListenPort:%d\r\n", GetLastError());
            break;
        }

        // 打印客户进程的一些信息
        printf("MessageType:%d\r\nClientID<%d,%d>\r\nTotalLength:%d\r\n",
            LPCRequertMessage.Header.u2.s2.Type,
            (ULONG_PTR)LPCRequertMessage.Header.ClientId.UniqueProcess,
            (ULONG_PTR)LPCRequertMessage.Header.ClientId.UniqueThread,
            LPCRequertMessage.Header.u1.s1.TotalLength);

        // 判断是否允许连接
        if (g_dwCurrentClient < MAX_COMMUN_NUMBER)
        {
            bOk = TRUE;
        }
        else
        {
            bOk = FALSE;
        }

        // 处理连接请求 - 根据bOk决定是否接受连接
        status = NtAcceptConnectPort(&hAcceptPort, 0, &LPCRequertMessage.Header, bOk, NULL, NULL);
        if (status)
        {
            printf("SetSecurityDescriptorDacl:%d\r\n", GetLastError());
            break;
        }

        // 完成Connect
        status = NtCompleteConnectPort(hAcceptPort);
        if (status)
        {
            printf("SetSecurityDescriptorDacl:%d\r\n", GetLastError());
            break;
        }

        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CommunicationThread, hAcceptPort, 0, NULL);
    }

    g_Stop = TRUE;
    if (hPort)
        NtClose(hPort);
    return status;
}

int main()
{
    InitLpc();
    StartServer(L"\\PortTest");

    return 0;
}