//Client.cpp
#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

int main(int argc, char* argv[])
{
    //�ж��Ƿ�������IP��ַ�Ͷ˿ں�
    if(argc!=3){
        printf("Usage: %s IPAddress PortNumber/n",argv[0]);
        exit(-1);
    }
    //���ַ�����IP��ַת��Ϊu_long
    unsigned long ip;
    if((ip=inet_addr(argv[1]))==INADDR_NONE){
        printf("���Ϸ���IP��ַ��%s",argv[1]);
        exit(-1);
    }
    //�Ѷ˿ں�ת��������
    short port;
    if((port = atoi(argv[2]))==0){
        printf("�˿ں�����");
        exit(-1);
    }
    printf("Connecting to %s:%d....../n",inet_ntoa(*(in_addr*)&ip),port);
    WSADATA wsa;
    //��ʼ���׽���DLL
    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){
        printf("�׽��ֳ�ʼ��ʧ��!");
        exit(-1);
    }
    //�����׽���
    SOCKET sock;
    if((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET){
        printf("�����׽���ʧ�ܣ�");
        exit(-1);
    }
    struct sockaddr_in serverAddress;
    memset(&serverAddress,0,sizeof(sockaddr_in));
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_addr.S_un.S_addr = ip;
    serverAddress.sin_port = htons(port);
    //�����ͷ�����������
    if(connect(sock,(sockaddr*)&serverAddress,sizeof(serverAddress))==SOCKET_ERROR){
        printf("��������ʧ�ܣ�");
        exit(-1);
    }
    char buf[4096];
    while(1){
        printf(">");
        //�ӿ���̨��ȡһ������
        gets_s(buf, sizeof(buf));
        //���͸�������
        if(send(sock,buf,(int)strlen(buf),0)==SOCKET_ERROR){
            printf("��������ʧ�ܣ�");
            exit(-1);
        }
        int bytes;
        if((bytes=recv(sock,buf,sizeof(buf),0))==SOCKET_ERROR){
            printf("��������ʧ��!/n");
            exit(-1);
        }
        buf[bytes]=0;
        printf("Message from %s: %s/n",inet_ntoa(serverAddress.sin_addr),buf);
    }
    //�����׽���ռ�õ���Դ
    WSACleanup();
    return 0;
}