
#include <windows.h>

VOID GetInstallPath(WCHAR *pwchInstallPath)
{
    LONG ret = 1;
    HKEY hAzureKey = NULL;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Azure",
        0,
        KEY_WRITE|KEY_READ,
        &hAzureKey) == ERROR_SUCCESS)
    {
        WCHAR szLocation[MAX_PATH] = {0};
        DWORD dwSize = sizeof(szLocation);
        DWORD dwType = REG_SZ;
        LONG ret = RegQueryValueExW(hAzureKey, L"UninstallString", 0, &dwType, (LPBYTE)&szLocation, &dwSize);
        //wprintf(L"RegQueryValueEx returns %d, dwSize=%d\n", ret, dwSize);
        if (ERROR_SUCCESS == ret)
        {
            //wprintf(L"Location: %s", szLocation);
        }

        //char *UninstallString = "\"f:\\NAT-T\\Install.exe\" -uninstall";
        RegCloseKey(hAzureKey);
    }

    return ret;
}

VOID DeleteRegUninstall()
{
    HKEY hMachineKey = NULL;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        0,
        KEY_WRITE|KEY_READ,
        &hMachineKey) == ERROR_SUCCESS)
    {
        RegDeleteKeyA(hMachineKey, "Azure");
        RegCloseKey(hMachineKey);
    }
}

VOID CreateRegUninstall(WCHAR *UninstallString)
{
    HKEY hMachineKey = NULL;
    HKEY hSoftKey = NULL;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        0,
        KEY_WRITE|KEY_READ,
        &hMachineKey) == ERROR_SUCCESS)
    {
        DWORD dw;
        //创建并打开HKEY_CURRENT_USER/"Software"/"Wincpp"
        if (RegCreateKeyExA(hMachineKey, "Azure", 0, REG_NONE,
            REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
            &hSoftKey, &dw) == ERROR_SUCCESS)
        {
            //char *UninstallString = "\"f:\\NAT-T\\Install.exe\" -uninstall";
            RegSetValueExA(hSoftKey, "DisplayName", NULL, REG_SZ, (BYTE*)&"Azure", 5);

            wcscat_s(UninstallString, MAX_PATH, L" -uninstall");
            RegSetValueExW(hSoftKey, L"UninstallString", NULL, REG_SZ, 
                (BYTE *)UninstallString, (ULONG)(wcslen(UninstallString) << 1));

            RegCloseKey(hSoftKey);
        }


        RegCloseKey(hMachineKey);
    }
}