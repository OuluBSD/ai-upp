#ifndef _Eon_Pool_h_
#define _Eon_Pool_h_


namespace Ecs {


class Pool : public VfsValueExt
{
	//using PoolVec = Array<Pool>;
	
	//Engine*				machine = 0;
	BitField<int>		freeze_bits;
	PoolId				id;
	//PoolVec				pools;
	
protected:
	friend class EntityStore;
	
	void SetId(PoolId i) {id = i;}
	
public:
	using PoolPtr = Ptr<Pool>;
	//using RScope		= RefScopeEnabler<Pool, EntityStore, PoolParent>;
	static PoolId GetNextId();
	CLASSTYPE(Pool)
	Pool(VfsValue& n);
	~Pool();
	
	typedef enum {
		BIT_OVERLAP,
		BIT_TRANSFORM,
	} Bit;
	
	void Visit(Vis& vis) override;
	
	PoolId GetId() const {return id;}
	
	void FreezeTransform()			{freeze_bits.Set(BIT_TRANSFORM, true);}
	void FreezeOverlap()			{freeze_bits.Set(BIT_OVERLAP, true);}
	bool IsFrozenTransform() const	{return freeze_bits.Is(BIT_TRANSFORM);}
	bool IsFrozenOverlap() const	{return freeze_bits.Is(BIT_OVERLAP);}
	
	//void				ReverseEntities();
	void				Clear();
	void				UnlinkDeep();
	void				UnrefDeep();
	void				UninitializeComponentsDeep();
	void				ClearComponentsDeep();
	void				ClearDeep();
	void				PruneFromContainer();
	void				Dump();
	String				GetTreeString(int indent=0);
	
	Pool*				GetParent() const;
	Engine&				GetEngine();
	bool				HasEntities() const;// {return !objects.IsEmpty();}
	//bool				HasPools() const {return !pools.IsEmpty();}
	
	void				Initialize(Entity& e, String prefab="Custom");
	EntityPtr			CreateEmpty(String id);
	EntityPtr			GetAddEmpty(String name);
	EntityPtr			Clone(const Entity& e);
	EntityPtr			RealizeEntityPath(const Vector<String>& path);
	
	template<typename PrefabT>
	EntityPtr Create() {
		static_assert(RTupleAllComponents<typename PrefabT::Components>::value, "Prefab should have a list of Components");
		
		Entity& e = val.add<Ecs::Entity>();
		//e.SetParent(this);
		e.SetId(GetNextId());
		PrefabT::Make(e);
		Initialize(e, PrefabT::GetComponentNames());
		
		return &e;
	}
	
	template<typename... ComponentTs>
	Vector<Tuple<EntityPtr,ComponentTs*...>> GetComponentsWithEntity() {
		static_assert(sizeof...(ComponentTs) > 0, "Need at least one component");
		static_assert(AllComponents<ComponentTs...>::value, "Ts should all derive from Component");
		
		Vector<Tuple<EntityPtr,ComponentTs*...>> components;
		auto ents = node.FindAll<Ecs::Entity>();
		for (auto& ent : ents) {
			auto requested_components = ent->TryGetComponents<ComponentTs...>();
			
			if (AllValidComponents(requested_components)) {
				Tuple<EntityPtr, ComponentTs*...> t(*ent, requested_components);
				components.Add(t);
			}
		}
		
		return components;
	}
	
	template<typename... ComponentTs>
	Vector<Tuple<ComponentTs*...>> GetComponents() {
		static_assert(sizeof...(ComponentTs) > 0, "Need at least one component");
		static_assert(AllComponents<ComponentTs...>::value, "Ts should all derive from Component");
		
		Vector<Tuple<ComponentTs*...>> components;
		
		auto ents = node.FindAll<Ecs::Entity>();
		for (auto& ent : ents) {
			auto requested_components = ent->TryGetComponents<ComponentTs...>();
			
			if (AllValidComponents(requested_components)) {
				components.Add(requested_components);
			}
		}
		
		return components;
	}
	
	
	void RemoveEntity(Entity* e);
	
