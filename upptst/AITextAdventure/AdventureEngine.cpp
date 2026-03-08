#include "AdventureEngine.h"

void AdventureEngine::InitSchoolGame() {
	world.Clear();
	inventory.Clear();
	
	// Library
	{
		auto& l = world.Add("Library");
		l.name = "Library";
		l.description = "A quiet place with shelves full of books. The air smells of old paper.";
		l.exits.Add("hallway", "Corridor");
		l.items.Add("Textbook");
	}
	
	// Corridor
	{
		auto& l = world.Add("Corridor");
		l.name = "Corridor";
		l.description = "A long hallway with students rushing to classes.";
		l.exits.Add("library", "Library");
		l.exits.Add("cafeteria", "Cafeteria");
		l.exits.Add("lecture", "Lecture Hall");
	}
	
	// Cafeteria
	{
		auto& l = world.Add("Cafeteria");
		l.name = "Cafeteria";
		l.description = "The hub of social life. Smells like coffee and cheap lunch.";
		l.exits.Add("hallway", "Corridor");
		l.items.Add("Coffee");
	}
	
	// Lecture Hall
	{
		auto& l = world.Add("Lecture Hall");
		l.name = "Lecture Hall";
		l.description = "Large amphitheater. A professor is preparing slides.";
		l.exits.Add("hallway", "Corridor");
	}
	
	current_location = "Corridor";
}

String AdventureEngine::Look() {
	if(world.Find(current_location) < 0) return "You are in the void.";
	
	const auto& l = world.Get(current_location);
	String res;
	res << "Location: " << l.name << "\n";
	res << l.description << "\n";
	
	if(l.exits.GetCount() > 0) {
		res << "Exits: ";
		for(int i = 0; i < l.exits.GetCount(); i++)
			res << (i > 0 ? ", " : "") << l.exits.GetKey(i);
		res << "\n";
	}
	
	if(l.items.GetCount() > 0) {
		res << "Items: ";
		for(int i = 0; i < l.items.GetCount(); i++)
			res << (i > 0 ? ", " : "") << l.items[i];
		res << "\n";
	}
	
	return res;
}

String AdventureEngine::Go(const String& direction) {
	if(world.Find(current_location) < 0) return "Error: invalid state.";
	
	auto& l = world.Get(current_location);
	int q = l.exits.Find(ToLower(direction));
	if(q < 0) return "You cannot go that way.";
	
	current_location = l.exits[q];
	return "You go to the " + current_location + ".\n" + Look();
}

String AdventureEngine::Pick(const String& item_name) {
	if(world.Find(current_location) < 0) return "Error: invalid state.";
	
	auto& l = world.Get(current_location);
	int q = -1;
	for(int i = 0; i < l.items.GetCount(); i++) {
		if(ToLower(l.items[i]) == ToLower(item_name)) {
			q = i;
			break;
		}
	}
	
	if(q < 0) return "There is no " + item_name + " here.";
	
	inventory.Add(l.items[q]);
	l.items.Remove(q);
	return "You picked up the " + item_name + ".";
}

String AdventureEngine::ListInventory() {
	if(inventory.IsEmpty()) return "Your inventory is empty.";
	String res = "Inventory: ";
	for(int i = 0; i < inventory.GetCount(); i++)
		res << (i > 0 ? ", " : "") << inventory[i];
	return res;
}

String AdventureEngine::Use(const String& item, const String& target) {
	return "You try to use " + item + (target.IsEmpty() ? "" : " on " + target) + ", but nothing happens.";
}

String AdventureEngine::Talk(const String& npc, const String& topic) {
	return npc + " doesn't seem to want to talk about " + topic + " right now.";
}

String AdventureEngine::GameCmd(const String& action) {
	if(action == "save") return "Game saved (simulated).";
	if(action == "load") return "Game loaded (simulated).";
	if(action == "new") { Reset(); return "New game started.\n" + Look(); }
	return "Unknown game command: " + action;
}

void AdventureEngine::Reset() {
	InitSchoolGame();
}