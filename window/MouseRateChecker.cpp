
#include <stdio.h>
#define _WIN32_WINNT  0x0501
#include <windows.h>

ULONG64 g_MouseMsgCount;

LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		SetTimer(hwnd, 1234, 1000, NULL);
        break;
	}

	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
		PostQuitMessage(0);
        break;
	}

	case WM_TIMER:
	{
        InvalidateRect(hwnd, NULL, true);
        break;
	}

	case WM_INPUT:
	{
		HRAWINPUT hrawinput = (HRAWINPUT)lparam;
		UINT size;
		GetRawInputData(hrawinput, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

		if (size <= sizeof(RAWINPUT))
		{
			RAWINPUT input;
			GetRawInputData(hrawinput, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER));

			if (input.header.dwType == RIM_TYPEMOUSE)
			{
                ++g_MouseMsgCount;
			}
		}
        break;
	}

	case WM_PAINT:
	{
        PAINTSTRUCT ps;
        HDC hdc;

        hdc = BeginPaint(hwnd, &ps);

        char tmp[100];
        ULONG len = sprintf(tmp, "%I64d\n", g_MouseMsgCount);
        TextOutA(hdc,5,5,tmp,len-1);

        EndPaint(hwnd, &ps);

        g_MouseMsgCount = 0;
        break;
	}

    default:
        return DefWindowProcW(hwnd, message, wparam, lparam);
	}
    
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int show)
{
    const WCHAR *windowClassName = L"MainWindowClass";
    const WCHAR *windowTitle     = L"Mouse Rate";

    WNDCLASSEXW windowClassEx;
    windowClassEx.cbSize        = sizeof(windowClassEx);
    windowClassEx.style         = CS_OWNDC;
    windowClassEx.lpfnWndProc   = windowProc;
    windowClassEx.hInstance     = hInstance;
    windowClassEx.cbClsExtra    = 0;
    windowClassEx.cbWndExtra    = 0;
    windowClassEx.hIcon         = LoadIconW(NULL, IDI_APPLICATION);
    windowClassEx.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    windowClassEx.hbrBackground = HBRUSH(GetStockObject(BLACK_BRUSH));
    windowClassEx.lpszMenuName  = NULL;
    windowClassEx.lpszClassName = windowClassName;
    windowClassEx.hIconSm       = NULL;
    
    if (RegisterClassExW(&windowClassEx))
    {
        HWND hwnd = CreateWindowExW(0, windowClassName, windowTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, 240, 480, NULL, NULL, hInstance, NULL);

        if (hwnd)
        {
            RAWINPUTDEVICE devices[2] = {0};
            //
            devices[0].usUsagePage = 0x01;
            devices[0].usUsage = 0x02;
            devices[0].dwFlags = 0x00;
            devices[0].hwndTarget = hwnd;
            //
            devices[1].usUsagePage = 0x01;
            devices[1].usUsage = 0x06;
            devices[1].dwFlags = 0x00;
            devices[1].hwndTarget = hwnd;

            if (RegisterRawInputDevices(devices, sizeof(devices)/sizeof(*devices), sizeof(*devices)))
            {
                ShowWindow(hwnd, show);
                UpdateWindow(hwnd);

                MSG message;
                while (GetMessageW(&message, NULL, 0, 0))
                {
                    TranslateMessage(&message);
                    DispatchMessageW(&message);
                }
            } 
        }
    }

    return 0;
}
