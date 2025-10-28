#pragma once
#ifndef _Core_CoWork_h_
#define _Core_CoWork_h_

#include "Core.h"
#include "Mt.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>
#include <exception>
#include <memory>

class CoWork : NoCopy {
    struct MJob : Moveable<MJob>, Link<2> {
        std::function<void()> fn;
        CoWork              *work = NULL;
        bool                 looper = false;
    };
    
    enum { SCHEDULED_MAX = 2048 };

public:
    struct Pool {
        Link<2>          *free;
        Link<2>           jobs;
        MJob              slot[SCHEDULED_MAX];
        int               waiting_threads;
        std::vector<std::thread> threads;
        bool              quit;

        std::mutex             lock;
        std::condition_variable waitforjob;
        
        void              Free(MJob& m);
        void              DoJob(MJob& m);
        void              PushJob(std::function<void()>&& fn, CoWork *work, bool looper = false);

        void              InitThreads(int nthreads);
        void              ExitThreads();

        Pool();
        ~Pool();

        static thread_local bool    finlock;

        bool DoJob();
        static void ThreadRun(int tno);
    };
    
    friend struct Pool;

    static Pool& GetPool();

    static thread_local int worker_index;
    static thread_local CoWork *current;

    std::condition_variable  waitforfinish;
    Link<2>            jobs; // global stack and CoWork stack as double-linked lists
    int                todo;
    bool               canceled;
    std::exception_ptr exc = nullptr; // workaround for sanitizer bug(?)
    std::function<void()>  looper_fn;
    int                looper_count;

    void Do0(std::function<void()>&& fn, bool looper);

    void Cancel0();
    void Finish0();
    
    Atomic             index;

public:
    static bool TrySchedule(std::function<void()>&& fn);
    static bool TrySchedule(const std::function<void()>& fn)      { return TrySchedule(fn); }
    static void Schedule(std::function<void()>&& fn);
    static void Schedule(const std::function<void()>& fn)         { Schedule(fn); }

    void     Do(std::function<void()>&& fn)                       { Do0(std::move(fn), false); }
    void     Do(const std::function<void()>& fn)                  { Do(fn); }

    void  operator&(const std::function<void()>& fn)           { Do(fn); }
    void  operator&(std::function<void()>&& fn)                { Do(std::move(fn)); }

    int  GetScheduledCount() const;

    static void FinLock();
    
    void Cancel();
    static bool IsCanceled();

    void Finish();
    
    bool IsFinished();
    
    void Reset();

    static bool IsWorker()                                    { return GetWorkerIndex() >= 0; }
    static int  GetWorkerIndex();
    static int  GetPoolSize();
    static void SetPoolSize(int n);

    CoWork();
    ~CoWork() noexcept(false);

// deprecated:
    void     Loop(std::function<void()>&& fn);
    void     Loop(const std::function<void()>& fn)                { Loop(fn); }

    void  operator*(const std::function<void()>& fn)           { Loop(fn); }
    void  operator*(std::function<void()>&& fn)                { Loop(std::move(fn)); }
    
    int      Next()                                           { return ++index - 1; }
};

struct CoWorkNX : CoWork {
    ~CoWorkNX() noexcept(true) {}
};

inline
void CoDo(std::function<void()>&& fn)
{
    CoWork co;
    co.Do(std::move(fn));
}

inline
void CoDo_ST(std::function<void()>&& fn)
{
    fn();
}

inline
void CoDo(bool co, std::function<void()>&& fn)
{
    if(co)
        CoDo(std::move(fn));
    else
        CoDo_ST(std::move(fn));
}

template <typename Fn>
void CoFor(int n, Fn iterator)
{
    std::atomic<int> ii(0);
    CoDo([&] {
        for(int i = ii++; i < n; i = ii++)
            iterator(i);
    });
}

template <typename Fn>
void CoFor_ST(int n, Fn iterator)
{
    for(int i = 0; i < n; i++)
        iterator(i);
}

template <typename Fn>
void CoFor(bool co, int n, Fn iterator)
{
    if(co)
        CoFor(n, iterator);
    else
        CoFor_ST(n, iterator);
}

template <class T>
class CoWorkerResources {
    int          workercount;
    Buffer<T>    res;
    
public:
    int GetCount() const  { return workercount + 1; }
    T& operator[](int i)  { return res[i]; }

    T& Get()              { int i = CoWork::GetWorkerIndex(); return res[i < 0 ? workercount : i]; }
    T& operator~()        { return Get(); }
    
    T *begin()            { return ~res; }
    T *end()              { return ~res + GetCount(); }
    
    CoWorkerResources()   { workercount = CoWork::GetPoolSize(); res.Alloc(GetCount()); }

    CoWorkerResources(Event<T&> initializer) : CoWorkerResources() {
        for(int i = 0; i < GetCount(); i++)
            initializer(res[i]);
    }
};

template <class Ret>
class AsyncWork {
    template <class Ret2>
    struct Imp {
        CoWork co;
        Ret2   ret;
    
        template<class Function, class... Args>
        void        Do(Function&& f, Args&&... args) { co.Do([=]() { ret = f(args...); }); }
        const Ret2& Get()                            { return ret; }
        Ret2        Pick()                           { return pick(ret); }
    };

    struct ImpVoid {
        CoWork co;
    
        template<class Function, class... Args>
        void        Do(Function&& f, Args&&... args) { co.Do([=]() { f(args...); }); }
        void        Get()                            {}
        void        Pick()                           {}
    };
    
    using ImpType = typename std::conditional<std::is_void<Ret>::value, ImpVoid, Imp<Ret>>::type;
    
    std::unique_ptr<ImpType> imp;
    
public:
    template< class Function, class... Args>
    void  Do(Function&& f, Args&&... args)          { imp = std::make_unique<ImpType>(); imp->Do(f, args...); }

    void        Cancel()                            { if(imp) imp->co.Cancel(); }
    static bool IsCanceled()                        { return CoWork::IsCanceled(); }
    bool        IsFinished()                        { return imp && imp->co.IsFinished(); }
    Ret         Get()                               { ASSERT(imp); imp->co.Finish(); return imp->Get(); }
    Ret         operator~()                         { return Get(); }
    Ret         Pick()                              { ASSERT(imp); imp->co.Finish(); return imp->Pick(); }
    
    AsyncWork& operator=(AsyncWork&&) = default;
    AsyncWork(AsyncWork&&) = default;

    AsyncWork()                                     {}
    ~AsyncWork()                                    { if(imp) imp->co.Cancel(); }
};

template< class Function, class... Args>
AsyncWork<
#ifdef CPP_17
    std::invoke_result_t<Function, Args...>
#else
    typename std::result_of<
        typename std::decay<Function>::type
            (typename std::decay<Args>::type...)
    >::type
#endif
>
Async(Function&& f, Args&&... args)
{
    AsyncWork<
#ifdef CPP_17
    std::invoke_result_t<Function, Args...>
#else
    typename std::result_of<
        typename std::decay<Function>::type
            (typename std::decay<Args>::type...)
    >::type
#endif
    > h;
    h.Do(f, args...);
    return std::move(h);
}

#endif