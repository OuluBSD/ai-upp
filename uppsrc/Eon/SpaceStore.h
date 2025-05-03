#ifndef _Eon_SpaceStore_h_
#define _Eon_SpaceStore_h_


class SpaceStore : public System<SpaceStore> {
	SpaceVec		root;
	
	Mutex			lock;
	
	enum {
		READ,
		WRITE
	};
	
	void InitRoot();
public:
	SYS_CTOR_(SpaceStore) {InitRoot();}
	SYS_DEF_VISIT_(vis || root)
	
	SpacePtr GetRoot() {
		if (root.IsEmpty())
			return SpacePtr();
		Space& l = root[0];
		return &l;
	}
	Space* GetRootPtr() {
		if (root.IsEmpty())
			return 0;
		Space& l = root[0];
		return &l;
	}
	SpaceVec& GetRootVec()	{return root;}
	
	
	static ParallelTypeCls::Type GetSerialType() {return ParallelTypeCls::LOOP_STORE;}
	
protected:
	void Update(double) override;
	bool Initialize() override;
	void Stop() override;
	void Uninitialize() override;
	
	
};

using SpaceStorePtr = Ptr<SpaceStore>;


#endif
