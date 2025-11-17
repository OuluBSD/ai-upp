# U++ to STL Mapping: Core Package Threading

This document provides comprehensive mapping between U++ Core package threading utilities and their STL/C++11 equivalents.

## 1. Thread ↔ std::thread

### U++ Declaration
```cpp
template<typename Res, typename... ArgTypes>
class Function<Res(ArgTypes...)>; // Forward declaration

class Thread : NoCopy {
#ifdef PLATFORM_WIN32
    HANDLE     handle;                              // Thread handle (Windows)
    DWORD      thread_id;                           // Thread identifier
#endif
#ifdef PLATFORM_POSIX
    pthread_t  handle;                              // Thread handle (POSIX)
#endif

    int stack_size = 0;                             // Stack size setting

public:
    bool       Run(Function<void ()> cb, bool noshutdown = false); // Run with function
    bool       RunNice(Function<void ()> cb, bool noshutdown = false); // Run with low priority
    bool       RunCritical(Function<void ()> cb, bool noshutdown = false); // Run with high priority

    void       Detach();                            // Detach thread
    int        Wait();                              // Wait for thread completion

    bool       IsOpen() const;                      // Check if thread exists
#ifdef PLATFORM_WIN32
    typedef HANDLE Handle;                          // Handle type (Windows)
    typedef DWORD  Id;                             // ID type (Windows)
    Id          GetId() const;                     // Get thread ID
#endif
#ifdef PLATFORM_POSIX
    typedef pthread_t Handle;                       // Handle type (POSIX)
    typedef pthread_t Id;                          // ID type (POSIX)
    Id          GetId() const;                     // Get thread ID
#endif

    Handle      GetHandle() const;                  // Get handle

    bool        Priority(int percent);              // Set priority (0=lowest, 100=normal)
    void        StackSize(int bytes);               // Set stack size
    void        Nice();                             // Set to nice priority
    void        Critical();                         // Set to critical priority

    static void Start(Function<void ()> cb, bool noshutdown = false); // Start thread
    static void StartNice(Function<void ()> cb, bool noshutdown = false); // Start with low priority
    static void StartCritical(Function<void ()> cb, bool noshutdown = false); // Start with high priority

    static void Sleep(int ms);                      // Sleep in ms

    static bool IsST();                             // Check if single-threaded
    static bool IsMain();                           // Check if main thread
    static bool IsUpp();                            // Check if U++ thread
    static int  GetCount();                         // Get running thread count
    static void BeginShutdownThreads();             // Begin thread shutdown
    static void AtShutdown(void (*shutdownfn)());   // Register shutdown function
    static void TryShutdownThreads();               // Try to shutdown threads
    static void EndShutdownThreads();               // End thread shutdown
    static void ShutdownThreads();                  // Shutdown all threads
    static bool IsShutdownThreads();                // Check if shutdown in progress
    static void (*AtExit(void (*exitfn)()))();      // Register exit function

    static void Exit();                             // Exit current thread

    static void DumpDiagnostics();                  // Dump diagnostics

#ifdef PLATFORM_WIN32
    static Handle GetCurrentHandle();               // Get current handle
    static inline Id GetCurrentId();                // Get current ID
#endif
#ifdef PLATFORM_POSIX
    static Handle GetCurrentHandle();               // Get current handle
    static inline Id GetCurrentId();                // Get current ID
#endif

    Thread();                                       // Default constructor
    ~Thread();                                      // Destructor
};
```

