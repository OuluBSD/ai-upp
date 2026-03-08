#include <Maestro/Maestro.h>
#include "AdventureEngine.h"

NAMESPACE_UPP

class AdventureToolBase : public MaestroTool {
protected:
	AdventureEngine& engine;
public:
	AdventureToolBase(AdventureEngine& e) : engine(e) {}
};

class GoTool : public AdventureToolBase {
public:
	GoTool(AdventureEngine& e) : AdventureToolBase(e) {}
	virtual String GetName() const override { return "go"; }
	virtual String GetDescription() const override { return "Move to a different location (e.g., go('library'))."; }
	virtual Value  GetSchema() const override {
		ValueMap vm; vm.Add("direction", "string"); return vm;
	}
	virtual Value Execute(const ValueMap& params) const override {
		return engine.Go(params["direction"]);
	}
};

class UseTool : public AdventureToolBase {
public:
	UseTool(AdventureEngine& e) : AdventureToolBase(e) {}
	virtual String GetName() const override { return "use"; }
	virtual String GetDescription() const override { return "Use an item from inventory or on a target."; }
	virtual Value  GetSchema() const override {
		ValueMap vm; vm.Add("item", "string"); vm.Add("target", "string"); return vm;
	}
	virtual Value Execute(const ValueMap& params) const override {
		return engine.Use(params["item"], params["target"]);
	}
};

class PickTool : public AdventureToolBase {
public:
	PickTool(AdventureEngine& e) : AdventureToolBase(e) {}
	virtual String GetName() const override { return "pick"; }
	virtual String GetDescription() const override { return "Pick up an item from the current location."; }
	virtual Value  GetSchema() const override {
		ValueMap vm; vm.Add("item", "string"); return vm;
	}
	virtual Value Execute(const ValueMap& params) const override {
		return engine.Pick(params["item"]);
	}
};

class TalkTool : public AdventureToolBase {
public:
	TalkTool(AdventureEngine& e) : AdventureToolBase(e) {}
	virtual String GetName() const override { return "talk"; }
	virtual String GetDescription() const override { return "Talk to an NPC about a topic."; }
	virtual Value  GetSchema() const override {
		ValueMap vm; vm.Add("npc", "string"); vm.Add("topic", "string"); return vm;
	}
	virtual Value Execute(const ValueMap& params) const override {
		return engine.Talk(params["npc"], params["topic"]);
	}
};

class LookTool : public AdventureToolBase {
public:
	LookTool(AdventureEngine& e) : AdventureToolBase(e) {}
	virtual String GetName() const override { return "look"; }
	virtual String GetDescription() const override { return "Describe the current location, exits and items."; }
	virtual Value  GetSchema() const override { return Value(); }
	virtual Value Execute(const ValueMap& params) const override {
		return engine.Look();
	}
};

class InventoryTool : public AdventureToolBase {
public:
	InventoryTool(AdventureEngine& e) : AdventureToolBase(e) {}
	virtual String GetName() const override { return "inventory"; }
	virtual String GetDescription() const override { return "List items currently in your possession."; }
	virtual Value  GetSchema() const override {
		ValueMap vm; vm.Add("action", "string"); // e.g., 'list', 'use', 'give'
		return vm;
	}
	virtual Value Execute(const ValueMap& params) const override {
		return engine.ListInventory();
	}
};

class GameTool : public AdventureToolBase {
public:
	GameTool(AdventureEngine& e) : AdventureToolBase(e) {}
	virtual String GetName() const override { return "game"; }
	virtual String GetDescription() const override { return "Game management: save, load, exit, new."; }
	virtual Value  GetSchema() const override {
		ValueMap vm; vm.Add("action", "string"); return vm;
	}
	virtual Value Execute(const ValueMap& params) const override {
		return engine.GameCmd(params["action"]);
	}
};

class QuitTool : public AdventureToolBase {
public:
	QuitTool(AdventureEngine& e) : AdventureToolBase(e) {}
	virtual String GetName() const override { return "quit"; }
	virtual String GetDescription() const override { return "Exit the game."; }
	virtual Value  GetSchema() const override { return Value(); }
	virtual Value Execute(const ValueMap& params) const override {
		return "Player requested to quit the game.";
	}
};

void RegisterAdventureTools(MaestroToolRegistry& reg, AdventureEngine& engine) {
	reg.Add(new GoTool(engine));
	reg.Add(new UseTool(engine));
	reg.Add(new PickTool(engine));
	reg.Add(new TalkTool(engine));
	reg.Add(new LookTool(engine));
	reg.Add(new InventoryTool(engine));
	reg.Add(new GameTool(engine));
	reg.Add(new QuitTool(engine));
}

END_UPP_NAMESPACE
