#ifndef _Eon_Container_h_
#define _Eon_Container_h_


class EnvState : public Pte<EnvState>
{
	VectorMap<dword, Value> data;
	String name;
	
public:
	void Visit(Vis& vis) {}
	void SetName(String s) {name = s;}
	const String& GetName() const {return name;}
	
	
	bool&	SetBool(dword key, bool b);
	int&	SetInt(dword key, int i);
	
	bool&	GetBool(dword key);
	int&	GetInt(dword key);
	
	template <class T>
	T& Set(dword key) {
		Value& o = data.GetAdd(key);
		if (o.Is<T>())
			return const_cast<T&>(o.Get<T>());
		else
			return CreateRawValue<T>(o);
	}
	
	template <class T>
	T* Get(dword key) {
		int i = data.Find(key);
		if (i < 0)
			return 0;
		Value& o = data[i];
		if (o.Is<T>())
			return &const_cast<T&>(o.Get<T>());
		return 0;
	}
};

//using ExchangeBaseParent	= RefParent1<MetaSpaceBase>;
//using EnvStateParent		= ExchangeBaseParent;
using EnvStatePtr			= Ptr<EnvState>;
using StateVec				= Array<EnvState>;


inline String Demangle(const char* name) {
	#if defined __GNUG__ && (defined flagGCC || defined flagCLANG)
	int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name ;
    #endif
    return name;
}


class TypeId : Moveable<TypeId>
{
	const std::type_info* info = 0;
public:
	TypeId() {}
	TypeId(const TypeId& id) : info(id.info) {}
	
	
    hash_t GetHashValue() const { return info ? (hash_t)std::type_index(*info).hash_code() : 0; }
    char const* name() const { return info ? info->name() : ""; }
    String DemangledName() const {return Demangle(name());}
	String CleanDemangledName() const {
		String s(DemangledName());
		if (s.Find("") == 0)
			s = s.Mid(6);
		return s;
	}
	
	void operator=(const TypeId& id) {info = id.info;}
    bool operator==(TypeId const& other) const {return GetHashValue() == other.GetHashValue();}
    bool operator!=(TypeId const& other) const {return GetHashValue() != other.GetHashValue();}
    bool operator<(TypeId const& other) const {return GetHashValue() < other.GetHashValue();}
    
};

#endif
