#ifndef _Eon_Atom_h_
#define _Eon_Atom_h_

class WorldState;
class Plan;
class LinkBase;
class Space;

template <class T> inline SideStatus MakeSide(const AtomTypeCls& src_type, const WorldState& from, const AtomTypeCls& sink_type, const WorldState& to) {Panic("Unimplemented"); NEVER(); return SIDE_NOT_ACCEPTED;}
//template <class T> inline RefT_Atom<T> AtomBase_Static_As(AtomBase*) {return RefT_Atom<T>();}

class AtomBase :
	public MetaNodeExt,
	public PacketForwarderData
{
	
	
public:
	using AtomBasePtr = Ptr<AtomBase>;
	
	struct CustomerData {
		RealtimeSourceConfig	cfg;
		off32_gen				gen;
		
		CustomerData();
		~CustomerData();
	};
	
	
	
protected:
	friend class Eon::ScriptLoopLoader;
	friend class Eon::ScriptDriverLoader;
	friend class Loop;
	friend class Space;
	
	int						id = -1;
	bool					is_running = false;
	bool					is_initialized = false;
	
	void					SetId(int i) {id = i;}
	void					SetRunning(bool b=true) {is_running = b;}
	void					SetInitialized(bool b=true) {is_initialized = b;}
	
protected:
	friend class LinkBase;
	
	Mutex					fwd_lock;
	IfaceConnTuple			iface;
	LinkBase*				link = 0;
	AtomBase*				atom_dependency = 0;
	int						dep_count = 0;
	EscValue				user_data;
	
	
public:
	virtual AtomTypeCls		GetType() const = 0;
	virtual void			CopyTo(AtomBase* atom) const = 0;
	virtual bool			Initialize(const WorldState& ws) = 0;
	virtual bool			InitializeAtom(const WorldState& ws) = 0;
	virtual void			Uninitialize() = 0;
	virtual void			UninitializeAtom() = 0;
	virtual void			VisitSource(Vis& vis) = 0;
	virtual void			VisitSink(Vis& vis) = 0;
	virtual void			ClearSinkSource() = 0;
	virtual ISourcePtr		GetSource() = 0;
	virtual ISinkPtr		GetSink() = 0;
	virtual bool			Send(RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) = 0;
	
	virtual bool			Start() {return true;}
	virtual void			Stop() {}
	virtual void			Visit(Vis& vis) {}
	virtual bool			PostInitialize() {return true;}
	virtual void			Update(double dt) {}
	virtual String			ToString() const;
	virtual bool			IsReady(PacketIO& io) {return true;}
	virtual void			UpdateConfig(double dt) {Panic("Unimplemented"); NEVER();}
	virtual bool			Recv(int sink_ch, const Packet& in);
	virtual void			Finalize(RealtimeSourceConfig& cfg) {}
	// internal format should be sink:0
	virtual bool			Consume(const void* data, int len) {Panic("Unimplemented"); return false;}
	virtual bool			IsForwardReady() {Panic("Unimplemented"); NEVER(); return false;}
	virtual void			ForwardPacket(PacketValue& in, PacketValue& out) {Panic("Unimplemented"); NEVER();}
	virtual bool			AttachContext(AtomBase& a) {Panic("Unimplemented"); NEVER(); return false;}
	virtual void			DetachContext(AtomBase& a) {Panic("Unimplemented"); NEVER();}
	virtual RealtimeSourceConfig* GetConfig() {return 0;}
	virtual bool			NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {return false;}
	
	EscValue&				UserData() {return user_data;}
	bool					IsRunning() const {return is_running;}
	bool					IsInitialized() const {return is_initialized;}
	void					AddAtomToUpdateList();
	void					RemoveAtomFromUpdateList();
	void					SetDependency(AtomBase* a) {if (atom_dependency) atom_dependency->dep_count--; atom_dependency = a; if (a) a->dep_count++;}
	AtomBase*				GetDependency() const {return atom_dependency;}
	int						GetDependencyCount() const {return dep_count;}
	void					ClearDependency() {SetDependency(0);}
	void					UpdateSinkFormat(ValCls val, ValueFormat fmt);
	void					PostContinueForward();
	void					SetQueueSize(int queue_size);
	
	//Machine&				GetMachine();
	void					UninitializeDeep();
	void					SetInterface(const IfaceConnTuple& iface);
	const IfaceConnTuple&	GetInterface() const;
	int						FindSourceWithValDev(ValDevCls vd);
	int						FindSinkWithValDev(ValDevCls vd);
	void					SetPrimarySinkQueueSize(int i);
	
public:
	AtomBase(MetaNode& n);
	virtual ~AtomBase();
	
	template <class T> T* GetSourceT() {return dynamic_cast<T*>(&*this->GetSource());}
	template <class T> T* GetSinkT()   {return dynamic_cast<T*>(&*this->GetSink());}
	
	Space*			GetSpace();
	Space&			GetParent();
	LinkBase*		GetLink();
	int				GetId() const {return id;}
	
	template <class T> Ptr<T> As() {return ::UPP::CastPtr<T>(this);}
	
	template <class S, class R>
	void AddToSystem(R ref) {
		TODO/*Ptr<S> sys = GetMachine().Get<S>();
		if (sys)
			sys->Add(ref);*/
	}
	
	template <class S, class R>
	void RemoveFromSystem(R ref) {
		TODO/*Ptr<S> sys = GetMachine().Get<S>();
		if (sys)
			sys->Remove(ref);*/
	}
	
	template <class ValDevSpec, class T> bool LinkManually(T& o, String* err_msg=0);
	
	
};





