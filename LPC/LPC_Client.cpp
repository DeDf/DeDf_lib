
#include <stdio.h>
#include "LPC.h"

HANDLE ConnectPortW(WCHAR *pwchPortName)
{
    HANDLE hPort = NULL;

    UNICODE_STRING usPortName;
    RtlInitUnicodeString(&usPortName, pwchPortName);

    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    SecurityQos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQos.ImpersonationLevel = SecurityImpersonation;
    SecurityQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQos.EffectiveOnly = FALSE;

    ULONG MaxMessageLength;
    NtConnectPort(&hPort, &usPortName, &SecurityQos, NULL, NULL, &MaxMessageLength, NULL, NULL);

    if (MaxMessageLength < sizeof(PORT_MESSAGE) + MESSAGE_LENGTH)
    {
        NtClose(hPort);
        hPort = NULL;
    }

    return hPort;
}

int main()
{
    InitLpc();

    WCHAR *wchPortName = L"\\PortTest";
    HANDLE hPort = ConnectPortW(wchPortName);
    if (hPort)
    {
        printf("ConnectPortW(%ws) success~\n", wchPortName);
        
        NTSTATUS status = 0;
        TRANSFERED_MESSAGE LPCRecvMsg = { 0 };
        TRANSFERED_MESSAGE LPCSendMsg = { 0 };

        InitializeMessageHeader(&LPCRecvMsg.Header, sizeof(TRANSFERED_MESSAGE), 0);
        InitializeMessageHeader(&LPCSendMsg.Header, sizeof(TRANSFERED_MESSAGE), 0);
        status = NtReplyWaitReceivePort(hPort, NULL, NULL, &LPCRecvMsg.Header);

        while (!status)
        {
            LPCSendMsg.Header = LPCRecvMsg.Header;

            printf("scanf:");
            scanf_s("%ws", LPCSendMsg.MessageText, MESSAGE_LENGTH);
            status = NtReplyWaitReceivePort(hPort, NULL, &LPCSendMsg.Header, &LPCRecvMsg.Header);
        }
    }

    return 0;
}