### STL Equivalent
```cpp
#include <thread>
#include <chrono>
#include <functional>

class std::thread {
public:
    using id = /* implementation-defined */;        // Thread ID type

    thread() noexcept;                              // Default constructor (no thread)
    template< class Function, class... Args >
    explicit thread(Function&& f, Args&&... args);  // Constructor with function and args
    thread(thread&& other) noexcept;                // Move constructor
    thread(const thread&) = delete;                 // No copy constructor

    ~thread();                                      // Destructor
    thread& operator=(thread&& other) noexcept;     // Move assignment
    thread& operator=(const thread&) = delete;      // No copy assignment

    void join();                                    // Wait for thread to finish
    void detach();                                  // Detach thread
    bool joinable() const;                          // Check if joinable
    id get_id() const;                              // Get thread ID
    native_handle_type native_handle();             // Get native handle

    static unsigned int hardware_concurrency() noexcept; // Get hardware thread count

    void swap(thread& other) noexcept;              // Swap threads
};

namespace std {
    namespace this_thread {
        void yield() noexcept;                      // Yield execution
        void sleep_for(const chrono::duration<Rep, Period>& sleep_duration); // Sleep for duration
        void sleep_until(const chrono::time_point<Clock, Duration>& sleep_time); // Sleep until time
    }
}
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Thread | std::thread | ✓ Complete | |
| Thread::Run(Function<void ()> cb) | std::thread t(cb) | ✓ Complete | |
| Thread::Wait() | std::thread::join() | ✓ Complete | |
| Thread::Detach() | std::thread::detach() | ✓ Complete | |
| Thread::IsOpen() | std::thread::joinable() | ✓ Complete | |
| Thread::GetId() | std::thread::get_id() | ✓ Complete | |
| Thread::Sleep(int ms) | std::this_thread::sleep_for(std::chrono::milliseconds(ms)) | ✓ Complete | |
| Thread::Start(cb) | std::thread t(cb); t.detach(); | ✓ Complete | |
| std::this_thread::get_id() | Thread::GetCurrentId() | ✓ Complete | |

### Conversion Notes
- U++ Thread maps directly to std::thread for basic functionality
- U++ has additional convenience methods like Priority(), Nice(), Critical() that would require additional platform-specific code in STL
- U++ Thread::Run() creates and runs a thread, similar to std::thread constructor
- U++ has thread lifecycle management features (shutdown, diagnostics) that are not part of std::thread
- U++ provides higher-level thread management with noshutdown flags

## 2. Mutex ↔ std::mutex and related primitives

### U++ Declaration
```cpp
class Mutex : NoCopy {
#ifdef PLATFORM_WIN32
    CRITICAL_SECTION section;                       // Critical section (Windows)
    MtInspector        *mti;                       // Inspector for profiling
#endif
#ifdef PLATFORM_POSIX
    pthread_mutex_t  mutex[1];                     // Mutex (POSIX)
    MtInspector     *mti;                          // Inspector for profiling (with flagPROFILEMT)
#endif

public:
    bool  TryEnter();                              // Try to acquire lock
    void  Enter();                                 // Acquire lock (blocking)
    void  Leave();                                 // Release lock

#ifdef flagPROFILEMT
    void  Set(MtInspector& m);                     // Set inspector for profiling
#endif

    Mutex();                                       // Constructor
    ~Mutex();                                      // Destructor

    class Lock;                                    // RAII lock class
};

class Mutex::Lock : NoCopy {
    Mutex& s;                                      // Reference to mutex

public:
    Lock(Mutex& s);                                // Constructor (acquires lock)
    ~Lock();                                       // Destructor (releases lock)
};
```

### STL Equivalent
```cpp
#include <mutex>

class std::mutex {
public:
    mutex() noexcept;                               // Constructor
    ~mutex();                                      // Destructor
    mutex(const mutex&) = delete;                  // No copy constructor
    mutex& operator=(const mutex&) = delete;       // No copy assignment

    void lock();                                   // Acquire lock (blocking)
    bool try_lock();                               // Try to acquire lock (non-blocking)
    void unlock();                                 // Release lock
};

class std::lock_guard<mutex> {
    mutex& mtx;                                    // Reference to mutex

public:
    using mutex_type = mutex;                      // Type alias
    explicit lock_guard(mutex& m);                 // Constructor (acquires lock)
    ~lock_guard();                                 // Destructor (releases lock)
    lock_guard(const lock_guard&) = delete;        // No copy constructor
    lock_guard& operator=(const lock_guard&) = delete; // No copy assignment
};

