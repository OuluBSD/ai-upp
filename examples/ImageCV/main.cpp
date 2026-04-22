#include <CtrlLib/CtrlLib.h>
#include <ComputerVision/ComputerVision.h>
#include <ComputerVision/TemplateMatchCtrl.h>

using namespace Upp;

class ImageCVWindow : public TopWindow {
public:
	typedef ImageCVWindow CLASSNAME;

	TemplateMatchCtrl drawer;

	ImageCVWindow() {
		Title("ImageCV - Template Matching");
		Sizeable().Zoomable();
		Add(drawer.SizePos());

		Image full  = StreamRaster::LoadFileAny(GetDataFile("full.jpg"));
		Image small = StreamRaster::LoadFileAny(GetDataFile("small.jpg"));
		drawer.SetInputs(full, small);
	}
};

GUI_APP_MAIN {
	ImageCVWindow().Run();
}
