#include "CardBoardEditor.h"

NAMESPACE_UPP

CardBoardEditorWindow::CardBoardEditorWindow()
{
	Title("CardBoardEditor").Sizeable().Zoomable();
	SetRect(0, 0, 1100, 780);

	AddFrame(menu_);
	menu_.Set(THISBACK(MainMenu));

	AddFrame(toolbar_);
	toolbar_.Set(THISBACK(ToolBar));

	canvas_.GetDocument().MakePokerSample();
	Add(canvas_.SizePos());
	RefreshPanels();
}

void CardBoardEditorWindow::DockInit()
{
	Register(tree_);
	Register(properties_);
	Register(diagnostics_);
	ResetLayout();
	CacheDefaultLayout();
	loaded_ = true;
}

void CardBoardEditorWindow::Close()
{
	TopWindow::Close();
}

void CardBoardEditorWindow::MainMenu(Bar& bar)
{
	bar.Sub("File", THISBACK(MenuFile));
	bar.Sub("View", THISBACK(MenuView));
	bar.Sub("Help", THISBACK(MenuHelp));
}

void CardBoardEditorWindow::MenuFile(Bar& bar)
{
	bar.Add("New poker sample", [=] {
		canvas_.GetDocument().MakePokerSample();
		RefreshPanels();
		canvas_.Refresh();
	});
	bar.Separator();
	bar.Add("Exit", [=] { Close(); });
}

void CardBoardEditorWindow::MenuView(Bar& bar)
{
	bar.Add("Refresh diagnostics", [=] { RefreshPanels(); });
	bar.Add("Reset layout", THISBACK(ResetLayout));
	bar.Separator();
	DockWindowMenu(bar);
}

void CardBoardEditorWindow::MenuHelp(Bar& bar)
{
	bar.Add("Dump document to log", [=] {
		String out;
		canvas_.GetDocument().DumpTree(out);
		LOG(out);
	});
}

void CardBoardEditorWindow::ToolBar(Bar& bar)
{
	bar.Add("Sample", [=] {
		canvas_.GetDocument().MakePokerSample();
		RefreshPanels();
		canvas_.Refresh();
	});
	bar.Add("Dump", [=] { RefreshPanels(); });
	bar.Add("Reset", THISBACK(ResetLayout));
}

void CardBoardEditorWindow::ResetLayout()
{
	for(DockableCtrl* ctrl : GetDockableCtrls())
		DockWindow::Close(*ctrl);
	DockLeft(tree_);
	DockRight(properties_);
	DockBottom(diagnostics_);
}

void CardBoardEditorWindow::RefreshPanels()
{
	tree_.SetDocument(canvas_.GetDocument());
	properties_.SetDocument(canvas_.GetDocument());
	diagnostics_.SetText(canvas_.GetDiagnostics());
}

void CardBoardEditorWindow::CacheDefaultLayout()
{
	StringStream out;
	SerializeWindow(out);
	default_layout_data_ = out.GetResult();
}

bool RunCardBoardEditorCli(const Vector<String>& args)
{
	bool dump_tree = false;
	bool dump_rects = false;
	Size size(610, 438);

	for(int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if(arg == "--dump-sample-tree")
			dump_tree = true;
		else
		if(arg == "--dump-sample-rects")
			dump_rects = true;
		else
		if(arg.StartsWith("--size=")) {
			Vector<String> parts = Split(arg.Mid(7), 'x');
			if(parts.GetCount() != 2) {
				Cout() << "invalid --size, expected WIDTHxHEIGHT\n";
				SetExitCode(2);
				return true;
			}
			size.cx = ScanInt(parts[0]);
			size.cy = ScanInt(parts[1]);
		}
		else
		if(arg == "--help") {
			Cout() << "CardBoardEditor options:\n"
			       << "  --dump-sample-tree\n"
			       << "  --dump-sample-rects\n"
			       << "  --size=WIDTHxHEIGHT\n";
			return true;
		}
	}

	if(!dump_tree && !dump_rects)
		return false;

	CardBoardDocument document;
	document.MakePokerSample();
	String error = document.Validate();
	if(!error.IsEmpty()) {
		Cout() << "validation failed: " << error << "\n";
		SetExitCode(1);
		return true;
	}
	String out;
	if(dump_tree)
		document.DumpTree(out);
	if(dump_rects)
		document.DumpRects(out, size);
	Cout() << out;
	return true;
}

END_UPP_NAMESPACE
