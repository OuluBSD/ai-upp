#include "Sequencer.h"

#define IMAGECLASS SequencerImg
#define IMAGEFILE <Sequencer/Images.iml>
#include <Draw/iml_source.h>

NAMESPACE_UPP

FileSel& SequencerFs()
{
	static FileSel fs;
	return fs;
}

Sequencer::Sequencer()
{
	TopWindow::Title("Sequencer");
	TopWindow::Icon(SequencerImg::icon());
	Sizeable().MaximizeBox().MinimizeBox();
	
	Ctrl::AddFrame(menubar);
	Ctrl::AddFrame(TopSeparatorFrame());
	Ctrl::AddFrame(toolbar);
	Ctrl::AddFrame(statusbar);
	Ctrl::Add(editor.SizePos());
	
	menubar.Set(THISFN(MainBar));
	WhenClose = THISBACK(Destroy);
	menubar.WhenHelp = toolbar.WhenHelp = statusbar;
	
	editor.ClearModify();
	SetBar();
	editor.WhenRefreshBar = THISBACK(SetBar);
	
	OpenMain();
	ActiveFocus(editor);
}

void Sequencer::SerializeApp(Stream& s) {
	int version = 1;
	s / version;
	s % SequencerFs();
	if (version >= 1)
		s % lrufile();
}

void Sequencer::DragAndDrop(Point, PasteClip& d) {
	if(IsAvailableFiles(d)) {
		Vector<String> fn = GetFiles(d);
		for(int open = 0; open < 2; open++) {
			for(int i = 0; i < fn.GetCount(); i++) {
				String ext = GetFileExt(fn[i]);
				if(FileExists(fn[i]) && (ext == ".rtf" || ext == ".qtf")) {
					if(open)
						OpenFile(fn[i]);
					else {
						if(d.Accept())
							break;
						return;
					}
				}
			}
			if(!d.IsAccepted())
				return;
		}
	}
}

void Sequencer::FrameDragAndDrop(Point p, PasteClip& d) {
	DragAndDrop(p, d);
}

void Sequencer::SetBar() {
	toolbar.Set(THISBACK(MainBar));
}

void Sequencer::MainBar(Bar& bar) {
	bar.Sub("File", [=](Bar& bar) {
		bar.Add("New Session", CtrlImg::new_doc(), THISBACK(New))
			.Key(K_CTRL_N)
			.Help("Open new session");
		bar.Add("Open..", CtrlImg::open(), THISBACK(Open))
		   .Key(K_CTRL_O)
		   .Help("Open existing session");
		bar.Add("Save", CtrlImg::save(), THISBACK(Save))
			.Key(K_CTRL_S)
			.Help("Save current session");
		bar.Add("Save As", CtrlImg::save_as(), THISBACK(SaveAs))
			.Help("Save current session with a new name");
		
		if(bar.IsMenuBar()) {
			if(lrufile().GetCount())
				lrufile()(bar, THISBACK(OpenFile));
			bar.Separator();
			bar.Add("Exit", THISBACK(Destroy))
				.Key(K_CTRL_Q)
				.Help("Exit application");
		}
	});
}

void Sequencer::New() {}
void Sequencer::Open() {}
void Sequencer::Save() {}
void Sequencer::Save0() {}
void Sequencer::SaveAs() {}
void Sequencer::SaveAll() {}
void Sequencer::Load(const String& filename) {}
void Sequencer::OpenFile(const String& fn) {}
void Sequencer::Destroy() { TopWindow::Close(); }
void Sequencer::Index() {}
void Sequencer::About() {}

END_UPP_NAMESPACE
