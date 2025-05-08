#ifndef _Eon_Exchange_h_
#define _Eon_Exchange_h_


class ExchangeSourceProvider;
class ExchangeSideSourceProvider;
class MetaSpaceBase;


template<class T> class OffsetGen;
class ExchangePoint;


template<class T>
struct OffsetLoop {
	using limits = std::numeric_limits<T>;
	using Gen = OffsetGen<T>;
	
	Gen* gen;
	T value;
	
	
	//OffsetLoop() {}
	//OffsetLoop(T value) : value(value) {}
	OffsetLoop(Gen* g, T value) : gen(g), value(value) {}
	OffsetLoop(Gen* g) : gen(g), value(0) {}
	OffsetLoop(Gen& g) : gen(&g), value(0) {}
	OffsetLoop(const OffsetLoop& o) {*this = o;}
	
	OffsetLoop& operator=(const OffsetLoop& o) {gen = o.gen; value = o.value; return *this;}
	
	void Clear() {value = 0;}
	bool operator==(const OffsetLoop& o) const {return o.value == value;}
	bool operator!=(const OffsetLoop& o) const {return o.value != value;}
	void operator+=(const OffsetLoop& o) {value += o.value;}
	void operator-=(const OffsetLoop& o) {value -= o.value;}
	bool operator< (const OffsetLoop& o) {return value <  o.value;}
	bool operator<=(const OffsetLoop& o) {return value <= o.value;}
	bool operator>=(const OffsetLoop& o) {return value >= o.value;}
	bool operator> (const OffsetLoop& o) {return value >  o.value;}
	
	OffsetLoop& operator++() {++value; return *this;}
	OffsetLoop operator++(int) {return OffsetLoop(gen, value++);}
	operator bool() const {return value;}
	operator T() const {return value;}
	
	void SetMax() {value = limits::max();}
	void SetMin() {value = limits::min();}
	void TestSetMin(OffsetLoop v) {if (v.value < value) value = v.value;}
	void TestSetMax(OffsetLoop v) {if (v.value > value) value = v.value;}
	
	String ToString() const {return UPP::AsString(value);}
	
	
	static OffsetLoop GetDifference(OffsetLoop min, OffsetLoop max) {
		OffsetLoop ret(min.gen, 0);
		if (min != max)
			ret.value =
				min.value < max.value ?
					max.value - min.value :
					ret.value = limits::max() - min.value + 1 + max.value - limits::min();
		return ret;
	}
	
	static int64 GetDifferenceI64(OffsetLoop overflow_anchor, OffsetLoop min, OffsetLoop max) {
		int64 anchor = overflow_anchor.value;
		int64 a = (int64)min.value;
		int64 b = (int64)max.value;
		if (b < anchor)
			b += (int64)limits::max() - (int64)limits::min();
		return b - a;
	}
	
};

template<class T>
class OffsetGen {
	using Offset = OffsetLoop<T>;
	
	Offset value;
public:
	
	OffsetGen() : value(this) {}
	
	Offset Create() {return ++value;}
	Offset GetCurrent() const {return value;}
	
	String ToString() const {return "gen(" + value.ToString() + ")";}
	
	//operator Offset() {return ++value;}
};


using off32 = OffsetLoop<dword>;
using off32_gen = OffsetGen<dword>;





struct RealtimeSourceConfig {
	double time_delta = 0;
	double time_total = 0;
	double sync_dt = 3.0;
	double sync_age = 0;
    dword last_sync_src_frame = 0;
	dword frames_after_sync = 0;
	dword src_frame = 0;
	bool enable_sync = false;
	bool sync = 0;
	bool render = 0;
	off32 cur_offset, prev_offset;
	
	
	RealtimeSourceConfig(off32_gen& gen) : cur_offset(gen), prev_offset(gen) {}
	
	void Update(double dt, bool buffer_full);
	//void SetOffset(off32 begin, off32 end) {begin_offset = begin; end_offset = end;}
};

using RTSrcConfig = RealtimeSourceConfig;

#define MIN_AUDIO_BUFFER_SAMPLES 1024




class ExchangeBase
{
	bool fail = false;
	
public:
	ExchangeBase();
	virtual ~ExchangeBase();
	
	bool IsFail() const {return fail;}
	void SetFail() {fail = true;}
	void ClearFail() {fail = false;}
};













struct ExchangeProviderCookie : Pte<ExchangeProviderCookie> {};
typedef Ptr<ExchangeProviderCookie> CookiePtr;





