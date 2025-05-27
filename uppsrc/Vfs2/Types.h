#ifndef _Vfs2_Types_h_
#define _Vfs2_Types_h_

#define DATASET_ITEM(type, name, desc) struct type;
EXT_LIST
#undef DATASET_ITEM
struct SrcTextData;

// Deprecated
// DatasetPtrs is used only classes with "public SolverBase"
// The SolverBase is also being shifted out.
struct DatasetPtrs {
	#define DATASET_ITEM(type, name, desc) Ptr<type> name;
	DATASET_LIST
	#undef DATASET_ITEM
	
	bool editable_biography = false;
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p);
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
	void Clear();
	
	// Prevent forward declaration errors with "class _"
	template <class T, class _> struct Getter {static T& Get(DatasetPtrs& d);};
	template <class T> T& Get() {return Getter<T,int>::Get(*this);}
};

// "int I" is required because the operation "p.name = o;" must be delayed after full
// declaration, which has been done when VfsValueExtFactory::Register is being called.
template <class T, int I>
struct DatasetAssigner {
	static void Set(DatasetPtrs& p, T* o);
};

#define DATASET_ITEM(type, name, desc) \
template <int I> struct DatasetAssigner<type,I> {\
	static void Set(DatasetPtrs& p, type* o) {p.name = o;}\
};
EXT_LIST
#undef DATASET_ITEM

#define DATASET_ITEM(type, name, desc) \
template <class _> struct DatasetPtrs::Getter<type,_> {static type& Get(DatasetPtrs& p) {return *p.name;}};
EXT_LIST
#undef DATASET_ITEM

void FillDataset(DatasetPtrs& p, VfsValue& n, Component* this_comp);

#endif
