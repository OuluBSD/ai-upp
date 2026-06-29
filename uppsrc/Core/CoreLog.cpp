#include "Core/Core.h"

namespace Upp {

static const char *sLevelName(int level)
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

void CoreLog::Add(int level, const String& component, const String& message)
{
	String line;
	line << '[' << component << "][" << sLevelName(level) << "] " << message;
	log_.Add(line);
	if(log_sink_)
		log_sink_->Add(level, component, message);
}

} // namespace Upp
