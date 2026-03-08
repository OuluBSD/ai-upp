#ifndef _GuboDemo_GuboDemo_h_
#define _GuboDemo_GuboDemo_h_

#include <GuboLib/GuboLib.h>
#include <Draw/Cuboid/Cuboid.h>
#include <CtrlLib/CtrlLib.h>

NAMESPACE_UPP

class GuboDemo : public TopGubo {
	TimeStop ts;
	float animation_speed = 3.0;

public:
	typedef GuboDemo CLASSNAME;
	GuboDemo();
	void Paint(Draw3& d) override;
};

END_UPP_NAMESPACE

#endif