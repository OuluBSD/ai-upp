#include "GameTable.h"
#include "TexasHoldemLocalGame.h"
#include <Poker/LocalEngineFactory.h>
#include <GameRules/Game.h>
#include <GameRules/HandInterface.h>
#include <GameRules/BeroInterface.h>
#include <GameRules/PlayerInterface.h>
#include <GameRules/PlayerData.h>
#include <EditorCommon/ConfigFile.h>
#include <GameRules/EngineLog.h>

NAMESPACE_UPP

bool TexasHoldemIsPs6pProvider(const String& provider)
{
	String p = ToLower(TrimBoth(provider));
	return p == "ps_6p" || p == "ps-6p" || p == "pokerstars-6p";
}

String TexasHoldemProviderLayoutProfile(const String& provider)
{
	return TexasHoldemIsPs6pProvider(provider) ? "ps-6p" : "texas-holdem";
}

std::shared_ptr<Game> StartTexasHoldemLocalGame(GameTable& table,
                                                const TexasHoldemLocalGameOptions& options,
                                                class ConfigFile& config, EngineLog& engineLog)
{
	PlayerDataList players;
	String human_nick = config.readConfigString("Nick");
	if (human_nick.IsEmpty())
		human_nick = "Player";

	players.push_back(std::make_shared<PlayerData>(0, 0, PLAYER_TYPE_HUMAN, PLAYER_RIGHTS_ADMIN, true));
	players.back()->SetName(human_nick);

	for (int i = 1; i < options.num_players; i++) {
		players.push_back(std::make_shared<PlayerData>(i, i, PLAYER_TYPE_COMPUTER, PLAYER_RIGHTS_NORMAL, false));
		players.back()->SetName(Format("Computer %d", i));
	}

	GameData game_data;
	game_data.maxNumberOfPlayers = options.num_players;
	game_data.startMoney = options.start_cash;
	game_data.firstSmallBlind = 10;
	game_data.guiSpeed = options.game_speed;

	StartData start_data;
	start_data.numberOfPlayers = options.num_players;
	start_data.startDealerPlayerId = 0;

	auto factory = std::make_shared<LocalEngineFactory>();
	auto game = std::make_shared<Game>(&table, factory, players, game_data, start_data, 1, &engineLog, &config);
	if (options.seed >= 0)
		game->SetBaseSeed(options.seed);

	table.SetGame(game);
	table.SetProjectContext(options.project_name, TexasHoldemProviderLayoutProfile(options.provider));
	table.SetScriptAutomationEnabled(options.script_automation);

	game->initHand();
	game->startHand();
	return game;
}

int TexasHoldemCurrentTurnUid(const std::shared_ptr<Game>& game)
{
	if (!game)
		return -1;
	std::shared_ptr<HandInterface> hand = game->getCurrentHand();
	if (!hand)
		return -1;
	std::shared_ptr<BeroInterface> bero = hand->getCurrentBeRo();
	if (!bero)
		return -1;
	return (int)bero->getCurrentPlayersTurnId();
}

// -- Shared human auto-act helpers (moved from RunLocalGame.cpp; see task 0106) --
//
// These implement exactly the check/call/raise/fold/all-in math that
// `--local-game-script --auto-human` already relies on (previously file-local
// statics in RunLocalGame.cpp named identically without the TexasHoldem
// prefix). They are moved here, not reimplemented, so both RunLocalGame.cpp
// and StepTexasHoldemLocalGameAction below share one proven implementation.

std::shared_ptr<PlayerInterface> TexasHoldemGetHumanOnTurn(const std::shared_ptr<Game>& game,
                                                           const std::shared_ptr<BeroInterface>& bero)
{
	if (!game || !bero)
		return nullptr;
	int turn_id = (int)bero->getCurrentPlayersTurnId();
	if (turn_id < 0)
		return nullptr;
	std::shared_ptr<PlayerInterface> p = game->getPlayerByUniqueId((unsigned)turn_id);
	if (!p || p->getMyType() != PLAYER_TYPE_HUMAN)
		return nullptr;
	return p;
}

void TexasHoldemApplyHumanFold(const std::shared_ptr<Game>& game, const std::shared_ptr<BeroInterface>& bero)
{
	std::shared_ptr<PlayerInterface> p = TexasHoldemGetHumanOnTurn(game, bero);
	if (!p)
		return;
	auto hand = game->getCurrentHand();
	if (hand)
		hand->setPreviousPlayerID(p->getMyID());
	p->setMyAction(PLAYER_ACTION_FOLD, true);
}

