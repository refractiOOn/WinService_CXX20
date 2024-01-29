#include "ServiceInstaller.h"
#include <iostream>

void InstallService(const std::wstring &serviceName, DWORD startType)
{
    wchar_t path[MAX_PATH];
    SC_HANDLE manager = NULL, service = NULL;

    if (GetModuleFileNameW(NULL, path, ARRAYSIZE(path)) == 0)
    {
        std::wcerr << "GetModuleFileNameW failed" << std::endl;
        goto Cleanup;
    }

    manager = OpenSCManager(NULL,
                            NULL,
                            SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
    if (!manager)
    {
        std::wcerr << "OpenSCManager failed" << std::endl;
        goto Cleanup;
    }

    service = CreateServiceW(manager,
                             serviceName.c_str(),
                             serviceName.c_str(),
                             SERVICE_QUERY_STATUS,
                             SERVICE_WIN32_OWN_PROCESS,
                             startType,
                             SERVICE_ERROR_NORMAL,
                             path,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
    if (!service)
    {
        std::wcerr << "CreateServiceW failed" << std::endl;
        goto Cleanup;
    }

    std::wcout << serviceName << " is installed" << std::endl;

Cleanup:
    if (manager)
    {
        CloseServiceHandle(manager);
        manager = NULL;
    }
    if (service)
    {
        CloseServiceHandle(service);
        service = NULL;
    }
}

void UninstallService(const std::wstring &serviceName)
{
    SC_HANDLE manager = NULL, service = NULL;
    SERVICE_STATUS status = {};

    manager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!manager)
    {
        std::wcerr << "OpenSCManager failed" << std::endl;
        goto Cleanup;
    }

    service = OpenServiceW(manager, serviceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
    if (!service)
    {
        std::wcerr << "OpenServiceW failed" << std::endl;
        goto Cleanup;
    }

    if (ControlService(service, SERVICE_CONTROL_STOP, &status))
    {
        std::wcout << "Stopping " << serviceName << std::endl;
        Sleep(1000);

        while (QueryServiceStatus(service, &status))
        {
            if (status.dwCurrentState == SERVICE_STOP_PENDING)
            {
                std::wcout << "." << std::endl;
                Sleep(1000);
            }
            else break;
        }

        if (status.dwCurrentState == SERVICE_STOPPED)
            std::wcout << serviceName << " is stopped" << std::endl;
        else
            std::wcerr << serviceName << " failed to stop" << std::endl;
    }

    if (!DeleteService(service))
    {
        std::wcerr << "DeleteService failed" << std::endl;
        goto Cleanup;
    }

    std::wcout << serviceName << " is uninstalled" << std::endl;

Cleanup:
    if (manager)
    {
        CloseServiceHandle(manager);
        manager = NULL;
    }
    if (service)
    {
        CloseServiceHandle(service);
        service = NULL;
    }
}