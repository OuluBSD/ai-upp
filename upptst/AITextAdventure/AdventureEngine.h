#ifndef _AITextAdventure_AdventureEngine_h_
#define _AITextAdventure_AdventureEngine_h_

#include <Core/Core.h>

using namespace Upp;

struct AdventureItem : Moveable<AdventureItem> {
	String name;
	String description;
};

struct AdventureLocation : Moveable<AdventureLocation> {
	String name;
	String description;
	VectorMap<String, String> exits;
	Vector<String> items;
};

class AdventureEngine {
public:
	VectorMap<String, AdventureLocation> world;
	String current_location;
	Vector<String> inventory;
	
	void InitSchoolGame();
	
	String Look();
	String Go(const String& direction);
	String Pick(const String& item);
	String Use(const String& item, const String& target);
	String Talk(const String& npc, const String& topic);
	String ListInventory();
	String GameCmd(const String& action);
	
	void Reset();
	
	AdventureEngine() { InitSchoolGame(); }
};

#endif