
#include "setup.h"
#include "stdio.h"

VOID DeleteMyService(CHAR *szServiceName)
{
    SC_HANDLE hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if( hSCM)
    {
        SC_HANDLE hSvc = OpenServiceA( hSCM, szServiceName, SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
        if( hSvc )
        {
            SERVICE_STATUS status;
            ControlService( hSvc, SERVICE_CONTROL_STOP, &status);
            int i = 5;
            while(i--)
            {
                if ( QueryServiceStatus( hSvc, &status) )
                {
                    if( status.dwCurrentState == SERVICE_STOPPED)
                    {
                        //OutputDebugStringA("service stoped !\n");
                        break;
                    }
                }
//                 else
//                     OutputDebugStringA("QueryServiceStatus error !\n");

                Sleep(200);
            }

            DeleteService(hSvc);
            CloseServiceHandle(hSvc);
        }
    
        CloseServiceHandle(hSCM);
    }
}

BOOL InstallService(WCHAR *szServiceName, WCHAR *szFilePath)
{
    BOOL bRet = FALSE;
    WCHAR tmp[MAX_PATH];

    SC_HANDLE hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if( hSCM == NULL)
    {
        //OutputDebugStringA( "open SCManager error !");
        return bRet;
    }
    
    SC_HANDLE hSvc = OpenServiceW( hSCM, szServiceName, SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_STOP );
    if( hSvc == NULL)
    {
        hSvc = CreateServiceW(hSCM, szServiceName, szServiceName,  
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,  
        szFilePath, NULL, NULL, NULL, NULL, NULL);

        bRet = StartService(hSvc, 0, NULL);
        if (bRet)
        {
            swprintf_s(tmp, sizeof(tmp)/2, L"%ws server install success ~", szServiceName);
            MessageBoxW(NULL, tmp, L"tips", MB_OK);
        }
        else
        {
            swprintf_s(tmp, sizeof(tmp)/2, L"%ws server start failed !", szServiceName);
            MessageBoxW(NULL, tmp, L"tips", MB_OK);
        }

        CloseServiceHandle(hSvc);
    }
    else
    {
        swprintf_s(tmp, sizeof(tmp)/2, L"%ws server is Exist !", szServiceName);
        MessageBoxW(NULL, tmp, L"tips", MB_OK);
    }

    CloseServiceHandle(hSCM);
    return bRet;
}