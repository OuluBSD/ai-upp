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
	bar.Add("Dump render report to stdout/log", THISBACK(PrintDiagnosticsToStdout));
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

void CardBoardEditorWindow::PrintDiagnosticsToStdout()
{
	String out = canvas_.GetRenderReport();
	LOG(out);
	Cout() << out;
}

int RunCardBoardEditorCli(const Vector<String>& args)
{
	bool dump_tree = false;
	bool dump_rects = false;
	bool render_report = false;
	Size size(610, 438);

	for(int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if(arg == "--dump-sample-tree")
			dump_tree = true;
		else
		if(arg == "--dump-sample-rects")
			dump_rects = true;
		else
		if(arg == "--render-report")
			render_report = true;
		else
		if(arg.StartsWith("--size=")) {
			Vector<String> parts = Split(arg.Mid(7), 'x');
			if(parts.GetCount() != 2) {
				Cout() << "invalid --size, expected WIDTHxHEIGHT\n";
				return 2;
			}
			size.cx = ScanInt(parts[0]);
			size.cy = ScanInt(parts[1]);
			if(size.cx <= 0 || size.cy <= 0) {
				Cout() << "invalid --size, dimensions must be positive\n";
				return 2;
			}
		}
		else
		if(arg == "--help") {
			Cout() << "CardBoardEditor options:\n"
			       << "  --dump-sample-tree\n"
			       << "  --dump-sample-rects\n"
			       << "  --render-report\n"
			       << "  --size=WIDTHxHEIGHT\n";
			return 0;
		}
	}

	if(!dump_tree && !dump_rects && !render_report)
		return -1;

	CardBoardDocument document;
	document.MakePokerSample();
	String error = document.Validate();
	if(!error.IsEmpty()) {
		Cout() << "validation failed: " << error << "\n";
		return 1;
	}
	String out;
	if(dump_tree)
		document.DumpTree(out);
	if(dump_rects)
		document.DumpRects(out, size);
	if(render_report)
		document.RenderReport(out, size, document.MakePokerSampleState());
	Cout() << out;
	return 0;
}

END_UPP_NAMESPACE