template <class R>
class ExchangeProviderT {
	using ExchangeProviderTmpl = ExchangeProviderT<R>;
	
private:
	ExchangePoint* expt = 0;
	R dst;
	
protected:
	friend class ExchangeSinkProvider;
	friend class ExchangeSourceProvider;
	friend class ExchangeSideSinkProvider;
	friend class ExchangeSideSourceProvider;
	
	int IsLink(R r) const {return r == dst;}
	
	void SetLink(ExchangePoint* expt, R dst) {
		ASSERT_(!this->dst, "Link is already set");
		this->expt = expt;
		this->dst = dst;
	}
	
	
public:
	void Visit(Vis& vis) {(vis & expt) & dst;}
	
	
	void				ClearLink() {expt = 0; dst = 0;}
	ExchangePoint*		GetExPt() const {return expt;}
	R					GetLink() const {return dst;}
	
};


class ExchangeProviderBase : public Pte<ExchangeProviderBase>
{
public:
	virtual String GetConfigString() {return String();}
	
};


class ExchangeSinkProvider :
	public ExchangeProviderBase
{
	
protected:
	friend class ExchangeSourceProvider;
	
	using ExProv = ExchangeProviderT<ExchangeSourceProvider*>;
	
	ExProv base;
	
public:
	using SinkProv = Ptr<ExchangeSinkProvider>;
	using SourceProv = ExchangeSourceProvider*;
	using Cookie = CookiePtr;
	
protected:
	friend class ExchangePoint;
	
	void SetSource(ExchangePoint* expt, ExchangeSourceProvider* src) {base.SetLink(expt, src);}
	int IsSource(ExchangeSourceProvider* src) {return base.IsLink(src);}
	
	virtual void OnLink(SourceProv src, Cookie src_c, Cookie sink_c) {}
	
public:
	ExchangeSinkProvider();
	virtual ~ExchangeSinkProvider();
	
	void						ClearLink() {base.ClearLink();}
	void						Visit(Vis& vis) {vis("base", base, VISIT_NODE);}
	ExchangePoint*				GetExPt() const {return base.GetExPt();}
	ExchangeSourceProvider*		GetSinkLink() const {return base.GetLink();}
	
};

typedef Ptr<ExchangeSinkProvider> ExchangeSinkProviderPtr;


class ExchangeSourceProvider :
	public ExchangeProviderBase
{
public:
	using ExProv = ExchangeProviderT<ExchangeSinkProviderPtr>;
	
private:
	ExProv base;
	
	static bool print_debug;
	
public:
	using SinkProv = ExchangeSinkProviderPtr;
	using SourceProv = Ptr<ExchangeSourceProvider>;
	using Cookie = CookiePtr;
	
protected:
	friend class ExchangePoint;
	
	void SetSink(ExchangePoint* expt, ExchangeSinkProviderPtr sink) {base.SetLink(expt, sink);}
	int IsSink(ExchangeSinkProviderPtr sink) {return base.IsLink(sink);}
	virtual void OnLink(SinkProv sink, Cookie src_c, Cookie sink_c) {}
	
public:
	ExchangeSourceProvider();
	virtual ~ExchangeSourceProvider();
	
	virtual bool				Accept(SinkProv sink, Cookie& src_c, Cookie& sink_c) {return true;}
	void						Link(ExchangePoint* expt, SinkProv sink, Cookie& src_c, Cookie& sink_c);
	void						ClearLink() {base.ClearLink();}
	void						Visit(Vis& vis) {base.Visit(vis);}
	ExchangePoint*				GetExPt() const {return base.GetExPt();}
	ExchangeSinkProvider*		GetSourceLink() const {return base.GetLink();}
	
};

typedef Ptr<ExchangeSourceProvider> ExchangeSourceProviderPtr;








