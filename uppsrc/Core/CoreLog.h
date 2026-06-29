#ifndef _Core_CoreLog_h_
#define _Core_CoreLog_h_

class CoreLog {
public:
	CoreLog() {}

	void SetSink(AppLog* sink) { log_sink_ = sink; }
	AppLog* GetSink() const    { return log_sink_; }

	void Add(int level, const String& component, const String& message);
	void Clear() { log_.Clear(); }

	const Vector<String>& GetLog() const { return log_; }

private:
	AppLog* log_sink_ = nullptr;
	mutable Vector<String> log_;
};

inline void LogInfo(CoreLog& log, const String& component, const String& message)
{
	log.Add(APPLOG_LEVEL_INFO, component, message);
}

inline void LogInfo(AppLog& log, const String& component, const String& message)
{
	log.Add(APPLOG_LEVEL_INFO, component, message);
}

inline void LogWarn(CoreLog& log, const String& component, const String& message)
{
	log.Add(APPLOG_LEVEL_WARNING, component, message);
}

inline void LogWarn(AppLog& log, const String& component, const String& message)
{
	log.Add(APPLOG_LEVEL_WARNING, component, message);
}

inline void LogError(CoreLog& log, const String& component, const String& message)
{
	log.Add(APPLOG_LEVEL_ERROR, component, message);
}

inline void LogError(AppLog& log, const String& component, const String& message)
{
	log.Add(APPLOG_LEVEL_ERROR, component, message);
}

inline void LogDebug(CoreLog& log, const String& component, const String& message)
{
	log.Add(APPLOG_LEVEL_DEBUG, component, message);
}

inline void LogDebug(AppLog& log, const String& component, const String& message)
{
	log.Add(APPLOG_LEVEL_DEBUG, component, message);
}

#endif
