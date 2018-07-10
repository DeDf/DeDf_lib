
#include <stdio.h>
#include <windows.h>
#include <ShlObj.h>

#pragma comment(lib, "Ws2_32.lib")

// success retrun NATT_IP, error return 0
ULONG GetNATTIP ()
{
    ULONG IP = 0;
    WCHAR wchIniFile[MAX_PATH];
    HANDLE hDirectory;
    WCHAR wchNATT_IP[20];
    char chNATT_IP[20];

    SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, wchIniFile);
    wcscat(wchIniFile, L"\\ComodoVPN");

    #define CREATE_DIRECTORY 0x03000000

    hDirectory = CreateFileW(
        wchIniFile,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,
        CREATE_NEW,
        CREATE_DIRECTORY | FILE_ATTRIBUTE_DIRECTORY,
        NULL
        );

    if (hDirectory != INVALID_HANDLE_VALUE)
        CloseHandle(hDirectory);

    wcscat(wchIniFile, L"\\config.ini");  
    
    GetPrivateProfileStringW(L"NAT-T", L"Server", L"", wchNATT_IP, sizeof(wchNATT_IP), wchIniFile);

    if (!wchNATT_IP[0])
    {
        WritePrivateProfileStringW(L"NAT-T", L"Server", L"", wchIniFile);
    }
    else
    {
        WideCharToMultiByte( CP_ACP, 0, wchNATT_IP, -1, 
            chNATT_IP, sizeof(chNATT_IP), NULL, NULL );

        IP = inet_addr(chNATT_IP);
        if (IP == -1)
            IP = 0;
    }

    return IP;
}

int main()
{
    return GetNATTIP();
}