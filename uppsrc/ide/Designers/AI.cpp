#include "Designers.h"

#ifdef flagAI
#include <AI/AI.h>

struct IdeAIEditPos : Moveable<IdeAIEditPos> {
	Time               filetime = Null;
	LineEdit::EditPos  editpos;
	LineEdit::UndoData undodata;
};

static VectorMap<String, IdeAIEditPos>& sEPai()
{
	static VectorMap<String, IdeAIEditPos> x;
	return x;
}

IdeAIDes::IdeAIDes()
{
	editor << [=] { delay.KillSet(250, [=] { Preview(); }); };
	splitter.Horz(editor, preview);
	Add(splitter.SizePos());
	if(TheIde())
		editor.SetFont(((Ide *)TheIde())->editorfont);
}

void IdeAIDes::Preview()
{
	int sc = preview.GetSb();
	preview <<= MarkdownConverter().Tables().ToQtf(editor.Get());
	preview.SetSb(sc);
}

void IdeAIDes::SaveEditPos()
{
	if(filename.GetCount()) {
		IdeAIEditPos& p = sEPai().GetAdd(filename);
		p.filetime = FileGetTime(filename);;
		p.undodata = editor.PickUndoData();
		p.editpos = editor.GetEditPos();
	}
}

bool IdeAIDes::Load(const char *filename_)
{
	filename = filename_;
	FileIn in(filename);
	if(in) {
		editor.Load(in, CHARSET_UTF8);
		IdeAIEditPos& ep = sEPai().GetAdd(filename);
		if(ep.filetime == FileGetTime(filename)) {
			editor.SetEditPos(ep.editpos);
			editor.SetPickUndoData(pick(ep.undodata));
		}
		Preview();
		return true;
	}
	return false;
}

void IdeAIDes::Save()
{
	FileOut out(filename);
	editor.Save(out, CHARSET_UTF8);
}

void IdeAIDes::EditMenu(Bar& menu)
{
//	EditTools(menu);
}

void IdeAIDes::GotFocus()
{
	editor.SetFocus();
}

void IdeAIDes::Serialize(Stream& s)
{
}

void SerializeAIDesPos(Stream& s)
{
	VectorMap<String, IdeAIEditPos>& filedata = sEPai();
	s.Magic();
	s.Magic(0);
	if(s.IsStoring()) {
		for(int i = 0; i < filedata.GetCount(); i++) {
			String fn = filedata.GetKey(i);
			if(!fn.IsEmpty()) {
				FindFile ff(fn);
				IdeAIEditPos& ep = filedata[i];
				if(ff && ff.GetLastWriteTime() == ep.filetime) {
					s % fn;
					s % ep.filetime;
					s % ep.editpos;
				}
			}
		}
		String end;
		s % end;
	}
	else {
		String fn;
		filedata.Clear();
		for(;;) {
			s % fn;
			if(fn.IsEmpty()) break;
			IdeAIEditPos& ep = filedata.GetAdd(fn);
			s % ep.filetime;
			s % ep.editpos;
		}
	}
}

bool IsAIFile(const char *path)
{
	return ToLower(GetFileExt(path)) == ".aion";
}

struct AIDesModule : public IdeModule {
	virtual String       GetID() { return "AIDesModule"; }

	virtual Image FileIcon(const char *path) {
		return IsAIFile(path) ? IdeCommonImg::AI() : Null;
	}

	virtual IdeDesigner *CreateDesigner(const char *path, byte) {
		if(IsAIFile(path)) {
			IdeAIDes *d = new IdeAIDes;
			LoadFromGlobal(*d, "aides-ctrl");
			if(d->Load(path)) {
				return d;
			}
			delete d;
			return NULL;
		}
		return NULL;
	}

	virtual void Serialize(Stream& s) {
		int version = 0;
		s / version;
		SerializeAIDesPos(s);
	}
};

INITIALIZER(IdeAIDes)
{
	RegisterIdeModule(Single<AIDesModule>());
	RegisterGlobalConfig("aides-ctrl");
}

#endif
