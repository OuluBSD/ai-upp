#include <Painter/Painter.h>
#include <EditorCommon/EditorCommon.h>
#include <GameRules/GameRules.h>
#include <Poker/Poker.h>
using namespace Upp;

#include <plugin/png/png.h>

class DummyCardClassifier {
public:
	bool Load(const String&, const String&) { return true; }
};

class RecognitionEngine {
public:
	RuleManager rule_mgr;
	DummyCardClassifier card_clf;
	void SetAnchor(const Image& img) {}
	GameState ProcessGpu(GpuPreprocessEngine& gpu) { return GameState(); }
};


class HeadlessGuiInterface : public GuiInterface {
	std::shared_ptr<Game> game;
	std::vector<std::function<void()>> callbacks;
public:
	bool turn_active = false;
	virtual void SetGame(std::shared_ptr<Game> g) override { game = g; }
	
	void RunCallbacks() {
		std::vector<std::function<void()>> q = std::move(callbacks);
		callbacks.clear();
		for (auto& c : q) c();
	}

	virtual void initGui(int speed) override {}
	virtual void refreshGameLabels(TexasRound state) const override {}
	virtual void nextRoundCleanGui() override {}
	virtual void logNewGameHandMsg(int gameID, int HandID) override {}
	virtual void flushLogAtGame(int gameID) override {}
	virtual void logNewBlindsSetsMsg(int sbSet, int bbSet, String sbName, String bbName) override {}
	virtual void flushLogAtHand() override {}
	virtual void dealHoleCards() override {}
	virtual void refreshPot() override {}
	virtual void refreshSet() override {}
	virtual void nextPlayerAnimation() override {
		turn_active = true;
		callbacks.push_back([=] {
			if (game && game->getCurrentHand() && game->getCurrentHand()->getCurrentBeRo()) {
				auto bero = game->getCurrentHand()->getCurrentBeRo();
				int tid = bero->getCurrentPlayersTurnId();
				if (tid >= 0) {
					auto p = game->getPlayerByUniqueId((unsigned)tid);
					if (p && p->getMyType() == PLAYER_TYPE_COMPUTER) {
						bero->nextPlayer();
					}
				}
			}
		});
	}
	virtual void flipHolecardsAllIn() override {}
	virtual void logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4 = -1, int card5 = -1) override {}
	virtual void refreshGroupbox(int playerID, int type) override {}
	virtual void preflopAnimation1() override {}
	virtual void flopAnimation1() override {}
	virtual void turnAnimation1() override {}
	virtual void riverAnimation1() override {}
	virtual void postRiverAnimation1() override {}
	virtual void logPlayerActionMsg(String playName, int action, int setValue) override {}
	virtual void logFlipHoleCardsMsg(String playerName, int card1, int card2, int cardsValueInt = -1, String showHas = "shows") override {}
	virtual void logWinningHandMsg(String playerName, String handName, int amount) override {}
	virtual void dealBeRoCards(TexasRound state) override {}
	virtual void beRoAnimation2(TexasRound state) override {}
	virtual void meInAction() override {
		Cout() << "DEBUG: HERO TURN detected by engine.\n";
	}
	virtual void postRiverRunAnimation1() override {}
	virtual void refreshCash() override {}
	virtual void refreshAction(int playerID, int action) override {}
	virtual void SignalNetClientError(int errorId, int osErrorCode) override {}
};

void HeadlessLog(String s) {
	Cout() << "SCRIPT: " << s << "\n";
}

