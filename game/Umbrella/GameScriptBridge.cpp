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
