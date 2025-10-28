#pragma once
#ifndef _Core_Daemon_h_
#define _Core_Daemon_h_

#include "Core.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// Simple daemon class for background services
class Daemon {
private:
    std::thread work_thread;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> should_stop{false};
    std::atomic<bool> is_running{false};
    String name;
    
public:
    Daemon(const String& daemon_name = "Daemon");
    virtual ~Daemon();
    
    // Start the daemon
    bool Start();
    
    // Stop the daemon
    void Stop();
    
    // Check if the daemon is running
    bool IsRunning() const { return is_running.load(); }
    
    // Check if daemon should stop
    bool ShouldStop() const { return should_stop.load(); }
    
    // Get daemon name
    const String& GetName() const { return name; }
    
    // Main work function - override in derived classes
    virtual void Work() = 0;
    
    // Service function - override in derived classes
    virtual void Service() { Work(); }
    
    // Wait for the daemon to finish
    void Join();
    
    // Get thread ID
    std::thread::id GetThreadId() const;
    
private:
    // Internal worker function
    void Worker();
};

// Timer daemon for periodic tasks
class TimerDaemon : public Daemon {
private:
    int interval_ms;
    std::atomic<int64_t> tick_count{0};
    
public:
    TimerDaemon(int interval = 1000, const String& daemon_name = "TimerDaemon");
    
    // Set interval in milliseconds
    void SetInterval(int ms) { interval_ms = ms; }
    int GetInterval() const { return interval_ms; }
    
    // Get tick count
    int64_t GetTickCount() const { return tick_count.load(); }
    
    // Override work function for timer functionality
    virtual void Work() override;
    
    // Override service function
    virtual void Service() override;
};

// File system monitor daemon
class FSWatcherDaemon : public Daemon {
private:
    Vector<String> watch_paths;
    int poll_interval;
    
public:
    FSWatcherDaemon(int poll_interval_ms = 1000, const String& daemon_name = "FSWatcherDaemon");
    
    // Add path to watch
    void AddWatchPath(const String& path);
    
    // Remove path from watch
    void RemoveWatchPath(const String& path);
    
    // Clear all paths
    void ClearWatchPaths();
    
    // Override work function for file system monitoring
    virtual void Work() override;
    
    // Override service function
    virtual void Service() override;
    
    // Callback for when a file changes
    virtual void OnFileChange(const String& filepath);
};

// Network daemon for handling network operations
class NetworkDaemon : public Daemon {
private:
    int port;
    bool listening;
    
public:
    NetworkDaemon(int port_num = 8080, const String& daemon_name = "NetworkDaemon");
    
    // Start listening on port
    bool Listen();
    
    // Stop listening
    void StopListening();
    
    // Override work function for network operations
    virtual void Work() override;
    
    // Override service function
    virtual void Service() override;
    
    // Handle incoming connection
    virtual void OnConnection(int socket_fd);
};

#endif