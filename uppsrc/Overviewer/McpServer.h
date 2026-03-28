#ifndef _Overviewer_McpServer_h_
#define _Overviewer_McpServer_h_

#include <Core/Core.h>
#include "Overviewer.h"

using namespace Upp;

class McpServer {
public:
	McpServer(OverviewerProject& p) : project(p) {}

	void Run();

private:
	OverviewerProject& project;
	Vector<String> current_scan;

	Json ValorizeResponse(bool success, const Value& result, const String& error = "");
	void ProcessRequest(const String& line);

	// Operations
	Value OpenProject(const Value& args);
	Value SaveProject(const Value& args);
	Value GetProjectInfo(const Value& args);
	Value ListEntries(const Value& args);
	Value GetEntry(const Value& args);
	Value SetNote(const Value& args);
	Value SetNumeric(const Value& args);
	Value SetFlags(const Value& args);
	Value AddTag(const Value& args);
	Value RemoveTag(const Value& args);
	Value AddListItem(const Value& args);
	Value UpdateListItem(const Value& args);
	Value RemoveListItem(const Value& args);
	Value MoveEntry(const Value& args);
	Value RefreshScan(const Value& args);
	Value FindEntriesWithFlag(const Value& args);
	Value FindEntriesMissingNumeric(const Value& args);
	Value FindEntriesByTag(const Value& args);
	Value GetRegistryTags(const Value& args);

	// Helpers
	void DoScan();
	bool IsPathValid(const String& path);
	String GetAbsPath(const String& rel_path);
	uint32 StringToFlag(const String& name);
};

#endif
