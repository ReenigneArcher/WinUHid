#include "pch.h"

#include <newdev.h>
#include <cfgmgr32.h>

#define INITGUID
#include <devpkey.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "newdev.lib")

#include <string>

#define UNRESOLVED_INF_PATH L"%ProgramFiles%\\WinUHid\\WinUHidDriver.inf"
#define UNQUALIFIED_HARDWARE_ID L"WinUHid"
#define HARDWARE_IDS (L"Root\\" UNQUALIFIED_HARDWARE_ID L"\0")

#ifndef REGSTR_VAL_MAX_HCID_LEN
#define REGSTR_VAL_MAX_HCID_LEN 1024
#endif

std::wstring GetInfPath()
{
    std::wstring ret;
    DWORD chars = ExpandEnvironmentStringsW(UNRESOLVED_INF_PATH, NULL, 0);
    if (chars != 0) {
        ret.resize(chars);
        chars = ExpandEnvironmentStringsW(UNRESOLVED_INF_PATH, &ret[0], (DWORD)ret.size());
        ret.resize(chars);
    }
    return ret;
}

//
// If Uninstall, returns true if no matching devices remain.
// If !Uninstall, returns true if any matching devices are found.
//
bool EnumerateExistingDevices(bool Uninstall)
{
    HDEVINFO devInfoSet = SetupDiGetClassDevsW(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    SP_DEVINFO_DATA devInfoData = {};
    DWORD deviceIndex;
    bool ret = Uninstall ? true : false;

    devInfoData.cbSize = sizeof(devInfoData);
    for (deviceIndex = 0; SetupDiEnumDeviceInfo(devInfoSet, deviceIndex, &devInfoData); deviceIndex++) {
        DEVPROPTYPE propType;
        WCHAR hardwareIds[REGSTR_VAL_MAX_HCID_LEN] = {};

        if (!SetupDiGetDevicePropertyW(devInfoSet, &devInfoData, &DEVPKEY_Device_HardwareIds, &propType, (PBYTE)hardwareIds, (DWORD)sizeof(hardwareIds), NULL, 0)) {
            WcaLog(LOGMSG_STANDARD, "No hardware IDs found for device index: %u", deviceIndex);
            continue;
        }
        else if (propType != DEVPROP_TYPE_STRING_LIST) {
            WcaLog(LOGMSG_STANDARD, "Unexpected property type for device index: %u (type: %u)", deviceIndex, propType);
            continue;
        }

        //
        // Skip devices that don't match our hardware IDs
        //
        if (RtlEqualMemory(HARDWARE_IDS, hardwareIds, sizeof(HARDWARE_IDS))) {
            WcaLog(LOGMSG_STANDARD, "Found a matching device at index: %u", deviceIndex);

            if (Uninstall) {
                if (!DiUninstallDevice(NULL, devInfoSet, &devInfoData, 0, NULL)) {
                    WcaLog(LOGMSG_STANDARD, "Failed to remove device: %u (error: %d)", deviceIndex, GetLastError());
                    ret = false;
                }
            }
            else {
                ret = true;
            }
        }
    }

    if (GetLastError() != ERROR_NO_MORE_ITEMS) {
        WcaLog(LOGMSG_STANDARD, "SetupDiEnumDeviceInfo() failed: %u", GetLastError());
        if (Uninstall) {
            //
            // We can't guarantee we got everything, so return false
            //
            ret = false;
        }
    }

    if (devInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devInfoSet);
    }
    return ret;
}

UINT __stdcall CreateRootDevice(
    __in MSIHANDLE hInstall
)
{
    HRESULT hr = S_OK;
    DWORD err = ERROR_SUCCESS;
    HDEVINFO devInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA devInfoData = {};
    std::wstring infPath = GetInfPath();
    GUID classGuid;
    WCHAR className[MAX_CLASS_NAME_LEN];

    hr = WcaInitialize(hInstall, __FUNCTION__);
    ExitOnFailure(hr, "Failed to initialize");

    //
    // If we already have a root device, don't create another one
    //
    if (EnumerateExistingDevices(false)) {
        WcaLog(LOGMSG_STANDARD, "Root device already exists");
        goto LExit;
    }

    WcaLog(LOGMSG_STANDARD, "Creating a new root device");

    if (!SetupDiGetINFClassW(infPath.c_str(), &classGuid, className, ARRAYSIZE(className), 0)) {
        ExitWithLastError(hr, "SetupDiGetINFClassW() failed");
    }

    devInfoSet = SetupDiCreateDeviceInfoList(&classGuid, NULL);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        ExitWithLastError(hr, "SetupDiCreateDeviceInfoList() failed");
    }

    devInfoData.cbSize = sizeof(devInfoData);
    if (!SetupDiCreateDeviceInfoW(devInfoSet, UNQUALIFIED_HARDWARE_ID, &classGuid, NULL, NULL, DICD_GENERATE_ID, &devInfoData)) {
        ExitWithLastError(hr, "SetupDiCreateDeviceInfoW() failed");
    }

    if (!SetupDiSetDeviceRegistryPropertyW(devInfoSet, &devInfoData, SPDRP_HARDWAREID, (LPBYTE)HARDWARE_IDS, sizeof(HARDWARE_IDS))) {
        ExitWithLastError(hr, "SetupDiSetDeviceRegistryPropertyW(SPDRP_HARDWAREID) failed");
    }

    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, devInfoSet, &devInfoData)) {
        ExitWithLastError(hr, "SetupDiCallClassInstaller(DIF_REGISTERDEVICE) failed");
    }

LExit:
    if (devInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devInfoSet);
    }
    err = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(err);
}

UINT __stdcall DeleteRootDevice(
    __in MSIHANDLE hInstall
)
{
    HRESULT hr = S_OK;
    DWORD err = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, __FUNCTION__);
    ExitOnFailure(hr, "Failed to initialize");

    WcaLog(LOGMSG_STANDARD, "Deleting root device");

    //
    // Uninstall all matching root devices
    //
    if (!EnumerateExistingDevices(true)) {
        hr = E_FAIL;
        goto LExit;
    }

LExit:
    err = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(err);
}

UINT __stdcall InstallDriver(
    __in MSIHANDLE hInstall
)
{
    HRESULT hr = S_OK;
    DWORD err = ERROR_SUCCESS;
    std::wstring infPath = GetInfPath();

    hr = WcaInitialize(hInstall, __FUNCTION__);
    ExitOnFailure(hr, "Failed to initialize");

    WcaLog(LOGMSG_STANDARD, "Performing driver installation");

    //
    // This stages the driver in the driver store and installs it on all compatible devices
    //
    if (!DiInstallDriverW(NULL, infPath.c_str(), 0, NULL)) {
        ExitWithLastError(hr, "DiInstallDriverW() failed");
    }

LExit:
    err = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(err);
}

UINT __stdcall UninstallDriver(
    __in MSIHANDLE hInstall
)
{
    HRESULT hr = S_OK;
    DWORD err = ERROR_SUCCESS;
    std::wstring infPath = GetInfPath();

    hr = WcaInitialize(hInstall, __FUNCTION__);
    ExitOnFailure(hr, "Failed to initialize");

    WcaLog(LOGMSG_STANDARD, "Unstaging driver from driver store");

    //
    // This API wasn't introduced until Win10 1703, but that's earlier than
    // VhfUm.dll which our driver itself depends on.
    //
    if (!DiUninstallDriverW(NULL, infPath.c_str(), 0, NULL)) {
        ExitWithLastError(hr, "DiUninstallDriverW() failed");
    }

LExit:
    err = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(err);
}