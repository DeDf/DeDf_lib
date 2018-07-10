//Client.cpp
#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

int main(int argc, char* argv[])
{
    //判断是否输入了IP地址和端口号
    if(argc!=3){
        printf("Usage: %s IPAddress PortNumber/n",argv[0]);
        exit(-1);
    }
    //把字符串的IP地址转化为u_long
    unsigned long ip;
    if((ip=inet_addr(argv[1]))==INADDR_NONE){
        printf("不合法的IP地址：%s",argv[1]);
        exit(-1);
    }
    //把端口号转化成整数
    short port;
    if((port = atoi(argv[2]))==0){
        printf("端口号有误！");
        exit(-1);
    }
    printf("Connecting to %s:%d....../n",inet_ntoa(*(in_addr*)&ip),port);
    WSADATA wsa;
    //初始化套接字DLL
    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){
        printf("套接字初始化失败!");
        exit(-1);
    }
    //创建套接字
    SOCKET sock;
    if((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET){
        printf("创建套接字失败！");
        exit(-1);
    }
    struct sockaddr_in serverAddress;
    memset(&serverAddress,0,sizeof(sockaddr_in));
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_addr.S_un.S_addr = ip;
    serverAddress.sin_port = htons(port);
    //建立和服务器的连接
    if(connect(sock,(sockaddr*)&serverAddress,sizeof(serverAddress))==SOCKET_ERROR){
        printf("建立连接失败！");
        exit(-1);
    }
    char buf[4096];
    while(1){
        printf(">");
        //从控制台读取一行数据
        gets_s(buf, sizeof(buf));
        //发送给服务器
        if(send(sock,buf,(int)strlen(buf),0)==SOCKET_ERROR){
            printf("发送数据失败！");
            exit(-1);
        }
        int bytes;
        if((bytes=recv(sock,buf,sizeof(buf),0))==SOCKET_ERROR){
            printf("接收数据失败!/n");
            exit(-1);
        }
        buf[bytes]=0;
        printf("Message from %s: %s/n",inet_ntoa(serverAddress.sin_addr),buf);
    }
    //清理套接字占用的资源
    WSACleanup();
    return 0;
}