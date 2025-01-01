#ifndef _AI_TextCtrl_TextDesigner_h_
#define _AI_TextCtrl_TextDesigner_h_

NAMESPACE_UPP

class SolverBaseIndicator : public ProgressIndicator {

	void SetProgress(int a, int t);

public:
	typedef SolverBaseIndicator CLASSNAME;
	SolverBaseIndicator();

	void Attach(SolverBase& sb);
	void AttachRemaining(Label& lbl);
};

class ToolAppCtrl : public Ctrl {
	String data, data_includes, data_filepath, data_dirpath;
	MetaNode* file_node = 0;
	
protected:
	String data_sha1;
	
	void SetFileNode(MetaNode* n) {file_node = n;}
public:
	SolverBaseIndicator prog;
	Label remaining;

public:
	typedef ToolAppCtrl CLASSNAME;
	virtual ~ToolAppCtrl();
	virtual void Data() = 0;
	virtual void ToolMenu(Bar& bar) { bar.Add("", TextImgs::placeholder16(), Callback()); }
	virtual String GetStatusText() { return String(); }
	
	void AddMenu();
	Entity& GetEntity();
	Component& GetComponent();
	//EditorPtrs& GetPointers() const; // TODO rename EditorPtrs
	DatasetPtrs GetDataset() const;
	Script& GetScript();
	//const Index<String>& GetTypeclasses() const;
	//const Vector<ContentType>& GetContents() const;
	//const Vector<String>& GetContentParts() const;
	
	bool IsScript() const;
	bool HasPointers() const;
	String GetComponentTitle() const;
	String GetFilePath() const {return data_filepath;}
	String GetFileIncludes() const {return data_includes;}
	String GetFileData() const {return data;}
	MetaNode* GetFileNode() const {return file_node;}
	
	void MakeComponentParts(ArrayCtrl& parts);
	void GetAttrs(const VectorMap<String, String>& data, VectorMap<String, String>& v);
	void UpdateMenu();
	
	virtual bool Load(const String& includes, const String& filename, Stream& in, byte charset);
	virtual bool LoadDirectory(const String& includes, const String& filename, byte charset);
	virtual void Save(Stream& s, byte charset);
	virtual void SaveDirectory(byte charset);
	virtual void SetEditPos(LineEdit::EditPos editpos) {}
	virtual void SetPickUndoData(LineEdit::UndoData undodata) {}
	virtual void Visit(NodeVisitor& vis) = 0;
	/*
	virtual void OnLoad(const String& data, const String& filepath) {}
	virtual void OnLoadDirectory(VersionControlSystem& vcs) {}
	virtual void OnSave(String& data, const String& filepath) {}
	virtual void OnSaveDirectory(VersionControlSystem& vcs) {}
	*/
	
	template <class T>
	void DoT(int fn)
	{
		DatasetPtrs p;
		TODO
		T& sdi = T::Get(p);
		prog.Attach(sdi);
		sdi.WhenRemaining <<
			[this](String s) { PostCallback([this, s]() { remaining.SetLabel(s); }); };
		if(fn == 0)
			sdi.Start();
		else
			sdi.Stop();
	}
};

END_UPP_NAMESPACE

#endif
