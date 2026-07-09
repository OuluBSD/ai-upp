#ifndef _CardEngine_TexasHoldemLocalGame_h_
#define _CardEngine_TexasHoldemLocalGame_h_

#include <Core/Core.h>
#include <memory>

NAMESPACE_UPP

class EngineLog;
class Game;
class GameTable;

bool TexasHoldemIsPs6pProvider(const String& provider);
String TexasHoldemProviderLayoutProfile(const String& provider);

struct TexasHoldemLocalGameOptions : Moveable<TexasHoldemLocalGameOptions> {
	int num_players = 10;
	int start_cash = 2000;
	int game_speed = 4;
	int seed = -1;
	String provider;
	String project_name = "default";
	bool script_automation = false;
};

std::shared_ptr<Game> StartTexasHoldemLocalGame(GameTable& table,
                                                const TexasHoldemLocalGameOptions& options,
                                                class ConfigFile& config, EngineLog& engineLog);
int TexasHoldemCurrentTurnUid(const std::shared_ptr<Game>& game);
bool StepTexasHoldemLocalGameAction(const std::shared_ptr<Game>& game);

END_UPP_NAMESPACE

#endif
