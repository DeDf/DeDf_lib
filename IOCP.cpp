
#include <stdio.h>
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib,"Ws2_32.lib")

#define ACCEPT      1000 
#define RECV_POSTED 1001
#define SEND_POSTED 1002

LPFN_ACCEPTEX     pAcceptEx;         // AcceptEx函数指针
/*
*结构体名称：PER_IO_DATA
*结构体功能：重叠I/O需要用到的结构体，临时记录IO数据
*/
typedef struct
{
    OVERLAPPED Overlapped;
    WSABUF DataBuff;
    int  OperationType;
    SOCKET client;
    char buffer[2048];
    
} PER_IO_DATA, *PPER_IO_DATA;

//工作线程流程
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
        //等待完成端口上SOCKET的完成
        GetQueuedCompletionStatus(hCompletionPort,
            &BytesTransferred,
            (LPDWORD)&lpCompletionKey,
            (LPOVERLAPPED *)&PerIoData,
            INFINITE);
        s = (SOCKET)lpCompletionKey;

        //检查是否有错误产生
        if(BytesTransferred == 0 &&
            (PerIoData->OperationType == RECV_POSTED ||
             PerIoData->OperationType == SEND_POSTED))
        {
            //关闭SOCKET
            printf(" SOCKET [%p] close ><\n", s);
            closesocket(s);
            free(PerIoData);
            continue;
        }

        //这是AcceptEx函数处理完成，在下面处理  
        if(PerIoData->OperationType == ACCEPT)     //处理连接操作  
        {
            sClient = PerIoData->client;
            // --------------------------投递AcceptEx---------------------------
            memset(&(PerIoData->Overlapped),0,sizeof(OVERLAPPED));
            //
            PerIoData->OperationType = ACCEPT;
            PerIoData->DataBuff.buf = PerIoData->buffer;
            PerIoData->DataBuff.len = sizeof(PerIoData->buffer);
            PerIoData->client = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
            //
            //调用AcceptEx函数，地址长度需要在原有的上面加上16个字节
            pAcceptEx(s,
                PerIoData->client,
                PerIoData->buffer,  
                PerIoData->DataBuff.len-((sizeof(SOCKADDR_IN)+16)*2),
                sizeof(SOCKADDR_IN)+16,
                sizeof(SOCKADDR_IN)+16,
                &RecvBytes,  
                &(PerIoData->Overlapped));
            // ---------------------------处理ACCEPT---------------------------
            printf("SOCKET [%p] Accept ~\n", sClient);

            //使用GetAcceptExSockaddrs函数 获得具体的各个地址参数.  
            setsockopt( sClient, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(s), sizeof(s) );

            //将新的客户套接字与完成端口连接  
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
        else if(PerIoData->OperationType == RECV_POSTED)   // 接收完成处理
        {
L_send:     //回应客户端
            ZeroMemory((LPVOID)&(PerIoData->Overlapped),sizeof(OVERLAPPED));
            PerIoData->OperationType = SEND_POSTED;
            strcpy_s(PerIoData->buffer, sizeof(PerIoData->buffer), "OK");
            PerIoData->DataBuff.buf = PerIoData->buffer;
            PerIoData->DataBuff.len = sizeof("OK");
            WSASend(s,&PerIoData->DataBuff,
                1,&SendBytes,0,&(PerIoData->Overlapped),NULL);
        }
        else if(PerIoData->OperationType == SEND_POSTED)   // 发送完成处理
        {
            //发送时的处理
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

    //创建服务器套接字，这里要注意的是最后一个参数必须为：WSA_FLAG_OVERLAPPED重叠模式
    SOCKET sockListen=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
    if(sockListen == SOCKET_ERROR)
    {
        printf("WSASocket() Error!\n");
        return -1;
    }

    GUID GuidAcceptEx = WSAID_ACCEPTEX;  // GUID，这个是识别AcceptEx函数必须的  
    DWORD dwBytes = 0;    
   if (WSAIoctl(                         // 获取AcceptEx函数指针
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

    //将监听套接字与完成端口绑定  
    CreateIoCompletionPort((HANDLE)sockListen,hCompletionPort,(ULONG_PTR)sockListen,0);  

    //之后为套接字绑定一个本地端口，用来监听客户端的连接
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

        //调用AcceptEx函数，地址长度需要在原有的上面加上16个字节
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

