#include "AITextAdventure.h"

static const char *json_system_prompt = 
	"You are playing a text adventure 'School Game 3'.\n"
	"COMMUNICATE ONLY IN JSON.\n"
	"Every response MUST be a single JSON object with this schema:\n"
	"{\n"
	"  \"thought\": \"Your internal monologue and description of surroundings\",\n"
	"  \"action\": \"tool_name\",\n"
	"  \"parameters\": { \"param1\": \"value1\", ... }\n"
	"}\n"
	"If no action is needed, use \"action\": \"none\".\n"
	"Available actions: go(direction), pick(item), look(), inventory().\n"
	"DO NOT use internal tools like 'run_shell_command'. ONLY use the adventure actions above.\n"
	"Current state:\n";

AITextAdventure::AITextAdventure() {
	Title("School Game 3 - AI Text Adventure (JSON Mode)");
	SetRect(0, 0, 1000, 700);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(tabs.SizePos());
	
	tabs.Add(game_chat.SizePos(), "Adventure");
	game_chat.send.WhenAction = THISBACK(SendCommand);
	
	tabs.Add(raw_chat.SizePos(), "AI Context");
	raw_chat.WhenEvent = [=](const MaestroEvent& e) { OnGameEvent(e); };
	raw_chat.WhenDone = [=] { OnGameTurnDone(); };
	
	RegisterAdventureTools(raw_chat.tools, engine);
	
	config.Load();
	
	const Vector<String>& cmdline = CommandLine();
	bool test_mode = false;
	String test_backend = "qwen";
	String cmd_model;
	
	for(int i = 0; i < cmdline.GetCount(); i++) {
		if(cmdline[i] == "--test") test_mode = true;
		if(cmdline[i] == "--backend" && i + 1 < cmdline.GetCount())
			test_backend = cmdline[i + 1];
		if(cmdline[i] == "--model" && i + 1 < cmdline.GetCount())
			cmd_model = cmdline[i + 1];
	}
	
	if(test_mode) {
		String tmp_dir = GetHomeDirectory() + "/.gemini/tmp/adventure_test";
		RealizeDirectory(tmp_dir);
		
		engine.Reset();
		game_chat.AddItem("System", "Welcome to School Game 3! (JSON TEST MODE)\n" + engine.Look());
		
		raw_chat.backend = test_backend;
		raw_chat.engine.working_dir = tmp_dir;
		if(!cmd_model.IsEmpty())
			raw_chat.engine.model = cmd_model;
		
		String prompt = json_system_prompt;
		prompt << engine.Look();
		
		raw_chat.input.SetData(prompt);
		PostCallback([=] { raw_chat.OnSend(); });
	} else {
		PostCallback([=] { NewGame(); });
	}
}

void AITextAdventure::MainMenu(Bar& bar) {
	bar.Sub("App", THISBACK(AppMenu));
	bar.Sub("Edit", THISBACK(EditMenu));
}

void AITextAdventure::AppMenu(Bar& bar) {
	bar.Add("New Game (AI)", THISBACK(NewGame)).Key(K_CTRL_N);
	bar.Add("Human Mode", [=] { 
		human_mode = !human_mode; 
		game_chat.AddItem("System", human_mode ? "Human Mode: ON" : "Human Mode: OFF (AI playing)");
	}).Check(human_mode);
	bar.Separator();
	bar.Add("Exit", THISBACK(Close));
}

void AITextAdventure::EditMenu(Bar& bar) {
	bar.Add("Send", [=] { 
		if(human_mode) SendCommand();
		else raw_chat.OnSend(); 
	}).Key(K_F5);
	bar.Separator();
	bar.Add("Copy All Chat", [=] { raw_chat.CopyAllChat(); });
	bar.Add("Copy Debug Info", [=] { raw_chat.CopyDebugData(); });
}

void AITextAdventure::NewGame() {
	NewSessionWindow sw(config);
	if(sw.Run() == IDOK) {
		engine.Reset();
		game_chat.AddItem("System", "Welcome to School Game 3!\n" + engine.Look());
		
		raw_chat.backend = sw.selected_backend;
		raw_chat.engine.working_dir = sw.selected_dir;
		if(!sw.session_id.IsEmpty())
			raw_chat.engine.session_id = sw.session_id;
		
		if(!human_mode) {
			String prompt = json_system_prompt;
			prompt << engine.Look();
			raw_chat.input.SetData(prompt);
			raw_chat.OnSend();
		}
	}
}

void AITextAdventure::OnGameEvent(const MaestroEvent& e) {
	// Filter local tool results to the game view immediately
	if(e.type == "tool_result" && e.tool_name != "result") {
		game_chat.AddItem("World", e.text);
	}
}

void AITextAdventure::OnGameTurnDone() {
	if(human_mode) return;
	
	String text = raw_chat.GetResponse();
	if(text.IsEmpty() || text == last_processed_text) return;
	last_processed_text = text;
	
	int start = text.Find('{');
	int end = text.ReverseFind('}');
	if(start >= 0 && end > start) {
		String json_str = text.Mid(start, end - start + 1);
		Value v = ParseJSON(json_str);
		if(!v.IsError() && v.Is<ValueMap>()) {
			String thought = v["thought"];
			String action = v["action"];
			ValueMap params = v["parameters"];
			
			if(!thought.IsEmpty())
				game_chat.AddItem("AI Student", thought);
			
			if(!action.IsEmpty()) {
				if (action != "none") {
					game_chat.AddItem("AI Action", action + "(" + AsJSON(params) + ")");
					
					const MaestroTool* t = raw_chat.tools.Find(action);
					if(t) {
						Value result = t->Execute(params);
						game_chat.AddItem("World", AsString(result));
						
						// Send result back as JSON-wrapped message
						ValueMap res;
						res.Add("type", "tool_result");
						res.Add("action", action);
						res.Add("result", result);
						
						raw_chat.input.SetData(AsJSON(res));
						// Automatically trigger next turn
						PostCallback([=] { raw_chat.OnSend(); });
					} else {
						game_chat.AddItem("System", "Error: Tool '" + action + "' not found.");
					}
				}
			}
		}
	}
}

void AITextAdventure::SendCommand() {
	String cmd = game_chat.input.GetData().ToString();
	if(cmd.IsEmpty()) return;
	
	game_chat.input.Clear();
	game_chat.AddItem("Human Player", cmd);
	
	String res;
	if(cmd.StartsWith("go ")) res = engine.Go(cmd.Mid(3));
	else if(cmd == "look") res = engine.Look();
	else if(cmd.StartsWith("pick ")) res = engine.Pick(cmd.Mid(5));
	else if(cmd == "inventory") res = engine.ListInventory();
	else res = "Unknown command. Try: go <dir>, look, pick <item>, inventory.";
	
	game_chat.AddItem("World", res);
}

GUI_APP_MAIN {
	AITextAdventure().Run();
}
