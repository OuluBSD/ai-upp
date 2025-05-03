#ifndef _Eon_EntityStore_h_
#define _Eon_EntityStore_h_


namespace Ecs {


class EntityStore : public System<EntityStore> {
    Vector<Entity*>					destroy_list;
	PoolVec							root;
	
	Mutex							lock;
	
	enum {
		READ,
		WRITE
	};
	
	void InitRoot();
public:
	ECS_SYS_CTOR_(EntityStore) {InitRoot();}
	ECS_SYS_DEF_VISIT_(vis || root)
	
	PoolPtr GetRoot()	{return *root.begin();}
	PoolVec& GetRootVec()	{return root;}
	
	EntityPtr FindEntity(String path);
	void AddToDestroyList(Entity* e);
	
	
protected:
	void Update(double) override;
	bool Initialize() override;
	void Uninitialize() override;
	
	
};


}


#endif