	template <class T>
	EntityPtr FindEntity(T* component) {
		if (!component)
			return EntityPtr();
		auto ents = node.FindAll<Ecs::Entity>();
		for (auto& ent : ents) {
			T* t = ent->Find<T>();
			if (t == component)
				return ent;
		}
		return EntityPtr();
	}
	
	EntityPtr FindEntityByName(String name);
	
	template<typename Tuple>
	bool AllValidComponents(const Tuple& components) {
		bool all_valid_components = true;
		components.ForEach([&](auto& component) {
			all_valid_components &= component && !component->IsDestroyed() && component->IsEnabled();
		});
		return all_valid_components;
	}
	
	//EntityVec& GetEntities() {return objects;}
	//PoolVec& GetPools() {return pools;}
	
	PoolPtr FindPool(String name);
	PoolPtr AddPool(String name);
	PoolPtr GetAddPool(String name);
	//EntityVec::Iterator			begin()			{return objects.begin();}
	//EntityVec::Iterator			end()			{return objects.end();}
	//PoolVec::Iterator			BeginPool()		{return pools.begin();}
	
	ComponentBasePtr RealizeComponentPath(const Vector<String>& path);
	
	/*void Visit(Vis& vis) {
		vis || objects || pools;
	}*/
	
};

//using PoolVec = Array<Pool>;
using PoolPtr = Ptr<Pool>;

class PoolHashVisitor : public Vis {
	CombineHash ch;
	
	//bool OnEntry(const RTTI& type, TypeCls derived, const char* derived_name, void* mem, LockedScopeRefCounter* ref) override;
public:
	
	operator hash_t() const {return ch;}
	
};



template<typename T>
T* Entity::FindNearestEntityWith() {
	T* c = Find<T>();
	if (!c) {
		Pool* p = &GetPool();
		while (p && !c) {
			auto ents = p->node.FindAll<Ecs::Entity>();
			for (Entity* e : ents) {
				c = e->Find<T>();
				if (c) break;
			}
			p = p->GetParent();
		}
	}
	return c;
}

#if 0
template <>
inline void ComponentBase::EtherizeRef(Stream& e, EntityPtr& ref) {
	thread_local static Vector<String> path;
	bool has_ref = !ref.IsEmpty();
	e % has_ref;
	if (has_ref) {
		if (e.IsLoading()) {
			e % path;
			ASSERT_(path.GetCount() >= 1, "Entity path is required");
			if (path.GetCount() < 1) return;
			Pool& root = ref->GetRoot();
			ref = root.RealizeEntityPath(path);
			ASSERT_(ref, "Couldn't get etherized entity from path");
		}
	}
	else {
		ref->GetEntityPath(path);
		e % path;
	}
}

template <class T>
void ComponentBase::EtherizeRef(Stream& e, Ptr<T>& ref) {
	thread_local static Vector<String> path;
	EntityPtr ent = GetEntity();
	bool has_ref = !ref.IsEmpty();
	e % has_ref;
	if (has_ref) {
		if (e.IsLoading()) {
			e % path;
			ASSERT_(path.GetCount() >= 2, "Entity and Component paths is required");
			if (path.GetCount() < 2) return;
			Pool& root = ent->GetRoot();
			ComponentBasePtr comp = root.RealizeComponentPath(path);
			ASSERT_(comp, "Component path couldn't be realized");
			ref = comp;
			ASSERT_(ref, "Couldn't get etherized component from path");
		}
		else {
			ref->GetComponentPath(path);
			e % path;
		}
	}
	else {
		if (e.IsStoring())
			ref.Clear();
	}
}

template <class T>
void ComponentBase::EtherizeRefContainer(Stream& e, T& cont) {
	int d = cont.GetCount();
	e % d;
	cont.SetCount(d);
	for (auto& ref : cont) {
		EtherizeRef(e, ref);
	}
}
#endif

}

#endif
