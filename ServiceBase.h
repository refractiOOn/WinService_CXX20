#pragma once

#include <string>
#include <Windows.h>

class ServiceBase
{
public:
    ServiceBase(std::wstring serviceName,
                bool canStop = true,
                bool canShutdown = true,
                bool canPauseContinue = false);
    virtual ~ServiceBase() = default;

    static BOOL Run(ServiceBase &service);
    void Stop();

    std::wstring Name() const;

protected:
    virtual void OnStart(DWORD argc, wchar_t **argv);
    virtual void OnStop();
    virtual void OnPause();
    virtual void OnContinue();
    virtual void OnShutdown();

    void SetServiceStatus(DWORD currentState,
                          DWORD exitCode = NO_ERROR,
                          DWORD waitHint = 0);
    void WriteEventLogEntry(const std::wstring &message, WORD type);
    void WriteErrorLogEntry(const std::wstring &function, DWORD error = GetLastError());

private:
    static void WINAPI ServiceMain(DWORD argc, wchar_t **argv);
    static void WINAPI ServiceCtrlHandler(DWORD ctrl);

    void Start(DWORD argc, wchar_t **argv);
    void Pause();
    void Continue();
    void Shutdown();

private:
    static ServiceBase *s_service;

    std::wstring m_name;

    SERVICE_STATUS m_status;
    SERVICE_STATUS_HANDLE m_statusHandle;

};