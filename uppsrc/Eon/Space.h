#ifndef _Eon_Space_h_
#define _Eon_Space_h_


class Loop;
class AtomBase;

class Space :
	public MetaSpaceBase
{
	mutable Machine*	machine = 0;
	//BitField<dword>		freeze_bits;
	String				name;
	String				prefab;
	SpaceId				id;
	int64				created = 0;
	int64				changed = 0;
	
protected:
	friend class SpaceStore;
	friend class ScriptLoader;
	
	Loop*		loop = 0;
	
	void SetId(SpaceId i) {id = i;}
	
public:
	typedef Space CLASSNAME;
	using SpacePtr = Ptr<Space>;
	//using SpaceVec = Array<Space>;
	static SpaceId GetNextId();
	
	Space(MetaNode& n);
	~Space();
	
	
	SpaceId GetId() const {return id;}
	
	void SetName(String s)			{name = s;}
	void SetPrefab(String s)		{prefab = s;}
	void SetCreated(int64 i)		{created = i;}
	void SetChanged(int64 i)		{changed = i;}
	
	void				Clear();
	void				StopDeep();
	void				Stop();
	void				UnlinkDeep();
	void				UnrefDeep();
	void				UninitializeAtomsDeep();
	void				ClearStatesDeep();
	void				ClearAtomsDeep();
	void				ClearDeep();
	void				Dump();
	String				GetTreeString(int indent=0);
	
	Loop*				GetLoop() const;
	Space*				GetParent() const;
	Machine&			GetMachine() const;
	String				GetName() const {return name;}
	String				GetDeepName() const;
	//bool				HasAtoms() const {return !atoms.IsEmpty();}
	bool				HasSpaces() const;// {return !spaces.IsEmpty();}
	
	void				Initialize(Space& l, String prefab="Custom");
	
	SpacePtr			CreateEmpty();
	SpacePtr			GetAddEmpty(String name);
	void				CopyTo(Space& l) const;
	
	bool				Link(AtomBase* src_comp, AtomBase* dst_comp, ValDevCls iface);
	
	AtomBasePtr			AsTypeCls(AtomTypeCls atom_type);
	AtomBasePtr			AddTypeCls(AtomTypeCls cls);
	AtomBasePtr			GetAddTypeCls(AtomTypeCls cls);
	AtomBasePtr			FindTypeCls(AtomTypeCls atom_type);
	SpacePtr			FindSpaceByName(String name);
	AtomBasePtr			FindAtom(AtomTypeCls atom_type);
	AtomBasePtr			FindDeepCls(AtomTypeCls atom_type);
	
	template <class T>
	Ptr<T> FindDeep() {return FindDeepCls(T::TypeIdClass());}
	
	
	AtomBasePtr			AddPtr(AtomBase* atom);
	void				InitializeAtoms();
	void				InitializeAtom(AtomBase& atom);
	void				InitializeAtomRef(AtomBase* atom) {return InitializeAtom(*atom);}
	void				UninitializeAtoms();
	void				ClearAtoms();
	void				AppendCopy(const Space& l);
	
	int					GetSpaceDepth() const;
	bool				HasSpaceParent(SpacePtr pool) const;
	
	void				UnlinkExchangePoints();
	
	template<typename T>
	T* FindCast() {
		auto atoms = node.FindAll<T>();
		for (auto& it : atoms) {
			T* o = CastPtr<T>(&*it.atom);
			if (o)
				return o;
		}
		return 0;
	}
	
	template<typename T> T* FindNearestAtomCast(int nearest_loop_depth);
	EnvStatePtr FindNearestState(String name);
	EnvStatePtr FindStateDeep(String name);
	
	Vector<EnvStatePtr> GetStates() const;// {return states;}
	Vector<AtomBasePtr> GetAtoms() const;// {return atoms;}
	Vector<SpacePtr> GetSpaces() const;// {return spaces;}
	
	SpacePtr AddSpace(String name="") {
		/*Space& p = spaces.Add();
		//p.SetParent(HierExBaseParent(0, this));
		p.SetName(name);
		p.SetId(GetNextId());
		return &p;*/
		TODO; return 0;
	}
	
	SpacePtr GetAddSpace(String name); /*{
		for (Space& pool : spaces)
			if (pool.GetName() == name)
				return &pool;
		return AddSpace(name);
	}*/
	
	EnvStatePtr AddState(String name=""); /*{
		EnvState& p = states.Add();
		//p.SetParent(this);
		p.SetName(name);
		return &p;
	}*/
	
	EnvStatePtr GetAddEnv(String name); /*{
		if (EnvStatePtr e = FindState(name))
			return e;
		return AddState(name);
	}*/
	
	EnvStatePtr FindState(String name); /*{
		for (EnvState& s : states)
			if (s.GetName() == name)
				return &s;
		return EnvStatePtr();
	}*/
	
	//AtomMap::Iterator			begin()			{return atoms.begin();}
	//AtomMap::Iterator			end()			{return atoms.end();}
	//SpaceVec::Iterator			BeginSpace()	{return spaces.begin();}
	
	void Visit(Vis& vis);
	void VisitSinks(Vis& vis);
	void VisitSources(Vis& vis);
	
private:
	StateVec				states;
	//AtomMap					atoms;
	//SpaceVec				spaces;
};

//using SpaceVec = Array<Space>;
using SpacePtr = Ptr<Space>;

#if 0
class SpaceHashVisitor : public Vis {
	CombineHash ch;
	
	bool OnEntry(const TypeCls& type, TypeCls derived, const char* derived_name, void* mem) override;
public:
	
	
	operator hash_t() const {return ch;}
	
};
#endif

template<typename T>
T* Space::FindNearestAtomCast(int nearest_space_depth) {
	if (auto r = FindCast<T>())
		return r;
	
	if (nearest_space_depth > 0) {
		auto spaces = node.FindAll<Space>();
		for (auto& space : spaces)
			if (auto ret = space->FindNearestAtomCast<T>(nearest_space_depth-1))
				return ret;
	}
	
	if (Space* p = GetParent())
		return p->FindNearestAtomCast<T>(nearest_space_depth);
	
	return 0;
}



#endif
