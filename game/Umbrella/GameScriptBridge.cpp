#include "Umbrella.h"
#include "GameScriptBridge.h"
#include "Player.h"

using namespace Upp;

// ============================================================================
// Python API Functions - Player State Query
// ============================================================================

// Python API: get_player_position() -> [x, y]
static PyValue py_get_player_position(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	const Player& player = bridge->GetGameScreen()->player;
	Pointf center = player.GetCenter();

	Vector<PyValue> result;
	result.Add(PyValue((double)center.x));
	result.Add(PyValue((double)center.y));
	return PyValue::FromVector(result);
}

// Python API: get_player_velocity() -> [vx, vy]
static PyValue py_get_player_velocity(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	const Player& player = bridge->GetGameScreen()->player;
	Pointf vel = player.GetVelocity();

	Vector<PyValue> result;
	result.Add(PyValue((double)vel.x));
	result.Add(PyValue((double)vel.y));
	return PyValue::FromVector(result);
}

// Python API: get_player_lives() -> int
static PyValue py_get_player_lives(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	int lives = bridge->GetGameScreen()->player.GetLives();
	return PyValue((int64)lives);
}

// Python API: get_player_score() -> int
static PyValue py_get_player_score(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	int score = bridge->GetGameScreen()->player.GetScore();
	return PyValue((int64)score);
}

// Python API: get_player_state() -> str ("IDLE", "ATTACKING", "GLIDING")
static PyValue py_get_player_state(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	const Player& player = bridge->GetGameScreen()->player;
	String stateStr;
	if(player.IsAttacking()) {
		stateStr = "ATTACKING";
	} else if(player.IsGliding()) {
		stateStr = "GLIDING";
	} else {
		stateStr = "IDLE";
	}

	return PyValue(stateStr.ToStd());
}

// Python API: is_player_on_ground() -> bool
static PyValue py_is_player_on_ground(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	bool onGround = bridge->GetGameScreen()->player.IsOnGround();
	return PyValue(onGround);
}

// ============================================================================
// Python API Functions - Player State Manipulation
// ============================================================================

// Python API: set_player_position(x, y)
static PyValue py_set_player_position(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	if(args.GetCount() < 2) {
		LOG("ERROR: set_player_position requires 2 arguments (x, y)");
		return PyValue::None();
	}

	float x = (float)args[0].AsDouble();
	float y = (float)args[1].AsDouble();

	bridge->GetGameScreen()->player.SetPosition(x, y);
	return PyValue::None();
}

// Python API: set_player_velocity(vx, vy)
static PyValue py_set_player_velocity(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	if(args.GetCount() < 2) {
		LOG("ERROR: set_player_velocity requires 2 arguments (vx, vy)");
		return PyValue::None();
	}

	float vx = (float)args[0].AsDouble();
	float vy = (float)args[1].AsDouble();

	bridge->GetGameScreen()->player.SetVelocity(Pointf(vx, vy));
	return PyValue::None();
}

// Python API: add_player_score(points)
static PyValue py_add_player_score(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	if(args.GetCount() < 1) {
		LOG("ERROR: add_player_score requires 1 argument (points)");
		return PyValue::None();
	}

	int points = (int)args[0].AsInt();
	bridge->GetGameScreen()->player.AddScore(points);
	return PyValue::None();
}

// ============================================================================
// Python API Functions - Enemy Management
// ============================================================================

// Python API: get_enemy_count() -> int
static PyValue py_get_enemy_count(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	int count = bridge->GetGameScreen()->enemies.GetCount();
	return PyValue((int64)count);
}

// Python API: get_enemy(index) -> dict {position, type, alive, active, captured, facing}
static PyValue py_get_enemy(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	if(args.GetCount() < 1) {
		LOG("ERROR: get_enemy requires 1 argument (index)");
		return PyValue::None();
	}

	int index = (int)args[0].AsInt();
	const Array<Enemy*>& enemies = bridge->GetGameScreen()->enemies;

	if(index < 0 || index >= enemies.GetCount()) {
		LOG("ERROR: get_enemy index out of range: " << index);
		return PyValue::None();
	}

	const Enemy* enemy = enemies[index];
	if(!enemy) {
		return PyValue::None();
	}

	// Create dict with enemy data
	PyValue result = PyValue::Dict();
	Rectf bounds = enemy->GetBounds();
	float centerX = bounds.left + bounds.Width() / 2.0f;
	float centerY = bounds.top + bounds.Height() / 2.0f;

	// Position as list [x, y]
	Vector<PyValue> pos;
	pos.Add(PyValue((double)centerX));
	pos.Add(PyValue((double)centerY));
	result.SetItem(PyValue("position"), PyValue::FromVector(pos));

	// Type as string
	String typeStr;
	switch(enemy->GetType()) {
		case ENEMY_PATROLLER: typeStr = "PATROLLER"; break;
		case ENEMY_JUMPER: typeStr = "JUMPER"; break;
		case ENEMY_SHOOTER: typeStr = "SHOOTER"; break;
	}
	result.SetItem(PyValue("type"), PyValue(typeStr.ToStd()));

	// States
	result.SetItem(PyValue("alive"), PyValue(enemy->IsAlive()));
	result.SetItem(PyValue("active"), PyValue(enemy->IsActive()));
	result.SetItem(PyValue("captured"), PyValue(enemy->IsCaptured()));
	result.SetItem(PyValue("thrown"), PyValue(enemy->IsThrown()));
	result.SetItem(PyValue("facing"), PyValue((int64)enemy->GetFacing()));

	return result;
}

