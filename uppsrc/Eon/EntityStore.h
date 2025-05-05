#ifndef _Eon_EntityStore_h_
#define _Eon_EntityStore_h_


namespace Ecs {

// TODO remove
class EntityStore : public System<EntityStore> {
    Vector<Entity*>					destroy_list; // not needed
	//PoolVec							root;
	
	Mutex							lock;
	
	enum {
		READ,
		WRITE
	};
	
	void InitRoot();
public:
	ECS_SYS_CTOR_(EntityStore) {InitRoot();}
	ECS_SYS_DEF_VISIT
	
	PoolPtr GetRoot();//	{return &root[0];}
	//PoolVec& GetRootVec()	{return root;}
	
	EntityPtr FindEntity(String path);
	void AddToDestroyList(Entity* e);
	
	
protected:
	void Update(double) override;
	bool Initialize() override;
	void Uninitialize() override;
	
	
};


}


#endif
