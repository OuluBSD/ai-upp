#ifndef _Core2_WorldState_h_
#define _Core2_WorldState_h_



namespace Eon {
class ScriptLoader;
class ScriptLoopLoader;
class ScriptDriverLoader;
}
class ActionPlanner;
class ActionNode;


struct BinaryWorldStateSession : Pte<BinaryWorldStateSession> {
	struct Item : Moveable<Item> {
		Value positive, negative;
	};
	VectorMap<String, Item> index;
	
};

struct BinaryWorldState {
	friend class ActionPlanner;
	friend class ActionPlannerWrapper;
	friend class ActionNode;
	
	Ptr<BinaryWorldStateSession> session;
	Vector<bool> values;
	Vector<bool> using_act;
	
	BinaryWorldState();
	void Clear();
	bool Set(int index, bool value);
	BinaryWorldState& operator=(const BinaryWorldState& src);
	hash_t GetHashValue() const;
};

class WorldState : public Moveable<WorldState> {

public:
	typedef enum : byte {
		INVALID,
		ADD_COMP,
	} Type;
	
protected:
	friend class ActionPlanner;
	friend class ActionPlannerWrapper;
	friend class ActionNode;
	friend class Eon::ScriptLoader;
	friend class Eon::ScriptLoopLoader;
	friend class Eon::ScriptDriverLoader;
	
	ValueMap values;
	
public:
	
	WorldState();
	void Clear();
	
	bool Set(int index, bool value);
	bool Set(int index, String value);
	bool Set(const String& key, bool value);
	bool Set(const String& key, String value);
	void SetTrue(const String& key);
	void SetFalse(const String& key);
	bool IsTrue(const String& key, bool def=false) const;
	bool IsFalse(const String& key, bool def=true) const;
	bool IsFalse(int idx) const;
	bool IsUndefined(const String& key) const;
	bool IsUndefined(int idx) const;
	bool IsEmpty() const;
	int GetValueCount() const;
	void FindKeys(String key_left, Index<String>& keys) const;
	String Get(const String& key, String def="") const;
	String Get(int idx) const;
	Size GetSize(const String& cx, const String& cy, Size def=Size(0,0)) const;
	int GetInt(const String& key, int def=0) const;
	double GetDouble(const String& key, double def=0) const;
	bool GetBool(const String& key, bool def=false) const;
	String GetString(const String& key, String def="") const;
	hash_t GetHashValue() const;
	String ToString() const;
	String GetFullString() const;
	bool Contains(const WorldState& ws) const;
	bool Conflicts(const WorldState& ws) const;
	int Compare(int idx, const WorldState& ws) const;
	
	WorldState& operator=(const WorldState& src);
	
	Value GetValues() const {return values;}
	
	bool operator==(const WorldState& ws) const {return GetHashValue() == ws.GetHashValue();}
	bool operator!=(const WorldState& ws) const {return GetHashValue() != ws.GetHashValue();}
	
};


namespace Eon {
using WorldState = ::Upp::WorldState;
}



#endif
