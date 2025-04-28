#ifndef _Eon_ValDevScope_h_
#define _Eon_ValDevScope_h_

class Ex;

class ValueBase :
	virtual public RealtimeStream
{
	bool locked = false;
	
public:
	ValueBase() = default;
	virtual ~ValueBase() = default;
	
	virtual void Exchange(Ex& e) = 0;
	virtual void SetFormat(ValueFormat f) = 0;
	virtual void SetMinQueueSize(int i) = 0;
	virtual void SetMaxQueueSize(int i) = 0;
	virtual int GetQueueSize() const = 0;
	virtual int GetMinPackets() const = 0;
	virtual int GetMaxPackets() const = 0;
	virtual ValueFormat GetFormat() const = 0;
	virtual bool IsQueueFull() const = 0;
	virtual PacketBuffer& GetBuffer() = 0;
	virtual void LockFormat() {}
	
	void Lock() {ASSERT(!locked); locked = true;}
	void Unlock() {ASSERT(locked); locked = false;}
	bool IsLocked() const {return locked;}
};


class DefaultExchangePoint :
	public ExchangePoint
{
	Loop* loop = 0;
	
public:
	typedef DefaultExchangePoint CLASSNAME;
	DefaultExchangePoint() {}
	~DefaultExchangePoint() {Deinit();}
	
	void Init(MetaSpaceBase* conn) override;
	void Deinit();
	void ForwardSetup(FwdScope& fwd) override;
	void ForwardAtom(FwdScope& fwd) override;
	void ForwardExchange(FwdScope& fwd) override;
	bool IsPacketStuck() override;
	
	void Destroy() {loop = 0;}
	
	
	Callback1<DefaultExchangePoint&> WhenEnterValExPtForward;
	
	Callback WhenLeaveValExPtForward;
	
};

using DefaultExchangePointPtr	= Ptr<DefaultExchangePoint>;


class Ex :
	public ExchangeBase
{
	DefaultExchangePoint* expt = 0;
	ValueBase* src = 0;
	const RealtimeSourceConfig* src_conf = 0;
	
public:
	Ex(DefaultExchangePoint* expt) : expt(expt) {}
	Ex(DefaultExchangePoint& expt) : expt(&expt) {}
	
	ValueBase&					Source() const {return *src;}
	const RealtimeSourceConfig&	SourceConfig() const {ASSERT(src_conf); return *src_conf;}
	DefaultExchangePoint&		GetExchangePoint() {return *expt;}
	
	void	Set(ValueBase& src, const RealtimeSourceConfig& conf) {this->src = &src; src_conf = &conf;}
	
};


class Proxy :
	public Value
{
	Value* o = 0;
	
public:
	Proxy() = default;
	Proxy(Value* o) : o(o) {}
	
	void Set(Value* o) {this->o = o;}
	
	operator bool() const {return o != 0;}
	void Exchange(Ex& e) override {if (o) o->Exchange(e);}
	int GetQueueSize() const override {if (o) return o->GetQueueSize(); return 0;}
	ValueFormat GetFormat() const override {if (o) return o->GetFormat(); return ValueFormat();}
	bool IsQueueFull() const override {if (o) return o->IsQueueFull(); return 0;}
	PacketBuffer& GetBuffer() override {if (o) return o->GetBuffer(); PANIC("Empty proxy");}
};


class SimpleValue :
	public Value
{
	ValueFormat		fmt;
	double			time = 0;
	PacketBuffer	buf;
	int				min_packets = 1;
	int				max_packets = 1;
	bool			lock_format = false;
	
public:
	~SimpleValue() {/*LOG("dtor SimpleValue " << HexStr((void*)this));*/ ASSERT(buf.IsEmpty());}
	void			Visit(Vis& vis) {}
	void			Clear() override {/*LOG("clear SimpleValue " << HexStr((void*)this));*/ fmt.Clear(); time = 0; buf.Clear(); min_packets = 1; max_packets = 2;}
	void			Exchange(Ex& e) override;
	int				GetQueueSize() const override;
	ValueFormat		GetFormat() const override;
	bool			IsQueueFull() const override;
	PacketBuffer&	GetBuffer() override {return buf;}
	void			SetFormat(ValueFormat fmt) override {ASSERT(!lock_format); this->fmt = fmt;}
	void			LockFormat() override {lock_format = true;}
	void			SetMinQueueSize(int i) override {min_packets = i; max_packets = max(i, max_packets);}
	void			SetMaxQueueSize(int i) override {max_packets = i; min_packets = min(i, min_packets);}
	int				GetMinPackets() const override {return min_packets;}
	int				GetMaxPackets() const override {return max_packets;}
	Packet			Pick();
	void			AddPacket(Packet& p) {GetBuffer().Add(p);}
};


bool Convert(const ValueFormat& src_fmt, const byte* src, const ValueFormat& dst_fmt, byte* dst);
bool Convert(const Packet& src, Packet& dst, bool keep_tracking=true);


#endif
