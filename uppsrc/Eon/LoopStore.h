#ifndef _Eon_LoopStore_h_
#define _Eon_LoopStore_h_


class LoopStore : public System<LoopStore> {
	//LoopVec				root;
	
	Mutex				lock;
	
	enum {
		READ,
		WRITE
	};
	
	void InitRoot();
public:
	SYS_CTOR_(LoopStore) {InitRoot();}
	SYS_DEF_VISIT
	
	LoopPtr GetRoot(); /*{
		if (root.IsEmpty())
			return LoopPtr();
		Loop& l = root[0];
		return &l;
	}*/
	//LoopVec& GetRootVec()	{return root;}
	
	
	static ParallelTypeCls::Type GetSerialType() {return ParallelTypeCls::LOOP_STORE;}
	
protected:
	void Update(double) override;
	bool Initialize() override;
	void Uninitialize() override;
	
	
};

using LoopStorePtr = Ptr<LoopStore>;


#endif
