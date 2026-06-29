#include "MainWindow.h"

DebugLog::DebugLog()
{
	Add(text_.SizePos());
	text_.SetReadOnly();

	Add(clear_btn_.BottomPos(0, 20).HCenterPos(80));
	clear_btn_.SetLabel("Clear");
	clear_btn_ <<= THISBACK(Clear);
}

void DebugLog::Log(const String& msg)
{
	String t = text_.Get();
	Time now = GetSysTime();
	t << Format("[%02d:%02d:%02d] ", now.hour, now.minute, now.second)
	  << msg << "\n";
	text_.Set(t);
	text_.SetCursor(text_.GetLength());
}

void DebugLog::Clear()
{
	text_.Set(String());
}
