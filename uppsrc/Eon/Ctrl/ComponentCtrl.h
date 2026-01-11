#ifndef _EonCtrl_ComponentCtrl_h_
#define _EonCtrl_ComponentCtrl_h_


class ComponentCtrl : public ParentCtrl {
	
	
public:
	typedef ComponentCtrl CLASSNAME;
	ComponentCtrl() {}
	
	virtual void SetComponent(Component& c) = 0;
	
};

typedef ComponentCtrl*(*ComponentCtrlFactory)();
typedef VectorMap<TypeCls, ComponentCtrlFactory> ComponentCtrlFactoryMap;

void RegisterComponentCtrlFactory(TypeCls comp, ComponentCtrlFactory fac);
ComponentCtrl* NewComponentCtrl(TypeCls comp);

template <class T> ComponentCtrl* ComponentCtrlFactoryFn() {return new T();}
template <class I, class C> void MakeComponentCtrlFactory() {
	RegisterComponentCtrlFactory(AsTypeCls<I>(), &ComponentCtrlFactoryFn<C>);
}


#endif