// Python API: get_all_enemies() -> list of dicts
static PyValue py_get_all_enemies(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	const Array<Enemy*>& enemies = bridge->GetGameScreen()->enemies;
	Vector<PyValue> result;

	for(int i = 0; i < enemies.GetCount(); i++) {
		Vector<PyValue> indexArg;
		indexArg.Add(PyValue((int64)i));
		PyValue enemyData = py_get_enemy(indexArg, user_data);
		if(!enemyData.IsNone()) {
			result.Add(enemyData);
		}
	}

	return PyValue::FromVector(result);
}

// Python API: kill_enemy(index)
static PyValue py_kill_enemy(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	if(args.GetCount() < 1) {
		LOG("ERROR: kill_enemy requires 1 argument (index)");
		return PyValue::None();
	}

	int index = (int)args[0].AsInt();
	Array<Enemy*>& enemies = bridge->GetGameScreen()->enemies;

	if(index < 0 || index >= enemies.GetCount()) {
		LOG("ERROR: kill_enemy index out of range: " << index);
		return PyValue::None();
	}

	Enemy* enemy = enemies[index];
	if(enemy) {
		enemy->Kill();
	}

	return PyValue::None();
}

// Python API: clear_enemies()
static PyValue py_clear_enemies(const Vector<PyValue>& args, void* user_data) {
	GameScriptBridge* bridge = (GameScriptBridge*)user_data;
	if(!bridge || !bridge->GetGameScreen()) {
		return PyValue::None();
	}

	bridge->GetGameScreen()->ClearEnemies();
	return PyValue::None();
}

// ============================================================================
// GameScriptBridge Implementation
// ============================================================================

GameScriptBridge::GameScriptBridge() {
	gameScreen = nullptr;
}

void GameScriptBridge::RegisterGameAPI() {
	VectorMap<PyValue, PyValue>& globals = vm.GetGlobals();

	// Query functions
	globals.GetAdd("get_player_position") =
		PyValue::Function("get_player_position", py_get_player_position, this);
	globals.GetAdd("get_player_velocity") =
		PyValue::Function("get_player_velocity", py_get_player_velocity, this);
	globals.GetAdd("get_player_lives") =
		PyValue::Function("get_player_lives", py_get_player_lives, this);
	globals.GetAdd("get_player_score") =
		PyValue::Function("get_player_score", py_get_player_score, this);
	globals.GetAdd("get_player_state") =
		PyValue::Function("get_player_state", py_get_player_state, this);
	globals.GetAdd("is_player_on_ground") =
		PyValue::Function("is_player_on_ground", py_is_player_on_ground, this);

	// Manipulation functions
	globals.GetAdd("set_player_position") =
		PyValue::Function("set_player_position", py_set_player_position, this);
	globals.GetAdd("set_player_velocity") =
		PyValue::Function("set_player_velocity", py_set_player_velocity, this);
	globals.GetAdd("add_player_score") =
		PyValue::Function("add_player_score", py_add_player_score, this);

	// Enemy query functions
	globals.GetAdd("get_enemy_count") =
		PyValue::Function("get_enemy_count", py_get_enemy_count, this);
	globals.GetAdd("get_enemy") =
		PyValue::Function("get_enemy", py_get_enemy, this);
	globals.GetAdd("get_all_enemies") =
		PyValue::Function("get_all_enemies", py_get_all_enemies, this);

	// Enemy manipulation functions
	globals.GetAdd("kill_enemy") =
		PyValue::Function("kill_enemy", py_kill_enemy, this);
	globals.GetAdd("clear_enemies") =
		PyValue::Function("clear_enemies", py_clear_enemies, this);
}

bool GameScriptBridge::ExecuteScript(const String& scriptPath) {
	if(!FileExists(scriptPath)) {
		LOG("ERROR: Script not found: " << scriptPath);
		return false;
	}

	String source = LoadFile(scriptPath);
	if(source.IsEmpty()) {
		LOG("ERROR: Failed to load script: " << scriptPath);
		return false;
	}

	try {
		Tokenizer tokenizer;
		tokenizer.SkipPythonComments(true);
		if(!tokenizer.Process(source, scriptPath)) {
			LOG("ERROR: Failed to tokenize script");
			return false;
		}
		tokenizer.CombineTokens();

		PyCompiler compiler(tokenizer.GetTokens());
		Vector<PyIR> ir;
		compiler.Compile(ir);

		vm.SetIR(ir);
		vm.Run();
		return true;
	}
	catch(const Exc& e) {
		LOG("ERROR: Script exception: " << e);
		return false;
	}
}
