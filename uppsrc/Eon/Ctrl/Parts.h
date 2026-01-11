#ifndef _Eon_Ctrl_Parts_h_
#define _Eon_Ctrl_Parts_h_


class VfsTreeCtrl : public ParentCtrl {
	Ptr<Engine>			mach;
	VfsPath				selected;
	TreeCtrl			tree;
	uint64				last_hash = 0;

	void OnCursor();
	hash_t GetPathTreeHash() const;
	
	void AddPath(int parent, VfsValue& parent_vfs);
	
public:
	typedef VfsTreeCtrl CLASSNAME;
	VfsTreeCtrl();
	
	void SetEngine(Engine& m);
	void Updated() override;
	void Data();
	
	VfsPath GetSelected() {return selected;}
	
	Callback WhenVfsChanged;
	
};


class VfsListCtrl : public ParentCtrl {
	Ptr<Engine>			mach;
	VfsPath				scope;
	VfsValuePtr			selected;
	ArrayCtrl			list;
	uint64				last_hash = 0;

	void OnCursor();
	hash_t GetVfsListHash() const;
	VfsValue* GetCurrent() const;
public:
	typedef VfsListCtrl CLASSNAME;
	VfsListCtrl();
	
	void SetScope(VfsPath path);
	void Updated() override;
	void Data();
	
	VfsValuePtr GetSelected() {return selected;}
	
	Callback WhenVfsChanged;
	
};


class VfsBrowserCtrl : public ParentCtrl {
	Splitter			vsplit;
	VfsTreeCtrl			vfs_tree;
	VfsListCtrl			vfs_list;
	VfsPath				scope;
	
	void OnVfsCursorChanged();
	
public:
	typedef VfsBrowserCtrl CLASSNAME;
	VfsBrowserCtrl();
	
	void SetEngine(Engine& m) {vfs_tree.SetEngine(m);}
	void Updated() override;
	void Data();
	VfsValuePtr GetSelected() {return vfs_list.GetSelected();}
	
	Callback WhenVfsChanged;
	
};

class VfsContentCtrl : public ParentCtrl {
	TreeCtrl tree;
	Image ent_icon, comp_icon, iface_icon, vfs_icon;
	VfsValuePtr selected;
	int64 vfs_changed_time = -1;
	
	void OnCursor();
	void AddTreeVfs(int tree_i, VfsValue& v);
	
public:
	typedef VfsContentCtrl CLASSNAME;
	VfsContentCtrl();
	
	void Updated() override;
	
	void SetSelected(VfsValuePtr v) {selected = v;}
	void GetCursor(VfsValuePtr& v);
	
	Callback WhenContentCursor;
	
};

// TODO new router based ctrls
class InterfaceListCtrl : public ParentCtrl {
public:
	virtual void SetSelected(VfsValuePtr v) {}
	virtual void GetCursor(ComponentPtr& c, ExchangeProviderBasePtr& b) {}
	
	Callback WhenInterfaceCursor;
};

#endif

