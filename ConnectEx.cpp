#include <stdio.h>
#include <WinSock2.h>
#include <MSWSock.h>

#pragma comment(lib, "Ws2_32.lib")

struct _mswsock_func {
    LPFN_CONNECTEX ConnectEx;
} mswsock_func;

int main(int argc, char *argv[])
{
    int ret;
    //
    WSADATA wsaData;
    ret = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (ret) {
        printf("WSAStartup failed: %d\n", ret);
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock == INVALID_SOCKET) {
        printf("socket: %d\n", WSAGetLastError());
        return 1;
    }

    DWORD dwBytes;
    GUID guid = WSAID_CONNECTEX;
    ret = WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guid, sizeof(guid),
        &mswsock_func.ConnectEx, sizeof(mswsock_func.ConnectEx),
        &dwBytes, NULL, NULL);

    if (ret) {
        printf("WSAIoctl() Error loading mswsock!ConnectEx(): %d\n", WSAGetLastError());
        return 1;
    }

    /* ConnectEx requires the socket to be initially bound. */
    {
        struct sockaddr_in addr;
        ZeroMemory(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = 0;
        ret = bind(sock, (SOCKADDR*) &addr, sizeof(addr));
        if (ret) {
            printf("bind failed: %d\n", WSAGetLastError());
            return 1;
        }
    }

    /* Issue ConnectEx and wait for the operation to complete. */
    {
        OVERLAPPED ol;
        ZeroMemory(&ol, sizeof(ol));

        sockaddr_in addr;
        ZeroMemory(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("173.194.37.36"); // google.com
        addr.sin_port = htons(80);

        BOOL ok = mswsock_func.ConnectEx(sock, (SOCKADDR*) &addr, sizeof(addr), NULL, 0, NULL, &ol);
        if (ok)
        {
            printf("ConnectEx succeeded immediately\n");
        }
        else if (WSAGetLastError() == ERROR_IO_PENDING)
        {
            printf("ConnectEx pending\n");

            DWORD numBytes;
            ok = GetOverlappedResult((HANDLE) sock, &ol, &numBytes, TRUE);
            if (ok)
                printf("ConnectEx succeeded\n");
            else
                printf("ConnectEx failed: %d\n", WSAGetLastError());
        }
        else
        {
            printf("ConnectEx failed: %d\n", WSAGetLastError());
            return 1;
        }
    }

    /* Make the socket more well-behaved. */
    ret = setsockopt(sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
    if (ret) {
        printf("SO_UPDATE_CONNECT_CONTEXT failed: %d\n", WSAGetLastError());
        return 1;
    }

    /* This will fail if SO_UPDATE_CONNECT_CONTEXT was not performed. */
    ret = shutdown(sock, SD_BOTH);
    if (ret) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Done\n");
    return 0;
}