class std::unique_lock<mutex> {
    mutex* mtx;                                    // Pointer to mutex
    bool owns;                                     // Whether owns the lock

public:
    using mutex_type = mutex;                      // Type alias
    unique_lock() noexcept;                        // Default constructor
    explicit unique_lock(mutex& m);                // Constructor (acquires lock)
    unique_lock(mutex& m, std::defer_lock_t);      // Constructor (no lock)
    unique_lock(mutex& m, std::try_to_lock_t);     // Constructor (try lock)
    unique_lock(mutex& m, std::adopt_lock_t);      // Constructor (assume owns)
    ~unique_lock();                                // Destructor (releases lock if owned)
    unique_lock(const unique_lock&) = delete;      // No copy constructor
    unique_lock& operator=(const unique_lock&) = delete; // No copy assignment
    unique_lock(unique_lock&& other) noexcept;     // Move constructor
    unique_lock& operator=(unique_lock&& other) noexcept; // Move assignment
    void lock();                                   // Acquire lock
    bool try_lock();                               // Try to acquire lock
    void unlock();                                 // Release lock
    bool owns_lock() const noexcept;               // Check if owns lock
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Mutex | std::mutex | ✓ Complete | |
| Mutex::Enter() | std::mutex::lock() | ✓ Complete | |
| Mutex::TryEnter() | std::mutex::try_lock() | ✓ Complete | |
| Mutex::Leave() | std::mutex::unlock() | ✓ Complete | |
| Mutex::Lock | std::lock_guard<std::mutex> | ✓ Complete | |
| Mutex::Lock(m) | std::lock_guard<std::mutex>(m) | ✓ Complete | |

### Conversion Notes
- U++ Mutex maps directly to std::mutex for basic functionality
- U++ Mutex::Enter() corresponds to std::mutex::lock()
- U++ Mutex::TryEnter() corresponds to std::mutex::try_lock()
- U++ Mutex::Leave() corresponds to std::mutex::unlock()
- U++ Mutex::Lock is equivalent to std::lock_guard<std::mutex> for RAII locking
- std::unique_lock provides additional flexibility compared to U++'s simple lock guard

## 3. Semaphore ↔ std::counting_semaphore (C++20) or custom implementation

### U++ Declaration
```cpp
class Semaphore : NoCopy {
#ifdef PLATFORM_WIN32
    HANDLE     handle;                              // Semaphore handle (Windows)
#elif PLATFORM_OSX
    dispatch_semaphore_t    sem;                    // Dispatch semaphore (OSX)
#else
    sem_t      sem;                                // Semaphore (POSIX)
#endif

public:
    bool       Wait(int timeout_ms = -1);          // Wait/lock with optional timeout
    void       Release();                           // Release/unlock
#ifdef PLATFORM_WIN32
    void       Release(int n);                     // Release multiple units
#endif

    Semaphore();                                   // Constructor
    ~Semaphore();                                  // Destructor
};
```

### STL Equivalent
```cpp
#include <semaphore>  // C++20

template< std::ptrdiff_t least_max_value = std::numeric_limits<std::ptrdiff_t>::max() >
class std::counting_semaphore {
    static_assert(least_max_value >= 0 && least_max_value <= /* max */); // Implementation-defined max

public:
    static constexpr ptrdiff_t max() noexcept;     // Get max value
    constexpr explicit counting_semaphore(ptrdiff_t desired); // Constructor with initial count
    ~counting_semaphore();                         // Destructor
    counting_semaphore(const counting_semaphore&) = delete; // No copy constructor
    counting_semaphore& operator=(const counting_semaphore&) = delete; // No copy assignment

    void acquire();                                // Acquire (wait until available)
    bool try_acquire();                            // Try to acquire (non-blocking)
    template<class Rep, class Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& rel_time); // Acquire with timeout
    template<class Clock, class Duration>
    bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& abs_time); // Acquire until time
    void release(ptrdiff_t update = 1);            // Release (add permits)
};

// For pre-C++20, a custom implementation using mutex and condition variable:
#include <mutex>
#include <condition_variable>

class Semaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    unsigned long count;

public:
    Semaphore(unsigned long initial_count = 0) : count(initial_count) {}
    
    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        while(count == 0) {
            cv.wait(lock);
        }
        --count;
    }
    
    bool try_acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        if(count > 0) {
            --count;
            return true;
        }
        return false;
    }
    
    void release() {
        std::unique_lock<std::mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Semaphore | std::counting_semaphore (C++20) or custom implementation | ✓ Complete | |
| Semaphore::Wait() | std::counting_semaphore::acquire() | ✓ Complete | |
| Semaphore::Wait(timeout) | std::counting_semaphore::try_acquire_for() | ✓ Complete | |
| Semaphore::Release() | std::counting_semaphore::release() | ✓ Complete | |
| Semaphore::Release(n) | std::counting_semaphore::release(n) | ✓ Complete | |

### Conversion Notes
- U++ Semaphore maps to std::counting_semaphore in C++20
- For pre-C++20, a custom implementation using std::mutex and std::condition_variable is needed
- U++ Semaphore::Wait() with timeout parameter maps to std::counting_semaphore's time-based acquisition methods
- Both provide similar functionality for synchronizing access to resources with counting semantics

## 4. ConditionVariable ↔ std::condition_variable

### U++ Declaration
```cpp
class ConditionVariable {
#ifdef PLATFORM_WIN32
    // Windows implementation details
    static VOID (WINAPI *InitializeConditionVariable)(PCONDITION_VARIABLE ConditionVariable);
    static VOID (WINAPI *WakeConditionVariable)(PCONDITION_VARIABLE ConditionVariable);
    static VOID (WINAPI *WakeAllConditionVariable)(PCONDITION_VARIABLE ConditionVariable);
    static BOOL (WINAPI *SleepConditionVariableCS)(PCONDITION_VARIABLE ConditionVariable, PCRITICAL_SECTION CriticalSection, DWORD dwMilliseconds);
    CONDITION_VARIABLE cv[1];                       // Condition variable (Windows)

