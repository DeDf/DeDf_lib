
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define DebugInLogFile
#ifdef DebugInLogFile
#define printf(format, ...)  WriteLogFile(format, __VA_ARGS__)
#endif

static HANDLE g_hLogFile;

void OpenLogFile(void)
{
    CHAR buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, sizeof(buf));
    CHAR *p = strrchr(buf, '\\');

    *p = 0;
    strcat_s(buf, sizeof(buf), "\\natt.log");

    g_hLogFile = CreateFileA(buf,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (g_hLogFile == INVALID_HANDLE_VALUE)
    {
        OutputDebugStringA("CreateFile natt.log fail!\n");
    }
}

void WriteLogFile(const char *format, ...)
{
    ULONG FileSize = GetFileSize(g_hLogFile, NULL);
    if (FileSize < 0x100000)
    {
        DWORD dwPtr = SetFilePointer(g_hLogFile, 0, NULL, FILE_END);
        if (dwPtr == INVALID_SET_FILE_POINTER)
        {
            DWORD dwError = GetLastError();
        }
    }

    char buffer[500];
    time_t nowtime;  
    nowtime = time(NULL); //获取日历时间  

    struct tm local;  
    localtime_s(&local, &nowtime);  //获取当前系统时间

    sprintf_s(buffer, sizeof(buffer), "%04d.%02d.%02d %02d:%02d:%02d ",
        local.tm_year+1900, local.tm_mon+1, local.tm_mday, 
        local.tm_hour, local.tm_min, local.tm_sec);

    va_list args;
    va_start (args, format);
    DWORD dwWritenSize = vsprintf_s (buffer+20, sizeof(buffer)-20, format, args);
    va_end (args);

    BOOL bRet = WriteFile(g_hLogFile, buffer, dwWritenSize+20, &dwWritenSize, NULL);
    if (!bRet)
    {
        OutputDebugStringA("WriteFile natt.log failed !\n");
    }
}

void CloseLogFile(void)
{
    CloseHandle(g_hLogFile);
    g_hLogFile = 0;
}