int RunHeadlessCardEngine(const ScreenGameCliOptions& opt) {
	Cout() << "--- Starting Headless TexasHoldem Engine ---" << "\n";
	
	auto engineLog = std::make_unique<EngineLog>(nullptr);
	auto gui = std::make_shared<HeadlessGuiInterface>();
	
	PlayerDataList pdList;
	pdList.push_back(std::make_shared<PlayerData>(0, 0, PLAYER_TYPE_HUMAN, PLAYER_RIGHTS_NORMAL, false));
	pdList.back()->SetName("Hero");
	
	for (int i = 1; i < 6; i++) {
		pdList.push_back(std::make_shared<PlayerData>(i, i, PLAYER_TYPE_COMPUTER, PLAYER_RIGHTS_NORMAL, false));
		pdList.back()->SetName(Format("Bot %d", i));
	}
	
	GameData gData; gData.maxNumberOfPlayers = 6; gData.startMoney = 1000; gData.firstSmallBlind = 10; gData.guiSpeed = 100;
	StartData sData; sData.numberOfPlayers = 6; sData.startDealerPlayerId = 0;
	auto factory = std::make_shared<LocalEngineFactory>();
	auto game = std::make_shared<Game>(gui.get(), factory, pdList, gData, sData, 1, engineLog.get(), nullptr);
	gui->SetGame(game);
	
	game->initHand();
	game->startHand();
	gui->turn_active = true;	
	String project = opt.project_name;
	if (project.IsEmpty()) project = "default";
	
	GpuPreprocessEngine gpu;
	RecognitionEngine recognition;
	bool recognition_initialized = false;
	
	GameScript gs;
	gs.SetLogCallback(callback(HeadlessLog));
	gs.SetBruteforceFinder([&](const String& rule_name, const Image& img, const Rect* search, Rect& out, double& score) {
		int q = recognition.rule_mgr.FindRule(rule_name);
		if (q >= 0) {
			out = recognition.rule_mgr.GetRule(q).rect;
			score = 1.0;
			Cout() << "DEBUG: BruteforceFinder found rule=" << rule_name << " rect=" << out << "\n";
			return true;
		}
		Cout() << "DEBUG: BruteforceFinder MISS rule=" << rule_name << "\n";
		return false;
	});
	
	String common_code = LoadProjectCommonCode(project, "");
	String plat_code = LoadPlatformCode(project, "texas-holdem", "");
	String combined = common_code + "\n" + plat_code;
	if (!gs.LoadAndInit(combined)) {
		Cout() << "Error: Failed to load project scripts: " << gs.GetError() << "\n";
		return 1;
	}
	Cout() << "DEBUG: Scripts loaded and initialized.\n";
	
	gs.action_proxy = [&](int action_id) {
		auto p = game->getPlayerByNumber(0);
		if (p) {
			Cout() << "SCRIPT PROXY: Performing action " << action_id << "\n";
			PlayerAction a = PLAYER_ACTION_NONE;
			if (action_id == 0) a = PLAYER_ACTION_FOLD;
			else if (action_id == 1) a = PLAYER_ACTION_CHECK;
			else if (action_id == 2) a = PLAYER_ACTION_RAISE;
			else if (action_id == 3) a = PLAYER_ACTION_ALLIN;
			p->setMyAction(a, true);
			gui->nextPlayerAnimation();
		}
	};
	
		int frame_idx = 0;
		while (frame_idx < 100) { 
			Cout() << "DEBUG: Simulation loop frame " << frame_idx << " GameID=" << game->getMyGameID() << " HandID=" << game->getCurrentHandID() << "\n";
			try {
				frame_idx++;
				Size tableSize(1920, 1080);
				Vector<String> gameLog;
			
			Image frame;
			{
				ImageBuffer ib(tableSize);
				BufferPainter bp(ib);
				bp.Begin();
				TexasRenderer::Render(bp, *game, gameLog, tableSize);
				bp.End();
				frame = ib;
			}
			PNGEncoder().SaveFile("debug_frame.png", frame);			
			if (!recognition_initialized) {
				GpuPreprocessConfig cfg;
				cfg.backend = "egl-gl";
				cfg.route = "auto";
				if (!gpu.Initialize(cfg)) {
					Cout() << "ERROR: GpuPreprocessEngine initialization failed: " << gpu.GetStats().last_error << "\n";
				}
				String rules_path = AppendFileName(GetCurrentDirectory(), "bin/pokerth.rules.json");
				recognition.rule_mgr.Load(rules_path);
				recognition.rule_mgr.SetActivePlatform("v13i");
				recognition.card_clf.Load(AppendFileName(GetCurrentDirectory(), "bin/data/convnet_models"), "v13i");
				recognition.SetAnchor(frame);
				recognition_initialized = true;
			}
			
			if (!gpu.PrepareFrame(frame)) continue;
			
			// 1. Capture Visual State
			GameState visual_state = recognition.ProcessGpu(gpu);
			
			GameState internal_state;
			internal_state.active_platform = "v13i";
			auto p0 = game->getPlayerByNumber(0);
			internal_state.my_turn = p0 && p0->getMyTurn();
			internal_state.pot = game->getCurrentHand() ? game->getCurrentHand()->getBoard()->getPot() : 0;
			if (game->getCurrentHand() && game->getCurrentHand()->getBoard()) {
				const int* bc = game->getCurrentHand()->getBoard()->getMyCards();
				Cout() << "DEBUG: Engine Board Cards: [" << bc[0] << ", " << bc[1] << ", " << bc[2] << ", " << bc[3] << ", " << bc[4] << "]\n";
				for (int i = 0; i < 5; i++) if (bc[i] >= 0) internal_state.community_cards.Add(bc[i]);
			}
			Cout() << "DEBUG: Calling RunStep...\n";
			gs.RunStep(internal_state);
			Cout() << "DEBUG: Calling RunCommonMain...\n";
			PyValue common_state = gs.RunCommonMain(frame);
			Cout() << "DEBUG: Script execution done.\n";
			
			bool board_match = (visual_state.community_cards.GetCount() == internal_state.community_cards.GetCount());
			if (board_match) {
				for (int i = 0; i < visual_state.community_cards.GetCount(); i++)
					if (visual_state.community_cards[i] != internal_state.community_cards[i]) { board_match = false; break; }
			}
			
			bool state_match = board_match && (visual_state.my_turn == internal_state.my_turn) && (visual_state.pot == internal_state.pot);
			if (!state_match) {
				Cout() << "MISMATCH at frame " << frame_idx << "! "
				       << "Visual: Board=" << visual_state.community_cards 
				       << " Turn=" << (int)visual_state.my_turn
				       << " Pot=" << visual_state.pot << " | "
				       << "Internal: Board=" << internal_state.community_cards 
				       << " Turn=" << (int)internal_state.my_turn
				       << " Pot=" << internal_state.pot << "\n";
			}

			if (gui->turn_active) {
				gui->turn_active = false;
			}
			
			if (game->getCurrentHand() && game->getCurrentHand()->getCurrentBeRo()) {
				auto bero = game->getCurrentHand()->getCurrentBeRo();
				int tid = bero->getCurrentPlayersTurnId();
				if (tid >= 0) {
					auto p = game->getPlayerByUniqueId((unsigned)tid);
					if (p) {
						Cout() << "DEBUG: Current turn: " << p->getMyName() << " (ID=" << tid << ")\n";
						if (p->getMyType() == PLAYER_TYPE_COMPUTER) {
							bero->nextPlayer();
						} else if (p->getMyTurn()) {
							p->setMyAction(PLAYER_ACTION_CHECK, true);
							gui->nextPlayerAnimation();
						}
					}
				} else {
					Cout() << "DEBUG: No active turn (tid=" << tid << ")\n";
					if (!game->isGameOver()) {
						game->initHand();
						game->startHand();
						gui->turn_active = true;
					}
				}
			}

			// Process callbacks from HeadlessGuiInterface
			gui->RunCallbacks();

			if (game->isGameOver()) break;
			Sleep(100);
		} catch (const Exc& e) {
			Cout() << "ERROR in simulation loop: " << e << "\n";
		} catch (...) {
			Cout() << "UNKNOWN ERROR in simulation loop\n";
		}
	}
	
	Cout() << "--- Headless Session Finished ---" << "\n";
	return 0;
}
