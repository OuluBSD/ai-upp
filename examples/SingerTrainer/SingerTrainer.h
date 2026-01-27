#ifndef _SingerTrainer_SingerTrainer_h_
#define _SingerTrainer_SingerTrainer_h_

#include <Core/Core.h>
#include <ByteVM/ByteVM.h>

#ifdef flagGUI
#include <CtrlLib/CtrlLib.h>
#include "Scripting.h"
#include "ExerciseEngine.h"
#include "VocalPlotter.h"
#endif

namespace Upp {

enum VocalMode {
	FRY,
	MODAL,
	COMPRESSED,
	HEAD,
	FALSETTO,
	SUBHARMONIC,
	DISTORTION
};

#ifdef flagGUI

class SingerTrainer : public TopWindow {
public:
	typedef SingerTrainer CLASSNAME;
	SingerTrainer();
	~SingerTrainer();

	void Initialize();
	void UninitializeDeep();
	void Data();

private:
	Splitter main_splitter;
	Splitter right_splitter;
	Splitter bottom_splitter;
	
	ArrayCtrl  history;
	VocalPlotter draw_area;
	ParentCtrl stats_area;
	Label      stats_label;
	Button     start_button;
	ProgressIndicator accuracy_meter;
	SliderCtrl user_input_slider;
	DocEdit    notes;

	TimeCallback tc;

	GuiBridge scripting;
	ExerciseEngine engine;
};

#endif

}

#endif
