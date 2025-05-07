#ifndef _EonLib_EcsPhysics_h_
#define _EonLib_EcsPhysics_h_


#include <EcsLocal/EcsLocal.h>
#include <Physics/Physics.h>

#include "TPrefab.h"


NAMESPACE_UPP


#define FYS_ECS_CLS_LIST(f) \
	FYS_CLS(System, f)

#define FYS_CLS(x, f) struct f##x;
	#define FYS_SYS(x) FYS_ECS_CLS_LIST(x)
		FYS_FYSSYS_LIST
	#undef FYS_SYS
#undef FYS_CLS


#define FYS_CLS(x, f) struct f##x : x##T<f##Fys> {//RTTI_DECL1(f##x,Base); f##x(Engine& e) : Base(e){} void Visit(Vis& vis) override {VIS_THIS(Base)}};
	#define FYS_SYS(x) FYS_ECS_CLS_LIST(x)
		FYS_FYSSYS_LIST
	#undef FYS_SYS
#undef FYS_CLS


END_UPP_NAMESPACE


#endif
