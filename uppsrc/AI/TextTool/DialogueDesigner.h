#ifndef _AI_TextTool_DialogueDesigner_h_
#define _AI_TextTool_DialogueDesigner_h_

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
	static DbField GetFieldType();
	static String GetExt();
	
	Des();
};

using DialogueDes = Des<ScriptReferenceMakerCtrl>;
using SourceTextDes = Des<SourceTextCtrl>;


INITIALIZE(DialogueDes)
INITIALIZE(SourceTextDes)


END_UPP_NAMESPACE

#endif
