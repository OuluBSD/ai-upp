#ifndef _SingerTrainer_VocalPlotter_h_
#define _SingerTrainer_VocalPlotter_h_

#include <CtrlLib/CtrlLib.h>

namespace Upp {

class ExerciseEngine;

class VocalPlotter : public Ctrl {
	ExerciseEngine* engine = nullptr;

public:
	void SetEngine(ExerciseEngine& e) { engine = &e; }
	virtual void Paint(Draw& w) override;
};

}

#endif
