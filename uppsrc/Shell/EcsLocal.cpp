#include "EcsShell.h"

NAMESPACE_UPP



/*void BindEcsEventsBase(Serial::EcsEventsBase* b) {
	Ecs::GetActiveEngine().Get<Ecs::EventSystem>()->Attach(b);
}*/

/*template <class Gfx>
void BindGfxBuffer(String id, Parallel::BufferT<Gfx>* b) {
	TODO
}*/

void BindEcsToSerial() {
	
	
	//Serial::EcsEventsBase::WhenInitialize << callback(BindEcsEventsBase);
	
	/*#ifdef flagSDL2
	BufferT<SdlSwGfx>::WhenLinkInit << callback(BindGfxBuffer<SdlSwGfx>);
	#ifdef flagOGL
	BufferT<SdlOglGfx>::WhenLinkInit << callback(BindGfxBuffer<SdlOglGfx>);
	#endif
	#endif*/
}



END_UPP_NAMESPACE
