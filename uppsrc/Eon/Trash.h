#ifndef _Eon_Trash_h_
#define _Eon_Trash_h_

struct Atom : MetaNodeExt {
	
	struct CustomerData {
		RealtimeSourceConfig	cfg;
		off32_gen				gen;
		
		CustomerData();
		~CustomerData();
	};
	
protected:
	friend class ScriptLoopLoader;
	friend class ScriptDriverLoader;
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
	Atom*					atom_dependency = 0;
	int						dep_count = 0;
	EscValue				user_data;
	
	
public:
	virtual AtomTypeCls		GetType() const = 0;
	virtual void			CopyTo(Atom* atom) const = 0;
	virtual bool			Initialize(const WorldState& ws) = 0;
	virtual bool			InitializeAtom(const WorldState& ws) = 0;
	virtual void			Uninitialize() = 0;
	virtual void			UninitializeAtom() = 0;
	virtual void			VisitSource(Vis& vis) = 0;
	virtual void			VisitSink(Vis& vis) = 0;
	virtual void			ClearSinkSource() = 0;
	virtual Atom*			GetSource() = 0;
	virtual Atom*			GetSink() = 0;
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
	virtual bool			AttachContext(Atom& a) {Panic("Unimplemented"); NEVER(); return false;}
	virtual void			DetachContext(Atom& a) {Panic("Unimplemented"); NEVER();}
	virtual RealtimeSourceConfig* GetConfig() {return 0;}
	virtual bool			NegotiateSinkFormat(LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {return false;}
	
	EscValue&				UserData() {return user_data;}
	bool					IsRunning() const {return is_running;}
	bool					IsInitialized() const {return is_initialized;}
	void					AddAtomToUpdateList();
	void					RemoveAtomFromUpdateList();
	void					SetDependency(Atom* a) {if (atom_dependency) atom_dependency->dep_count--; atom_dependency = a; if (a) a->dep_count++;}
	Atom*					GetDependency() const {return atom_dependency;}
	int						GetDependencyCount() const {return dep_count;}
	void					ClearDependency() {SetDependency(0);}
	void					UpdateSinkFormat(ValCls val, ValueFormat fmt);
	void					PostContinueForward();
	void					SetQueueSize(int queue_size);
	
	Machine&				GetMachine();
	void					UninitializeDeep();
	void					SetInterface(const IfaceConnTuple& iface);
	const IfaceConnTuple&	GetInterface() const;
	int						FindSourceWithValDev(ValDevCls vd);
	int						FindSinkWithValDev(ValDevCls vd);
	void					SetPrimarySinkQueueSize(int i);
	
public:
	Atom();
	virtual ~Atom();
	
	
	Space*			GetSpace();
	Space&			GetParent();
	LinkBase*		GetLink();
	int				GetId() const {return id;}
	
	template <class ValDevSpec, class T> bool LinkManually(T& o, String* err_msg=0);
	
	
};

NAMESPACE_UPP



END_UPP_NAMESPACE

#endif
