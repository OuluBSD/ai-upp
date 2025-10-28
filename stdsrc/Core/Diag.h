#ifndef _Core_Diag_h_
#define _Core_Diag_h_

#include "Core.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>

// Diagnostic levels
enum class DiagnosticLevel {
    None = 0,
    Error = 1,
    Warning = 2,
    Info = 3,
    Debug = 4,
    Verbose = 5
};

// Diagnostic message structure
struct DiagnosticMessage {
    DiagnosticLevel level;
    String category;
    String message;
    String file;
    int line;
    std::chrono::steady_clock::time_point timestamp;
    std::thread::id thread_id;
    
    DiagnosticMessage(DiagnosticLevel lvl, const String& cat, const String& msg, 
                     const String& f, int l) 
        : level(lvl), category(cat), message(msg), file(f), line(l), 
          timestamp(std::chrono::steady_clock::now()), thread_id(std::this_thread::get_id()) {}
};

// Diagnostic handler interface
class DiagnosticHandler {
public:
    virtual ~DiagnosticHandler() = default;
    virtual void HandleDiagnostic(const DiagnosticMessage& msg) = 0;
};

// Console diagnostic handler
class ConsoleDiagnosticHandler : public DiagnosticHandler {
public:
    void HandleDiagnostic(const DiagnosticMessage& msg) override;
};

// File diagnostic handler
class FileDiagnosticHandler : public DiagnosticHandler {
private:
    std::ofstream file_stream;
    
public:
    FileDiagnosticHandler(const String& filename);
    ~FileDiagnosticHandler();
    void HandleDiagnostic(const DiagnosticMessage& msg) override;
};

// Diagnostic system
class Diagnostics {
private:
    static std::vector<std::shared_ptr<DiagnosticHandler>> handlers;
    static std::mutex diag_mutex;
    static DiagnosticLevel min_level;
    static std::map<String, DiagnosticLevel> category_levels;
    
public:
    static void AddHandler(std::shared_ptr<DiagnosticHandler> handler);
    static void RemoveHandler(std::shared_ptr<DiagnosticHandler> handler);
    static void SetMinLevel(DiagnosticLevel level);
    static void SetCategoryLevel(const String& category, DiagnosticLevel level);
    static DiagnosticLevel GetCategoryLevel(const String& category);
    static void Log(DiagnosticLevel level, const String& category, const String& message, 
                   const String& file = "", int line = 0);
    
    static void Error(const String& category, const String& message, 
                     const String& file = "", int line = 0) {
        Log(DiagnosticLevel::Error, category, message, file, line);
    }
    
    static void Warning(const String& category, const String& message, 
                       const String& file = "", int line = 0) {
        Log(DiagnosticLevel::Warning, category, message, file, line);
    }
    
    static void Info(const String& category, const String& message, 
                    const String& file = "", int line = 0) {
        Log(DiagnosticLevel::Info, category, message, file, line);
    }
    
    static void Debug(const String& category, const String& message, 
                     const String& file = "", int line = 0) {
        Log(DiagnosticLevel::Debug, category, message, file, line);
    }
    
    static void Verbose(const String& category, const String& message, 
                       const String& file = "", int line = 0) {
        Log(DiagnosticLevel::Verbose, category, message, file, line);
    }
    
    static void Flush();
};

// Diagnostic macros
#define DIAG_ERROR(cat, msg) Diagnostics::Error(cat, msg, __FILE__, __LINE__)
#define DIAG_WARNING(cat, msg) Diagnostics::Warning(cat, msg, __FILE__, __LINE__)
#define DIAG_INFO(cat, msg) Diagnostics::Info(cat, msg, __FILE__, __LINE__)
#define DIAG_DEBUG(cat, msg) Diagnostics::Debug(cat, msg, __FILE__, __LINE__)
#define DIAG_VERBOSE(cat, msg) Diagnostics::Verbose(cat, msg, __FILE__, __LINE__)

// Performance diagnostic
class PerfDiagnostic {
private:
    String operation_name;
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point last_checkpoint;
    
public:
    PerfDiagnostic(const String& name);
    ~PerfDiagnostic();
    
    void Checkpoint(const String& checkpoint_name);
};

// Memory diagnostic
class MemoryDiagnostic {
public:
    static size_t GetUsedMemory();
    static size_t GetPeakMemory();
    static void ReportMemoryUsage();
    static void ResetPeakMemory();
};

// Resource diagnostic
class ResourceDiagnostic {
private:
    static std::map<String, int> resource_counts;
    static std::mutex resource_mutex;
    
public:
    static void RegisterResource(const String& resource_type);
    static void UnregisterResource(const String& resource_type);
    static int GetResourceCount(const String& resource_type);
    static void ReportResourceUsage();
};

// Diagnostic utilities
class DiagnosticUtils {
public:
    static String LevelToString(DiagnosticLevel level);
    static String FormatTimestamp(const std::chrono::steady_clock::time_point& tp);
    static String FormatDuration(const std::chrono::steady_clock::time_point& start, 
                                const std::chrono::steady_clock::time_point& end);
};

// Scoped diagnostic for function calls
class FunctionDiagnostic {
private:
    String function_name;
    
public:
    FunctionDiagnostic(const String& func_name);
    ~FunctionDiagnostic();
};

#define FUNC_DIAG() FunctionDiagnostic _func_diag_(__FUNCTION__)

#endif