#ifndef _CoreAlt_Container_h_
#define _CoreAlt_Container_h_

NAMESPACE_UPP


#if 0
template <class T>
class Pick {
	T* var = NULL;
public:
	Pick(T& var) : var(&var) {}
	Pick(Pick&& p) : var(p.var) {p.var = NULL;}
	Pick(const Pick& p) : var(p.var) {}
	T& Get() const {return *var;}
};

template <class T> Pick<T> PickFn(T& o) {return Pick<T>(o);}

// The file is included in Topside and this is required to prevent regular U++ errors
#ifdef LIBTOPSIDE
template <class T> Pick<T> pick(T& o) {return Pick<T>(o);}
#endif
#endif


#undef RTTI_STRING_FN
#define RTTI_STRING_FN(TypeString) MAKE_STATIC_LOCAL(::UPP::String, s); s = TypeString; return s;


END_UPP_NAMESPACE

#endif
