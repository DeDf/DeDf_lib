
#include <stdio.h>
#define _WIN32_WINNT  0x0501
#include <windows.h>

// usUsagePage
// 1 for generic desktop controls
// 2 for simulation controls
// 3 for vr
// 4 for sport
// 5 for game
// 6 for generic device
// 7 for keyboard
// 8 for leds
// 9 button
// 
// usUsage values when usUsagePage is 1
// 0 - undefined
// 1 - pointer
// 2 - mouse
// 3 - reserved
// 4 - joystick
// 5 - game pad
// 6 - keyboard
// 7 - keypad
// 8 - multi-axis controller
// 9 - Tablet PC contro

ULONG g_MouseMsgCount;

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
        RAWINPUT input;
        UINT size = sizeof(RAWINPUT);
        GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER));

        if (size <= sizeof(RAWINPUT))
        {
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
        HDC hdc = BeginPaint(hwnd, &ps);

        char tmp[100];
        ULONG len = sprintf(tmp, "%d\n", g_MouseMsgCount);
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
            RAWINPUTDEVICE MouseDevice;
            //
            MouseDevice.usUsagePage = 0x01;  // 1 for generic desktop controls
            MouseDevice.usUsage = 0x02;      //  2 - mouse
            MouseDevice.dwFlags = 0x00;
            MouseDevice.hwndTarget = hwnd;

            if (RegisterRawInputDevices(&MouseDevice, 1, sizeof(MouseDevice)))
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
