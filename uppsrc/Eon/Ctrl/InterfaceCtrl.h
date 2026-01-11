#ifndef _EonCtrl_InterfaceCtrl_h_
#define _EonCtrl_InterfaceCtrl_h_


class InterfaceCtrl : public ParentCtrl {
	
	
public:
	typedef InterfaceCtrl CLASSNAME;
	InterfaceCtrl() {}
	
	virtual void SetInterface(ComponentPtr c, ExchangeProviderBasePtr b) = 0;
	
};

typedef InterfaceCtrl*(*InterfaceCtrlFactory)();
typedef VectorMap<TypeCls, InterfaceCtrlFactory> InterfaceCtrlFactoryMap;

void RegisterInterfaceCtrlFactory(TypeCls iface, InterfaceCtrlFactory fac);
InterfaceCtrl* NewInterfaceCtrl(TypeCls iface);

template <class T> InterfaceCtrl* InterfaceCtrlFactoryFn() {return new T();}
template <class I, class C> void MakeInterfaceCtrlFactory() {
	RegisterInterfaceCtrlFactory(AsTypeCls<I>(), &InterfaceCtrlFactoryFn<C>);
}


#endif
