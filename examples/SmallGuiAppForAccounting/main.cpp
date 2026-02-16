#include <CtrlLib/CtrlLib.h>
#include <RichText/RichText.h>
#include <AI/LogicGui/LogicGui.h>

using namespace Upp;

class SmallGuiAppForAccounting : public TopWindow {
public:
	typedef SmallGuiAppForAccounting CLASSNAME;
	
	MenuBar    menu;
	ToolBar    tool;
	ArrayCtrl  ledger;
	Button     previewButton;
	Label      balanceLabel;
	Button     addButton;
	Button     removeButton;
	RichTextView reportCheck;

	virtual bool Access(Visitor& v) override {
		v.AccessLabel("mainWindow");
		v.AccessLabel("balanceLabel");
		v.AccessAction("ledgerList", [=] { ledger.WhenAction(); });
		v.AccessLabel("menu");
		v.AccessAction("previewButton", [=] { previewButton.WhenAction(); });
		v.AccessAction("addButton", [=] { addButton.WhenAction(); });
		v.AccessAction("removeButton", [=] { removeButton.WhenAction(); });
		v.AccessAction("reportCheck", [=] {});
		return TopWindow::Access(v);
	}

	void MainMenu(Bar& bar) {
		bar.Sub("File", [=](Bar& bar) {
			bar.Add("Exit", [=] { Close(); });
		});
	}
	
	void Preview() {
		TopWindow win;
		win.LayoutId("previewWindow");
		RichTextView view;
		view.LayoutId("previewReport");
		view.SetQTF(reportCheck.GetQTF());
		view.SetZoom(Zoom(1, 1));
		view.Background(White());
		win.Add(view.SizePos());
		win.SetRect(0, 0, 600, 400);
		win.Title("Report Preview");
		
		win.Run();
	}

	void MainTool(Bar& bar) {
		bar.Add(addButton);
		bar.Add(removeButton);
		bar.Add("Preview", THISBACK(Preview));
	}

	void UpdateBalance() {
		double sum = 0;
		String qtf;
		qtf << "[@6 {{2000:5000:3000 [ * Date]:: [ * Description]:: [ *> Amount]";
		for(int i = 0; i < ledger.GetCount(); i++) {
			Value v = ledger.Get(i, 2);
			if(!IsNull(v))
				sum += (double)v;
			qtf << "- [ " << (String)ledger.Get(i, 0) << "]:: [ " << (String)ledger.Get(i, 1) 
			    << "]:: [ > " << Format("%.2f", (double)ledger.Get(i, 2)) << "]";
		}
		qtf << "}}]";
		balanceLabel = "Balance: " + Format("%.2f", sum);
		reportCheck.SetQTF(qtf);
	}

	SmallGuiAppForAccounting() {
		Title("Small Accounting App");

		LayoutId("mainWindow");

		AddFrame(menu);
		AddFrame(tool);
		menu.Set(THISBACK(MainMenu));
		tool.Set(THISBACK(MainTool));
		
		menu.LayoutId("menu");
		tool.LayoutId("toolbar");
		
		addButton.SetLabel("Add");
		addButton.LayoutId("addButton");
		
		removeButton.SetLabel("Remove");
		removeButton.LayoutId("removeButton");

		ledger.AddColumn("Date");
		ledger.AddColumn("Description");
		ledger.AddColumn("Amount");
		
		// Sample data
		ledger.Add("2026-02-14", "Opening Balance", 1000.00);
		ledger.Add("2026-02-15", "Office Supplies", -50.25);
		ledger.Add("2026-02-16", "Client Payment", 250.00);
		
		ledger.LayoutId("ledgerList");

		previewButton.SetLabel("Preview");
		previewButton <<= THISBACK(Preview);
		previewButton.LayoutId("previewButton");

		Add(ledger.HSizePos().VSizePos(0, 30));
		Add(previewButton.BottomPos(0, 30).LeftPos(0, 200));
		Add(balanceLabel.BottomPos(0, 30).RightPos(0, 200));
		balanceLabel.SetAlign(ALIGN_RIGHT);
		balanceLabel.SetFont(StdFont().Bold());
		balanceLabel.LayoutId("balanceLabel");
		
		reportCheck.LayoutId("reportCheck");
		reportCheck.Hide();
		Add(reportCheck);
		
		UpdateBalance();

		PostCallback([=] {
			const Vector<String>& cmd = CommandLine();
			for(int i = 0; i < cmd.GetCount(); i++) {
				if(cmd[i] == "--test") {
					Ctrl::CheckConstraints();
					exit(0);
				}
			}
		});
	}
};

GUI_APP_MAIN
{
	LinkLogicGui();

	String root = GetCurrentDirectory();
	bool found = false;
	while(!root.IsEmpty()) {
		if(DirectoryExists(AppendFileName(root, "docs/maestro"))) {
			found = true;
			break;
		}
		String next = GetFileFolder(root);
		if(next == root) break;
		root = next;
	}
	
	if(found) {
		String constr_dir = AppendFileName(root, "docs/maestro/plans/constraints");
		RLOG("Loading constraints from: " << constr_dir);
		FindFile ff(AppendFileName(constr_dir, "*.ugui"));
		while(ff) {
			RLOG("  Found file: " << ff.GetPath());
			String c = LoadFile(ff.GetPath());
			Vector<String> lines = Split(c, '\n');
			for(const String& l : lines) {
				String s = TrimLeft(TrimRight(l));
				if(s.GetCount() && !s.StartsWith("YOLO") && !s.StartsWith("Loaded") && !s.StartsWith("Hook")) {
					RLOG("    Adding constraint: " << s);
					Ctrl::constraints.Add(s);
				}
			}
			ff.Next();
		}
	}
	else {
		RLOG("Maestro project root NOT found!");
	}

	RLOG("Starting app with " << Ctrl::constraints.GetCount() << " constraints.");
	SmallGuiAppForAccounting().Run();
}
