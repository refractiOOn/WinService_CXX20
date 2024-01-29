#pragma once

#include <memory>
#include <stdexcept>

class ThreadPool
{
    template<typename T>
    using CallbackType = std::pair<void (T::*)(), T *>;

public:
    template <typename T>
    static void QueueUserWorkItem(void (T::*function)(),
        T *object, ULONG flags = WT_EXECUTELONGFUNCTION)
    {
        std::unique_ptr<CallbackType<T>> ptr(std::make_unique<CallbackType<T>>(function, object));

        if (::QueueUserWorkItem(ThreadProc<T>, ptr.get(), flags))
        {
            ptr.release();
        }
        else
        {
            throw std::runtime_error("QueueUserWorkItem failed");
        }
    }

private:
    template <typename T>
    static DWORD WINAPI ThreadProc(PVOID context)
    {
        std::unique_ptr<CallbackType<T>> ptr(static_cast<CallbackType<T> *>(context));

        (ptr->second->*ptr->first)();
        return 0;
    }
};