    struct WaitingThread {                         // For Windows XP fallback
        Semaphore      sem;
        WaitingThread *next;
    };
    Mutex          mutex;
    WaitingThread *head, *tail;
#endif

#ifdef PLATFORM_POSIX
    pthread_cond_t cv[1];                          // Condition variable (POSIX)
#endif

public:
    bool Wait(Mutex& m, int timeout_ms = -1);      // Wait with mutex and optional timeout
    void Signal();                                 // Signal one waiting thread
    void Broadcast();                              // Signal all waiting threads

    ConditionVariable();                           // Constructor
    ~ConditionVariable();                          // Destructor
};
```

### STL Equivalent
```cpp
#include <condition_variable>

class std::condition_variable {
public:
    condition_variable();                          // Constructor
    ~condition_variable();                         // Destructor
    condition_variable(const condition_variable&) = delete; // No copy constructor
    condition_variable& operator=(const condition_variable&) = delete; // No copy assignment

    void notify_one() noexcept;                    // Notify one waiting thread
    void notify_all() noexcept;                    // Notify all waiting threads
    
    void wait(std::unique_lock<std::mutex>& lock); // Wait with unique_lock
    template< class Predicate >
    void wait(std::unique_lock<std::mutex>& lock, Predicate pred); // Wait with predicate

    template< class Rep, class Period >
    std::cv_status wait_for(std::unique_lock<std::mutex>& lock, 
                            const std::chrono::duration<Rep, Period>& rel_time); // Wait for duration
    template< class Rep, class Period, class Predicate >
    bool wait_for(std::unique_lock<std::mutex>& lock,
                  const std::chrono::duration<Rep, Period>& rel_time,
                  Predicate pred);                 // Wait for duration with predicate

