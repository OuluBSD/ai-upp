#ifndef _Umbrella_MainMenuScreen_h_
#define _Umbrella_MainMenuScreen_h_

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

class MainMenuScreen : public TopWindow {
private:
	Label titleLabel;
	Button playButton;
	Button editorButton;
	Button settingsButton;
	Button quitButton;

public:
	MainMenuScreen();

	void ShowGameScreen();
	void ShowEditorScreen();
	void ShowSettings();
};

#endif
