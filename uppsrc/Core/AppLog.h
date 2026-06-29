#ifndef _Core_AppLog_h_
#define _Core_AppLog_h_

enum AppLogLevel {
	APPLOG_LEVEL_TRACE   = 0,
	APPLOG_LEVEL_DEBUG   = 1,
	APPLOG_LEVEL_INFO    = 2,
	APPLOG_LEVEL_WARNING = 3,
	APPLOG_LEVEL_ERROR   = 4,
};

struct AppLogRecord : Moveable<AppLogRecord> {
	Time   time;
	int    level    = APPLOG_LEVEL_INFO;
	String channel;
	String message;
	String file;
	int    line     = 0;
	String function;

	void Jsonize(JsonIO& json);
};

class AppLog {
public:
	AppLog() {}

	void Add(int level, const String& channel, const String& message,
	         const char *file = nullptr, int line = 0,
	         const char *function = nullptr);

	const Vector<AppLogRecord>& GetRecords() const { return records_; }
	void Clear()                                    { records_.Clear(); }

	void SetForwardToUppLog(bool b) { forward_to_upp_log_ = b; }
	bool IsForwardToUppLog() const  { return forward_to_upp_log_; }

	Event<const AppLogRecord&> WhenRecord;

private:
	Vector<AppLogRecord> records_;
	bool forward_to_upp_log_ = true;
};

#define APPLOG_TRACE(log, channel, message) \
	(log).Add(APPLOG_LEVEL_TRACE,   (channel), (message), __FILE__, __LINE__, __func__)

#define APPLOG_DEBUG(log, channel, message) \
	(log).Add(APPLOG_LEVEL_DEBUG,   (channel), (message), __FILE__, __LINE__, __func__)

#define APPLOG_INFO(log, channel, message) \
	(log).Add(APPLOG_LEVEL_INFO,    (channel), (message), __FILE__, __LINE__, __func__)

#define APPLOG_WARN(log, channel, message) \
	(log).Add(APPLOG_LEVEL_WARNING, (channel), (message), __FILE__, __LINE__, __func__)

#define APPLOG_ERROR(log, channel, message) \
	(log).Add(APPLOG_LEVEL_ERROR,   (channel), (message), __FILE__, __LINE__, __func__)

#endif
