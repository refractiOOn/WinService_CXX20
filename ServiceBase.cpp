#include "ServiceBase.h"
#include <stdexcept>
#include <strsafe.h>

ServiceBase *ServiceBase::s_service = nullptr;

ServiceBase::ServiceBase(std::wstring serviceName,
                         bool canStop,
                         bool canShutdown,
                         bool canPauseContinue) :
    m_name(std::move(serviceName)), m_statusHandle(NULL)
{
    DWORD controlsAccepted = 0;
    if (canStop) controlsAccepted |= SERVICE_ACCEPT_STOP;
    if (canShutdown) controlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
    if (canPauseContinue) controlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;

    m_status.dwControlsAccepted = controlsAccepted;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_status.dwWin32ExitCode = NO_ERROR;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

BOOL ServiceBase::Run(ServiceBase &service)
{
    s_service = &service;

    SERVICE_TABLE_ENTRYW serviceTable[] =
    {
        { &service.m_name[0], ServiceMain },
        { NULL, NULL }
    };

    return StartServiceCtrlDispatcherW(serviceTable);
}

void ServiceBase::Stop()
{
    DWORD originalState = m_status.dwCurrentState;
    try
    {
        SetServiceStatus(SERVICE_STOP_PENDING);
        OnStop();
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD error)
    {
        WriteErrorLogEntry(L"Service Stop", error);
        SetServiceStatus(originalState);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(originalState);
    }
}

std::wstring ServiceBase::Name() const { return m_name; }
void ServiceBase::OnStart(DWORD argc, wchar_t **argv) {}
void ServiceBase::OnStop() {}
void ServiceBase::OnPause() {}
void ServiceBase::OnContinue() {}
void ServiceBase::OnShutdown() {}

void ServiceBase::SetServiceStatus(DWORD currentState, DWORD exitCode, DWORD waitHint)
{
    static DWORD checkPoint = 1;

    m_status.dwCurrentState = currentState;
    m_status.dwWin32ExitCode = exitCode;
    m_status.dwWaitHint = waitHint;
    m_status.dwCheckPoint = ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED)) ? 0 : checkPoint++;

    ::SetServiceStatus(m_statusHandle, &m_status);
}

void ServiceBase::WriteEventLogEntry(const std::wstring &message, WORD type)
{
    HANDLE eventSource = NULL;
    LPCWSTR strings[2] = { NULL, NULL };

    eventSource = RegisterEventSourceW(NULL, m_name.c_str());
    if (!eventSource) return;

    strings[0] = m_name.c_str();
    strings[1] = message.c_str();

    ReportEventW(eventSource,
                 type,
                 0,
                 0,
                 NULL,
                 2,
                 0,
                 strings,
                 NULL);
    DeregisterEventSource(eventSource);
}

void ServiceBase::WriteErrorLogEntry(const std::wstring &function, DWORD error)
{
    wchar_t message[260];
    StringCchPrintfW(message, ARRAYSIZE(message), L"%s failed w/err 0x%08lx", function.c_str(), error);
    WriteEventLogEntry(message, EVENTLOG_ERROR_TYPE);
}

void ServiceBase::ServiceMain(DWORD argc, wchar_t **argv)
{
    s_service->m_statusHandle = RegisterServiceCtrlHandlerW(s_service->m_name.c_str(), ServiceCtrlHandler);
    if (!s_service->m_statusHandle)
    {
        throw std::runtime_error("RegisterServiceCtrlHandlerW failed");
    }

    s_service->Start(argc, argv);
}

void ServiceBase::ServiceCtrlHandler(DWORD ctrl)
{
    switch (ctrl)
    {
    case SERVICE_CONTROL_STOP: s_service->Stop(); break;
    case SERVICE_CONTROL_PAUSE: s_service->Pause(); break;
    case SERVICE_CONTROL_CONTINUE: s_service->Continue(); break;
    case SERVICE_CONTROL_SHUTDOWN: s_service->Shutdown(); break;
    case SERVICE_CONTROL_INTERROGATE: default: break;
    }
}

void ServiceBase::Start(DWORD argc, wchar_t **argv)
{
    try
    {
        SetServiceStatus(SERVICE_START_PENDING);
        OnStart(argc, argv);
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD dwError)
    {
        WriteErrorLogEntry(L"Service Start", dwError);
        SetServiceStatus(SERVICE_STOPPED, dwError);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_STOPPED);
    }
}

void ServiceBase::Pause()
{
    try
    {
        SetServiceStatus(SERVICE_PAUSE_PENDING);
        OnPause();
        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (DWORD error)
    {
        WriteErrorLogEntry(L"Service Pause", error);
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to pause.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_RUNNING);
    }
}

void ServiceBase::Continue()
{
    try
    {
        SetServiceStatus(SERVICE_CONTINUE_PENDING);
        OnContinue();
        SetServiceStatus(SERVICE_RUNNING);
    }
    catch (DWORD error)
    {
        WriteErrorLogEntry(L"Service Continue", error);
        SetServiceStatus(SERVICE_PAUSED);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to resume.", EVENTLOG_ERROR_TYPE);
        SetServiceStatus(SERVICE_PAUSED);
    }
}

void ServiceBase::Shutdown()
{
    try
    {
        OnShutdown();
        SetServiceStatus(SERVICE_STOPPED);
    }
    catch (DWORD error)
    {
        WriteErrorLogEntry(L"Service Shutdown", error);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to shut down.", EVENTLOG_ERROR_TYPE);
    }
}