struct Atom :
	public AtomBase,
	public DefaultInterfaceSink,
	public DefaultInterfaceSource
{
public:
	using SinkT = DefaultInterfaceSink;
	using SourceT = DefaultInterfaceSource;
	
	
	Atom(MetaNode& n) : AtomBase(n) {}
	
	bool InitializeAtom(const WorldState& ws) override {
		return SinkT::Initialize() && SourceT::Initialize();
	}
	
	void UninitializeAtom() override {
		SinkT::Uninitialize();
		SourceT::Uninitialize();
	}
	
	void ClearSinkSource() override {
		SinkT::ClearSink();
		SourceT::ClearSource();
	}
	
	void Visit(Vis& vis) override {
		vis.VisitT<AtomBase>("AtomBase", *this);
		vis.VisitT<SinkT>("SinkT", *this);
		vis.VisitT<SourceT>("SourceT", *this);
	}
	
	void VisitSource(Vis& vis) override {
		vis.VisitT<SourceT>("SourceT", *this);
	}
	
	void VisitSink(Vis& vis) override {
		vis.VisitT<SinkT>("SinkT", *this);
	}

	void CopyTo(AtomBase* target) const override {
		ASSERT(target->GetType() == ((AtomBase*)this)->GetType());
	    
	    TODO
	}
	
	
	InterfaceSourcePtr GetSource() override {
		InterfaceSource* src = static_cast<InterfaceSource*>(this);
		ASSERT(src);
		return InterfaceSourcePtr(src);
	}
	
	InterfaceSinkPtr GetSink() override {
		InterfaceSink* sink = static_cast<InterfaceSink*>(this);
		ASSERT(sink);
		return InterfaceSinkPtr(sink);
	}
	
	AtomBase* AsAtomBase() override {return static_cast<AtomBase*>(this);}
	void ClearSink() override {TODO}
	void ClearSource() override {TODO}
	
	
	static ParallelTypeCls::Type GetSerialType() {return ParallelTypeCls::CUSTOM_ATOM;}
	
	
};



//using AtomRefMap	= ArrayMap<AtomTypeCls,Ptr<AtomBase>>;
using AtomBasePtr = Ptr<AtomBase>;

class AtomMap {
	struct Item : Moveable<Item> {
		AtomTypeCls type;
		AtomBasePtr atom;
	};
	Vector<Item> atoms;
	
	void Add(AtomTypeCls type, AtomBasePtr ptr) {
		Item& i = atoms.Add();
		i.type = type;
		i.atom = ptr;
	}
	void ReturnAtom(AtomStore& s, AtomBase* c);
	
public:
	using Iterator = Vector<Item>::Iterator;
	
	AtomMap() {}
	
	#define IS_EMPTY_SHAREDPTR(x) (x.IsEmpty())
	
	void Dump();
	bool IsEmpty() const {return atoms.IsEmpty();}
	int GetCount() const {return atoms.GetCount();}
	AtomBase& operator[](int i) {return *atoms[i].atom;}
	void Clear() {atoms.Clear();}
	Iterator begin() {return atoms.begin();}
	Iterator end() {return atoms.end();}
	
	template<typename AtomT>
	Ptr<AtomT> Get() {
		CXX2A_STATIC_ASSERT(AtomStore::IsAtom<AtomT>::value, "T should derive from Atom");
		
		int i = Find(AsParallelTypeCls<AtomT>());
		ASSERT(i >= 0);
		if (i < 0)
			throw Exc("Could not find atom " + (String)AsTypeName<AtomT>());
		
		return &atoms[i];
	}
	
	template<typename AtomT>
	Ptr<AtomT> Find() {
		CXX2A_STATIC_ASSERT(AtomStore::IsAtom<AtomT>::value, "T should derive from Atom");
		
		int i = Find(AsParallelTypeCls<AtomT>());
		if (i < 0)
			return Null;
		else
			return &atoms[i];
	}
	
	template<typename AtomT>
	void Add(AtomT* atom) {
		CXX2A_STATIC_ASSERT(AtomStore::IsAtom<AtomT>::value, "T should derive from Atom");
		
		AtomTypeCls type = atom->GetType();
		ASSERT(type.IsValid());
		auto& it = atoms.Add();
		it.type = type;
		it.atom = atom;
	}
	
	#if 0
	template<typename AtomT>
	void Remove(AtomStorePtr s) {
		CXX2A_STATIC_ASSERT(AtomStore::IsAtom<AtomT>::value, "T should derive from Atom");
		
		AtomMapBase::Iterator iter = AtomMapBase::Find(AsParallelTypeCls<AtomT>());
		ASSERT_(iter, "Tried to remove non-existent atom");
		
		TODO //iter.value().Uninitialize();
		
		//iter.value().Destroy();
		
		ReturnAtom(*s, iter.value.GetItem()->value.Detach());
		AtomMapBase::Remove(iter);
	}
	#endif
	
	void AddBase(AtomBase* atom) {
		AtomTypeCls type = atom->GetType();
		ASSERT(type.IsValid());
		auto& it = atoms.Add();
		it.type = type;
		it.atom = atom;
	}
	
	#undef IS_EMPTY_SHAREDPTR
	
};

#endif