class ExchangeSideSinkProvider :
	public ExchangeProviderBase
{
	
protected:
	friend class ExchangeSideSourceProvider;
	
	using ExProv = ExchangeProviderT<ExchangeSideSourceProvider*>;
	
	ExProv base;
	
public:
	using SideSinkProv = Ptr<ExchangeSideSinkProvider>;
	using SideSourceProv = ExchangeSideSourceProvider*;
	using Cookie = CookiePtr;
	
protected:
	friend class ExchangePoint;
	
	void SetSideSource(ExchangePoint* expt, ExchangeSideSourceProvider* src) {base.SetLink(expt, src);}
	int IsSideSource(ExchangeSideSourceProvider* src) {return base.IsLink(src);}
	
	virtual void OnLink(SideSourceProv src, Cookie src_c, Cookie sink_c) {}
	
public:
	ExchangeSideSinkProvider();
	virtual ~ExchangeSideSinkProvider();
	
	void							ClearLink() {base.ClearLink();}
	void							Visit(Vis& vis) {base.Visit(vis);}
	ExchangePoint*					GetExPt() const {return base.GetExPt();}
	ExchangeSideSourceProvider*		GetSideSinkLink() const {return base.GetLink();}
	
};

using ExchangeSideSinkProviderPtr = Ptr<ExchangeSideSinkProvider>;

class ExchangeSideSourceProvider :
	public ExchangeProviderBase
{
public:
	using ExProv = ExchangeProviderT<ExchangeSideSinkProviderPtr>;
	
private:
	ExProv base;
	
	static bool print_debug;
	
public:
	using SideSinkProv = ExchangeSideSinkProviderPtr;
	using SideSourceProv = Ptr<ExchangeSideSourceProvider>;
	using Cookie = CookiePtr;
	
protected:
	friend class ExchangePoint;
	
	void SetSideSink(ExchangePoint* expt, ExchangeSideSinkProvider* sink) {base.SetLink(expt, sink);}
	int IsSideSink(ExchangeSideSinkProviderPtr sink) {return base.IsLink(sink);}
	virtual void OnLink(SideSinkProv sink, Cookie src_c, Cookie sink_c) {}
	
public:
	ExchangeSideSourceProvider();
	virtual ~ExchangeSideSourceProvider();
	
	virtual bool					Accept(SideSinkProv sink, Cookie& src_c, Cookie& sink_c) {return true;}
	void							Link(ExchangePoint* expt, SideSinkProv sink, Cookie& src_c, Cookie& sink_c);
	void							ClearLink() {base.ClearLink();}
	void							Visit(Vis& vis) {base.Visit(vis);}
	ExchangePoint*					GetExPt() const {return base.GetExPt();}
	ExchangeSideSinkProvider*		GetSideSourceLink() const {return base.GetLink();}
	
};

typedef Ptr<ExchangeSideSourceProvider> ExchangeSideSourceProviderPtr;





class PacketForwarder;

class FwdScope {
	static const int QUEUE_SIZE = 16;
	PacketForwarder* next[QUEUE_SIZE];
	PacketForwarder* cur;
	PacketForwarder* first;
	int read_i, write_i;
	RealtimeSourceConfig* cfg;
	bool is_failed = false;
	bool is_break = false;
	bool is_once = false;
	bool is_looped = false;
	
public:
	
	
	FwdScope() {Clear();}
	FwdScope(PacketForwarder* cb, RealtimeSourceConfig& cfg) {Clear(); SetCfg(cfg); AddNext(cb); ActivateNext();}
	FwdScope(PacketForwarder& cb, RealtimeSourceConfig& cfg) {Clear(); SetCfg(cfg); AddNext(cb); ActivateNext();}
	
	void Clear();
	void ForwardWeak();
	void Forward();
	void ForwardAddNext();
	
	void SetCfg(RealtimeSourceConfig& cfg) {this->cfg = &cfg;}
	void SetFailed(bool b=true) {is_failed = b;}
	void SetOnce(bool b=true) {is_once = b;}
	void Break(bool b=true) {is_break = b;}
	void LoopComplete(bool b=true) {is_looped = b;}
	void AddNext(PacketForwarder& cb) {AddNext(&cb);}
	void AddNext(PacketForwarder* cb);
	void ActivateNext();
	
	bool HasCurrent() const {return !is_failed && cur != 0 && !is_looped && !is_once;}
	RealtimeSourceConfig& Cfg() {ASSERT(cfg); return *cfg;}
	bool IsFailed() const {return is_failed;}
	bool IsBreak() const {return is_break;}
	bool IsOnce() const {return is_once;}
	bool IsLoopComplete() const {return is_looped;}
	int GetPos() const {return read_i-1;}
	PacketForwarder* GetCurrent() const {return cur;}
	String GetFlagString() const;
	
	void operator++(int) {ActivateNext();}
	operator bool() const {return HasCurrent();}
	
};

