#include "ScriptIDE.h"

NAMESPACE_UPP

class StandaloneGameWindow;

static Vector<StandaloneGameWindow*>& StandaloneWindows()
{
	static Vector<StandaloneGameWindow*> windows;
	return windows;
}

static String ResolveStandaloneGameTitle(const String& path)
{
	String title;
	Value gs = ParseJSON(LoadFile(path));
	if(!gs.IsVoid()) {
		title = AsString(gs["description"]);
		if(title.IsEmpty())
			title = AsString(gs["title"]);
		if(title.IsEmpty())
			title = AsString(gs["name"]);
		Value metadata = gs["metadata"];
		if(!metadata.IsVoid()) {
			if(title.IsEmpty())
				title = AsString(metadata["title"]);
			if(title.IsEmpty())
				title = AsString(metadata["name"]);
		}
	}

	if(title.IsEmpty()) {
		String file_title = GetFileTitle(path);
		String dir = GetFileDirectory(path);
		if(dir.EndsWith(String(DIR_SEP, 1)))
			dir.TrimLast();
		String parent = GetFileName(dir);
		String lower_file_title = ToLower(file_title);
		if(!parent.IsEmpty() && (lower_file_title == "game" || lower_file_title == "visual_ci"))
			title = parent;
		else
			title = file_title;
	}

	title = TrimBoth(title);
	return title.IsEmpty() ? String("Card Game") : title;
}

class StandaloneGameWindow : public TopWindow {
public:
	typedef StandaloneGameWindow CLASSNAME;

	StandaloneGameWindow(const String& path, RunMode mode)
		: path(path), mode(mode)
	{
		host.Create();
		Add(host->GetCtrl().SizePos());
		host->Load(path);

		Size canvas = host->GetLayout().GetSize();
		if(canvas.cx <= 0 || canvas.cy <= 0)
			canvas = Size(800, 600);

		SetRect(0, 0, canvas.cx, canvas.cy);
		SetMinSize(Size(max(400, canvas.cx / 2), max(300, canvas.cy / 2)));
		Title(ResolveStandaloneGameTitle(path) + " - Card Game");
		Icon(Icons::Run());
		Sizeable().Zoomable().CenterScreen();
	}

	void SetSelfOwned(bool b)
	{
		self_owned = b;
	}

	void OpenWindow()
	{
		RegisterWindow();
		Open();
		Start();
	}

	void RunWindow()
	{
		RegisterWindow();
		Start();
		TopWindow::Run();
	}

	virtual ~StandaloneGameWindow()
	{
		UnregisterWindow();
	}

	virtual void Close() override
	{
		if(closing)
			return;
		closing = true;
		UnregisterWindow();
		if(host)
			host->Stop();
		TopWindow::Close();
		if(self_owned)
			PostCallback([this] { delete this; });
	}

	bool IsHostRunning() const
	{
		return host && host->IsRunning();
	}

private:
	One<CardGameDocumentHost> host;
	String path;
	RunMode mode = RunMode::Run;
	bool closing = false;
	bool self_owned = false;

	void Start()
	{
		if(!host)
			return;
		if(mode == RunMode::Debug)
			host->Debug();
		else if(mode == RunMode::Profile)
			host->Profile();
		else
			host->Run();
	}

	void RegisterWindow()
	{
		bool known = false;
		for(int i = 0; i < StandaloneWindows().GetCount(); i++) {
			if(StandaloneWindows()[i] == this) {
				known = true;
				break;
			}
		}
		if(!known)
			StandaloneWindows().Add(this);
	}

	void UnregisterWindow()
	{
		auto& windows = StandaloneWindows();
		for(int i = windows.GetCount() - 1; i >= 0; --i) {
			if(windows[i] == this)
				windows.Remove(i);
		}
	}
};

void OpenStandaloneGameWindow(const String& path, RunMode mode)
{
	if(path.IsEmpty())
		return;
	StandaloneGameWindow* win = new StandaloneGameWindow(path, mode);
	win->SetSelfOwned(true);
	win->OpenWindow();
}

void RunStandaloneGameWindow(const String& path, RunMode mode)
{
	if(path.IsEmpty())
		return;
	StandaloneGameWindow win(path, mode);
	win.RunWindow();
}

int GetOpenStandaloneGameWindowCount()
{
	return StandaloneWindows().GetCount();
}

int GetRunningStandaloneGameWindowCount()
{
	int running = 0;
	const auto& windows = StandaloneWindows();
	for(int i = 0; i < windows.GetCount(); i++) {
		StandaloneGameWindow* w = windows[i];
		if(w && w->IsHostRunning())
			running++;
	}
	return running;
}

void CloseAllStandaloneGameWindows()
{
	Vector<StandaloneGameWindow*> windows;
	const auto& all = StandaloneWindows();
	for(int i = 0; i < all.GetCount(); i++) {
		if(all[i])
			windows.Add(all[i]);
	}
	for(int i = 0; i < windows.GetCount(); i++) {
		StandaloneGameWindow* w = windows[i];
		if(w)
			w->Close();
	}
}

END_UPP_NAMESPACE
