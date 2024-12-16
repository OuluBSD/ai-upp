#ifndef _AI_TextTool_Designer_h_
#define _AI_TextTool_Designer_h_

NAMESPACE_UPP

template <class T>
struct Des : IdeDesigner, ParentCtrl {
	Ide*			ide = 0;
	String			filename;
	T				edit;
	TimeCallback	delay;

	virtual void GotFocus();
	virtual String GetFileName() const        { return filename; }
	virtual void   Save();
	virtual void   SaveEditPos();
	virtual void   EditMenu(Bar& menu);
	virtual Ctrl&  DesignerCtrl()             { return *this; }
	
	virtual void   Serialize(Stream& s);
	
	void    Preview();

	bool   Load(const String& includes, const String& filename);
	static String GetIDStatic();
	static int GetNodeKind();
	static bool AcceptsExt(String e);
	
	Des();
};

using SourceTextDes = Des<SourceTextCtrl>;
using EnvEditorDes = Des<EnvEditorCtrl>;
using EntityEditorDes = Des<EntityEditorCtrl>;


INITIALIZE(SourceTextDes)
INITIALIZE(EnvEditorDes)
INITIALIZE(EntityEditorDes)


END_UPP_NAMESPACE

#endif
