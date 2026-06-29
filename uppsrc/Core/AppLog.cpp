#include "Core/Core.h"

namespace Upp {

void AppLogRecord::Jsonize(JsonIO& json)
{
	json("time",     time)
	    ("level",    level)
	    ("channel",  channel)
	    ("message",  message)
	    ("file",     file)
	    ("line",     line)
	    ("function", function);
}

static const char* sLevelStr(int level)
{
	switch(level) {
	case APPLOG_LEVEL_TRACE:   return "TRACE";
	case APPLOG_LEVEL_DEBUG:   return "DEBUG";
	case APPLOG_LEVEL_INFO:    return "INFO";
	case APPLOG_LEVEL_WARNING: return "WARN";
	case APPLOG_LEVEL_ERROR:   return "ERROR";
	default:                   return "?";
	}
}

void AppLog::Add(int level, const String& channel, const String& message,
                 const char *file, int line, const char *function)
{
	AppLogRecord r;
	r.time     = GetSysTime();
	r.level    = level;
	r.channel  = channel;
	r.message  = message;
	r.file     = file ? GetFileName(file) : String();
	r.line     = line;
	r.function = function ? String(function) : String();
	records_.Add(r);
	if(forward_to_upp_log_) {
		if(file && line > 0)
			LOG(Format("[%s][%s] %s (%s:%d)", channel, sLevelStr(level), message,
			           GetFileName(file), line));
		else
			LOG(Format("[%s][%s] %s", channel, sLevelStr(level), message));
	}
	WhenRecord(r);
}

}
