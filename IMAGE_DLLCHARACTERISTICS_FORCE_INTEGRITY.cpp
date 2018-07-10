
#include <windows.h>
#include <stdio.h>

#define BUFSIZE 4096

int main()
{
    HANDLE hFile;
    DWORD  dwBytesRead, dwBytesWritten, dwBufSize=BUFSIZE;
    char buffer[BUFSIZE]; 

    // Open the existing file. 
    hFile = CreateFileW(L".\\objchk_win7_amd64\\amd64\\1.sys",// file name 
        GENERIC_READ | GENERIC_WRITE,   // open for read & write
        FILE_SHARE_READ,                // share read
        NULL,                           // default security 
        OPEN_EXISTING,                  // existing file only 
        FILE_ATTRIBUTE_NORMAL,          // normal file 
        NULL);                          // no template 
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        printf("Could not open file.");
        return 0;
    }

    if (ReadFile(hFile, buffer, 4096, &dwBytesRead, NULL)) 
    {
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)buffer;
        PCHAR p = buffer;
        p += pDosHeader->e_lfanew;
        p += 4;
        PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)p;
        p += sizeof(IMAGE_FILE_HEADER);
        PIMAGE_OPTIONAL_HEADER64 pOptionalHeader = (PIMAGE_OPTIONAL_HEADER64)p;

        // DllChar | IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY
        ULONG offset = (ULONG)((PCHAR)&pOptionalHeader->DllCharacteristics - buffer);
        SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
        ReadFile(hFile, buffer, 2, &dwBytesRead, NULL);
        PUSHORT pDllChar = (PUSHORT)buffer;
        USHORT DllChar = *pDllChar | IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY;
        SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
        WriteFile(hFile, &DllChar, 2, &dwBytesWritten, NULL);

        // Ð£ÑéºÍ
        offset = offset - 2 - 4;
        ULONG checksum = 0x4365;
        SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
        WriteFile(hFile, &checksum, 4, &dwBytesWritten, NULL);

        CloseHandle(hFile); 
    }
    return 0;
}