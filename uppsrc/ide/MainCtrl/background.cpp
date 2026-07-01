#ifdef flagGUI
#include "MainCtrl.h"

void GatherAllFiles(const String& path, Index<String>& filei, VectorMap<String, String>& file)
{
	if(path.GetCount() == 0)
		return;
	Sleep(0); // This is supposed to be superlazy
	for(FindFile ff(path + "/*.*"); ff && !Thread::IsShutdownThreads(); ff.Next())
		if(ff.IsFolder() && *ff.GetName() != '.')
			GatherAllFiles(ff.GetPath(), filei, file);
		else
		if(ff.IsFile()) {
			String p = NormalizePath(ff.GetPath());
			String lp = ToLower(p);
			if(filei.Find(lp) < 0) {
				filei.Add(lp);
				file.Add(GetFileName(p), p);
			}
		}
}

CoEvent ide_bg_scheduler;

void IdeBackgroundThread()
{
	while(!Thread::IsShutdownThreads()) {
		VectorMap<String, String> file;
		Index<String> filei;

		Index<String> dir = GetAllNests(true);
		
		for(String d : dir)
			GatherAllFiles(d, filei, file);

		if(TheIde() && TheIde()->search_downloads)
			GatherAllFiles(GetDownloadFolder(), filei, file);

		SetAllSourceFiles(pick(file), dir.PickKeys());
	
		ide_bg_scheduler.Wait();
	}
}

void StartIdeBackgroundThread()
{
	Thread::AtShutdown([] {
		ide_bg_scheduler.Broadcast();
	});
	Thread::StartNice(IdeBackgroundThread);
}

void TriggerIdeBackgroundThread(int delay)
{
	static TimeCallback tm;
	tm.KillSet(delay, [=] { ide_bg_scheduler.Broadcast(); });
}

void StartIdeMcpThread()
{
	// Start MCP server on IDE startup (always on; port 7326 by default)
	static bool mcp_started = false;
	if(!mcp_started) {
		McpConfig cfg;
		StartMcpServer(cfg);
		mcp_started = true;
	}
}
#endif // flagGUI
