#include <ide/ide.h>
#include "TextTool.h"

NAMESPACE_UPP

template <class T>
Des<T>::Des()
{
	edit << [=] { delay.KillSet(250, [=] { Preview(); }); };
	Add(edit.SizePos());
	if(TheIde())
		edit.SetFont(((Ide*)TheIde())->editorfont);
}

template <class T>
void Des<T>::Preview() { edit.Data(); }

template <class T>
bool Des<T>::Load(const String& includes, const String& filename_)
{
	filename = filename_;
	FileIn in(filename);
	if(in) {
		// TODO check if faster cache can be loaded based on same sha1
		edit.Load(includes, filename, in, CHARSET_UTF8);
		IdeEditPos& ep = sEPai().GetAdd(filename);
		if(ep.filetime == FileGetTime(filename)) {
			edit.SetEditPos(ep.editpos);
			edit.SetPickUndoData(pick(ep.undodata));
		}
		Preview();
		return true;
	}
	return false;
}

template <class T>
void Des<T>::SaveEditPos()
{
	
}

template <class T>
void Des<T>::Save()
{
	// TODO check sha1 if the persistent file is needed to be overwritten
	// TODO check if fast-cached file needs to be overwritten
	FileOut out(filename);
	edit.Save(out, CHARSET_UTF8);
}

template <class T>
void Des<T>::EditMenu(Bar& menu)
{
	//	EditTools(menu);
}

template <class T>
void Des<T>::GotFocus() { edit.SetFocus(); }

template <class T>
void Des<T>::Serialize(Stream& s) {}

template <class T>
String Des<T>::GetIDStatic() {return T::GetID();}

template <class T>
DbField Des<T>::GetFieldType() {return T::GetFieldType();}

template <class T>
String Des<T>::GetExt() {return T::GetExt();}


//template<> struct Des<ScriptReferenceMakerCtrl>;




bool IsExtFile(const char* path, String ext)
{
	String n = GetFileName(path);
	String e = ToLower(GetFileExt(path));
	return e == ext;
}

template <class T>
struct DesModule : public IdeModule {
	static String GetIDStatic() { return T::GetIDStatic() + "-ctrl"; }
	virtual String GetID() { return T::GetIDStatic() + "DesModule"; }

	virtual Image FileIcon(const char* path)
	{
		return IsExtFile(path, T::GetExt()) ? IdeCommonImg::AI() : Null;
	}

	virtual bool AcceptsFile(const char* path) { return IsExtFile(path, T::GetExt()); }

	IdeDesigner* CreateSolver(Ide* ide, const char* path, byte charset)
	{
		TaskMgr::Setup(ide);
		if(IsExtFile(path, T::GetExt())) {
			T* d = new T;
			LoadFromGlobal(*d, GetIDStatic());
			if(d->Load(ide->GetCurrentIncludePath(), path))
				return d;
			delete d;
			return NULL;
		}
		return NULL;
	}
	
	virtual IdeDesigner* CreateSolver(const char* path, byte) { return NULL; }

	virtual void Serialize(Stream& s)
	{
		int version = 0;
		s / version;
	}
};

using DialogueDesModule = DesModule<DialogueDes>;
using SourceTextDesModule = DesModule<SourceTextDes>;

INITIALIZER(DialogueDes)
{
	RegisterIdeModule(Single<DialogueDesModule>());
	RegisterGlobalConfig(DialogueDesModule::GetIDStatic());
}

INITIALIZER(SourceTextDes)
{
	RegisterIdeModule(Single<SourceTextDesModule>());
	RegisterGlobalConfig(SourceTextDesModule::GetIDStatic());
}






END_UPP_NAMESPACE
