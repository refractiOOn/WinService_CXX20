#include "ServiceInstaller.h"
#include "SampleService.h"
#include <iostream>

int wmain(int argc, wchar_t **argv)
{
    const std::wstring serviceName = L"WindowsService";
    const DWORD startType = SERVICE_DEMAND_START;

    if (argc > 1 && *argv[1] == L'-')
    {
        if (_wcsicmp(L"install", argv[1] + 1) == 0)
        {
            InstallService(serviceName, startType);
        }
        else if (_wcsicmp(L"uninstall", argv[1] + 1) == 0)
        {
            UninstallService(serviceName);
        }
    }
    else
    {
        std::wcout << L"Parameters:\n"
                   << L" -install to install the service.\n"
                   << L" -uninstall to uninstall the service." << std::endl;

        SampleService service(serviceName);
        if (!SampleService::Run(service))
        {
            std::wcerr << L"Service failed to run" << std::endl;
        }
    }

    return 0;
}
