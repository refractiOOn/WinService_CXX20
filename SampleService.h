#pragma once

#include "ServiceBase.h"

class SampleService : public ServiceBase
{
public:
    SampleService(std::wstring serviceName,
                  bool canStop = true,
                  bool canShutdown = true,
                  bool canPauseContinue = false);
    ~SampleService() override;

protected:
    void OnStart(DWORD argc, wchar_t **argv) override;
    void OnStop() override;

    void ServiceWorkerThread();

private:
    bool m_stopping;
    HANDLE m_stoppedEvent;

};