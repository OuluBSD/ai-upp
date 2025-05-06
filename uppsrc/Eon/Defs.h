#ifndef _Eon_Defs_h_
#define _Eon_Defs_h_

// If "debug realtime errors" is defined (with DEBUG_RT)
#ifdef flagDEBUG_RT
	#define DEBUG_RT_PIPE 1
	#ifdef flagDEBUG
		#define RTLOG(x) DLOG(x)
	#else
		#define RTLOG(x) RLOG(x)
	#endif
#else
	#define DEBUG_RT_PIPE 0
	#define RTLOG(x) {}
#endif

#ifdef flagDEBUG
	#define DEFAULT_AUDIO_QUEUE_SIZE	10
#else
	#define DEFAULT_AUDIO_QUEUE_SIZE	4
#endif

#ifndef CXX2A_STATIC_ASSERT
	#if defined flagMSC && !defined flagUWP
		#define CXX2A_STATIC_ASSERT(x, y) static_assert(x, y)
	#else
		#define CXX2A_STATIC_ASSERT(x, y)
	#endif
#endif

#define ECS_SYS_CTOR_(x) \
	CLASSTYPE(x) \
	x(MetaNode& e) : System<x>(e)

#define ECS_SYS_CTOR(x) \
	CLASSTYPE(x) \
	x(MetaNode& m) : System<x>(m) {}
#define ECS_SYS_CTOR_DERIVED(x, derived_from) \
	CLASSTYPE(x) \
	x(MetaNode& m) : derived_from(m) {}
#define ECS_SYS_DEF_VISIT void Visit(Vis& vis) override {}
#define ECS_SYS_DEF_VISIT_(x) void Visit(Vis& vis) override {x;}
#define PREFAB_BEGIN(x) \
struct x##_ : RTTIBase {RTTI_DECL0(x##_)}; \
\
struct x : \
	x##_, \
	TS::ECS::EntityPrefab<

#define PREFAB_END \
> { \
	 \
    static Components Make(TS::ECS::Entity& e) \
    { \
        auto components = EntityPrefab::Make(e); \
		return components; \
    } \
};

#define COMP_DEF_VISIT void Visit(Vis& vis) override {}
#define COMP_DEF_VISIT_(x) void Visit(Vis& vis) override {x;}

#define COPY_PANIC(T) void operator=(const T& t) {Panic("Can't copy " #T);}

using EntityId				= int;
using PoolId				= int;

#endif
