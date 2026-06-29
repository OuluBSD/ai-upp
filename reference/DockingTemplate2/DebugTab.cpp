#include "MainWindow.h"

static const char* sLevelName(int level)
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

Color DebugLog::LevelColor(int level)
{
	switch(level) {
	case APPLOG_LEVEL_TRACE:   return SColorDisabled();
	case APPLOG_LEVEL_DEBUG:   return SColorShadow();
	case APPLOG_LEVEL_INFO:    return SColorText();
	case APPLOG_LEVEL_WARNING: return Color(160, 100, 0);
	case APPLOG_LEVEL_ERROR:   return Color(200, 0, 0);
	default:                   return SColorText();
	}
}

DebugLog::DebugLog()
{
	list_.AddColumn("Time",    70);
	list_.AddColumn("Level",   55);
	list_.AddColumn("Channel", 90);
	list_.AddColumn("Message");
	list_.AddColumn("Location", 160);
	list_.SetLineCy(18);

	Add(list_.HSizePos().VSizePos(0, 24));
	Add(clear_btn_.BottomPos(0, 20).HCenterPos(80));
	clear_btn_.SetLabel("Clear");
	clear_btn_ <<= THISBACK(Clear);
}

void DebugLog::AddRecord(const AppLogRecord& r)
{
	Color ink  = LevelColor(r.level);
	String ts  = Format("%02d:%02d:%02d", r.time.hour, r.time.minute, r.time.second);
	String loc = r.file.IsEmpty() ? String() : r.file + ":" + IntStr(r.line);

	list_.Add(
		AttrText(ts).Ink(ink),
		AttrText(sLevelName(r.level)).Ink(ink).Bold(),
		AttrText(r.channel).Ink(ink),
		AttrText(r.message).Ink(ink),
		AttrText(loc).Ink(ink)
	);

	list_.SetCursor(list_.GetCount() - 1);
}

void DebugLog::Clear()
{
	list_.Clear();
}
