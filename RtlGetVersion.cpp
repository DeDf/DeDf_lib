#include <stdio.h>
#include <windows.h>

typedef LONG (__stdcall *fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);

int main(void)
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll)
    {
        fnRtlGetVersion pRtlGetVersion = (fnRtlGetVersion)GetProcAddress(hNtdll, "RtlGetVersion");
        if (pRtlGetVersion)
        {
            RTL_OSVERSIONINFOW VersionInformation = { 0 };
            VersionInformation.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

            LONG status = pRtlGetVersion(&VersionInformation);
            if (!status)
            {
                ULONG dwMajorVersion = VersionInformation.dwMajorVersion;
                ULONG dwMinorVersion = VersionInformation.dwMinorVersion;
                ULONG dwBuildNumber  = VersionInformation.dwBuildNumber;

                printf("winver %d.%d, build : %d\n", dwMajorVersion, dwMinorVersion, dwBuildNumber);
            }
        }
    }

    getchar();
    return 0;
}
