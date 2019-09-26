
#include <windows.h>
#include <NTSecAPI.h>

#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define MESSAGE_LENGTH		0x100
#define MAX_COMMUN_NUMBER	10

typedef struct _CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _PORT_MESSAGE
{
    union
    {
        struct
        {
            USHORT DataLength;          // Length of data following the header (bytes)
            USHORT TotalLength;         // Length of data + sizeof(PORT_MESSAGE)
        } s1;
        ULONG Length;
    } u1;

    union
    {
        struct
        {
            USHORT Type;
            USHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;

    union
    {
        CLIENT_ID ClientId;
        double   DoNotUseThisField;     // Force quadword alignment
    };

    ULONG  MessageId;                   // Identifier of the particular message instance

    union
    {
        ULONG_PTR ClientViewSize;       // Size of section created by the sender (in bytes)
        ULONG  CallbackId;              // 
    };

} PORT_MESSAGE, *PPORT_MESSAGE;

typedef struct _TRANSFERED_MESSAGE
{
    PORT_MESSAGE Header;
    WCHAR   MessageText[MESSAGE_LENGTH];

} TRANSFERED_MESSAGE, *PTRANSFERED_MESSAGE;

#define InitializeMessageHeader(ph, l, t)                              \
{                                                                      \
    (ph)->u1.s1.TotalLength      = (USHORT)(l);                        \
    (ph)->u1.s1.DataLength       = (USHORT)(l - sizeof(PORT_MESSAGE)); \
    (ph)->u2.s2.Type             = (USHORT)(t);                        \
    (ph)->u2.s2.DataInfoOffset   = 0;                                  \
    (ph)->ClientId.UniqueProcess = NULL;                               \
    (ph)->ClientId.UniqueThread  = NULL;                               \
    (ph)->MessageId              = 0;                                  \
    (ph)->ClientViewSize         = 0;                                  \
}

#define InitializeObjectAttributes( p, n, a, r, s ) {   \
    (p)->Length = sizeof( LSA_OBJECT_ATTRIBUTES );      \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
}

NTSTATUS(NTAPI* NtCreatePort)(PHANDLE PortHandle, PLSA_OBJECT_ATTRIBUTES ObjectAttributes, ULONG MaxConnectionInfoLength, ULONG MaxMessageLength, PULONG MaxPoolUsage);
NTSTATUS(NTAPI* NtAcceptConnectPort)(PHANDLE PortHandle, PVOID PortContext, PPORT_MESSAGE ConnectionRequest, BOOL AcceptConnection, PVOID ServerView, PVOID ClientView);
NTSTATUS(NTAPI* NtCompleteConnectPort)(HANDLE PortHandle);
NTSTATUS(NTAPI* NtReplyWaitReceivePort)(HANDLE PortHandle, PVOID* PortContext, PPORT_MESSAGE ReplyMessage, PPORT_MESSAGE ReceiveMessage);
NTSTATUS(NTAPI* NtRequestWaitReplyPort)(HANDLE PortHandle, PPORT_MESSAGE RequestMessage, PPORT_MESSAGE ReplyMessage);
NTSTATUS(NTAPI* RtlInitUnicodeString)(PUNICODE_STRING DestinationString, PCWSTR SourceString);
NTSTATUS(NTAPI* NtListenPort)(HANDLE PortHandle, PPORT_MESSAGE ConnectionRequest);
NTSTATUS(NTAPI* NtReplyPort)(HANDLE PortHandle, PPORT_MESSAGE ReplyMessage);
NTSTATUS(NTAPI* NtClose)(HANDLE Handle);
NTSTATUS(NTAPI* NtConnectPort)(PHANDLE PortHandle, PUNICODE_STRING PortName, PSECURITY_QUALITY_OF_SERVICE SecurityQos, PVOID ClientView, PVOID ServerView, PULONG MaxMessageLength, PVOID ConnectionInformation, PULONG ConnectionInformationLength);
NTSTATUS(NTAPI* NtRequestPort)(HANDLE PortHandle, PPORT_MESSAGE RequestMessage);

void InitLpc()
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll)
    {
        (FARPROC&)NtCreatePort = GetProcAddress(hNtdll, "NtCreatePort");
        (FARPROC&)NtAcceptConnectPort = GetProcAddress(hNtdll, "NtAcceptConnectPort");
        (FARPROC&)NtCompleteConnectPort = GetProcAddress(hNtdll, "NtCompleteConnectPort");
        (FARPROC&)NtReplyWaitReceivePort = GetProcAddress(hNtdll, "NtReplyWaitReceivePort");
        (FARPROC&)NtRequestWaitReplyPort = GetProcAddress(hNtdll, "NtRequestWaitReplyPort");
        (FARPROC&)RtlInitUnicodeString = GetProcAddress(hNtdll, "RtlInitUnicodeString");
        (FARPROC&)NtListenPort = GetProcAddress(hNtdll, "NtListenPort");
        (FARPROC&)NtReplyPort = GetProcAddress(hNtdll, "NtReplyPort");
        (FARPROC&)NtClose = GetProcAddress(hNtdll, "NtClose");
        (FARPROC&)NtConnectPort = GetProcAddress(hNtdll, "NtConnectPort");
        (FARPROC&)NtRequestPort = GetProcAddress(hNtdll, "NtRequestPort");
    }
}