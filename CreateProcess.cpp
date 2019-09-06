
#include <stdio.h>
#include <windows.h>

// int KillProcess(ULONG nProcessID)
// {
//     HANDLE hProcessHandle = OpenProcess(PROCESS_TERMINATE, FALSE, nProcessID );
//     return TerminateProcess( hProcessHandle, 4 );
// }

int CreateProc(WCHAR *pwchCmdLine)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if (!CreateProcessW( NULL,   // No module name (use command line)
        pwchCmdLine,    // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)            // Pointer to PROCESS_INFORMATION structure
        ) 
    {
        printf( "CreateProcess failed (%d)\n", GetLastError() );
        return 1;
    }

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

    return 0;
}

int main()
{
    WCHAR p[] = {L"nslookup www.test1test.com"};
    Sleep(20000);
    CreateProc(p);
    return 0;
}