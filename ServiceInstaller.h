#pragma once

#include <string>
#include <Windows.h>

void InstallService(const std::wstring &serviceName, DWORD startType);
void UninstallService(const std::wstring &serviceName);