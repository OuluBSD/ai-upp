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
	tree_.WhenElementSelected = THISBACK(SelectElement);
	properties_.WhenChanged = THISBACK(ApplyPropertyChanges);
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
		selected_path_.Clear();
		current_file_.Clear();
		RefreshPanels();
		canvas_.Refresh();
	});
	bar.Separator();
	bar.Add("Open JSON...", THISBACK(OpenJsonFile));
	bar.Add("Save JSON", THISBACK(SaveJsonFile));
	bar.Add("Save JSON As...", THISBACK(SaveJsonFileAs));
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
		selected_path_.Clear();
		current_file_.Clear();
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
	tree_.SetDocument(canvas_.GetDocument(), selected_path_);
	if(selected_path_.IsEmpty())
		properties_.SetDocument(canvas_.GetDocument());
	else
		properties_.SetElement(canvas_.GetDocument(), selected_path_);
	diagnostics_.SetText(canvas_.GetDiagnostics());
}

void CardBoardEditorWindow::SelectElement(const String& path)
{
	selected_path_ = path;
	properties_.SetElement(canvas_.GetDocument(), selected_path_);
	diagnostics_.SetText(canvas_.GetDiagnostics());
}

void CardBoardEditorWindow::ApplyPropertyChanges()
{
	tree_.SetDocument(canvas_.GetDocument(), selected_path_);
	diagnostics_.SetText(canvas_.GetDiagnostics());
	canvas_.Refresh();
}

void CardBoardEditorWindow::OpenJsonFile()
{
	FileSel fs;
	fs.Type("CardBoard JSON", "*.json");
	fs.AllFilesType();
	if(!current_file_.IsEmpty())
		fs.ActiveDir(GetFileDirectory(current_file_));
	if(fs.ExecuteOpen("Open CardBoard JSON"))
		LoadJsonFileFrom(~fs);
}

void CardBoardEditorWindow::SaveJsonFile()
{
	if(current_file_.IsEmpty())
		SaveJsonFileAs();
	else
		SaveJsonFileTo(current_file_);
}

void CardBoardEditorWindow::SaveJsonFileAs()
{
	FileSel fs;
	fs.Type("CardBoard JSON", "*.json");
	fs.DefaultExt(".json");
	fs.AllFilesType();
	if(!current_file_.IsEmpty())
		fs.ActiveDir(GetFileDirectory(current_file_));
	if(fs.ExecuteSaveAs("Save CardBoard JSON"))
		SaveJsonFileTo(~fs);
}

bool CardBoardEditorWindow::SaveJsonFileTo(const String& path)
{
	if(!SaveFile(path, canvas_.GetDocument().StoreJson())) {
		String message = "Failed to save CardBoard JSON: " + path;
		LOG(message);
		Exclamation(message);
		return false;
	}
	current_file_ = path;
	LOG("Saved CardBoard JSON: " << path);
	return true;
}

