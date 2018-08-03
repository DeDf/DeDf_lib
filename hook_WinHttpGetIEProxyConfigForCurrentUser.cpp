
#include "stdio.h"
#define _WIN32_WINNT 0x0502
#include "winsock2.h"
#include <winhttp.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"winhttp.lib")

extern "C"
{
LONG WINAPI DetourTransactionBegin(VOID);
LONG WINAPI DetourUpdateThread(HANDLE hThread);
PVOID WINAPI DetourFindFunction(LPCSTR pszModule, LPCSTR pszFunction);
LONG WINAPI DetourAttach(PVOID *ppPointer, PVOID pDetour);
LONG WINAPI DetourDetach(PVOID *ppPointer, PVOID pDetour);
LONG WINAPI DetourTransactionCommit(VOID);

typedef BOOL (__stdcall *WinHttpGetIEProxyConfigForCurrentUser_t)(
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *pProxyConfig
    );

WinHttpGetIEProxyConfigForCurrentUser_t true_WinHttpGetIEProxyConfigForCurrentUser;

BOOL __stdcall hook_WinHttpGetIEProxyConfigForCurrentUser(
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *pProxyConfig
    )
{
    if (pProxyConfig)
    {
        OutputDebugStringA("WinHttpGetIEProxyConfigForCurrentUser() ");
        PWCHAR proxy = (PWCHAR)GlobalAlloc(0x40, 100);
        memset(pProxyConfig, 0, sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG));
        wcscpy(proxy, L"socks=192.168.0.254:1080");
        OutputDebugStringW(proxy);
        OutputDebugStringA("\n");
        pProxyConfig->lpszProxy = proxy;
        return TRUE;
    }

    return FALSE;
}

LONG InstallHook()
{
    DetourTransactionBegin();
    DetourUpdateThread( GetCurrentThread() );
    *(PVOID*)&true_WinHttpGetIEProxyConfigForCurrentUser =
        DetourFindFunction( "winhttp.dll", "WinHttpGetIEProxyConfigForCurrentUser" );
    DetourAttach( (PVOID*)&true_WinHttpGetIEProxyConfigForCurrentUser,
                   (PVOID)&hook_WinHttpGetIEProxyConfigForCurrentUser );
    return DetourTransactionCommit();
}

LONG UnInstallHook()
{
    DetourTransactionBegin();
    DetourUpdateThread( GetCurrentThread() );
    DetourDetach( (PVOID*)&true_WinHttpGetIEProxyConfigForCurrentUser,
                   (PVOID)&hook_WinHttpGetIEProxyConfigForCurrentUser );
    return DetourTransactionCommit();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch(ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        InstallHook();
        OutputDebugStringA("[Sock5Client] I'm Run ~\n");
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        UnInstallHook();
        OutputDebugStringA("[Sock5Client] I'm Quit.\n");
        break;
    }

    return TRUE;
}

};