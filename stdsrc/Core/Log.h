// Simple logging utilities and LOG macro

namespace LogDetail {
    enum class Level { Debug = 0, Info = 1, Warn = 2, Error = 3 };

    struct State {
        inline static std::mutex mtx;
        inline static std::ofstream file;
        inline static bool to_stderr = true;
        inline static Level min_level = Level::Info;
    };

    inline const char* LevelText(Level l) {
        switch(l) {
            case Level::Debug: return "DEBUG";
            case Level::Info:  return "INFO";
            case Level::Warn:  return "WARN";
            case Level::Error: return "ERROR";
        }
        return "INFO";
    }

    inline std::string NowIsoLocal() {
        using namespace std::chrono;
        auto now = system_clock::now();
        std::time_t tt = system_clock::to_time_t(now);
        std::tm lt{};
    #ifdef _WIN32
        localtime_s(&lt, &tt);
    #else
        localtime_r(&tt, &lt);
    #endif
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                      lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec);
        return std::string(buf);
    }
}

// Public API (ends up in Upp namespace via Core.h wrapper)

inline void LogSetLevel(int level) {
    using LogDetail::Level; using LogDetail::State;
    std::lock_guard<std::mutex> lk(State::mtx);
    if(level <= 0) State::min_level = Level::Debug;
    else if(level == 1) State::min_level = Level::Info;
    else if(level == 2) State::min_level = Level::Warn;
    else State::min_level = Level::Error;
}

inline void LogToFile(const char* path, bool append = true) {
    using LogDetail::State;
    std::lock_guard<std::mutex> lk(State::mtx);
    if(State::file.is_open()) State::file.close();
    std::ios::openmode m = std::ios::out | std::ios::binary;
    if(append) m |= std::ios::app; else m |= std::ios::trunc;
    State::file.open(path ? path : "log.txt", m);
}

inline void LogToStdErr(bool enable = true) {
    using LogDetail::State;
    std::lock_guard<std::mutex> lk(State::mtx);
    State::to_stderr = enable;
}

inline void LogWriteImpl(int level, const char* file, int line, const std::string& msg) {
    using LogDetail::Level; using LogDetail::State; using LogDetail::LevelText; using LogDetail::NowIsoLocal;
    Level lev = Level::Info;
    if(level <= 0) lev = Level::Debug; else if(level == 1) lev = Level::Info; else if(level == 2) lev = Level::Warn; else lev = Level::Error;
    std::lock_guard<std::mutex> lk(State::mtx);
    if((int)lev < (int)State::min_level) return;
    std::string ts = NowIsoLocal();
    std::string linebuf;
    linebuf.reserve(64 + msg.size());
    linebuf.append("[").append(LevelText(lev)).append("] ").append(ts).append(" ");
    if(file) {
        linebuf.append("(").append(file);
        if(line > 0) { char num[16]; std::snprintf(num, sizeof(num), ":%d", line); linebuf.append(num); }
        linebuf.append(") ");
    }
    linebuf.append(msg);
    linebuf.push_back('\n');
    if(State::file.is_open()) {
        State::file.write(linebuf.data(), (std::streamsize)linebuf.size());
        State::file.flush();
    }
    if(State::to_stderr) {
        std::fwrite(linebuf.data(), 1, linebuf.size(), stderr);
        std::fflush(stderr);
    }
}

#define LOG_FILE 0x01
#define LOG_COUT 0x02

inline void StdLogSetup(int flags, const char* path = nullptr, bool append = true) {
    LogToStdErr((flags & LOG_COUT) != 0);
    if(flags & LOG_FILE) {
        String p;
        if(path)
            p = String(path);
        else
            p = AppendFileName(GetTempPath(), "std.log");
        LogToFile(p.Begin(), append);
    }
}

#ifndef LOG
#define LOG(expr) do { std::ostringstream _upp_log_os_; _upp_log_os_ << expr; Upp::LogWriteImpl(1, __FILE__, __LINE__, _upp_log_os_.str()); } while(0)
#endif