class PacketForwarder : public Pte<PacketForwarder>
{
public:
	virtual void ForwardSetup(FwdScope& fwd) {}
	virtual void ForwardAtom(FwdScope& fwd) {Panic("not implemented");}
	virtual void ForwardExchange(FwdScope& fwd) {Panic("not implemented");}
	virtual bool IsPacketStuck() {Panic("not implemented"); return true;}
	virtual bool IsLoopComplete(FwdScope& fwd) {Panic("not implemented"); return true;}
	virtual TypeCls GetType() const = 0;
	virtual String GetSecondaryName() {return "";}
	virtual void* GetSecondaryPtr() {return 0;}
	PacketForwarder& GetPacketForwarder() {return *this;}
	String GetDynamicName() const {return GetType().GetName();}
	
};

class PacketForwarderData
{
public:
	
};

class ExchangePoint :
	public PacketForwarder
{
	
protected:
	friend class MetaSpaceBase;
	
	ExchangeSourceProviderPtr	src;
	ExchangeSinkProviderPtr		sink;
	CookiePtr					src_cookie;
	CookiePtr					sink_cookie;
	
public:
	typedef ExchangePoint CLASSNAME;
	ExchangePoint();
	virtual ~ExchangePoint();
	
	virtual void Init(MetaSpaceBase* mdir) = 0;
	
	void Clear();
	void Set(ExchangeSourceProviderPtr src, ExchangeSinkProviderPtr sink);
	void Set(ExchangeSourceProviderPtr src, ExchangeSinkProviderPtr sink, CookiePtr sink_cookie, CookiePtr src_cookie);
	void Visit(Vis& vis) {vis & src & sink & src_cookie & sink_cookie;}
	bool IsLoopComplete(FwdScope& fwd) override {return false;}
	
	ExchangeSourceProviderPtr Source() {return src;}
	ExchangeSinkProviderPtr Sink() {return sink;}
	CookiePtr SourceCookie() {return src_cookie;}
	CookiePtr SinkCookie() {return sink_cookie;}
	
};

typedef Ptr<ExchangePoint> ExchangePointPtr;


class MetaMachineBase : public MetaNodeExt
{
	
public:
	using MetaNodeExt::MetaNodeExt;
	void Visit(Vis&) override {}
};

class MetaSystemBase : public MetaNodeExt
{
public:
	using MetaNodeExt::MetaNodeExt;
	virtual ~MetaSystemBase() {}
	void Visit(Vis&) override {}
};

class MetaSpaceBase : public MetaNodeExt
{
	
protected:
	Vector<ExchangePointPtr> pts;
	
public:
	typedef MetaSpaceBase CLASSNAME;
	MetaSpaceBase(MetaNode& n);
	virtual ~MetaSpaceBase();
	
	virtual void UnlinkAll();
	virtual TypeCls GetType() const = 0;
	
	template <class T>
	Ptr<T> Add() {
		T* o = new T();
		pts.Add(o);
		o->SetParent(this);
		return o;
	}

	template <class T>
	Ptr<T> Add(T* o) {
		if (o) {
			pts.Add(o);
			o->SetParent(this);
			return o;
		}
		else return Ptr<T>();
	}
	
	ExchangePointPtr Add(TypeCls expt);

	//Ptr<ExchangePoint> Add(TypeCls valdev_spec);
	
	void Remove(ExchangePoint* expt);
	
	String ToString() const;
	
	void Visit(Vis& vis) override {vis || pts;}
	
	
	
public:
	
	typedef ExchangePoint* (*NewExpt)();
	struct ExptData : Moveable<ExptData> {
		NewExpt new_fn;
	};
	typedef ArrayMap<TypeCls,ExptData> ExptMap;
	static ArrayMap<TypeCls,ExptData>& ExptDataMap() {MAKE_STATIC(ExptMap, m); return m;}
	template <class T> static ExchangePoint* New() {return new T();}
	template <class T> static void RegisterExchangePoint() {
		ExptData& d = ExptDataMap().GetAdd(AsTypeCls<T>());
		d.new_fn = &New<T>;
	}
	
};

class MetaDirectoryBase : public MetaNodeExt
{
	
public:
	typedef MetaDirectoryBase CLASSNAME;
	MetaDirectoryBase(MetaNode& n);
	virtual ~MetaDirectoryBase();
	
	String ToString() const;
	virtual TypeCls GetType() const = 0;
	
	void Visit(Vis& vis) {}
	
};


#endif
