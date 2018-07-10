
#include <windows.h>

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