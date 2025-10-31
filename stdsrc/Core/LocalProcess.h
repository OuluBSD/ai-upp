#pragma once
#ifndef _Core_LocalProcess_h_
#define _Core_LocalProcess_h_

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <future>
#include "Core.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
extern char **environ;
#endif

// LocalProcess - platform-independent process execution for stdsrc

class LocalProcess {
private:
#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    HANDLE hInputWrite;
    HANDLE hInputRead;
    HANDLE hOutputWrite;
    HANDLE hOutputRead;
    HANDLE hErrorWrite;
    HANDLE hErrorRead;
#else
    pid_t pid;
    int stdin_pipe[2];
    int stdout_pipe[2];
    int stderr_pipe[2];
#endif
    
    bool running;
    int exit_code;
    std::string working_dir;
    std::vector<std::string> environment;
    
    // Threading for async operations
    std::thread io_thread;
    std::mutex io_mutex;
    std::condition_variable io_cv;
    bool io_finished;
    
    // Buffers for I/O
    std::string stdout_buffer;
    std::string stderr_buffer;
    std::string stdin_buffer;
    
    // Callbacks
    std::function<void(const std::string&)> stdout_callback;
    std::function<void(const std::string&)> stderr_callback;
    
    void InitializePipes();
    void ClosePipes();
    void StartIOThread();
    void IOThreadFunc();
    
public:
    // Constructors
    LocalProcess();
    LocalProcess(const LocalProcess& other) = delete;
    LocalProcess(LocalProcess&& other) noexcept;
    LocalProcess& operator=(const LocalProcess& other) = delete;
    LocalProcess& operator=(LocalProcess&& other) noexcept;
    
    // Destructor
    ~LocalProcess();
    
    // Process execution
    bool Start(const std::string& command, const std::vector<std::string>& args = {});
    bool Start(const std::vector<std::string>& command_and_args);
    
    // I/O operations
    bool Write(const std::string& data);
    bool WriteLine(const std::string& line);
    
    std::string Read();
    std::string ReadLine();
    std::string ReadAll();
    
    std::string ReadError();
    std::string ReadErrorLine();
    std::string ReadAllError();
    
    // Process control
    bool Wait(int timeout_ms = -1);
    bool Terminate();
    bool Kill();
    
    // Status
    bool IsRunning() const;
    int GetExitCode() const;
    bool HasExited() const;
    
    // Configuration
    void SetWorkingDirectory(const std::string& dir);
    void SetEnvironment(const std::vector<std::string>& env);
    void AddEnvironmentVariable(const std::string& key, const std::string& value);
    
    // Callbacks for asynchronous output
    void SetStdoutCallback(std::function<void(const std::string&)> callback);
    void SetStderrCallback(std::function<void(const std::string&)> callback);
    
    // Utility functions
    static std::string Quote(const std::string& arg);
    static std::vector<std::string> SplitCommandLine(const std::string& command_line);
    
    // Platform-specific functions
#ifdef _WIN32
    PROCESS_INFORMATION GetProcessInfo() const { return process_info; }
    DWORD GetProcessId() const { return process_info.dwProcessId; }
#else
    pid_t GetProcessId() const { return pid; }
#endif
    
    // Static utility functions
    static bool Execute(const std::string& command, 
                       const std::vector<std::string>& args,
                       std::string& output, 
                       std::string& error, 
                       int timeout_ms = -1);
    
    static std::pair<std::string, std::string> Execute(const std::string& command,
                                                        const std::vector<std::string>& args = {},
                                                        int timeout_ms = -1);
    
    // Async execution
    std::future<std::pair<std::string, std::string>> ExecuteAsync(const std::string& command,
                                                                  const std::vector<std::string>& args = {});
    
    // Streaming operators
    template<typename Stream>
    void Serialize(Stream& s) {
        // Processes are not serializable, but we can serialize configuration
        s % working_dir % environment;
    }
    
    // String representation
    std::string ToString() const;
};

// Global functions
inline std::pair<std::string, std::string> ExecuteProcess(const std::string& command,
                                                           const std::vector<std::string>& args = {},
                                                           int timeout_ms = -1) {
    return LocalProcess::Execute(command, args, timeout_ms);
}