    template< class Clock, class Duration >
    std::cv_status wait_until(std::unique_lock<std::mutex>& lock,
                              const std::chrono::time_point<Clock, Duration>& abs_time); // Wait until time
    template< class Clock, class Duration, class Predicate >
    bool wait_until(std::unique_lock<std::mutex>& lock,
                    const std::chrono::time_point<Clock, Duration>& abs_time,
                    Predicate pred);               // Wait until time with predicate
};

enum class std::cv_status { no_timeout, timeout }; // Status for timed waits
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| ConditionVariable | std::condition_variable | ✓ Complete | |
| ConditionVariable::Wait(m) | std::condition_variable::wait(lock) | ✓ Complete | |
| ConditionVariable::Signal() | std::condition_variable::notify_one() | ✓ Complete | |
| ConditionVariable::Broadcast() | std::condition_variable::notify_all() | ✓ Complete | |
| ConditionVariable::Wait(m, timeout) | std::condition_variable::wait_for() | ✓ Complete | |

### Conversion Notes
- U++ ConditionVariable maps directly to std::condition_variable
- U++ ConditionVariable::Wait() requires a U++ Mutex, while STL requires std::unique_lock<std::mutex>
- U++ ConditionVariable::Signal() maps to std::condition_variable::notify_one()
- U++ ConditionVariable::Broadcast() maps to std::condition_variable::notify_all()
- U++ ConditionVariable::Wait() with timeout maps to std::condition_variable's time-based methods

## 5. RWMutex ↔ std::shared_mutex (C++17)

### U++ Declaration
```cpp
class RWMutex : NoCopy {
#ifdef PLATFORM_WIN32
    LONG   m_count, m_rdwake;                      // Count and wake flags (Windows)
    HANDLE m_wrwset, m_rdwset;                     // Event handles for waiting (Windows)
    CRITICAL_SECTION m_wrlock;                     // Write lock critical section (Windows)
#endif
#ifdef PLATFORM_POSIX
    pthread_rwlock_t rwlock[1];                    // Read-write lock (POSIX)
#endif

public:
    void EnterWrite();                             // Enter write lock
    void LeaveWrite();                             // Leave write lock
    void EnterRead();                              // Enter read lock
    void LeaveRead();                              // Leave read lock

    RWMutex();                                    // Constructor
    ~RWMutex();                                   // Destructor

    class ReadLock;                               // RAII read lock
    class WriteLock;                              // RAII write lock
};

class RWMutex::ReadLock : NoCopy {
    RWMutex& s;                                   // Reference to RWMutex

public:
    ReadLock(RWMutex& s);                         // Constructor (acquires read lock)
    ~ReadLock();                                  // Destructor (releases lock)
};

class RWMutex::WriteLock : NoCopy {
    RWMutex& s;                                   // Reference to RWMutex

public:
    WriteLock(RWMutex& s);                        // Constructor (acquires write lock)
    ~WriteLock();                                 // Destructor (releases lock)
};
```

### STL Equivalent
```cpp
#include <shared_mutex>

class std::shared_mutex {
public:
    shared_mutex();                                // Constructor
    ~shared_mutex();                               // Destructor
    shared_mutex(const shared_mutex&) = delete;    // No copy constructor
    shared_mutex& operator=(const shared_mutex&) = delete; // No copy assignment

    void lock();                                   // Exclusive lock (write)
    bool try_lock();                               // Try exclusive lock
    void unlock();                                 // Unlock exclusive
    void lock_shared();                            // Shared lock (read)
    bool try_lock_shared();                        // Try shared lock
    void unlock_shared();                          // Unlock shared
};

class std::shared_lock<shared_mutex> {
    shared_mutex* mtx;                             // Pointer to mutex
    bool owns;                                     // Whether owns the lock

public:
    using mutex_type = shared_mutex;               // Type alias
    shared_lock() noexcept;                        // Default constructor
    explicit shared_lock(shared_mutex& m);         // Constructor (acquires shared lock)
    shared_lock(shared_mutex& m, std::defer_lock_t); // Constructor (no lock)
    shared_lock(shared_mutex& m, std::try_to_lock_t); // Constructor (try lock)
    shared_lock(shared_mutex& m, std::adopt_lock_t); // Constructor (assume owns)
    ~shared_lock();                                // Destructor (releases if owned)
    shared_lock(const shared_lock&) = delete;      // No copy constructor
    shared_lock& operator=(const shared_lock&) = delete; // No copy assignment
    shared_lock(shared_lock&& other) noexcept;     // Move constructor
    shared_lock& operator=(shared_lock&& other) noexcept; // Move assignment
    void lock();                                   // Acquire shared lock
    bool try_lock();                               // Try to acquire shared lock
    void unlock();                                 // Release shared lock
    bool owns_lock() const noexcept;               // Check if owns lock
};

// For read lock RAII: std::shared_lock<std::shared_mutex>
// For write lock RAII: std::lock_guard<std::shared_mutex> or std::unique_lock<std::shared_mutex>
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| RWMutex | std::shared_mutex | ✓ Complete | |
| RWMutex::EnterRead() | std::shared_mutex::lock_shared() | ✓ Complete | |
| RWMutex::LeaveRead() | std::shared_mutex::unlock_shared() | ✓ Complete | |
| RWMutex::EnterWrite() | std::shared_mutex::lock() | ✓ Complete | |
| RWMutex::LeaveWrite() | std::shared_mutex::unlock() | ✓ Complete | |
| RWMutex::ReadLock | std::shared_lock<std::shared_mutex> | ✓ Complete | |
| RWMutex::WriteLock | std::lock_guard<std::shared_mutex> or std::unique_lock<std::shared_mutex> | ✓ Complete | |

### Conversion Notes
- U++ RWMutex maps directly to std::shared_mutex for read-write locking
- U++ RWMutex::EnterRead/LeaveRead maps to std::shared_mutex::lock_shared/unlock_shared
- U++ RWMutex::EnterWrite/LeaveWrite maps to std::shared_mutex::lock/unlock
- U++ RWMutex::ReadLock maps to std::shared_lock<std::shared_mutex>
- U++ RWMutex::WriteLock maps to std::lock_guard<std::shared_mutex> or std::unique_lock<std::shared_mutex>

## 6. Atomic Operations ↔ std::atomic

### U++ Declaration
```cpp
// Atomic is a class template that wraps platform-specific atomic operations:
class Atomic {
    int value;                                     // The atomic value

public:
    Atomic();                                      // Default constructor
    Atomic(int init);                              // Constructor with initial value
    operator int() const;                          // Conversion to int
    int operator=(int newvalue);                   // Assignment
    int operator++();                              // Pre-increment
    int operator--();                              // Pre-decrement
    int operator++(int);                           // Post-increment
    int operator--(int);                           // Post-decrement
    Atomic& operator+=(int x);                     // Add and assign
    Atomic& operator-=(int x);                     // Subtract and assign
    int operator+=(int x) volatile;                // Add and return (volatile)
    int operator-=(int x) volatile;                // Subtract and return (volatile)
    int Read() const;                              // Read value
    int Write(int newvalue);                       // Write new value
};

// Usage in other classes:
struct Prec {
    PteBase *ptr;
    Atomic   n;                                    // Reference counter
};

// These functions are also defined for atomic operations:
int AtomicInc(int& x);                             // Atomic increment
int AtomicDec(int& x);                             // Atomic decrement
int AtomicXchg(int& x, int newvalue);             // Atomic exchange
int AtomicAdd(int& x, int v);                     // Atomic add
bool AtomicCmpXchg(int& x, int newvalue, int condition); // Atomic compare and exchange
```

### STL Equivalent
```cpp
#include <atomic>

template< class T >
struct std::atomic {
    bool is_lock_free() const noexcept;            // Check if lock-free
    void store(T desired, std::memory_order order = std::memory_order_seq_cst) noexcept; // Store value
    T load(std::memory_order order = std::memory_order_seq_cst) const noexcept; // Load value
    T operator=(T desired) noexcept;                // Assignment
    operator T() const noexcept;                    // Conversion operator
    T exchange(T desired, std::memory_order order = std::memory_order_seq_cst) noexcept; // Exchange
    bool compare_exchange_weak(T& expected, T desired, 
                               std::memory_order success, std::memory_order failure) noexcept; // Compare exchange weak
    bool compare_exchange_strong(T& expected, T desired, 
                                 std::memory_order success, std::memory_order failure) noexcept; // Compare exchange strong
    
