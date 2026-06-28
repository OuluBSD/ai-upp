#ifdef flagMAIN
#include "StartWindow.h"
#include "ClientThread.h"
#include "AvatarManager.h"
#include <GameRules/EngineLog.h>
#include <EditorCommon/ConfigFile.h>
#include <EditorCommon/Tools.h>
#include "GameTable.h"
#include <Poker/LocalEngineFactory.h>
#include <GameRules/Game.h>
#include <GameRules/PlayerData.h>
#include <GameRules/HandInterface.h>
#include <GameRules/BeroInterface.h>
#include <GameRules/BoardInterface.h>
#include <GameRules/PlayerInterface.h>
#include "RunLocalGame.h"
#include "RemoteClient.h"
#include <cstdlib>

using namespace Upp;

NAMESPACE_UPP

namespace {
constexpr int LOCAL_GAME_DEFAULT_NUM_PLAYERS = 10;
constexpr int LOCAL_GAME_DEFAULT_START_CASH = 2000;
constexpr int LOCAL_GAME_DEFAULT_SPEED = 4;
}

END_UPP_NAMESPACE

#ifdef flagMAIN
GUI_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	
	for (int i = 0; i < args.GetCount(); i++) {
		if (args[i] == "--fastcrash") {
			Upp::InstallPanicMessageBox([](const char* title, const char* text) {
				UPP::Cerr() << "PANIC: " << title << ": " << text << "\n";
				std::_Exit(1);
			});
		}
	}
	
	if (args.GetCount() > 0 && (args[0] == "--help" || args[0] == "-h" || args[0] == "-help")) {
		Cout() << "Usage: CardEngine [options]\n"
		       << "Options:\n"
		       << "  --test-assets          Verify critical assets\n"
		       << "  --dump-render-image    Dump a snapshot of the testing project to tmp/pokerth_screenshot.png\n"
		       << "  --project <name>       Project name for --dump-render-image (default: testing)\n"
		       << "  --out <path>           Output path for --dump-render-image\n"
		       << "  --test-gameplay        Run a short gameplay simulation test\n"
		       << "  --local-game           Run a standard local game\n"
		       << "  --local-game-script    Run a scripted local game (use --help with this for more sub-options)\n"
		       << "  --connect <addr>       Connect to a remote server\n"
		       << "  --fastcrash            Crash immediately on fatal errors/assertions (no message box)\n";
		return;
	}
	class ConfigFile config(nullptr, false);
	AvatarManager avatarManager;
	auto engineLog = std::make_shared<EngineLog>(&config);

	if (args.GetCount() > 0 && args[0] == "--test-assets") {
		String dataDir = Tools::GetDataDir();
		if (DirectoryExists(dataDir)) {
			String testFile = AppendFileName(dataDir, "gfx/cards/default/back.png");
			if (FileExists(testFile)) {
				UPP::Cout() << "SUCCESS: Assets found and accessible.\n";
				Exit(0);
			} else {
				UPP::Cerr() << "ERROR: Critical asset missing: " << testFile << "\n";
				Exit(1);
			}
		} else {
			UPP::Cerr() << "ERROR: Data directory does not exist: " << dataDir << "\n";
			Exit(1);
		}
	}
	
	if (args.GetCount() > 0 &&
	    (args[0] == "--dump-render" || args[0] == "--dump-render-image" || args[0] == "--dump-render-normal")) {
		bool image_mode = args[0] != "--dump-render-normal";
		String out_path = AppendFileName(GetCurrentDirectory(), "tmp/pokerth_screenshot.png");
		String project_name = "testing";
		for (int i = 1; i < args.GetCount(); i++) {
			if (args[i] == "--out" && i + 1 < args.GetCount()) {
				out_path = args[++i];
			}
			else if (args[i] == "--project" && i + 1 < args.GetCount()) {
				project_name = args[++i];
			}
		}
		class ConfigFile config(nullptr, false);
		auto engineLog = std::make_shared<EngineLog>(&config);
		GameTable table;
		table.SetRect(0, 0, 1024, 648);
		table.Layout(); // Force layout without opening
		table.SetProjectContext(project_name, "texas-holdem");

		PlayerDataList pdList;
		pdList.push_back(std::make_shared<PlayerData>(0, 0, PLAYER_TYPE_HUMAN, PLAYER_RIGHTS_ADMIN, true));
		pdList.back()->SetName("Player");
		for (int i = 1; i < 6; i++) {
			pdList.push_back(std::make_shared<PlayerData>(i, i, PLAYER_TYPE_COMPUTER, PLAYER_RIGHTS_NORMAL, false));
			pdList.back()->SetName(Format("Computer %d", i));
		}
		GameData gData; gData.maxNumberOfPlayers = 6; gData.startMoney = 1000; gData.firstSmallBlind = 10; gData.guiSpeed = 10;
		StartData sData; sData.numberOfPlayers = 6; sData.startDealerPlayerId = 0;
		auto factory = std::make_shared<LocalEngineFactory>();
		auto game = std::make_shared<Game>(&table, factory, pdList, gData, sData, 1, engineLog.get(), &config);
		table.SetGame(game);
		game->initHand();
		game->startHand();

		// Just a small sleep to let internal state settle, no ProcessEvents
		Sleep(500);

		if (!table.DumpSnapshot(out_path, image_mode)) {
			Cerr() << "ERROR: failed to dump snapshot to " << out_path << "\n";
			std::_Exit(2);
		}
		Cout() << "SUCCESS: Snapshot dumped to " << out_path << " mode=" << (image_mode ? "image" : "normal") << "\n";
		Cout().Flush();
		// Dump-render is a snapshot utility path; force immediate exit to avoid
		// GUI teardown instability in X11 debug runs.
		std::_Exit(0);
	}
	
	if (args.GetCount() > 0 && args[0] == "--test-gameplay") {
		FileAppend testLog("test_gameplay.txt");
		struct StdoutGui : public GuiInterface {
			FileAppend& out;
			std::shared_ptr<Game> m_game;
			StdoutGui(FileAppend& o) : out(o) {}
			virtual bool isTestMode() const override { return true; }
			virtual void SetGame(std::shared_ptr<Game> game) override { m_game = game; }
			virtual void initGui(int speed) override {}
			virtual void refreshGameLabels(TexasRound state) const override {}
			virtual void nextRoundCleanGui() override {}
			virtual void logNewGameHandMsg(int gameID, int HandID) override { 
				out << "HAND START: " << HandID << "\n"; out.Flush();
			}
			virtual void flushLogAtGame(int gameID) override {}
			virtual void logNewBlindsSetsMsg(int sbSet, int bbSet, String sbName, String bbName) override {}
			virtual void flushLogAtHand() override {}
			virtual void dealHoleCards() override {}
			virtual void refreshPot() override {}
			virtual void refreshSet() override {}
			virtual void nextPlayerAnimation() override {
				PostCallback([this] {
					Sleep(100);
					if (m_game && m_game->getCurrentHand()) m_game->getCurrentHand()->getCurrentBeRo()->nextPlayer();
				});
			}
			virtual void flipHolecardsAllIn() override {}
			virtual void logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4, int card5) override {
				out << "BOARD: round=" << roundID << " cards=" << card1 << "," << card2 << "," << card3 << "," << card4 << "," << card5 << "\n";
				out.Flush();
			}
			virtual void refreshGroupbox(int playerID, int type) override {}
			virtual void preflopAnimation1() override { nextPlayerAnimation(); }
			virtual void flopAnimation1() override { nextPlayerAnimation(); }
			virtual void turnAnimation1() override { nextPlayerAnimation(); }
			virtual void riverAnimation1() override { nextPlayerAnimation(); }
			virtual void postRiverAnimation1() override { nextPlayerAnimation(); }
			virtual void logPlayerActionMsg(String playName, int action, int setValue) override {
				out << "ACTION: " << playName << " action=" << action << " value=" << setValue << "\n";
				out.Flush();
			}
			virtual void logFlipHoleCardsMsg(String playerName, int card1, int card2, int cardsValueInt, String showHas) override {}
			virtual void logWinningHandMsg(String playerName, String handName, int amount) override {
				out << "WINNER: " << playerName << " wins " << amount << " with " << handName << "\n"; out.Flush();
			}
			virtual void dealBeRoCards(TexasRound state) override {}
			virtual void beRoAnimation2(TexasRound state) override {}
			virtual void meInAction() override {}
			virtual void postRiverRunAnimation1() override {}
			virtual void refreshCash() override {}
			virtual void refreshAction(int playerID, int action) override {}
			virtual void SignalNetClientError(int errorId, int osErrorCode) override {}
		};
		
		StdoutGui gui(testLog);
		auto factory = std::make_shared<LocalEngineFactory>();
		PlayerDataList pdList;
		for (int i = 0; i < 10; i++) {
			pdList.push_back(std::make_shared<PlayerData>(i, i, i == 0 ? PLAYER_TYPE_HUMAN : PLAYER_TYPE_COMPUTER, PLAYER_RIGHTS_NORMAL, i == 0));
			pdList.back()->SetName(i == 0 ? "Human" : Format("Bot%d", i));
		}
		
		GameData gData; gData.maxNumberOfPlayers = 10; gData.startMoney = 2000; gData.firstSmallBlind = 10;
		StartData sData; sData.numberOfPlayers = 10; sData.startDealerPlayerId = 0;
		
		auto game = std::make_shared<Game>(&gui, factory, pdList, gData, sData, 1, engineLog.get(), &config);
		gui.SetGame(game);
		game->initHand();
		game->startHand();
		
		TimeStop ts;
		while(ts.Elapsed() < 5000) {
			UPP::Ctrl::Ctrl::ProcessEvents();
			Sleep(10);
			if (game->isGameOver()) break;
		}
		UPP::Cout() << "Test complete.\n";
		UPP::Cout().Flush();
		return;
	}

	if (args.GetCount() > 0 && args[0] == "--local-game") {
		RunLocalGame(LOCAL_GAME_DEFAULT_NUM_PLAYERS, LOCAL_GAME_DEFAULT_START_CASH, LOCAL_GAME_DEFAULT_SPEED, config, *engineLog);
		return;
	}
	
	if (args.GetCount() > 0 && args[0] == "--local-game-script") {
		int num_players = LOCAL_GAME_DEFAULT_NUM_PLAYERS;
		int start_cash = LOCAL_GAME_DEFAULT_START_CASH;
		int game_speed = LOCAL_GAME_DEFAULT_SPEED;
		int max_ticks = 500;
		int sleep_ms = 5;
		int seed = -1;
		bool verbose = false;
		bool headless = false;
		bool has_ticks = false;
		bool auto_human_action = false;
		bool no_wait_between_actions = false;
		bool has_auto_human_arg = false;
		bool has_no_wait_arg = false;
		String project_name = "default";
		String script_path;
		String dump_loop_state_json;
		for (int i = 1; i < args.GetCount(); i++) {
			String a = args[i];
			auto need = [&](const char* name)->String {
				if (i + 1 >= args.GetCount()) {
					Cerr() << "Missing value for " << name << "\n";
					return String();
				}
				return args[++i];
			};
			if (a == "--script") script_path = need("--script");
			else if (a == "--project") project_name = need("--project");
			else if (a == "--num-players") num_players = max(2, StrInt(need("--num-players")));
			else if (a == "--start-cash") start_cash = max(100, StrInt(need("--start-cash")));
			else if (a == "--game-speed") game_speed = max(1, StrInt(need("--game-speed")));
			else if (a == "--ticks") { max_ticks = max(1, StrInt(need("--ticks"))); has_ticks = true; }
			else if (a == "--sleep-ms") sleep_ms = max(0, StrInt(need("--sleep-ms")));
			else if (a == "--seed") seed = max(0, StrInt(need("--seed")));
			else if (a == "--dump-loop-state-json") dump_loop_state_json = need("--dump-loop-state-json");
			else if (a == "--verbose") verbose = true;
			else if (a == "--headless" || a == "--cli") headless = true;
			else if (a == "--auto-human") { auto_human_action = true; has_auto_human_arg = true; }
			else if (a == "--no-wait") { no_wait_between_actions = true; has_no_wait_arg = true; }
			else if (a == "--fastcrash") { /* handled globally */ }
			else {				Cerr() << "Unknown arg: " << a << "\n";
				Cerr() << "Usage: --local-game-script [--project name] [--script path] [--num-players N] [--start-cash N] [--game-speed N] [--ticks N] [--sleep-ms N] [--seed N] [--dump-loop-state-json path] [--verbose] [--headless|--cli] [--auto-human] [--no-wait]\n";
				Exit(1);
				return;
			}
		}
		if (headless) {
			if (!has_auto_human_arg) auto_human_action = true;
			if (!has_no_wait_arg) no_wait_between_actions = true;
		}
		if (!has_ticks && !headless)
			max_ticks = -1; // GUI mode runs until window is closed by user.
		int rc = RunLocalGameScripted(num_players, start_cash, game_speed, config, *engineLog,
		                              project_name, script_path, max_ticks, sleep_ms, seed, verbose, dump_loop_state_json, headless,
		                              auto_human_action, no_wait_between_actions);
		Exit(rc);
		return;
	}

	struct MainGui : public GuiInterface {
		std::shared_ptr<Game> m_game;
		StartWindow startWindow;
		GameTable table;

		MainGui() {}

		virtual void SetGame(std::shared_ptr<Game> game) override { m_game = game; table.SetGame(game); }
		virtual bool isTestMode() const override { return false; }

		virtual void initGui(int speed) override {}
		virtual void refreshGameLabels(TexasRound state) const override {}
		virtual void nextRoundCleanGui() override {}
		virtual void logNewGameHandMsg(int gameID, int HandID) override {}
		virtual void flushLogAtGame(int gameID) override {}
		virtual void logNewBlindsSetsMsg(int sbSet, int bbSet, String sbName, String bbName) override {}
		virtual void flushLogAtHand() override {}
		virtual void dealHoleCards() override { table.dealHoleCards(); }
		virtual void refreshPot() override { table.refreshPot(); }
		virtual void refreshSet() override { table.refreshSet(); }
		virtual void nextPlayerAnimation() override { table.nextPlayerAnimation(); }
		virtual void flipHolecardsAllIn() override { table.flipHolecardsAllIn(); }
		virtual void logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4, int card5) override {
			table.logDealBoardCardsMsg(roundID, card1, card2, card3, card4, card5);
		}
		virtual void refreshGroupbox(int playerID, int type) override { table.refreshGroupbox(playerID, type); }
		virtual void preflopAnimation1() override { table.preflopAnimation1(); }
		virtual void flopAnimation1() override { table.flopAnimation1(); }
		virtual void turnAnimation1() override { table.turnAnimation1(); }
		virtual void riverAnimation1() override { table.riverAnimation1(); }
		virtual void postRiverAnimation1() override { table.postRiverAnimation1(); }
		virtual void logPlayerActionMsg(String playName, int action, int setValue) override {
			table.logPlayerActionMsg(playName, action, setValue);
		}
		virtual void logFlipHoleCardsMsg(String playerName, int card1, int card2, int cardsValueInt, String showHas) override {
			table.logFlipHoleCardsMsg(playerName, card1, card2, cardsValueInt, showHas);
		}
		virtual void logWinningHandMsg(String playerName, String handName, int amount) override {
			table.logWinningHandMsg(playerName, handName, amount);
		}
		virtual void dealBeRoCards(TexasRound state) override { table.dealBeRoCards(state); }
		virtual void beRoAnimation2(TexasRound state) override { table.beRoAnimation2(state); }
		virtual void meInAction() override { table.meInAction(); }
		virtual void postRiverRunAnimation1() override { table.postRiverRunAnimation1(); }
		virtual void refreshCash() override { table.refreshCash(); }
		virtual void refreshAction(int playerID, int action) override { table.refreshAction(playerID, action); }
		virtual void SignalNetClientError(int errorId, int osErrorCode) override { table.SignalNetClientError(errorId, osErrorCode); }
	};

	        MainGui mainGui;

	        

	        RemoteClient remote;

	        for (int i = 0; i < args.GetCount(); i++) {

	                if (args[i] == "--connect" && i + 1 < args.GetCount()) {

	                        remote.WhenUpdate = [&mainGui](const ScreenGameState& s) {

	                                mainGui.table.SetRemoteState(s);

	                        };

	                        remote.Start(args[i + 1]);

	                        mainGui.startWindow.Hide();

	                        mainGui.table.OpenMain();

	                        break;

	                }

	        }

	

	        	        if (!mainGui.table.IsOpen()) {

	

	        	                mainGui.startWindow.Init(config, engineLog);

	

	        	                mainGui.startWindow.Run();

	

	        	        } else {

	                Ctrl::EventLoop();

	}
}
#endif

#endif
