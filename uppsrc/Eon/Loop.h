#ifndef _Eon_Loop_h_
#define _Eon_Loop_h_



class Loop :
	public MetaDirectoryBase
{
	String				prefab;
	LoopId				id;
	
protected:
	friend class LoopStore;
	friend class Eon::ScriptLoader;
	
	Space*		space = 0;
	
	void SetId(LoopId i) {id = i;}
	
public:
	CLASSTYPE(Loop)
	using LoopPtr = Ptr<Loop>;
	//using LoopVec = Array<Loop>;
	static LoopId GetNextId();
	
	Loop(MetaNode& n);
	~Loop();
	
	LoopId GetId() const {return id;}
	
	void SetPrefab(String s)		{prefab = s;}
	
	void				Clear();
	void				ClearInterfacesDeep();
	void				UnrefDeep();
	void				UninitializeLinksDeep();
	void				ClearDeep();
	void				Dump();
	String				GetTreeString(int indent=0);
	
	Loop*				GetParent() const;
	Space*				GetSpace() const;
	String				GetDeepName() const;
	bool				HasAtoms() const;//{return !links.IsEmpty();}
	bool				HasLoops() const;// {return !loops.IsEmpty();}
	
	void				Initialize(Loop& l, String prefab="Custom");
	
	LoopPtr				CreateEmpty();
	LoopPtr				GetAddEmpty(String name);
	
	bool				MakeLink(AtomBasePtr src_atom, AtomBasePtr dst_atom);
	
	void				OnChange();
	LinkBasePtr			AddTypeCls(LinkTypeCls cls);
	LinkBasePtr			GetAddTypeCls(LinkTypeCls cls);
	LinkBasePtr			FindTypeCls(LinkTypeCls atom_type);
	LoopPtr				FindLoopByName(String name);
	
	
	
	LinkBasePtr			AddPtr(LinkBase* link);
	void				InitializeLink(LinkBase& atom);
	void				InitializeLinks();
	void				AppendCopy(const Loop& l);
	
	int					GetLoopDepth() const;
	bool				HasLoopParent(LoopPtr pool) const;
	
	//LoopVec& GetLoops() {return loops;}
	
	LoopPtr AddLoop(String name="");
	LoopPtr GetAddLoop(String name);
	EnvStatePtr GetAddEnv(String name) {return space->GetAddEnv(name);}
	
	void Visit(Vis& vis) override;
	
private:
	//LinkMap					links;
	//LoopVec					loops;
};

//using LoopVec = Array<Loop>;
using LoopPtr = Ptr<Loop>;

class LoopHashVisitor : public Vis {
	CombineHash ch;
	
	//bool OnEntry(const RTTI& type, TypeCls derived, const char* derived_name, void* mem, LockedScopeRefCounter* ref) override;
public:
	
	
	operator hash_t() const {return ch;}
	
};


#endif
