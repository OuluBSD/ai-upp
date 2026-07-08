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

static String ResolveStartWindowFormPath()
{
	Vector<String> candidates;
	candidates.Add(AppendFileName(GetCurrentDirectory(), "game/TexasHoldem/StartWindow.form"));
	candidates.Add(AppendFileName(GetFileDirectory(GetExeFilePath()), "../game/TexasHoldem/StartWindow.form"));
	candidates.Add(ConfigFile("StartWindow.form"));
	for (const String& candidate : candidates) {
		if (FileExists(candidate))
			return candidate;
	}
	return ConfigFile("StartWindow.form");
}

StartWindow::StartWindow()
{
	Sizeable().Zoomable();
	Title("PKR - Texas Hold'em");

	String form_path = ResolveStartWindowFormPath();
	if (!ui.Load(form_path))
		ASSERT_(false, "Could not load StartWindow.form");
	ui.SetScaleMode(Form::SCALE_FIT);
	if (!ui.Layout("Default"))
		ASSERT_(false, "StartWindow.form missing Default layout");
	Add(ui.SizePos());
	ui.SignalHandler = callback(this, &StartWindow::HandleUiSignal);

	providerChoice = dynamic_cast<DropList*>(ui.GetCtrl("providerChoice"));
	ASSERT(providerChoice);
	FillProviders();

	m_config = nullptr;
	UpdateTitleFromProvider();
}

void StartWindow::FillProviders()
{
	ASSERT(providerChoice);
	providerChoice->Clear();
	providerChoice->Add("PokerTH", t_("PokerTH"));
	providerChoice->Add("Classic", t_("Classic"));
	providerChoice->Add("Minimal", t_("Minimal"));
	providerChoice->SetData("PokerTH");
	m_provider = AsString(providerChoice->GetData());
}

String StartWindow::SelectedProvider() const
{
	if (providerChoice) {
		String provider = AsString(providerChoice->GetData());
		if (!provider.IsEmpty())
			return provider;
	}
	return m_provider.IsEmpty() ? String("PokerTH") : m_provider;
}

void StartWindow::UpdateTitleFromProvider()
{
	String provider = SelectedProvider();
	m_provider = provider;
	Title(Format("PKR - Texas Hold'em [%s]", provider));
}

void StartWindow::HandleUiSignal(const String& path, const String& op, const String& action)
{
	if (path == "providerChoice") {
		UpdateTitleFromProvider();
		return;
	}
	if (op != "OnAction")
		return;
	if (action == "LocalGame")
		OnLocalGame();
	else if (action == "InternetGame")
		OnInternetGame();
	else if (action == "NetworkGame")
		OnNetworkCreate();
	else if (action == "JoinNetwork")
		OnNetworkJoin();
	else if (action == "Settings")
		OnSettings();
	else if (action == "Log")
		OnLog();
	else if (action == "Info")
		OnInfo();
	else if (action == "Quit")
		OnQuit();
}

extern void RunLocalGame(int numPlayers, int startCash, int gameSpeed, class ConfigFile& config, EngineLog& engineLog);

void StartWindow::OnLocalGame()
{
	UpdateTitleFromProvider();
	NewGameWindow dlg;
	if (m_config) dlg.Init(*m_config);
	if (dlg.Run() != IDOK)
		return;

	if (m_engineLog && m_config)
		RunLocalGame((int)dlg.numPlayers.GetData(), (int)dlg.startCash.GetData(), (int)dlg.gameSpeed.GetData(), *m_config, *m_engineLog);
}

void StartWindow::OnInternetGame()
{
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
	PromptOK(Format("PKR U++ Port v0.1\nProvider: %s\n1:1 Parity with PokerTH project.", SelectedProvider()));
}

void StartWindow::OnSettings()
{
	SettingsWindow dlg;
	if (m_config) dlg.Init(*m_config);
	dlg.Run();
}

void StartWindow::OnLog()
{
	PromptOK(t_("Log window not implemented yet."));
}

void StartWindow::OnQuit()
{
	Break();
}

END_UPP_NAMESPACE
