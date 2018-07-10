
#include <stdio.h>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

#define MAXDATASIZE 100

char get[] = {"GET /index.html HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n"};

int main()
{
    SOCKET sock;
    struct sockaddr_in server;
    char *pchIP = "61.135.169.125";
    USHORT port = 80;

    getchar();
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
    {
        printf("套接字初始化失败!\n");
        exit(-1);
    }

    if((sock=socket(AF_INET,SOCK_STREAM, 0))==-1)
    {
        printf("socket() error\n");
        exit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(pchIP);
    server.sin_port = htons(port);

    WSAEVENT e = WSACreateEvent();
    //WSAEventSelect(sock, e, FD_CONNECT | FD_READ);
    WSAEventSelect(sock, e, FD_CONNECT);
    if(connect(sock, (struct sockaddr *)&server, sizeof(server))==-1)
    {
        if(WSAGetLastError() == WSAEWOULDBLOCK)
        {
            printf("connect() asynchronous ~\n");
        }
        else
        {
            printf("connect() error\n");
            getchar();
            return -1;
        }
    }

    ULONG ret = WaitForSingleObject(e, 2000);
    if (ret)
    {
        printf("WaitForSingleObject() return 0x%08X !\n", ret);
    }

    send(sock, get, sizeof(get), 0);
    char buf[1024];
    int buflen = sizeof(buf);
    int sum = 0, num;
    ULONG WriteSum = 0;

    HANDLE pFile = CreateFileW(L"d:\\1.txt",
        GENERIC_WRITE,          
        0,
        NULL,               
        CREATE_ALWAYS,        //总是创建文件
        FILE_ATTRIBUTE_NORMAL, 
        NULL);

    if ( pFile == INVALID_HANDLE_VALUE)
    {
        printf("create file error!\n");
        CloseHandle(pFile);
        return FALSE;
    }

    WSAEventSelect(sock, e, FD_READ);

    while(1)
    {
        ResetEvent(e);
        num = recv(sock, buf, sizeof(buf), 0);
        printf(" %d\n", num);
        if (num)
        {
            if (num == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
            {
                printf("recv() asynchronous ~\n");
                ret = WaitForSingleObject(e, 2000);
                if (ret)
                {
                    printf("WaitForSingleObject() return 0x%08X !\n", ret);
                    break;
                }
            }
            else if (num > 0)
            {
                ULONG dwBytesWrite;
                WriteFile(pFile,buf,num,&dwBytesWrite,NULL);
                WriteSum += dwBytesWrite;
            }
        }
    }

    CloseHandle(pFile);
    printf("recv Sum : %d, WriteSum : %d\n", sum, WriteSum);
    getchar();
    return 0;
}