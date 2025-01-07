#ifndef _AI_TextTool_Designer_h_
#define _AI_TextTool_Designer_h_

NAMESPACE_UPP

template <class T>
struct EditPosCached {
	static VectorMap<String,Value>& EditPosCache()
	{
		static VectorMap<String,Value> cache;
		return cache;
	}
	
	static void SaveEditPosCache()
	{
		static StaticMutex mtx;
		String dir = ConfigFile("cfg");
		String clsname = typeid(T).name();
		String filename = clsname + ".edit_cache";
		String path0 = AppendFileName(dir, filename);
		String path1 = path0 + ".1";
		
		mtx.Enter();
		if (FileExists(path1))
			DeleteFile(path1);
		if (FileExists(path0))
			MoveFile(path0, path1);
		FileOut fout(path0);
		fout % EditPosCache();
		mtx.Leave();
	}
	
	static void LoadEditPosCache()
	{
		String dir = ConfigFile("cfg");
		String clsname = typeid(T).name();
		String filename = clsname + ".edit_cache";
		String path0 = AppendFileName(dir, filename);
		String path1 = path0 + ".1";
		bool p0 = FileExists(path0);
		bool p1 = FileExists(path1);
		
		// Check for invalid file
		String load_path;
		if (p0 && p1) {
			FileIn fin(path0);
			if (fin.GetSize() == 0)
				load_path = path1;
			else
				load_path = path0;
		}
		else if (p0)
			load_path = path0;
		else if (p1)
			load_path = path1;
		
		if (!load_path.IsEmpty()) {
			FileIn fin(load_path);
			fin % EditPosCache();
		}
	}
};

template <class T>
struct Des : IdeDesigner, ParentCtrl, EditPosCached<T> {
	struct FileHeaderParser {
		String dirpath;
		bool use_dir = false;
		bool Parse(Des<T>* des, const String& first_line);
		bool move_here = false;
		bool move_dir = false;
		bool is_parsed = false;
	};
	Ide*			ide = 0;
	String			filename;
	T				edit;
	TimeCallback	delay;

	virtual void GotFocus();
	virtual String GetFileName() const        { return filename; }
	virtual void   Save();
	virtual void   SaveEditPos();
	virtual void   RestoreEditPos();
	virtual void   EditMenu(Bar& menu);
	virtual Ctrl&  DesignerCtrl()             { return *this; }
	
	virtual void   Serialize(Stream& s);
	
	void    Preview();

	bool   Load(const String& includes, const String& filename);
	static String GetIDStatic();
	static int GetNodeKind();
	static bool AcceptsExt(String e);
	
	Des();
	~Des();
	
};

using SourceTextDes = Des<SourceTextCtrl>;
using EnvEditorDes = Des<EnvEditorCtrl>;
using EntityEditorDes = Des<EntityEditorCtrl>;


INITIALIZE(SourceTextDes)
INITIALIZE(EnvEditorDes)
INITIALIZE(EntityEditorDes)


END_UPP_NAMESPACE

#endif
