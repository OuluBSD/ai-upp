#include "GameTable.h"
#include "TexasHoldemLocalGame.h"
#include <Poker/LocalEngineFactory.h>
#include <GameRules/Game.h>
#include <GameRules/HandInterface.h>
#include <GameRules/BeroInterface.h>
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
	return true;
}

END_UPP_NAMESPACE
