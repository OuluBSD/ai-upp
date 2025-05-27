#ifndef _Core2_Atom_h_
#define _Core2_Atom_h_


class LinkBase;
class WorldState;





class PacketForwarderData {};


struct AtomBase :
	VfsValueExt,
	PacketForwarderData,
	Destroyable
{
	using AtomBasePtr = Ptr<AtomBase>;
	
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
	
	int64					idx = -1;
	bool					is_running = false;
	bool					is_initialized = false;
	
	void					SetIdx(int64 i) {idx = i;}
	void					SetRunning(bool b=true) {is_running = b;}
	void					SetInitialized(bool b=true) {is_initialized = b;}
	
protected:
	friend class LinkBase;
	
	Mutex					fwd_lock;
	IfaceConnTuple			iface;
	LinkBase*				link = 0;
	AtomBasePtr				atom_dependency;
	int						dep_count = 0;
	//Value					user_data; // use val.value instead
	
	
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
	
	Value&					UserData() {return val.value;}
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
	
	Engine&					GetMachine();
	void					UninitializeDeep();
	void					SetInterface(const IfaceConnTuple& iface);
	const IfaceConnTuple&	GetInterface() const;
	int						FindSourceWithValDev(ValDevCls vd);
	int						FindSinkWithValDev(ValDevCls vd);
	void					SetPrimarySinkQueueSize(int i);
	
public:
	AtomBase(VfsValue& n);
	virtual ~AtomBase();
	
	template <class T> T* GetSourceT() {return dynamic_cast<T*>(&*this->GetSource());}
	template <class T> T* GetSinkT()   {return dynamic_cast<T*>(&*this->GetSink());}
	
	//Space*			GetSpace();
	//Space&			GetParent();
	LinkBase*		GetLink();
	int64			GetIdx() const {return idx;}
	
	template <class T> Ptr<T> As() {return ::UPP::CastPtr<T>(this);}
	
	template <class S> void AddToSystem();
	template <class S> void RemoveFromSystem();
	
	template <class ValDevSpec, class T> bool LinkManually(T& o, String* err_msg=0);
	
	
};

using AtomBasePtr = Ptr<AtomBase>;

#endif
