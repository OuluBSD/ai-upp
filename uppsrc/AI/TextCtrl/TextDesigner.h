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
	String data, data_filepath;
	MenuBar menu;
	
protected:
	String data_sha1;
	
public:
	SolverBaseIndicator prog;
	Label remaining;

public:
	virtual ~ToolAppCtrl() {}
	virtual void Data() = 0;
	virtual void ToolMenu(Bar& bar) { bar.Add("", TextImgs::placeholder16(), Callback()); }
	virtual String GetStatusText() { return String(); }
	
	void AddMenu();
	TextDatabase& GetDatabase();
	Entity& GetEntity();
	Component& GetComponent();
	EditorPtrs& GetPointers() const; // TODO rename EditorPtrs
	DatasetPtrs& GetDataset() const;
	Script& GetScript();
	const Index<String>& GetTypeclasses() const;
	const Vector<ContentType>& GetContents() const;
	const Vector<String>& GetContentParts() const;
	
	bool IsScript() const;
	bool HasPointers() const;
	String GetComponentTitle() const;

	void MakeComponentParts(ArrayCtrl& parts);
	void GetAttrs(const VectorMap<String, String>& data, VectorMap<String, String>& v);

	virtual void Load(const String& includes, const String& filename, Stream& in, byte charset);
	virtual void Save(Stream& s, byte charset);
	virtual void SetEditPos(LineEdit::EditPos editpos) {}
	virtual void SetPickUndoData(LineEdit::UndoData undodata) {}
	virtual void OnLoad(const String& data, const String& filepath) {}
	virtual void OnSave(String& data, const String& filepath) {}
	
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
