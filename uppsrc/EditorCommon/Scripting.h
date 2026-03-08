#ifndef _GameCommon_Scripting_h_
#define _GameCommon_Scripting_h_

#ifdef None
#undef None
#endif
#include <ByteVM/ByteVM.h>
#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif
#include <EditorCommon/Recognition.h>

namespace Upp {

struct ScriptRuntimeObject : Moveable<ScriptRuntimeObject> {
	String name;
	Rect rect;
	VectorMap<String, String> props;
	
	ScriptRuntimeObject() {}
	ScriptRuntimeObject(const ScriptRuntimeObject& s) { *this = s; }
	ScriptRuntimeObject(const ScriptRuntimeObject& s, int) { *this = s; }
	void operator=(const ScriptRuntimeObject& s) {
		name = s.name;
		rect = s.rect;
		props <<= s.props;
	}
};

class GameScript {
	PyVM vm;
	Vector<PyIR> ir;
	bool loaded = false;
	String error;
	String loaded_code;
	String current_platform;
	Callback1<String> log_cb;
	bool trace_api = false;
	Function<bool(const String&, const Image&, Vector<Rect>&, Vector<double>&, const Rect*)> orb_finder;
	Function<bool(const String&, const Image&, const Rect*, Rect&, double&)> bruteforce_finder;
	Function<void(const Vector<Rect>&)> window_setter;
	Function<bool(const String&, const String&, String&)> rule_prop_getter;
	public:
	Function<void(int)> action_proxy;
	private:
	struct TrackerState : Moveable<TrackerState> {		Rect rect;
		double score = 0.0;
		int miss_count = 0;
		bool has_rect = false;
	};
	VectorMap<String, TrackerState> trackers;
	VectorMap<String, ScriptRuntimeObject> runtime_objects;
	struct TextTrackState : Moveable<TextTrackState> {
		Image patch;
		String text;
		double confidence = 0.0;
		String method;
		bool has_value = false;
	};
	VectorMap<String, TextTrackState> text_trackers;

public:
	GameScript();
	~GameScript();
	
	bool Load(const String& code);
	bool LoadFile(const String& path);
	bool LoadAndInit(const String& code);
	void Clear();
	
	void RunStep(const struct GameState& state);
	void SetCurrentPlatform(const String& name) { current_platform = name; }
	const String& GetCurrentPlatform() const { return current_platform; }
	
	PyValue RunCommonMain(const Image& img);
	bool    RunPlatformMain(const String& name, const PyValue& instance, const Image& img, const PyValue& common_state, Vector<Rect>& rects, Vector<double>& scores);
	
	bool CallPlatformHooks(const Image& img);
	bool CallNamedPlatformHooks(const String& name, const Image& img, Vector<Rect>& rects, Vector<double>& scores);
	int RunPlatformTracking(const Image& img);
	bool RunPlatformTracking(const Image& img, Vector<Rect>& rects, Vector<double>& scores);
	void SetLogCallback(Callback1<String> cb) { log_cb = cb; }
	void SetTraceApi(bool b) { trace_api = b; }
	void TraceApi(const String& s);
	void SetOrbFinder(Function<bool(const String&, const Image&, Vector<Rect>&, Vector<double>&, const Rect*)> fn) { orb_finder = fn; }
	void SetBruteforceFinder(Function<bool(const String&, const Image&, const Rect*, Rect&, double&)> fn) { bruteforce_finder = fn; }
	void SetWindowSetter(Function<void(const Vector<Rect>&)> fn) { window_setter = fn; }
	void SetRulePropGetter(Function<bool(const String&, const String&, String&)> fn) { rule_prop_getter = fn; }
	void SetActionProxy(Function<void(int)> fn) { action_proxy = fn; }
	bool FindOrb(const String& rule, const Image& img, Vector<Rect>& rects, Vector<double>& scores, const Rect* search = nullptr) {		if (orb_finder)
			return orb_finder(rule, img, rects, scores, search);
		return false;
	}
	bool FindBruteforce(const String& rule, const Image& img, const Rect* search, Rect& out, double& score) {
		if (bruteforce_finder)
			return bruteforce_finder(rule, img, search, out, score);
		return false;
	}
	void SetWindowRects(const Vector<Rect>& rects) {
		if (window_setter)
			window_setter(rects);
	}
	bool GetRuleProp(const String& rule, const String& key, String& value) const {
		if (rule_prop_getter)
			return rule_prop_getter(rule, key, value);
		return false;
	}
	bool TrackerGet(const String& name, Rect& rect, double& score, int& miss_count) const;
	void TrackerSet(const String& name, const Rect& rect, double score);
	int TrackerMarkMiss(const String& name);
	void TrackerClear(const String& name);
	bool TrackerShouldReset(const String& name, int miss_limit) const;
	void SetRuntimeObject(const String& name, const Rect& rect);
	void SetRuntimeObjectProp(const String& name, const String& key, const String& value);
	void ClearRuntimeObjects();
	void GetRuntimeObjects(Vector<ScriptRuntimeObject>& out) const;
	bool CompareAndUpdateTextPatch(const String& name, const Image& patch, int good_match_threshold, int& good_matches, int& total_matches);
	bool GetTrackedTextValue(const String& name, String& text, double& confidence, String& method) const;
	void SetTrackedTextValue(const String& name, const String& text, double confidence, const String& method);
	void ClearTrackedTextValues();
	void SetOpenAIKeyFile(const String& path);
	void SetOpenAIModel(const String& model);
	
	const String& GetError() const { return error; }
	const String& GetCode() const { return loaded_code; }
	bool IsLoaded() const { return loaded; }
};

}

#endif
