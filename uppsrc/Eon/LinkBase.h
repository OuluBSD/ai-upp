#ifndef _Eon_LinkBase_h_
#define _Eon_LinkBase_h_






class CustomerLink : public LinkBase {
	off32_gen	off_gen;
	
	
public:
	typedef CustomerLink CLASSNAME;
	CustomerLink();
	
	bool			Initialize(const WorldState& ws) override;
	void			Uninitialize() override;
	bool			ProcessPackets(PacketIO& io) override;
	RTSrcConfig*	GetConfig() override;
	void			Forward(FwdScope& fwd) override;
	bool			IsLoopComplete(FwdScope& fwd) override;
	void			Visit(Vis& vis) override {vis.VisitT<LinkBase>("LinkBase", *this);}
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
};

class PipeLink : public LinkBase {
	
	
public:
	typedef PipeLink CLASSNAME;
	PipeLink();
	
	bool			Initialize(const WorldState& ws) override;
	void			Uninitialize() override;
	bool			ProcessPackets(PacketIO& io) override;
	void			Visit(Vis& vis) override {vis.VisitT<LinkBase>("LinkBase", *this);}
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
};

class PipeOptSideLink : public LinkBase {
	bool finalize_on_side = false;
	
	
public:
	typedef PipeOptSideLink CLASSNAME;
	PipeOptSideLink();
	
	bool			Initialize(const WorldState& ws) override;
	void			Uninitialize() override;
	bool			ProcessPackets(PacketIO& io) override;
	void			Visit(Vis& vis) override {vis.VisitT<LinkBase>("LinkBase", *this);}
	void			SetFinalizeOnSide(bool b=true) {finalize_on_side = b;}
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
};

class IntervalPipeLink : public AsyncMemForwarderBase {
	RunningFlag			flag;
	
	
public:
	typedef IntervalPipeLink CLASSNAME;
	IntervalPipeLink();
	~IntervalPipeLink();
	
	bool			Initialize(const WorldState& ws) override;
	void			Uninitialize() override;
	void			Visit(Vis& vis) override {vis.VisitT<AsyncMemForwarderBase>("AsyncMemForwarderBase", *this);}
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
	void IntervalSinkProcess();
	
};

class PollerLink : public FramePollerBase {
	bool finalize_on_side = false;
	
public:
	typedef PollerLink CLASSNAME;
	PollerLink();
	~PollerLink();
	
	bool			Initialize(const WorldState& ws) override;
	void			Uninitialize() override;
	void			Visit(Vis& vis) override {vis.VisitT<FramePollerBase>("FramePollerBase", *this);}
	bool			IsReady(PacketIO& io) override;
	bool			ProcessPackets(PacketIO& io) final;
	void			SetFinalizeOnSide(bool b=true) {finalize_on_side = b;}
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
	
};

class ExternalPipeLink : public AsyncMemForwarderBase {
	
public:
	typedef ExternalPipeLink CLASSNAME;
	ExternalPipeLink();
	~ExternalPipeLink();
	
	bool			Initialize(const WorldState& ws) override;
	void			Uninitialize() override;
	void			Visit(Vis& vis) override {vis.VisitT<AsyncMemForwarderBase>("AsyncMemForwarderBase", *this);}
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
};

class DriverLink : public LinkBase {
	
public:
	typedef DriverLink CLASSNAME;
	DriverLink();
	~DriverLink();
	
	bool			Initialize(const WorldState& ws) override;
	void			Uninitialize() override;
	void			Visit(Vis& vis) override {vis.VisitT<LinkBase>("LinkBase", *this);}
	bool			ProcessPackets(PacketIO& io) override;
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
};

class MergerLink : public LinkBase
{
	byte scheduler_iter = 1;
	
public:
	MergerLink();
	bool Initialize(const WorldState& ws) override;
	bool PostInitialize() override;
	void Uninitialize() override;
	void Visit(Vis& vis) override {vis.VisitT<LinkBase>("LinkBase", *this);}
	bool IsReady(PacketIO& io) override;
	bool ProcessPackets(PacketIO& io) override;
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
};

class JoinerLink : public LinkBase
{
	byte scheduler_iter = 1;
	
public:
	JoinerLink();
	bool Initialize(const WorldState& ws) override;
	bool PostInitialize() override;
	void Uninitialize() override;
	void Visit(Vis& vis) override {vis.VisitT<LinkBase>("LinkBase", *this);}
	bool IsReady(PacketIO& io) override;
	bool ProcessPackets(PacketIO& io) override;
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
};

class SplitterLink : public LinkBase
{
	
public:
	SplitterLink();
	bool Initialize(const WorldState& ws) final;
	void Uninitialize() final;
	void Visit(Vis& vis) override {vis.VisitT<LinkBase>("LinkBase", *this);}
	bool IsReady(PacketIO& io) final;
	bool ProcessPackets(PacketIO& io) override;
	
	static LinkTypeCls GetType();
	LinkTypeCls GetLinkType() const override {return GetType();}
	
};




#endif