void CardBoardEditorWindow::LoadJsonFileFrom(const String& path)
{
	String json = LoadFile(path);
	if(json.IsVoid()) {
		String message = "Failed to read CardBoard JSON: " + path;
		LOG(message);
		Exclamation(message);
		return;
	}
	CardBoardDocument loaded;
	String error;
	if(!loaded.LoadJson(json, error)) {
		String message = "Failed to parse CardBoard JSON: " + error;
		LOG(message);
		Exclamation(message);
		return;
	}
	canvas_.SetDocument(loaded);
	current_file_ = path;
	selected_path_.Clear();
	RefreshPanels();
	LOG("Loaded CardBoard JSON: " << path);
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

static int CountReportLines(const String& report)
{
	int count = 0;
	for(int i = 0; i < report.GetCount(); i++)
		if(report[i] == '\n')
			count++;
	return count;
}

static int CountReportMarker(const String& report, const String& marker)
{
	int count = 0;
	int pos = 0;
	for(;;) {
		pos = report.Find(marker, pos);
		if(pos < 0)
			return count;
		count++;
		pos += marker.GetCount();
	}
}

static Color PropertyStringToColorCli(const String& text)
{
	String value = TrimBoth(text);
	if(value.IsEmpty())
		return Null;
	if(value.GetCount() == 7 && value[0] == '#') {
		int r = ScanInt("0x" + value.Mid(1, 2));
		int g = ScanInt("0x" + value.Mid(3, 2));
		int b = ScanInt("0x" + value.Mid(5, 2));
		return Color(r, g, b);
	}
	return Null;
}

static bool ApplyElementProperty(CardBoardDocument& document, const String& spec, String& error)
{
	int eq = spec.Find('=');
	int dot = -1;
	for(int i = 0; i < eq; i++)
		if(spec[i] == '.')
			dot = i;
	if(eq <= 0 || dot <= 0 || dot > eq) {
		error = "expected PATH.FIELD=VALUE";
		return false;
	}
	String path = spec.Left(dot);
	String field = spec.Mid(dot + 1, eq - dot - 1);
	String value = spec.Mid(eq + 1);
	CardBoardElement *element = document.FindElementPath(path);
	if(!element) {
		error = "element path not found: " + path;
		return false;
	}
	if(field == "id")
		element->id = value;
	else
	if(field == "label")
		element->label = value;
	else
	if(field == "binding")
		element->binding = value;
	else
	if(field == "asset")
		element->style.asset = value;
	else
	if(field == "font_face")
		element->style.font_face = value;
	else
	if(field == "font_height") {
		int height = ScanInt(value);
		if(height <= 0) {
			error = "font_height must be positive";
			return false;
		}
		element->style.font_height = height;
	}
	else
	if(field == "fill")
		element->style.fill = PropertyStringToColorCli(value);
	else
	if(field == "border")
		element->style.border = PropertyStringToColorCli(value);
	else
	if(field == "text")
		element->style.text = PropertyStringToColorCli(value);
	else {
		error = "unknown field: " + field;
		return false;
	}
	return true;
}

static void RenderSmokeReport(String& out, const CardBoardDocument& document, Size size)
{
	CardBoardState state = document.MakePokerSampleState();
	CardBoardRenderer renderer;
	CardBoardRenderDiagnostics diagnostics;
	SImageDraw surface(size);
	renderer.Render(surface, RectC(0, 0, size.cx, size.cy), document, state, diagnostics);
	Image image = surface;

	String report;
	document.RenderReport(report, size, state);
	hash_t image_hash = image.GetHashValue();
	hash_t raw_hash = memhash(image.Begin(), image.GetLength() * sizeof(RGBA));

	out.Cat(Format("RenderSmoke provider=%s game=%s size=%d`x%d image=%d`x%d pixels=%d image_hash=%lld raw_hash=%lld\n",
	               document.provider_id, document.game_family, size.cx, size.cy,
	               image.GetWidth(), image.GetHeight(), (int)image.GetLength(),
	               (int64)image_hash, (int64)raw_hash));
	out.Cat(Format("RenderSmoke report_lines=%d draw_nodes=%d cards=%d seats=%d buttons=%d chips=%d\n",
	               CountReportLines(report),
	               CountReportMarker(report, "draw type="),
	               CountReportMarker(report, "primitive=playing-card"),
	               CountReportMarker(report, "type=Seat"),
	               CountReportMarker(report, "primitive=button"),
	               CountReportMarker(report, "primitive=chip-stack")));
	out.Cat(Format("RenderSmoke assets_used=%d assets_missing=%d\n",
	               diagnostics.used_assets.GetCount(), diagnostics.missing_assets.GetCount()));
	for(const String& path : diagnostics.used_assets)
		out.Cat("RenderSmoke asset used " + path + "\n");
	for(const String& path : diagnostics.missing_assets)
		out.Cat("RenderSmoke asset missing " + path + "\n");
}

int RunCardBoardEditorCli(const Vector<String>& args)
{
	bool dump_tree = false;
	bool dump_rects = false;
	bool render_report = false;
	bool roundtrip_json = false;
	bool render_smoke = false;
	String save_sample_json;
	String load_json;
	Vector<String> set_elements;
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
		if(arg == "--roundtrip-json")
			roundtrip_json = true;
		else
		if(arg == "--render-smoke")
			render_smoke = true;
		else
		if(arg.StartsWith("--save-sample-json="))
			save_sample_json = arg.Mid(19);
		else
		if(arg.StartsWith("--load-json="))
			load_json = arg.Mid(12);
		else
		if(arg.StartsWith("--set-element="))
			set_elements.Add(arg.Mid(14));
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
			       << "  --roundtrip-json\n"
			       << "  --render-smoke\n"
			       << "  --save-sample-json=PATH\n"
			       << "  --load-json=PATH\n"
			       << "  --set-element=PATH.FIELD=VALUE\n"
			       << "  --size=WIDTHxHEIGHT\n";
			return 0;
		}
	}

	if(!dump_tree && !dump_rects && !render_report && !roundtrip_json && !render_smoke &&
	   set_elements.IsEmpty() &&
	   save_sample_json.IsEmpty() && load_json.IsEmpty())
		return -1;

	CardBoardDocument document;
	document.MakePokerSample();
	if(!load_json.IsEmpty()) {
		String json = LoadFile(load_json);
		if(json.IsVoid()) {
			Cout() << "failed to read JSON file: " << load_json << "\n";
			return 2;
		}
		String load_error;
		if(!document.LoadJson(json, load_error)) {
			Cout() << "failed to load CardBoard JSON: " << load_error << "\n";
			return 2;
		}
	}
	for(const String& spec : set_elements) {
		String set_error;
		if(!ApplyElementProperty(document, spec, set_error)) {
			Cout() << "failed to set element property: " << set_error << "\n";
			return 2;
		}
	}
	String error = document.Validate();
	if(!error.IsEmpty()) {
		Cout() << "validation failed: " << error << "\n";
		return 1;
	}
	if(!save_sample_json.IsEmpty()) {
		if(!SaveFile(save_sample_json, document.StoreJson())) {
			Cout() << "failed to save JSON file: " << save_sample_json << "\n";
			return 2;
		}
		Cout() << "saved " << save_sample_json << "\n";
	}
	String out;
	if(dump_tree)
		document.DumpTree(out);
	if(dump_rects)
		document.DumpRects(out, size);
	if(render_report)
		document.RenderReport(out, size, document.MakePokerSampleState());
	if(roundtrip_json) {
		String json = document.StoreJson();
		CardBoardDocument loaded;
		String load_error;
		if(!loaded.LoadJson(json, load_error)) {
			Cout() << "roundtrip failed: " << load_error << "\n";
			return 1;
		}
		String before;
		String after;
		document.RenderReport(before, size, document.MakePokerSampleState());
		loaded.RenderReport(after, size, loaded.MakePokerSampleState());
		if(before != after) {
			Cout() << "roundtrip render report mismatch\n";
			return 1;
		}
		out.Cat("roundtrip ok\n");
	}
	if(render_smoke)
		RenderSmokeReport(out, document, size);
	Cout() << out;
	return 0;
}

END_UPP_NAMESPACE
