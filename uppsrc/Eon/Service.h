#ifndef _Eon_Service_h_
#define _Eon_Service_h_


class EcsService : public DaemonService {
	EnetServiceServer* server = 0;
	Ecs::EntityPtr bound_entity;
	
	Ecs::EntityPtr ResolveEntity(Ecs::PoolRef& root, String path);
	
public:
	// Remote connection
	//GeomSerializer write, read;
	
	void ReceiveGeoms(Ether& in, Ether& out);
	void SendEngine(Ether& in, Ether& out);
	
public:
	typedef EcsService CLASSNAME;
	
	EcsService();
	bool Init(String name) override;
	void Update() override;
	void Stop() override;
	void Deinit() override;
	
};


#endif
