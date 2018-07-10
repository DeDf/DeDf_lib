
#include <stdio.h>
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib,"Ws2_32.lib")

#define ACCEPT      1000 
#define RECV_POSTED 1001
#define SEND_POSTED 1002

LPFN_ACCEPTEX     pAcceptEx;         // AcceptEx����ָ��
/*
*�ṹ�����ƣ�PER_IO_DATA
*�ṹ�幦�ܣ��ص�I/O��Ҫ�õ��Ľṹ�壬��ʱ��¼IO����
*/
typedef struct
{
    OVERLAPPED Overlapped;
    WSABUF DataBuff;
    int  OperationType;
    SOCKET client;
    char buffer[2048];
    
} PER_IO_DATA, *PPER_IO_DATA;

//�����߳�����
DWORD WINAPI ServerWorkerThread(LPVOID lpThreadParameter)
{
    HANDLE hCompletionPort = (HANDLE)lpThreadParameter;
    DWORD BytesTransferred;
    ULONG_PTR lpCompletionKey;
    PPER_IO_DATA PerIoData;
    SOCKET s, sClient;
    DWORD SendBytes,RecvBytes;
    DWORD Flags;

    while(1)
    {
        //�ȴ���ɶ˿���SOCKET�����
        GetQueuedCompletionStatus(hCompletionPort,
            &BytesTransferred,
            (LPDWORD)&lpCompletionKey,
            (LPOVERLAPPED *)&PerIoData,
            INFINITE);
        s = (SOCKET)lpCompletionKey;

        //����Ƿ��д������
        if(BytesTransferred == 0 &&
            (PerIoData->OperationType == RECV_POSTED ||
             PerIoData->OperationType == SEND_POSTED))
        {
            //�ر�SOCKET
            printf(" SOCKET [%p] close ><\n", s);
            closesocket(s);
            free(PerIoData);
            continue;
        }

        //����AcceptEx����������ɣ������洦��  
        if(PerIoData->OperationType == ACCEPT)     //�������Ӳ���  
        {
            sClient = PerIoData->client;
            // --------------------------Ͷ��AcceptEx---------------------------
            memset(&(PerIoData->Overlapped),0,sizeof(OVERLAPPED));
            //
            PerIoData->OperationType = ACCEPT;
            PerIoData->DataBuff.buf = PerIoData->buffer;
            PerIoData->DataBuff.len = sizeof(PerIoData->buffer);
            PerIoData->client = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
            //
            //����AcceptEx��������ַ������Ҫ��ԭ�е��������16���ֽ�
            pAcceptEx(s,
                PerIoData->client,
                PerIoData->buffer,  
                PerIoData->DataBuff.len-((sizeof(SOCKADDR_IN)+16)*2),
                sizeof(SOCKADDR_IN)+16,
                sizeof(SOCKADDR_IN)+16,
                &RecvBytes,  
                &(PerIoData->Overlapped));
            // ---------------------------����ACCEPT---------------------------
            printf("SOCKET [%p] Accept ~\n", sClient);

            //ʹ��GetAcceptExSockaddrs���� ��þ���ĸ�����ַ����.  
            setsockopt( sClient, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(s), sizeof(s) );

            //���µĿͻ��׽�������ɶ˿�����  
            CreateIoCompletionPort((HANDLE)sClient, hCompletionPort, (ULONG_PTR)sClient, 0);  

            PerIoData = (PPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
            if (PerIoData)
            {
                s = sClient;
                goto L_send;
            }
            else
            {
                printf("malloc() failed\n");
            }
        }
        else if(PerIoData->OperationType == RECV_POSTED)   // ������ɴ���
        {
L_send:     //��Ӧ�ͻ���
            ZeroMemory((LPVOID)&(PerIoData->Overlapped),sizeof(OVERLAPPED));
            PerIoData->OperationType = SEND_POSTED;
            strcpy_s(PerIoData->buffer, sizeof(PerIoData->buffer), "OK");
            PerIoData->DataBuff.buf = PerIoData->buffer;
            PerIoData->DataBuff.len = sizeof("OK");
            WSASend(s,&PerIoData->DataBuff,
                1,&SendBytes,0,&(PerIoData->Overlapped),NULL);
        }
        else if(PerIoData->OperationType == SEND_POSTED)   // ������ɴ���
        {
            //����ʱ�Ĵ���
            Flags = 0;
            ZeroMemory((LPVOID)&(PerIoData->Overlapped),sizeof(OVERLAPPED));
            PerIoData->OperationType = RECV_POSTED;
            PerIoData->DataBuff.buf = PerIoData->buffer;
            PerIoData->DataBuff.len = sizeof(PerIoData->buffer);
            WSARecv(s,&PerIoData->DataBuff,
                1,&RecvBytes,&Flags,&(PerIoData->Overlapped),NULL);
        }
    }

    return 0;
}

int main()
{
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData))
    {
        printf("WSAStartup() Error!\n");
        return -1;
    }

    HANDLE hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
    if(hCompletionPort == INVALID_HANDLE_VALUE)
    {
        printf("CreateIoCompletionPort() Error!\n");
        return -1;
    }

    // Determine how many processors are on the system.
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    // Create two worker threads for each processor.
    for(ULONG i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++)
    {
        HANDLE hThread =
            CreateThread(NULL, 0, ServerWorkerThread, (LPVOID)hCompletionPort, 0, NULL);
        printf("CreateThread [%d]~\n", i);
        CloseHandle(hThread);
    }

    //�����������׽��֣�����Ҫע��������һ����������Ϊ��WSA_FLAG_OVERLAPPED�ص�ģʽ
    SOCKET sockListen=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
    if(sockListen == SOCKET_ERROR)
    {
        printf("WSASocket() Error!\n");
        return -1;
    }

    GUID GuidAcceptEx = WSAID_ACCEPTEX;  // GUID�������ʶ��AcceptEx���������  
    DWORD dwBytes = 0;    
   if (WSAIoctl(                         // ��ȡAcceptEx����ָ��
        sockListen,   
        SIO_GET_EXTENSION_FUNCTION_POINTER,   
        &GuidAcceptEx,   
        sizeof(GuidAcceptEx),   
        &pAcceptEx,   
        sizeof(pAcceptEx),   
        &dwBytes,   
        NULL,   
        NULL))
    {
        printf("WSAIoctl() Error!\n");
        return -1;
    }

    int nReuseAddr = 1;
    if(setsockopt(sockListen,SOL_SOCKET,SO_REUSEADDR,(const char *)&nReuseAddr,sizeof(int)) != 0)
    {
        printf("setsockopt() Error!\n");
        return -1;
    }

    //�������׽�������ɶ˿ڰ�  
    CreateIoCompletionPort((HANDLE)sockListen,hCompletionPort,(ULONG_PTR)sockListen,0);  

    //֮��Ϊ�׽��ְ�һ�����ض˿ڣ����������ͻ��˵�����
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(2000);
    //
    if( bind(sockListen,(struct sockaddr *)&addr,sizeof(sockaddr_in)) )
    {
        printf("bind() Error!\n");
        return -1;
    }

    if( listen(sockListen, SOMAXCONN) )
    {
        printf("listen() Error!\n");
        return -1;
    }

    PPER_IO_DATA PerIoData = (PPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
    if (PerIoData)
    {
        memset(&(PerIoData->Overlapped),0,sizeof(OVERLAPPED));
        
        PerIoData->OperationType = ACCEPT;
        PerIoData->DataBuff.buf = PerIoData->buffer;
        PerIoData->DataBuff.len = sizeof(PerIoData->buffer);
        PerIoData->client = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);

        //����AcceptEx��������ַ������Ҫ��ԭ�е��������16���ֽ�
        pAcceptEx(sockListen,
            PerIoData->client,
            PerIoData->buffer,  
            PerIoData->DataBuff.len-((sizeof(SOCKADDR_IN)+16)*2),
            sizeof(SOCKADDR_IN)+16,
            sizeof(SOCKADDR_IN)+16,
            &dwBytes,  
            &(PerIoData->Overlapped));
    }

    Sleep(INFINITE);
    closesocket(sockListen);  
    WSACleanup(); 
    return 0;
}

