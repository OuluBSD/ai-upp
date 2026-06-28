#include "Hearts.h"

#ifdef flagWIN32
#define CY win32_CY_
#define FAR win32_FAR_
#include <windows.h>
#undef CY
#undef FAR
#endif

using namespace Upp;

class HeartsWindow : public TopWindow {
public:
	HeartsCtrl ctrl;
	MenuBar menu;

	typedef HeartsWindow CLASSNAME;

	HeartsWindow() {
		Title("Hearts Royal 💎");
		SetRect(0, 0, 800, 600);
		SetMinSize(Size(600, 450));

		// Enable standard window controls
		MinimizeBox();
		MaximizeBox();
		Sizeable();

		// Add components
		Add(ctrl.SizePos());
		AddFrame(menu);

		// Initialize menu bar
		menu.Set(THISBACK(MainMenu));
	}

	void MainMenu(Bar& bar) {
		bar.Add("File", THISBACK(FileMenu));
		bar.Add("Game", THISBACK(GameMenu));
		bar.Add("Help", THISBACK(HelpMenu));
	}

	void FileMenu(Bar& bar) {
		bar.Add("New Game", THISBACK(NewGame));
		bar.Add("Exit", THISBACK(ExitGame));
	}

	void GameMenu(Bar& bar) {
		bar.Add("Difficulty", THISBACK(DifficultyMenu));
		bar.Add("High Scores", THISBACK(HighScores));
	}

	void DifficultyMenu(Bar& bar) {
		bar.Add("Easy", THISBACK(SetDifficultyEasy))
		   .Radio(ctrl.difficulty == HeartsCtrl::DIFFICULTY_EASY);
		bar.Add("Medium", THISBACK(SetDifficultyMedium))
		   .Radio(ctrl.difficulty == HeartsCtrl::DIFFICULTY_MEDIUM);
		bar.Add("Hard", THISBACK(SetDifficultyHard))
		   .Radio(ctrl.difficulty == HeartsCtrl::DIFFICULTY_HARD);
	}

	void HelpMenu(Bar& bar) {
		bar.Add("About", THISBACK(About));
	}

	void NewGame() {
		ctrl.StartGame();
	}

	void ExitGame() {
		Close();
	}

	void HighScores() {
		ctrl.ShowHighScores();
	}

	void SetDifficultyEasy() {
		ctrl.difficulty = HeartsCtrl::DIFFICULTY_EASY;
	}

	void SetDifficultyMedium() {
		ctrl.difficulty = HeartsCtrl::DIFFICULTY_MEDIUM;
	}

	void SetDifficultyHard() {
		ctrl.difficulty = HeartsCtrl::DIFFICULTY_HARD;
	}

	void About() {
		ctrl.ShowAbout();
	}
};

GUI_APP_MAIN {
#ifdef flagWIN32
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif

	StdLogSetup(LOG_COUT | LOG_FILE);
	Cout() << "\nHearts GUI Application starting...\n";
	
	HeartsWindow win;

	const Vector<String>& cmd = CommandLine();
	for (const String& arg : cmd) {
		if (arg == "--autoplay") {
			Cout() << "Autoplay mode enabled\n";
			win.ctrl.autoplay_enabled = true;
			win.ctrl.ScheduleAiStep(500);
		}
	}

	Cout() << "Entering win.Run() event loop...\n";
	win.Run();
	Cout() << "win.Run() exited.\n";
}
