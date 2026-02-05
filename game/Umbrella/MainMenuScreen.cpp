#include "Umbrella.h"
#include "MainMenuScreen.h"
#include "MapEditor.h"

using namespace Upp;

MainMenuScreen::MainMenuScreen() {
	Title("Umbrella - Main Menu");
	Sizeable().Zoomable();
	SetRect(0, 0, 800, 600);

	// Title
	titleLabel.SetLabel("UMBRELLA GAME");
	titleLabel.SetFont(Arial(32).Bold());
	titleLabel.SetAlign(ALIGN_CENTER);
	Add(titleLabel.HSizePos(100, 100).TopPos(100, 50));

	// Play Button
	playButton.SetLabel("Play Game");
	playButton << [=] { ShowGameScreen(); };
	Add(playButton.HSizePos(250, 250).TopPos(250, 40));

	// Editor Button
	editorButton.SetLabel("Map Editor");
	editorButton << [=] { ShowEditorScreen(); };
	Add(editorButton.HSizePos(250, 250).TopPos(300, 40));

	// Settings Button
	settingsButton.SetLabel("Settings");
	settingsButton << [=] { ShowSettings(); };
	Add(settingsButton.HSizePos(250, 250).TopPos(350, 40));

	// Quit Button
	quitButton.SetLabel("Quit");
	quitButton << [=] { Break(); };
	Add(quitButton.HSizePos(250, 250).TopPos(400, 40));
}

void MainMenuScreen::ShowGameScreen() {
	// TODO: Create and show GameScreen
	PromptOK("Game screen not yet implemented!");
}

void MainMenuScreen::ShowEditorScreen() {
	Close();
	MapEditorApp().Run();
}

void MainMenuScreen::ShowSettings() {
	PromptOK("Settings screen not yet implemented!");
}