    // Arithmetic operations for integral types
    T fetch_add(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept;
    T fetch_sub(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept;
    T operator++() noexcept;                        // Pre-increment
    T operator++(int) noexcept;                     // Post-increment
    T operator--() noexcept;                        // Pre-decrement
    T operator--(int) noexcept;                     // Post-decrement
    T operator+=(T arg) noexcept;                   // Add and return
    T operator-=(T arg) noexcept;                   // Subtract and return
    
    atomic() noexcept = default;                    // Default constructor
    constexpr atomic(T desired) noexcept;          // Constructor with value
    atomic(const atomic&) = delete;                // No copy constructor
    atomic& operator=(const atomic&) = delete;     // No copy assignment
    atomic& operator=(T desired) noexcept;         // Assignment operator
};

// Specific specializations for common types
template<> struct std::atomic<int> { /* ... */ };
template<> struct std::atomic<unsigned int> { /* ... */ };
// ... other integer types
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Atomic | std::atomic<int> | ✓ Complete | |
| Atomic() | std::atomic<int>() | ✓ Complete | |
| Atomic(init) | std::atomic<int>(init) | ✓ Complete | |
| Atomic conversion to int | std::atomic<int> conversion to int | ✓ Complete | |
| Atomic assignment | std::atomic<int> assignment | ✓ Complete | |
| Atomic pre-increment | std::atomic<int>::operator++() | ✓ Complete | |
| Atomic post-increment | std::atomic<int>::operator++(int) | ✓ Complete | |
| Atomic pre-decrement | std::atomic<int>::operator--() | ✓ Complete | |
| Atomic post-decrement | std::atomic<int>::operator--(int) | ✓ Complete | |
| Atomic::Read() | std::atomic<int>::load() | ✓ Complete | |
| Atomic::Write() | std::atomic<int>::store() | ✓ Complete | |
| AtomicAdd(x, v) | std::atomic_fetch_add(x, v) | ✓ Complete | |
| AtomicInc(x) | std::atomic_fetch_add(x, 1) | ✓ Complete | |
| AtomicDec(x) | std::atomic_fetch_sub(x, 1) | ✓ Complete | |
| AtomicCmpXchg(x, newv, cond) | std::atomic_compare_exchange_strong() | ✓ Complete | |

### Conversion Notes
- U++ Atomic maps directly to std::atomic<int> for integer operations
- U++ Atomic provides convenience methods that are equivalent to std::atomic operations
- U++ AtomicInc/Dec functions map to std::atomic_fetch_add/sub operations
- U++ AtomicCmpXchg maps to std::atomic_compare_exchange_strong
- std::atomic provides more memory ordering options than U++ Atomic

## Summary of Threading Mappings

| U++ Threading Type | STL Equivalent | Notes |
|-------------------|----------------|-------|
| Thread | std::thread | Direct mapping with similar functionality |
| Mutex | std::mutex | Direct mapping with RAII lock guards |
| Semaphore | std::counting_semaphore (C++20) | Custom implementation needed for pre-C++20 |
| ConditionVariable | std::condition_variable | Direct mapping |
| RWMutex | std::shared_mutex | Direct mapping for read-write locks |
| Atomic | std::atomic<int> | Direct mapping for atomic operations |