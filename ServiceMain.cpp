
// sc create serT binpath= D:\test.exe
// sc start  serT
// sc query  serT
// sc stop   serT
// sc delete serT

#include <stdio.h>
#include <Windows.h>

#define  SERVICE_NAME  "NAT-T Server"

bool bRun = false;
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hService;

int	NattMain();

void WINAPI CtrlHandler(DWORD request)
{
    switch (request)
    {
    case SERVICE_CONTROL_STOP:
        bRun = false;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        bRun = false;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        break;
    }

    SetServiceStatus(hService, &ServiceStatus);
}

void WINAPI ServiceMain(int argc, char** argv)
{
    ServiceStatus.dwServiceType      = SERVICE_WIN32;
    ServiceStatus.dwCurrentState     = SERVICE_START_PENDING;

    //�ڱ�����ֻ����ϵͳ�ػ���ֹͣ�������ֿ�������
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_STOP;
    ServiceStatus.dwWin32ExitCode    = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint       = 0;
    ServiceStatus.dwWaitHint         = 0;

    hService = RegisterServiceCtrlHandlerA(SERVICE_NAME, CtrlHandler);
    if (hService == 0)
    {
        return;
    }

    //��SCM ��������״̬
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(hService, &ServiceStatus);

    //����Ϳ�ʼ����ѭ���ˣ������������Լ�ϣ���������Ĺ���
    bRun=true;
    NattMain();
}

void main()
{
    SERVICE_TABLE_ENTRYA ServiceTable[2];
    //
    ServiceTable[0].lpServiceName = SERVICE_NAME;
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)ServiceMain;
    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;

    StartServiceCtrlDispatcherA(ServiceTable);
}

