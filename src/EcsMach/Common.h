#ifndef _EcsMech_Common_h_
#define _EcsMech_Common_h_


NAMESPACE_ECS_BEGIN


class SystemBase;

class Entity;
class ComponentBase;
class Pool;
class ComponentMap;
class Renderable;
class Transform2D;
class Renderable;
class Engine;
class EntityStore;
class RegistrySystem;
class Overlap;
class ToolSystemBase;
class ToolComponent;
class Geom2DComponent;
class DebugAudioGeneratorExt;
class PortaudioSinkComponent;
class StaticVolumeComponent;
class Viewable;
class Viewport;
class Transform;
class ExtComponent;
class PaintComponent;
class ComponentStore;

using EcsSystemParent		= RefParent1<Engine>;

using EntityParent			= RefParent1<Pool>;
using PoolParent			= RefParent2<EntityStore,Pool>;

using ComponentBaseRef		= Ref<ComponentBase,		RefParent1<Entity>>;
using ComponentRef			= Ref<ComponentBase,		RefParent1<Entity>>;
using Transform2DRef		= Ref<Transform2D,			RefParent1<Entity>>;
using RenderableRef			= Ref<Renderable,			RefParent1<Entity>>;
using OverlapRef			= Ref<Overlap,				RefParent1<Entity>>;
using PaintComponentRef		= Ref<PaintComponent,		RefParent1<Entity>>;
using ToolComponentRef		= Ref<ToolComponent,		RefParent1<Entity>>;
using Geom2DComponentRef			= Ref<Geom2DComponent,			RefParent1<Entity>>;
using ViewableRef			= Ref<Viewable,				RefParent1<Entity>>;
using ViewportRef			= Ref<Viewport,				RefParent1<Entity>>;
using TransformRef			= Ref<Transform,			RefParent1<Entity>>;
using EntityRef				= Ref<Entity,				EntityParent>;
using PoolRef				= Ref<Pool,					PoolParent>;
using EntityStoreRef		= Ref<EntityStore,			EcsSystemParent>;
using RegistrySystemRef		= Ref<RegistrySystem,		EcsSystemParent>;
using ComponentStoreRef		= Ref<ComponentStore,		EcsSystemParent>;
using ToolSystemBaseRef		= Ref<ToolSystemBase,		EcsSystemParent>;
using ExtComponentRef		= Ref<ExtComponent,			RefParent1<Entity>>;

using DebugAudioGeneratorExtRef	= Ref<DebugAudioGeneratorExt,		RefParent1<Entity>>;
using PortaudioSinkComponentRef			= Ref<PortaudioSinkComponent,			RefParent1<Entity>>;
using StaticVolumeComponentRef			= Ref<StaticVolumeComponent,			RefParent1<Entity>>;

using EntityVec				= RefLinkedList<		Entity,			EntityParent>;
using PoolVec				= RefLinkedList<		Pool,			PoolParent>;

using VAR					= EntityRef;
using EntityId				= int32;
using PoolId				= int32;

template <class T>
using RefT_Entity			= Ref<T,					RefParent1<Entity>>;

template <class T>
using RefT_Pool				= Ref<T,					RefParent1<Pool>>;

template <class T>
using RefT_Engine			= Ref<T,					EcsSystemParent>;









NAMESPACE_ECS_END

#endif
