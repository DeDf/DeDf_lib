
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        printf("WSAStartup failed!\n");
        return -1;
    }

L_loop:
    char *host_name = "www.baidu.com";
    printf("www.baidu.com %p\n", host_name);
    struct hostent *remoteHost = gethostbyname(host_name);
    if (remoteHost)
    {
        char **pAlias;
        ULONG i;

        printf("gethostbyname(%s):\n", host_name);
        printf("\tOfficial name: %s\n", remoteHost->h_name);
        for (i = 0, pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++)
        {
            printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
        }

        printf("\tAddress length: %d\n", remoteHost->h_length); 
        printf("\tAddress type: ");

        switch (remoteHost->h_addrtype)
        {
        case AF_INET:
            {
                printf("AF_INET\n");
                i = 0;
                while (remoteHost->h_addr_list[i])
                {
                    UCHAR *IP = (UCHAR *) remoteHost->h_addr_list[i++];
                    printf("\tIP Address #%d: %d.%d.%d.%d\n", i, IP[0],IP[1],IP[2],IP[3]);
                }
            }
            break;
        case AF_NETBIOS:
            printf("AF_NETBIOS\n");
            break;
        default:
            printf(" %d\n", remoteHost->h_addrtype);
            break;
        } 
    }
    
    getchar();
    goto L_loop;
    return 0;
}