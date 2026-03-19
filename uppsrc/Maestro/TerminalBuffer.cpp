#include "TerminalBuffer.h"

namespace Upp {

TerminalBuffer::TerminalBuffer()
{
	terminal.History(true);
	terminal.SetHistorySize(10000); // Allow large history
	size = Size(80, 24);
	terminal.GetDefaultPage().SetSize(size);
	terminal.GetAlternatePage().SetSize(size);
}

void TerminalBuffer::SetSize(int cols, int rows)
{
	Mutex::Lock __(mutex);
	size = Size(cols, rows);
	terminal.GetDefaultPage().SetSize(size);
	terminal.GetAlternatePage().SetSize(size);
}

void TerminalBuffer::Write(const String& data)
{
	Mutex::Lock __(mutex);
	terminal.Write(data);
}

int TerminalBuffer::GetLineCount() const
{
	Mutex::Lock __(mutex);
	// VTPage::GetLineCount() returns lines + history
	return terminal.GetDefaultPage().GetLineCount();
}

String TerminalBuffer::GetLine(int line) const
{
	Mutex::Lock __(mutex);
	const VTPage& page = terminal.GetDefaultPage();
	if(line >= 0 && line < page.GetLineCount()) {
		return page[line].ToString();
	}
	return String();
}

const VTLine& TerminalBuffer::GetStructuredLine(int line) const
{
	Mutex::Lock __(mutex);
	const VTPage& page = terminal.GetDefaultPage();
	if(line >= 0 && line < page.GetLineCount()) {
		return page[line];
	}
	return VTLine::Void();
}

String TerminalBuffer::GetVisibleScreen() const
{
	Mutex::Lock __(mutex);
	const VTPage& page = terminal.GetDefaultPage();
	Size sz = page.GetSize();
	String out;
	for(int i = 0; i < sz.cy; i++) {
		out.Cat(page[i].ToString());
		out.Cat("\n");
	}
	return out;
}

const VTCell& TerminalBuffer::GetCell(int col, int row) const
{
	Mutex::Lock __(mutex);
	return terminal.GetDefaultPage().FetchCell(Point(col, row));
}

void TerminalBuffer::Clear()
{
	Mutex::Lock __(mutex);
	terminal.ClearHistory();
	terminal.GetDefaultPage().ErasePage();
	terminal.GetAlternatePage().ErasePage();
}

void TerminalBuffer::RunTest()
{
	// Test very large dimensions
	SetSize(2000, 1000);
	
	// Create a very long string (more than standard 80 or 132 chars)
	String longLine;
	for(int i = 0; i < 1500; i++) {
		longLine.Cat('A' + (i % 26));
	}
	
	Write(longLine + "\r\n");
	
	String readBack = GetLine(GetLineCount() - 2); // -1 is empty bottom, -2 is the one we wrote
	RLOG("Read back length: " << readBack.GetLength());
	if(readBack.GetLength() >= 1500) {
		RLOG("SUCCESS: No truncation observed for long line.");
	} else {
		RLOG("FAILURE: Line was truncated to " << readBack.GetLength());
	}
	
	// Test many lines
	for(int i = 0; i < 500; i++) {
		Write(Format("Line %d\r\n", i));
	}
	
	RLOG("Total line count: " << GetLineCount());
	if(GetLineCount() >= 500) {
		RLOG("SUCCESS: Large number of lines handled.");
	} else {
		RLOG("FAILURE: Line count is " << GetLineCount());
	}
}

}
