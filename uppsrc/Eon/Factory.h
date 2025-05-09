#ifndef _Eon_Factory_h_
#define _Eon_Factory_h_


class Factory {
	
public:
	
	// Interfaces
	struct Link : Moveable<Link> {
		AtomTypeCls		dst_atom;
		ValDevTuple		iface;
		ValDevCls		common_vd;
	};
	
	struct IfaceData : Moveable<IfaceData> {
		TypeCls			cls;
		ValDevCls		vd;
		String			name;
	};
	typedef VectorMap<ValDevTuple,IfaceData> IfaceMap;
	static IfaceMap& IfaceLinkDataMap() {MAKE_STATIC(IfaceMap, m); return m;}
	
	template <class T> static void RegisterInterfaceLink(DevCls dev, ValCls val) {
		ValDevCls vd(dev,val);
		IfaceData& d = IfaceLinkDataMap().GetAdd(vd);
		d.cls = AsTypeCls<T>();
		d.name = AsTypeName<T>();
		d.vd.dev = dev;
		d.vd.val = val;
		MetaSpaceBase::RegisterExchangePoint<T>();
	}
	
	
	
	
	
	// Atoms
	
	typedef AtomBase* (*NewFn)(MetaNode&);
	typedef LinkTypeCls (*GetLinkTypeFn)();
	struct AtomData : Moveable<AtomData> {
		NewFn			new_fn;
		String			name;
		AtomTypeCls		cls;
		TypeCls			rtti_cls;
		LinkTypeCls		link_type;
		Vector<String>	actions;
		
		Vector<Link>	sink_links;
		bool			searched_sink_links = false;
	};
	typedef VectorMap<AtomTypeCls,AtomData> AtomMap;
	static AtomMap& AtomDataMap() {MAKE_STATIC(AtomMap, m); return m;}
	
	template <class T> static AtomBase* CreateAtom(MetaNode& n) {return new T(n);}
	
	template <class T> static void RegisterAtom() {
		AtomTypeCls cls = T::GetAtomType();
		ASSERT(cls.IsValid());
		AtomData& d = AtomDataMap().GetAdd(cls);
		d.rtti_cls = AsTypeCls<T>();
		d.cls = cls;
		d.name = AsTypeName<T>();
		d.new_fn = &CreateAtom<T>;
		d.link_type = T::GetLinkType();
		d.actions.Add(T::GetAction());
	}
	
	static LinkedList<AtomTypeCls>& GetAtomTypes() {static LinkedList<AtomTypeCls> l; return l;}
	
	static void Dump();
	static const Vector<Link>& GetSinkAtoms(AtomTypeCls src_atom);
	static void RefreshLinks(AtomData& d);
	
	
	
	// Links
	
	typedef LinkBase* (*NewLinkFn)(MetaNode&);
	struct LinkData : Moveable<LinkData> {
		NewLinkFn		new_fn;
		String			name;
		LinkTypeCls		cls;
		TypeCls			rtti_cls;
	};
	typedef VectorMap<LinkTypeCls,LinkData> LinkMap;
	static LinkMap& LinkDataMap() {MAKE_STATIC(LinkMap, m); return m;}
	
	template <class T> static LinkBase* CreateLink(MetaNode& n) {return new T(n);}
	
	template <class T> static void RegisterLink() {
		LinkTypeCls cls = T::GetLinkTypeStatic();
		ASSERT(cls.IsValid());
		LinkData& d = LinkDataMap().GetAdd(cls);
		d.rtti_cls = AsTypeCls<T>();
		d.cls = cls;
		d.name = AsTypeName<T>();
		d.new_fn = &CreateLink<T>;
	}
	
	static LinkedList<LinkTypeCls>& GetLinkTypes() {static LinkedList<LinkTypeCls> l; return l;}
	
	static const Vector<Link>& GetSinkLinks(LinkTypeCls src_link);
	static void RefreshLinks(LinkData& d);
	static LinkTypeCls GetLinkLinkType(LinkTypeCls link);
	
	
};


#endif
