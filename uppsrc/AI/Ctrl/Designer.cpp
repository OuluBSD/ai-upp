#include "Ctrl.h"
#include <ide/ide.h>

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
void Des<T>::Preview() {
	try {
		edit.Data();
	}
	catch (NoPointerExc e) {
		LOG("Des< " << T::GetID() << ">: error: " << e);
	}
}

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
int Des<T>::GetNodeKind() {return T::GetNodeKind();}

template <class T>
bool Des<T>::AcceptsExt(String e) {return T::AcceptsExt(e);}


//template<> struct Des<ScriptReferenceMakerCtrl>;




template <class T>
struct DesModule : public IdeModule {
	static String GetIDStatic() { return T::GetIDStatic() + "-ctrl"; }
	virtual String GetID() { return T::GetIDStatic() + "DesModule"; }

	virtual Image FileIcon(const char* path)
	{
		return T::AcceptsExt(GetFileExt(path)) ? IdeCommonImg::AI() : Null;
	}

	virtual bool AcceptsFile(const char* path) { return T::AcceptsExt(GetFileExt(path)); }

	IdeDesigner* CreateSolver(Ide* ide, const char* path, byte charset)
	{
		TaskMgr::Setup(ide);
		if(T::AcceptsExt(GetFileExt(path))) {
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

#define INITIALIZE_MODULE(x) \
	using x##Module = DesModule<x>; \
	INITIALIZER(x) \
	{ \
		RegisterIdeModule(Single<x##Module>()); \
		RegisterGlobalConfig(x##Module::GetIDStatic()); \
	}

INITIALIZE_MODULE(SourceTextDes)
INITIALIZE_MODULE(EnvEditorDes)
INITIALIZE_MODULE(EntityEditorDes)

END_UPP_NAMESPACE
