// 1.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>

typedef NTSTATUS(WINAPI *fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
ULONG ulOSBuildNumber;

ULONG GetOSBuildNumber()
{
    if (ulOSBuildNumber)
        return ulOSBuildNumber;

    fnRtlGetVersion MyRtlGetVersion = (fnRtlGetVersion)
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");

    RTL_OSVERSIONINFOW VersionInformation = { 0 };
    if (MyRtlGetVersion)
        MyRtlGetVersion(&VersionInformation);

    ulOSBuildNumber = VersionInformation.dwBuildNumber;
    return ulOSBuildNumber;
}

void main()
{
    ULONG ver = GetOSBuildNumber();
    printf("%d\n", ver);
    getchar();
}
