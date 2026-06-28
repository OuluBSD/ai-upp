#include "StartWindow.h"
#include "GameTable.h"
#include "LobbyWindow.h"
#include "SettingsWindow.h"
#include "NewGameWindow.h"
#include <Poker/LocalEngineFactory.h>
#include <GameRules/EngineLog.h>
#include <GameRules/Game.h>
#include <GameRules/PlayerData.h>
#include "ServerManager.h"
#include <EditorCommon/ConfigFile.h>

NAMESPACE_UPP

StartWindow::StartWindow()
{
	CtrlLayout(*this);
	Title("PKR - Texas Hold'em");
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));

	btnLocalGame << [this] { OnLocalGame(); };
	btnInternetGame << [this] { OnInternetGame(); };
	btnNetworkGame << [this] { OnNetworkJoin(); };
	btnSettings << [this] { OnSettings(); };
	btnLog << [this] { OnLog(); };
	btnQuit << [this] { OnQuit(); };
	
	m_config = nullptr;
}

void StartWindow::MainMenu(Bar& bar)
{
	bar.Add(t_("App"), THISBACK(AppMenu));
	bar.Add(t_("Settings"), THISBACK(SettingsMenu));
}

void StartWindow::AppMenu(Bar& bar)
{
	bar.Add(t_("New local game"), THISBACK(OnLocalGame));
	bar.Add(t_("Internet game"), THISBACK(OnInternetGame));
	bar.Add(t_("Network game"), THISBACK(NetworkSubMenu));
	bar.Separator();
	bar.Add(t_("Info"), THISBACK(OnInfo));
	bar.Add(t_("Exit"), THISBACK(OnQuit));
}

void StartWindow::NetworkSubMenu(Bar& bar)
{
	bar.Add(t_("Create"), THISBACK(OnNetworkCreate));
	bar.Add(t_("Join"), THISBACK(OnNetworkJoin));
}

void StartWindow::SettingsMenu(Bar& bar)
{
	bar.Add(t_("Program Settings"), THISBACK(OnSettings));
}

extern void RunLocalGame(int numPlayers, int startCash, int gameSpeed, class ConfigFile& config, EngineLog& engineLog);

void StartWindow::OnLocalGame()
{
	NewGameWindow dlg;
	if (m_config) dlg.Init(*m_config);
	if (dlg.Run() != IDOK) return;

	if (m_engineLog && m_config) {
		RunLocalGame((int)dlg.numPlayers.GetData(), (int)dlg.startCash.GetData(), (int)dlg.gameSpeed.GetData(), *m_config, *m_engineLog);
	}
}

void StartWindow::OnInternetGame()
{
	// TODO: Internet game dialog
	LobbyWindow().Run();
}

void StartWindow::OnNetworkCreate()
{
	// TODO: Create network game
}

void StartWindow::OnNetworkJoin()
{
	LobbyWindow().Run();
}

void StartWindow::OnInfo()
{
	PromptOK(t_("PKR U++ Port v0.1\n1:1 Parity with PokerTH project."));
}

void StartWindow::OnSettings()
{
	SettingsWindow dlg;
	if (m_config) dlg.Init(*m_config);
	dlg.Run();
}

void StartWindow::OnLog()
{
	// TODO: Show logs
	PromptOK(t_("Log window not implemented yet."));
}

void StartWindow::OnQuit()
{
	Break();
}

END_UPP_NAMESPACE
