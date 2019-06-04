
#include <stdio.h>
#include <Windows.h>

// ÍøÒ×ÔÆÒôÀÖ»º´æ×ªmp3

int main()
{
    char chFilePath[260];

L_loop:
    scanf("%s", chFilePath);
    HANDLE hFileRead = CreateFileA(chFilePath,
        GENERIC_READ,          
        FILE_SHARE_READ,
        NULL,               
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    strcat(chFilePath, ".mp3");
    HANDLE hFileWrite = CreateFileA(chFilePath,
        GENERIC_WRITE,          
        FILE_SHARE_READ,
        NULL,               
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    char buf[4096];
    ULONG len;

    while (1)
    {
        if (!ReadFile(hFileRead, buf, sizeof(buf), &len, NULL))
            break;

        if (!len)
            break;

        for (ULONG i = 0; i < len; i++)
            buf[i] ^= 0xa3;

        WriteFile(hFileWrite, buf, len, &len, NULL);
    }

    CloseHandle(hFileWrite);
    CloseHandle(hFileRead);

    goto L_loop;
    return 0;
}