inline bool ExecuteProcess(const std::string& command,
                          const std::vector<std::string>& args,
                          std::string& output,
                          std::string& error,
                          int timeout_ms = -1) {
    return LocalProcess::Execute(command, args, output, error, timeout_ms);
}

// Streaming operator
template<typename Stream>
void operator%(Stream& s, LocalProcess& process) {
    process.Serialize(s);
}

// String conversion
inline std::string AsString(const LocalProcess& process) {
    return process.ToString();
}

// Implementation details
#ifdef _WIN32

inline void LocalProcess::InitializePipes() {
    // Create pipes for stdin, stdout, and stderr
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    
    // Stdin pipe
    if (!CreatePipe(&hInputRead, &hInputWrite, &sa, 0)) {
        throw std::runtime_error("Failed to create stdin pipe");
    }
    
    // Stdout pipe
    if (!CreatePipe(&hOutputRead, &hOutputWrite, &sa, 0)) {
        throw std::runtime_error("Failed to create stdout pipe");
    }
    
    // Stderr pipe
    if (!CreatePipe(&hErrorRead, &hErrorWrite, &sa, 0)) {
        throw std::runtime_error("Failed to create stderr pipe");
    }
    
    // Prevent child processes from inheriting write ends
    SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hOutputRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hErrorRead, HANDLE_FLAG_INHERIT, 0);
}

inline bool LocalProcess::Start(const std::string& command, const std::vector<std::string>& args) {
    // Prepare command line
    std::string cmd_line = Quote(command);
    for (const auto& arg : args) {
        cmd_line += " " + Quote(arg);
    }
    
    // Initialize process startup info
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hInputRead;
    si.hStdOutput = hOutputWrite;
    si.hStdError = hErrorWrite;
    
    // Create process
    if (!CreateProcess(NULL, const_cast<char*>(cmd_line.c_str()), NULL, NULL, TRUE, 0, NULL, 
                      working_dir.empty() ? NULL : working_dir.c_str(), &si, &process_info)) {
        return false;
    }
    
    running = true;
    StartIOThread();
    return true;
}

inline bool LocalProcess::Wait(int timeout_ms) {
    if (!running) return true;
    
    DWORD timeout = timeout_ms < 0 ? INFINITE : static_cast<DWORD>(timeout_ms);
    DWORD result = WaitForSingleObject(process_info.hProcess, timeout);
    
    if (result == WAIT_OBJECT_0) {
        DWORD exit_code;
        if (GetExitCodeProcess(process_info.hProcess, &exit_code)) {
            this->exit_code = static_cast<int>(exit_code);
        }
        running = false;
        return true;
    }
    
    return false;
}

inline bool LocalProcess::Terminate() {
    if (!running) return true;
    
    return TerminateProcess(process_info.hProcess, 1) != FALSE;
}

inline bool LocalProcess::Kill() {
    return Terminate();
}

#else // POSIX

inline void LocalProcess::InitializePipes() {
    // Create pipes for stdin, stdout, and stderr
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
        throw std::runtime_error("Failed to create pipes");
    }
}

inline bool LocalProcess::Start(const std::string& command, const std::vector<std::string>& args) {
    // Prepare arguments
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(command.c_str()));
    for (const auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);
    
    // Set up file actions for posix_spawn
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    
    posix_spawn_file_actions_adddup2(&file_actions, stdin_pipe[0], STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&file_actions, stdout_pipe[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&file_actions, stderr_pipe[1], STDERR_FILENO);
    
    posix_spawn_file_actions_addclose(&file_actions, stdin_pipe[0]);
    posix_spawn_file_actions_addclose(&file_actions, stdin_pipe[1]);
    posix_spawn_file_actions_addclose(&file_actions, stdout_pipe[0]);
    posix_spawn_file_actions_addclose(&file_actions, stdout_pipe[1]);
    posix_spawn_file_actions_addclose(&file_actions, stderr_pipe[0]);
    posix_spawn_file_actions_addclose(&file_actions, stderr_pipe[1]);
    
    // Set up attributes
    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    
    // Environment
    char** envp = environ;
    if (!environment.empty()) {
        std::vector<char*> env_vars;
        for (const auto& env_var : environment) {
            env_vars.push_back(const_cast<char*>(env_var.c_str()));
        }
        env_vars.push_back(nullptr);
        envp = env_vars.data();
    }
    
    // Spawn process
    int result = posix_spawn(&pid, command.c_str(), &file_actions, &attr, argv.data(), envp);
    
    posix_spawn_file_actions_destroy(&file_actions);
    posix_spawnattr_destroy(&attr);
    
    if (result == 0) {
        running = true;
        StartIOThread();
        return true;
    }
    
    return false;
}