void TexasHoldemApplyHumanCheckCall(const std::shared_ptr<Game>& game, const std::shared_ptr<BeroInterface>& bero)
{
	std::shared_ptr<PlayerInterface> p = TexasHoldemGetHumanOnTurn(game, bero);
	if (!p)
		return;
	const int highest = bero->getHighestSet();
	const int my_set_before = p->getMySet();
	int call_value = highest - my_set_before;
	if (call_value > p->getMyCash()) call_value = p->getMyCash();
	if (call_value < 0) call_value = 0;
	p->setMySet(call_value);
	auto hand = game->getCurrentHand();
	if (hand)
		hand->setPreviousPlayerID(p->getMyID());
	p->setMyAction(highest > my_set_before ? PLAYER_ACTION_CALL : PLAYER_ACTION_CHECK, true);
}

void TexasHoldemApplyHumanRaiseTo(const std::shared_ptr<Game>& game, const std::shared_ptr<BeroInterface>& bero,
                                  int target_total)
{
	std::shared_ptr<PlayerInterface> p = TexasHoldemGetHumanOnTurn(game, bero);
	if (!p)
		return;
	const int highest = bero->getHighestSet();
	const int current_total = p->getMySet();
	const int current_cash = p->getMyCash();
	const int max_total = current_total + current_cash;
	target_total = minmax(target_total, current_total, max_total);
	if (target_total <= current_total)
		return;
	p->setMySet(target_total - current_total);
	if (p->getMySet() > highest)
		bero->setHighestSet(p->getMySet());
	auto hand = game->getCurrentHand();
	if (hand)
		hand->setPreviousPlayerID(p->getMyID());
	if (target_total <= highest)
		p->setMyAction(target_total == highest ? PLAYER_ACTION_CALL : PLAYER_ACTION_CHECK, true);
	else if (p->getMyCash() == 0)
		p->setMyAction(PLAYER_ACTION_ALLIN, true);
	else
		p->setMyAction(PLAYER_ACTION_RAISE, true);
}

void TexasHoldemApplyHumanAllIn(const std::shared_ptr<Game>& game, const std::shared_ptr<BeroInterface>& bero)
{
	std::shared_ptr<PlayerInterface> p = TexasHoldemGetHumanOnTurn(game, bero);
	if (!p)
		return;
	TexasHoldemApplyHumanRaiseTo(game, bero, p->getMySet() + p->getMyCash());
}

bool TexasHoldemHumanNeedsAutoAct(const std::shared_ptr<Game>& game, const std::shared_ptr<BeroInterface>& bero)
{
	std::shared_ptr<PlayerInterface> p = TexasHoldemGetHumanOnTurn(game, bero);
	if (!p)
		return false;
	PlayerAction a = p->getMyAction();
	return a == PLAYER_ACTION_NONE || a == PLAYER_ACTION_SMALL_BLIND || a == PLAYER_ACTION_BIG_BLIND ||
	       p->getMySet() < bero->getHighestSet();
}

bool StepTexasHoldemLocalGameAction(const std::shared_ptr<Game>& game)
{
	if (!game)
		return false;
	std::shared_ptr<HandInterface> hand = game->getCurrentHand();
	if (!hand)
		return false;
	std::shared_ptr<BeroInterface> bero = hand->getCurrentBeRo();
	if (!bero)
		return false;
	bero->nextPlayer();
	// `--step-actions`/`--record-session` use this function as their sole
	// automated per-frame driver (see Main.cpp's stepping loop). There is no
	// GUI/script driver to answer LocalBero::nextPlayer()'s human-wait branch
	// (LocalBero.cpp:264-276), which intentionally stalls turn_uid on a human
	// seat and waits for an external caller to invoke setMyAction(...). Auto-act
	// as check/call here -- the same default policy `--local-game-script
	// --auto-human` already applies via TexasHoldemApplyHumanCheckCall above --
	// then advance the turn again so this one step still yields real progress
	// instead of freezing forever once the turn reaches the human seat.
	if (TexasHoldemHumanNeedsAutoAct(game, bero)) {
		TexasHoldemApplyHumanCheckCall(game, bero);
		bero->nextPlayer();
	}
	return true;
}

END_UPP_NAMESPACE
