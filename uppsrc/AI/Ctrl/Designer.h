#ifndef _AI_TextTool_Designer_h_
#define _AI_TextTool_Designer_h_

NAMESPACE_UPP

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