inline bool LocalProcess::Wait(int timeout_ms) {
    if (!running) return true;
    
    if (timeout_ms < 0) {
        int status;
        if (waitpid(pid, &status, 0) == pid) {
            if (WIFEXITED(status)) {
                exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                exit_code = -1;
            }
            running = false;
            return true;
        }
    } else {
        // Polling with timeout
        auto start_time = std::chrono::steady_clock::now();
        auto end_time = start_time + std::chrono::milliseconds(timeout_ms);
        
        while (std::chrono::steady_clock::now() < end_time) {
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                if (WIFEXITED(status)) {
                    exit_code = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    exit_code = -1;
                }
                running = false;
                return true;
            } else if (result == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            } else {
                return false;
            }
        }
    }
    
    return false;
}

inline bool LocalProcess::Terminate() {
    if (!running) return true;
    
    return kill(pid, SIGTERM) == 0;
}

inline bool LocalProcess::Kill() {
    if (!running) return true;
    
    return kill(pid, SIGKILL) == 0;
}

#endif

// Platform-independent implementations
inline LocalProcess::LocalProcess() 
    : running(false), exit_code(0), io_finished(false) {
    InitializePipes();
}

inline LocalProcess::~LocalProcess() {
    if (running) {
        Terminate();
        Wait();
    }
    ClosePipes();
}

inline void LocalProcess::ClosePipes() {
#ifdef _WIN32
    if (hInputWrite) CloseHandle(hInputWrite);
    if (hInputRead) CloseHandle(hInputRead);
    if (hOutputWrite) CloseHandle(hOutputWrite);
    if (hOutputRead) CloseHandle(hOutputRead);
    if (hErrorWrite) CloseHandle(hErrorWrite);
    if (hErrorRead) CloseHandle(hErrorRead);
    if (process_info.hProcess) CloseHandle(process_info.hProcess);
    if (process_info.hThread) CloseHandle(process_info.hThread);
#else
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);
#endif
}

inline void LocalProcess::SetWorkingDirectory(const std::string& dir) {
    working_dir = dir;
}

inline void LocalProcess::SetEnvironment(const std::vector<std::string>& env) {
    environment = env;
}

inline void LocalProcess::AddEnvironmentVariable(const std::string& key, const std::string& value) {
    environment.push_back(key + "=" + value);
}

inline bool LocalProcess::IsRunning() const {
    return running;
}

inline int LocalProcess::GetExitCode() const {
    return exit_code;
}

inline bool LocalProcess::HasExited() const {
    return !running;
}

inline std::string LocalProcess::Quote(const std::string& arg) {
#ifdef _WIN32
    // Windows quoting rules
    if (arg.find_first_of(" \t\n\v\"") == std::string::npos) {
        return arg;
    }
    
    std::string quoted = "\"";
    int backslash_count = 0;
    
    for (char c : arg) {
        if (c == '\\') {
            backslash_count++;
        } else if (c == '"') {
            quoted.append(backslash_count * 2 + 1, '\\');
            backslash_count = 0;
            quoted += '"';
        } else {
            quoted.append(backslash_count, '\\');
            backslash_count = 0;
            quoted += c;
        }
    }
    
    quoted.append(backslash_count * 2, '\\');
    quoted += '"';
    return quoted;
#else
    // POSIX quoting rules
    if (arg.find_first_of(" \t\n\v\"'\\$") == std::string::npos) {
        return arg;
    }
    
    std::string quoted = "'";
    for (char c : arg) {
        if (c == '\'') {
            quoted += "'\"'\"'";
        } else {
            quoted += c;
        }
    }
    quoted += "'";
    return quoted;
#endif
}

inline std::string LocalProcess::ToString() const {
    return "LocalProcess(running=" + std::to_string(running) + 
           ", exit_code=" + std::to_string(exit_code) + ")";
}

#endif