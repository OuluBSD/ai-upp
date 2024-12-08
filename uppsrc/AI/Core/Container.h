#ifndef _AI_TextCore_Container_h_
#define _AI_TextCore_Container_h_

NAMESPACE_UPP

struct ActionHeader : Moveable<ActionHeader> {
	String action, arg;

	ActionHeader() {}
	ActionHeader(ActionHeader&& a)
		: action(std::move(a.action))
		, arg(std::move(a.arg))
	{
	}
	ActionHeader(const ActionHeader& a)
	{
		action = a.action;
		arg = a.arg;
	}
	void operator=(const ActionHeader& a)
	{
		action = a.action;
		arg = a.arg;
	}
	bool operator==(const ActionHeader& a) const { return action == a.action && arg == a.arg; }
	hash_t GetHashValue() const
	{
		CombineHash c;
		c.Do(action);
		c.Put(1);
		c.Do(arg);
		return c;
	}
	bool operator()(const ActionHeader& a, const ActionHeader& b) const
	{
		if(a.action != b.action)
			return a.action < b.action;
		return a.arg < b.arg;
	}
	bool IsEmpty() const { return action.IsEmpty() || arg.IsEmpty(); }
	void Jsonize(JsonIO& json) { json("act", action)("arg", arg); }
	void Serialize(Stream& s) { s % action % arg; }
	void Trim()
	{
		action = TrimBoth(action);
		arg = TrimBoth(arg);
	}
};

struct AttrHeader : Moveable<AttrHeader> {
	String group, value;

	AttrHeader() {}
	AttrHeader(String g, String v)
		: group(g)
		, value(v)
	{
	}
	AttrHeader(AttrHeader&& a)
		: group(std::move(a.group))
		, value(std::move(a.value))
	{
	}
	AttrHeader(const AttrHeader& a)
	{
		group = a.group;
		value = a.value;
	}
	void operator=(const AttrHeader& a)
	{
		group = a.group;
		value = a.value;
	}
	bool operator==(const AttrHeader& a) const { return group == a.group && value == a.value; }
	hash_t GetHashValue() const
	{
		CombineHash c;
		c.Do(group);
		c.Put(1);
		c.Do(value);
		return c;
	}
	bool operator()(const AttrHeader& a, const AttrHeader& b) const
	{
		if(a.group != b.group)
			return a.group < b.group;
		return a.value < b.value;
	}
	bool IsEmpty() const { return group.IsEmpty() || value.IsEmpty(); }
	void Jsonize(JsonIO& json) { json("grp", group)("val", value); }
	void Serialize(Stream& s) { s % group % value; }
	void Trim()
	{
		group = TrimBoth(group);
		value = TrimBoth(value);
	}
};

enum {
	PART_BEGIN,
	PART_MID,
	PART_END,

	PART_COUNT
};

struct ContentType : Moveable<ContentType> {
	String key, parts[PART_COUNT];

	void Serialize(Stream& s) {s / key; for(int i = 0; i < PART_COUNT; i++) s / parts[i];}
	void Set(String key, String begin, String mid, String end)
	{
		this->key = key;
		parts[PART_BEGIN] = begin;
		parts[PART_MID] = mid;
		parts[PART_END] = end;
	}
};

END_UPP_NAMESPACE

#endif
