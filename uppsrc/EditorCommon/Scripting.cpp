#include <StrategyBridge/StrategyBridge.h>
#include <EditorCommon/Scripting.h>
#include <GameRules/EngineDefs.h>
#include <GameCommon/Poker/ServerListSim.h>
#include <ConvNet/ConvNet.h>
#ifdef PLATFORM_WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#undef None

namespace Upp {

static GameState* g_state = nullptr;
static const Image* g_image = nullptr;
static GameScript* g_script = nullptr;
static String g_convnet_model_dir;
static String g_openai_key_file;
static String g_openai_model = "gpt-4.1-mini";
extern bool g_cli_verbose;

class PyImageUserData : public PyUserData {
public:
	const Image* img = nullptr;
	virtual String GetTypeName() const override { return "Image"; }
	virtual PyValue GetAttr(const String& name) override {
		if (!img || img->IsEmpty())
			return PyValue::None();
		if (name == "width") return PyValue((int64)img->GetWidth());
		if (name == "height") return PyValue((int64)img->GetHeight());
		if (name == "stride") return PyValue((int64)img->GetWidth() * 4);
		if (name == "data") {
			int w = img->GetWidth();
			int h = img->GetHeight();
			String out;
			out.Reserve(w * h * 4);
			for (int y = 0; y < h; y++) {
				const RGBA* s = (*img)[y];
				out.Cat((const char*)s, w * 4);
			}
			return PyValue(out);
		}
		return PyValue::None();
	}
};

static PyImageUserData g_image_ud;

static Image Scale2xNearest(const Image& src) {
	if (src.IsEmpty())
		return src;
	Size sz = src.GetSize();
	ImageBuffer ib(sz.cx * 2, sz.cy * 2);
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* s = src[y];
		RGBA* d0 = ib[(y * 2) + 0];
		RGBA* d1 = ib[(y * 2) + 1];
		for (int x = 0; x < sz.cx; x++) {
			const RGBA p = s[x];
			d0[x * 2 + 0] = p;
			d0[x * 2 + 1] = p;
			d1[x * 2 + 0] = p;
			d1[x * 2 + 1] = p;
		}
	}
	return ib;
}

static bool DebugFlagEnabled(const char* key) {
	String v = GetEnv(key);
	v = ToLower(TrimBoth(v));
	return v == "1" || v == "true" || v == "yes" || v == "on";
}

static void SaveGrayPgm(const String& path, const Vector<byte>& gray, int w, int h) {
	if (w <= 0 || h <= 0 || gray.GetCount() < w * h)
		return;
	String out;
	out << "P5\n" << w << " " << h << "\n255\n";
	out.Cat((const char*)gray.Begin(), w * h);
	RealizeDirectory(GetFileFolder(path));
	SaveFile(path, out);
}

static void ScriptLog(Callback1<String>& cb, const String& msg) {
	if (cb)
		cb(msg);
}

static String ScriptIdentFromName(const String& name) {
	String out;
	out.Reserve(name.GetCount() + 1);
	for (int i = 0; i < name.GetCount(); i++) {
		int c = name[i];
		bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		          (c >= '0' && c <= '9') || c == '_';
		out.Cat(ok ? c : '_');
	}
	if (out.IsEmpty() || (out[0] >= '0' && out[0] <= '9'))
		out.Insert(0, '_');
	return out;
}

static String ResolveOpenAIKeyFilePath() {
	String p = TrimBoth(g_openai_key_file);
	if (p.IsEmpty())
		p = TrimBoth(GetEnv("SCREENGAME_OPENAI_KEY_FILE"));
	return p;
}

static String ResolveOpenAIModel() {
	String m = TrimBoth(g_openai_model);
	if (m.IsEmpty())
		m = TrimBoth(GetEnv("SCREENGAME_OPENAI_MODEL"));
	if (m.IsEmpty())
		m = "gpt-4.1-mini";
	return m;
}

static String LoadOpenAIKey() {
	static String cached_path;
	static String cached_key;
	String p = ResolveOpenAIKeyFilePath();
	if (p.IsEmpty())
		return String();
	if (!IsFullPath(p))
		p = AppendFileName(GetCurrentDirectory(), p);
	if (p == cached_path && !cached_key.IsEmpty())
		return cached_key;
	if (!FileExists(p))
		return String();
	String key = TrimBoth(LoadFile(p));
	if (key.IsEmpty())
		return String();
	cached_path = p;
	cached_key = key;
	return cached_key;
}

static bool ExtractJsonString(const Value& v, const char* key, String& out) {
	if (!v.Is<ValueMap>())
		return false;
	ValueMap m = v;
	Value q = m.Get(key, Value());
	if (IsNull(q))
		return false;
	out = (String)q;
	return true;
}

static bool ExtractJsonValue(const Value& v, const char* key, Value& out) {
	if (!v.Is<ValueMap>())
		return false;
	ValueMap m = v;
	Value q = m.Get(key, Value());
	if (IsNull(q))
		return false;
	out = q;
	return true;
}

static String ExtractFirstJsonObjectText(const String& s) {
	String t = TrimBoth(s);
	if (t.IsEmpty())
		return t;
	int b = t.Find('{');
	if (b < 0)
		return t;
	int depth = 0;
	for (int i = b; i < t.GetCount(); i++) {
		int c = t[i];
		if (c == '{')
			depth++;
		else if (c == '}') {
			depth--;
			if (depth == 0)
				return t.Mid(b, i - b + 1);
		}
	}
	return t;
}

static bool ExtractResponsesOutputText(const Value& root, String& out_text) {
	Value qout;
	if (!ExtractJsonValue(root, "output", qout) || !qout.Is<ValueArray>())
		return false;
	ValueArray arr = qout;
	for (int i = 0; i < arr.GetCount(); i++) {
		Value it = arr[i];
		Value qcont;
		if (!ExtractJsonValue(it, "content", qcont) || !qcont.Is<ValueArray>())
			continue;
		ValueArray content = qcont;
		for (int j = 0; j < content.GetCount(); j++) {
			Value c = content[j];
			String typ;
			ExtractJsonString(c, "type", typ);
			String txt;
			if (typ == "output_text" && ExtractJsonString(c, "text", txt) && !txt.IsEmpty()) {
				out_text = txt;
				return true;
			}
		}
	}
	return false;
}

static bool OpenAIRepairText(const String& raw_text, const String& example_text, const String& context_text,
                             String& out_text, double& out_confidence, String& out_status, String& out_error) {
	out_text = raw_text;
	out_confidence = 0.0;
	out_status = "disabled";
	out_error.Clear();
	String key = LoadOpenAIKey();
	if (key.IsEmpty()) {
		out_status = "openai_key_missing";
		return false;
	}
	String model = ResolveOpenAIModel();
	ValueMap payload;
	payload.Add("model", model);
	String prompt;
	prompt << "You repair OCR text.\n";
	prompt << "Return strict JSON only with keys: text (string), confidence (number 0..1), status (string).\n";
	prompt << "Preserve dynamic content and line breaks.\n\n";
	prompt << "raw_ocr_text:\n" << raw_text << "\n\n";
	if (!example_text.IsEmpty())
		prompt << "reference_example:\n" << example_text << "\n\n";
	if (!context_text.IsEmpty())
		prompt << "context:\n" << context_text << "\n\n";
	prompt << "Return only JSON.";
	payload.Add("input", prompt);

	HttpRequest req;
	req.Url("https://api.openai.com/v1/responses");
	req.ContentType("application/json");
	req.Header("Authorization", "Bearer " + key);
	String response = req.Post(AsJSON(Value(payload))).Execute();
	if (!req.IsSuccess()) {
		out_status = "openai_http_error";
		out_error = req.GetErrorDesc();
		return false;
	}
	Value root = ParseJSON(response);
	if (!root.Is<ValueMap>()) {
		out_status = "openai_json_error";
		out_error = "invalid_response_json";
		return false;
	}
	String content;
	if (!ExtractResponsesOutputText(root, content) || content.IsEmpty()) {
		out_status = "openai_no_content";
		return false;
	}
	content = ExtractFirstJsonObjectText(content);
	Value repaired = ParseJSON(content);
	if (!repaired.Is<ValueMap>()) {
		out_status = "openai_content_not_json";
		out_error = content;
		return false;
	}
	String txt;
	if (ExtractJsonString(repaired, "text", txt) && !txt.IsEmpty())
		out_text = txt;
	ValueMap rm = repaired;
	Value cf = rm.Get("confidence", Value());
	if (!IsNull(cf))
		out_confidence = minmax((double)cf, 0.0, 1.0);
	String st;
	if (ExtractJsonString(repaired, "status", st) && !st.IsEmpty())
		out_status = st;
	else
		out_status = "ok";
	return true;
}

static PyValue builtin_get_pot(const Vector<PyValue>& args, void*) {
	if (g_state) return PyValue((int64)g_state->pot);
	return PyValue(0);
}

static PyValue builtin_is_my_turn(const Vector<PyValue>& args, void*) {
	if (g_state) {
		return PyValue(g_state->my_turn);
	}
	return PyValue(false);
}

static PyValue builtin_is_found(const Vector<PyValue>& args, void*) {
	if (g_state) return PyValue(g_state->found);
	return PyValue(false);
}

static PyValue builtin_get_stack(const Vector<PyValue>& args, void*) {
	if (g_state && args.GetCount() > 0) {
		int i = args[0].AsInt();
		if (i >= 0 && i < g_state->stacks.GetCount())
			return PyValue((int64)g_state->stacks[i]);
	}
	return PyValue(0);
}

static PyValue builtin_get_stacks_count(const Vector<PyValue>& args, void*) {
	if (g_state) return PyValue((int64)g_state->stacks.GetCount());
	return PyValue(0);
}

static bool GetDictInt(const PyValue& v, const char* key, int& out);

static PyValue builtin_find_orb(const Vector<PyValue>& args, void*) {
	if (!g_script || !g_image)
		return PyValue::List();
	if (args.GetCount() < 1)
		return PyValue::List();
	String rule = args[0].ToString();
	g_script->TraceApi("find_orb(" + rule + ")");
	Rect search;
	const Rect* search_ptr = nullptr;
	if (args.GetCount() >= 5) {
		int x = args[1].AsInt();
		int y = args[2].AsInt();
		int w = args[3].AsInt();
		int h = args[4].AsInt();
		search = RectC(x, y, w, h);
		search_ptr = &search;
	}
	Vector<Rect> rects;
	Vector<double> scores;
	g_script->FindOrb(rule, *g_image, rects, scores, search_ptr);
	PyValue out = PyValue::List();
	for (int i = 0; i < rects.GetCount(); i++) {
		const Rect& r = rects[i];
		PyValue m = PyValue::Dict();
		m.SetItem(PyValue("x"), PyValue(r.left));
		m.SetItem(PyValue("y"), PyValue(r.top));
		m.SetItem(PyValue("w"), PyValue(r.GetWidth()));
		m.SetItem(PyValue("h"), PyValue(r.GetHeight()));
		m.SetItem(PyValue("score"), PyValue(i < scores.GetCount() ? scores[i] : 1.0));
		out.Add(m);
	}
	return out;
}

static PyValue builtin_find_bruteforce(const Vector<PyValue>& args, void*) {
	if (!g_script || !g_image)
		return PyValue::None();
	if (args.GetCount() < 1)
		return PyValue::None();
	String rule = args[0].ToString();
	g_script->TraceApi("find_bruteforce(" + rule + ")");
	Rect search;
	Rect* search_ptr = nullptr;
	if (args.GetCount() >= 5) {
		int x = args[1].AsInt();
		int y = args[2].AsInt();
		int w = args[3].AsInt();
		int h = args[4].AsInt();
		search = RectC(x, y, w, h);
		search_ptr = &search;
	}
	Rect out;
	double score = 0.0;
	if (!g_script->FindBruteforce(rule, *g_image, search_ptr, out, score))
		return PyValue::None();
	PyValue m = PyValue::Dict();
	m.SetItem(PyValue("x"), PyValue(out.left));
	m.SetItem(PyValue("y"), PyValue(out.top));
	m.SetItem(PyValue("w"), PyValue(out.GetWidth()));
	m.SetItem(PyValue("h"), PyValue(out.GetHeight()));
	m.SetItem(PyValue("score"), PyValue(score));
	return m;
}

static bool ParseRectValue(const PyValue& v, Rect& r) {
	if (v.GetType() == PY_DICT) {
		int x = 0, y = 0, w = 0, h = 0;
		int l = 0, t = 0, rr = 0, b = 0;
		if (GetDictInt(v, "x", x) && GetDictInt(v, "y", y) &&
		    GetDictInt(v, "w", w) && GetDictInt(v, "h", h)) {
			r = RectC(x, y, w, h);
			return true;
		}
		if (GetDictInt(v, "l", l) && GetDictInt(v, "t", t) &&
		    GetDictInt(v, "r", rr) && GetDictInt(v, "b", b)) {
			r = Rect(l, t, rr, b);
			return true;
		}
		return false;
	}
	if (v.GetType() == PY_LIST || v.GetType() == PY_TUPLE) {
		if (v.GetCount() >= 4) {
			int x = v.GetItem(0).AsInt();
			int y = v.GetItem(1).AsInt();
			int w = v.GetItem(2).AsInt();
			int h = v.GetItem(3).AsInt();
			r = RectC(x, y, w, h);
			return true;
		}
	}
	return false;
}

static bool ParseRectArgs(const Vector<PyValue>& args, int start, Rect& out);

static PyValue builtin_set_window_rects(const Vector<PyValue>& args, void*) {
	if (!g_script)
		return PyValue::None();
	Vector<Rect> rects;
	if (args.GetCount() >= 1) {
		const PyValue& v = args[0];
		if (v.GetType() == PY_LIST || v.GetType() == PY_TUPLE) {
			for (int i = 0; i < v.GetCount(); i++) {
				Rect r;
				if (ParseRectValue(v.GetItem(i), r))
					rects.Add(r);
			}
		} else {
			Rect r;
			if (ParseRectValue(v, r))
				rects.Add(r);
		}
	}
	g_script->SetWindowRects(rects);
	g_script->TraceApi("set_window_rects(count=" + AsString(rects.GetCount()) + ")");
	return PyValue::None();
}

static PyValue builtin_set_window_rect(const Vector<PyValue>& args, void*) {
	if (!g_script)
		return PyValue::None();
	Vector<Rect> rects;
	if (args.GetCount() >= 4) {
		int x = args[0].AsInt();
		int y = args[1].AsInt();
		int w = args[2].AsInt();
		int h = args[3].AsInt();
		rects.Add(RectC(x, y, w, h));
	}
	g_script->SetWindowRects(rects);
	g_script->TraceApi("set_window_rect(count=" + AsString(rects.GetCount()) + ")");
	return PyValue::None();
}

static PyValue builtin_get_rule_prop(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 2)
		return PyValue::None();
	String rule = args[0].ToString();
	String key = args[1].ToString();
	String value;
	bool ok = g_script->GetRuleProp(rule, key, value);
	if (g_script)
		g_script->TraceApi("get_rule_prop(" + rule + "," + key + ")");
	if (ok)
		return PyValue(value);
	if (args.GetCount() >= 3)
		return args[2];
	return PyValue::None();
}

static PyValue builtin_tracker_get(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 1)
		return PyValue::None();
	String name = args[0].ToString();
	g_script->TraceApi("tracker_get(" + name + ")");
	Rect rect;
	double score = 0.0;
	int miss = 0;
	if (!g_script->TrackerGet(name, rect, score, miss))
		return PyValue::None();
	PyValue m = PyValue::Dict();
	m.SetItem(PyValue("x"), PyValue(rect.left));
	m.SetItem(PyValue("y"), PyValue(rect.top));
	m.SetItem(PyValue("w"), PyValue(rect.GetWidth()));
	m.SetItem(PyValue("h"), PyValue(rect.GetHeight()));
	m.SetItem(PyValue("score"), PyValue(score));
	m.SetItem(PyValue("miss"), PyValue(miss));
	return m;
}

static PyValue builtin_tracker_set(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 2)
		return PyValue::None();
	String name = args[0].ToString();
	g_script->TraceApi("tracker_set(" + name + ")");
	Rect rect;
	if (!ParseRectValue(args[1], rect))
		return PyValue::None();
	double score = 1.0;
	if (args.GetCount() >= 3)
		score = args[2].AsDouble();
	g_script->TrackerSet(name, rect, score);
	return PyValue::None();
}

static PyValue builtin_tracker_mark_miss(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 1)
		return PyValue((int64)0);
	String name = args[0].ToString();
	g_script->TraceApi("tracker_mark_miss(" + name + ")");
	int miss = g_script->TrackerMarkMiss(name);
	return PyValue((int64)miss);
}

static PyValue builtin_tracker_clear(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 1)
		return PyValue::None();
	String name = args[0].ToString();
	g_script->TraceApi("tracker_clear(" + name + ")");
	g_script->TrackerClear(name);
	return PyValue::None();
}

static PyValue builtin_tracker_should_reset(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 2)
		return PyValue(false);
	String name = args[0].ToString();
	g_script->TraceApi("tracker_should_reset(" + name + ")");
	int miss_limit = max(1, args[1].AsInt());
	return PyValue(g_script->TrackerShouldReset(name, miss_limit));
}

static PyValue builtin_clear_result_objects(const Vector<PyValue>& args, void*) {
	if (!g_script)
		return PyValue::None();
	g_script->ClearRuntimeObjects();
	g_script->TraceApi("clear_result_objects()");
	return PyValue::None();
}

static PyValue builtin_set_result_object(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 2)
		return PyValue::None();
	String name = args[0].ToString();
	g_script->TraceApi("set_result_object(" + name + ")");
	Rect rect;
	if (!ParseRectValue(args[1], rect))
		return PyValue::None();
	g_script->SetRuntimeObject(name, rect);
	return PyValue::None();
}

static PyValue builtin_set_result_prop(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 3)
		return PyValue::None();
	String obj = args[0].ToString();
	g_script->TraceApi("set_result_prop(" + obj + "," + args[1].ToString() + ")");
	String key = args[1].ToString();
	String val = args[2].ToString();
	g_script->SetRuntimeObjectProp(obj, key, val);
	return PyValue::None();
}

static PyValue builtin_set_result_semantic(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 3)
		return PyValue::None();
	String obj = args[0].ToString();
	g_script->TraceApi("set_result_semantic(" + obj + "," + args[1].ToString() + ")");
	String key = args[1].ToString();
	String val = args[2].ToString();
	g_script->SetRuntimeObjectProp(obj, key, val);
	if (args.GetCount() >= 4)
		g_script->SetRuntimeObjectProp(obj, key + ".confidence", AsString(args[3].AsDouble()));
	if (args.GetCount() >= 5)
		g_script->SetRuntimeObjectProp(obj, key + ".method", args[4].ToString());
	return PyValue::None();
}

static PyValue builtin_compare_patch_orb(const Vector<PyValue>& args, void*) {
	PyValue out = PyValue::Dict();
	if (!g_script || !g_image || args.GetCount() < 5) {
		out.SetItem(PyValue("same"), PyValue(false));
		out.SetItem(PyValue("has_prev"), PyValue(false));
		out.SetItem(PyValue("good_matches"), PyValue((int64)0));
		out.SetItem(PyValue("matches"), PyValue((int64)0));
		out.SetItem(PyValue("status"), PyValue("bad_args"));
		return out;
	}
	String key = TrimBoth(args[0].ToString());
	Rect r;
	if (!ParseRectArgs(args, 1, r)) {
		out.SetItem(PyValue("same"), PyValue(false));
		out.SetItem(PyValue("has_prev"), PyValue(false));
		out.SetItem(PyValue("good_matches"), PyValue((int64)0));
		out.SetItem(PyValue("matches"), PyValue((int64)0));
		out.SetItem(PyValue("status"), PyValue("bad_rect"));
		return out;
	}
	int threshold = 16;
	if (args.GetCount() >= 6)
		threshold = max(1, args[5].AsInt());
	Image patch = CropRectSafe(*g_image, r);
	if (patch.IsEmpty()) {
		out.SetItem(PyValue("same"), PyValue(false));
		out.SetItem(PyValue("has_prev"), PyValue(false));
		out.SetItem(PyValue("good_matches"), PyValue((int64)0));
		out.SetItem(PyValue("matches"), PyValue((int64)0));
		out.SetItem(PyValue("status"), PyValue("empty_patch"));
		return out;
	}
	int good_matches = 0;
	int matches = 0;
	bool has_prev = g_script->CompareAndUpdateTextPatch(key, patch, threshold, good_matches, matches);
	bool same = has_prev && good_matches >= threshold;
	out.SetItem(PyValue("same"), PyValue(same));
	out.SetItem(PyValue("has_prev"), PyValue(has_prev));
	out.SetItem(PyValue("good_matches"), PyValue((int64)good_matches));
	out.SetItem(PyValue("matches"), PyValue((int64)matches));
	out.SetItem(PyValue("status"), PyValue(has_prev ? "ok" : "cold_start"));
	return out;
}

static PyValue builtin_get_tracked_text(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 1)
		return PyValue::None();
	String key = TrimBoth(args[0].ToString());
	String text, method;
	double confidence = 0.0;
	if (!g_script->GetTrackedTextValue(key, text, confidence, method))
		return PyValue::None();
	PyValue out = PyValue::Dict();
	out.SetItem(PyValue("text"), PyValue(text));
	out.SetItem(PyValue("confidence"), PyValue(confidence));
	out.SetItem(PyValue("method"), PyValue(method));
	out.SetItem(PyValue("status"), PyValue("ok"));
	return out;
}

static PyValue builtin_set_tracked_text(const Vector<PyValue>& args, void*) {
	if (!g_script || args.GetCount() < 2)
		return PyValue::None();
	String key = TrimBoth(args[0].ToString());
	String text = args[1].ToString();
	double confidence = args.GetCount() >= 3 ? args[2].AsDouble() : 0.0;
	String method = args.GetCount() >= 4 ? args[3].ToString() : String();
	g_script->SetTrackedTextValue(key, text, confidence, method);
	return PyValue::None();
}

static bool ParseRectArgs(const Vector<PyValue>& args, int start, Rect& out) {
	if (args.GetCount() >= start + 4) {
		int x = args[start + 0].AsInt();
		int y = args[start + 1].AsInt();
		int w = args[start + 2].AsInt();
		int h = args[start + 3].AsInt();
		out = RectC(x, y, w, h);
		return !out.IsEmpty();
	}
	if (args.GetCount() > start)
		return ParseRectValue(args[start], out);
	return false;
}

static String CompactOcrText(const String& s, bool preserve_newlines = false) {
	if (!preserve_newlines) {
		String out;
		bool prev_space = true;
		for (int i = 0; i < s.GetCount(); i++) {
			byte c = s[i];
			bool sp = (c == ' ' || c == '\t' || c == '\r' || c == '\n');
			if (sp) {
				if (!prev_space)
					out.Cat(' ');
				prev_space = true;
			}
			else {
				out.Cat(c);
				prev_space = false;
			}
		}
		return TrimBoth(out);
	}
	Vector<String> lines;
	String line;
	bool prev_space = true;
	for (int i = 0; i < s.GetCount(); i++) {
		byte c = s[i];
		if (c == '\r')
			continue;
		if (c == '\n') {
			String t = TrimBoth(line);
			if (!t.IsEmpty())
				lines.Add(pick(t));
			line.Clear();
			prev_space = true;
			continue;
		}
		bool sp = (c == ' ' || c == '\t');
		if (sp) {
			if (!prev_space)
				line.Cat(' ');
			prev_space = true;
		}
		else {
			line.Cat(c);
			prev_space = false;
		}
	}
	String t = TrimBoth(line);
	if (!t.IsEmpty())
		lines.Add(pick(t));
	return Join(lines, "\n");
}

static Vector<double> BuildGaussianKernel1D(double sigma) {
	int radius = max(1, (int)ceil(3.0 * sigma));
	int size = radius * 2 + 1;
	Vector<double> k;
	k.SetCount(size, 0.0);
	double sum = 0.0;
	for (int i = -radius; i <= radius; i++) {
		double v = exp(-(double)(i * i) / (2.0 * sigma * sigma));
		k[i + radius] = v;
		sum += v;
	}
	if (sum > 0.0) {
		for (int i = 0; i < size; i++)
			k[i] /= sum;
	}
	return k;
}

static void GaussianBlurGray(const Vector<byte>& src, int w, int h, double sigma, Vector<double>& out_blur) {
	out_blur.SetCount(w * h, 0.0);
	if (w <= 0 || h <= 0 || src.IsEmpty())
		return;
	Vector<double> k = BuildGaussianKernel1D(sigma);
	int radius = (k.GetCount() - 1) / 2;
	Vector<double> tmp;
	tmp.SetCount(w * h, 0.0);
	for (int y = 0; y < h; y++) {
		int row = y * w;
		for (int x = 0; x < w; x++) {
			double s = 0.0;
			for (int i = -radius; i <= radius; i++) {
				int xx = minmax(x + i, 0, w - 1);
				s += (double)src[row + xx] * k[i + radius];
			}
			tmp[row + x] = s;
		}
	}
	for (int y = 0; y < h; y++) {
		int row = y * w;
		for (int x = 0; x < w; x++) {
			double s = 0.0;
			for (int i = -radius; i <= radius; i++) {
				int yy = minmax(y + i, 0, h - 1);
				s += tmp[yy * w + x] * k[i + radius];
			}
			out_blur[row + x] = s;
		}
	}
}

static void UnsharpGrayInPlace(Vector<byte>& gray, int w, int h, double sigma, double amount, double threshold) {
	Vector<double> blur;
	GaussianBlurGray(gray, w, h, sigma, blur);
	int n = min(gray.GetCount(), blur.GetCount());
	for (int i = 0; i < n; i++) {
		double orig = (double)gray[i];
		double diff = orig - blur[i];
		if (fabs(diff) < threshold)
			continue;
		int v = (int)(orig + amount * diff + 0.5);
		gray[i] = (byte)minmax(v, 0, 255);
	}
}

struct TessRuntime {
	typedef void* TessBaseAPI;
#ifdef PLATFORM_WIN32
	HMODULE hTess = NULL;
#else
	void* hTess = NULL;
#endif
	TessBaseAPI (*TessBaseAPICreate)() = nullptr;
	int (*TessBaseAPIInit3)(TessBaseAPI, const char*, const char*) = nullptr;
	void (*TessBaseAPISetImage)(TessBaseAPI, const unsigned char*, int, int, int, int) = nullptr;
	int (*TessBaseAPISetVariable)(TessBaseAPI, const char*, const char*) = nullptr;
	void (*TessBaseAPISetPageSegMode)(TessBaseAPI, int) = nullptr;
	char* (*TessBaseAPIGetUTF8Text)(TessBaseAPI) = nullptr;
	void (*TessBaseAPIDelete)(TessBaseAPI) = nullptr;
	void (*TessDeleteText)(const char*) = nullptr;
	TessBaseAPI api = nullptr;
	bool loaded = false;
	String init_error;
	TessRuntime() {
#ifdef PLATFORM_WIN32
		String dllPath = "E:/active/msys64/clang64/bin/libtesseract-5.5.dll";
		hTess = LoadLibraryA(~dllPath);
#else
		String dllPath = "/usr/lib64/libtesseract.so";
		hTess = dlopen(~dllPath, RTLD_LAZY);
#endif
		if (!hTess) {
			init_error = "library_missing";
			return;
		}
#ifdef PLATFORM_WIN32
		#define GET_PROC(x) Tess##x = (decltype(Tess##x))GetProcAddress(hTess, "Tess" #x)
#else
		#define GET_PROC(x) Tess##x = (decltype(Tess##x))dlsym(hTess, "Tess" #x)
#endif
		GET_PROC(BaseAPICreate);
		GET_PROC(BaseAPIInit3);
		GET_PROC(BaseAPISetImage);
		GET_PROC(BaseAPISetVariable);
		GET_PROC(BaseAPISetPageSegMode);
		GET_PROC(BaseAPIGetUTF8Text);
		GET_PROC(BaseAPIDelete);
		GET_PROC(DeleteText);
#undef GET_PROC
		if (!TessBaseAPICreate || !TessBaseAPIInit3 || !TessBaseAPISetImage || !TessBaseAPIGetUTF8Text || !TessBaseAPIDelete || !TessDeleteText) {
			init_error = "symbols_missing";
			return;
		}
		api = TessBaseAPICreate();
		if (!api) {
			init_error = "api_create_failed";
			return;
		}
#ifdef PLATFORM_WIN32
		String tessDataPath = "E:/active/msys64/clang64/share/tessdata/";
#else
		String tessDataPath = "/usr/share/tessdata/";
#endif
		if (TessBaseAPIInit3(api, ~tessDataPath, "eng") != 0) {
			init_error = "api_init_failed";
			return;
		}
		loaded = true;
	}
	~TessRuntime() {
		if (api && TessBaseAPIDelete)
			TessBaseAPIDelete(api);
		if (hTess) {
#ifdef PLATFORM_WIN32
			FreeLibrary(hTess);
#else
			dlclose(hTess);
#endif
		}
	}
	String Read(const Image& img, const String& whitelist = String(), int psm = -1, bool preserve_newlines = false, bool apply_unsharp = false) {
		if (!loaded || img.IsEmpty())
			return String();
		int w = img.GetWidth();
		int h = img.GetHeight();
		int border = max(1, min(min(w, h) / 12, 4));
		double br = 0, bg = 0, bb = 0;
		int bcount = 0;
			for (int y = 0; y < h; y++) {
				const RGBA* s = img[y];
			for (int x = 0; x < w; x++) {
				bool edge = (x < border || y < border || x >= w - border || y >= h - border);
				if (!edge)
					continue;
				br += s[x].r;
				bg += s[x].g;
				bb += s[x].b;
				bcount++;
			}
		}
		if (bcount <= 0)
			bcount = 1;
		br /= bcount;
		bg /= bcount;
		bb /= bcount;
		double max_dist = 0.0;
		for (int y = 0; y < h; y++) {
			const RGBA* s = img[y];
			for (int x = 0; x < w; x++) {
				double dr = (double)s[x].r - br;
				double dg = (double)s[x].g - bg;
				double db = (double)s[x].b - bb;
				double d2 = dr * dr + dg * dg + db * db;
				if (d2 > max_dist)
					max_dist = d2;
			}
		}
		double dist_scale = sqrt(max_dist);
		if (dist_scale < 1.0)
			dist_scale = 1.0;
		Vector<byte> gray;
		gray.SetCount(w * h, 255);
		for (int y = 0; y < h; y++) {
			const RGBA* s = img[y];
			for (int x = 0; x < w; x++) {
				double dr = (double)s[x].r - br;
				double dg = (double)s[x].g - bg;
				double db = (double)s[x].b - bb;
				double dist = sqrt(dr * dr + dg * dg + db * db);
				double t = minmax(dist / dist_scale, 0.0, 1.0);
				double shaped = pow(t, 0.8);
				int v = 255 - (int)(255.0 * shaped + 0.5);
				gray[y * w + x] = (byte)minmax(v, 0, 255);
				}
			}
			if (apply_unsharp)
				UnsharpGrayInPlace(gray, w, h, 0.9, 2.0, 0.0);
		if (TessBaseAPISetVariable) {
			TessBaseAPISetVariable(api, "tessedit_char_whitelist", whitelist.IsEmpty() ? "" : ~whitelist);
		}
		if (TessBaseAPISetPageSegMode && psm >= 0)
			TessBaseAPISetPageSegMode(api, psm);
		TessBaseAPISetImage(api, gray.Begin(), w, h, 1, w);
		char* outText = TessBaseAPIGetUTF8Text(api);
		String out(outText ? outText : "");
		if (outText)
			TessDeleteText(outText);
		return CompactOcrText(out, preserve_newlines);
	}
};

static TessRuntime& GetTessRuntime() {
	static TessRuntime rt;
	return rt;
}

struct ConvNetModelRuntime {
	bool loaded = false;
	String load_status;
	int width = 0;
	int height = 0;
	int depth = 3;
	Vector<String> classes;
	ConvNet::Session session;
};

static ConvNetModelRuntime& SuitRuntime() {
	static ConvNetModelRuntime suit;
	return suit;
}

static ConvNetModelRuntime& RankRuntime() {
	static ConvNetModelRuntime rank;
	return rank;
}

static ConvNetModelRuntime& SelectConvNetRuntime(const String& tag) {
	if (tag == "suit")
		return SuitRuntime();
	return RankRuntime();
}

static void ResetConvNetRuntime(ConvNetModelRuntime& rt) {
	rt.loaded = false;
	rt.load_status.Clear();
	rt.width = 0;
	rt.height = 0;
	rt.depth = 3;
	rt.classes.Clear();
	rt.session.Clear();
}

static void ResetConvNetRuntimeCaches() {
	ResetConvNetRuntime(SuitRuntime());
	ResetConvNetRuntime(RankRuntime());
}

static String GetConvNetModelRoot() {
	String root = g_convnet_model_dir;
	if (root.IsEmpty()) {
		String env_dir = GetEnv("SCREENGAME_CONVNET_MODEL_DIR");
		if (!env_dir.IsEmpty())
			root = env_dir;
	}
	if (root.IsEmpty())
		root = AppendFileName(AppendFileName(GetCurrentDirectory(), "tmp"), "convnet_models");
	else if (!IsFullPath(root))
		root = AppendFileName(GetCurrentDirectory(), root);
	return root;
}
static PyValue builtin_set_convnet_model_dir(const Vector<PyValue>& args, void*) {
	if (args.GetCount() < 1)
		return PyValue::None();
	g_convnet_model_dir = TrimBoth(args[0].ToString());
	ResetConvNetRuntimeCaches();
	if (g_script)
		g_script->TraceApi("set_convnet_model_dir(" + g_convnet_model_dir + ")");
	return PyValue::None();
}

static bool LoadConvNetModel(const String& tag, ConvNetModelRuntime& rt) {
	if (rt.loaded)
		return true;
	rt.loaded = false;
	String root = GetConvNetModelRoot();
	String info_path = AppendFileName(root, tag + ".model.json");
	String bin_path = AppendFileName(root, tag + ".model.bin");
	if (!FileExists(info_path) || !FileExists(bin_path)) {
		info_path = AppendFileName(root, tag + ".json");
		bin_path = AppendFileName(root, tag + ".bin");
	}
	if (!FileExists(info_path) || !FileExists(bin_path)) {
		rt.load_status = "model_missing";
		return false;
	}
	try {
		Value v = ParseJSON(LoadFile(info_path));
		ValueMap m = v;
		int qi = m.Find("width");
		int qj = m.Find("height");
		if (qi >= 0) rt.width = (int)m.GetValue(qi);
		if (qj >= 0) rt.height = (int)m.GetValue(qj);
		int qd = m.Find("depth");
		if (qd >= 0) rt.depth = (int)m.GetValue(qd);
		int qc = m.Find("classes");
		if (qc >= 0) {
			ValueArray a = m.GetValue(qc);
			rt.classes.SetCount(a.GetCount());
			for (int i = 0; i < a.GetCount(); i++)
				rt.classes[i] = AsString(a[i]);
		}
	}
	catch (...) {
		rt.load_status = "model_info_parse_failed";
		return false;
	}
	if (rt.width <= 0 || rt.height <= 0 || rt.depth <= 0 || rt.classes.IsEmpty()) {
		rt.load_status = "model_info_invalid";
		return false;
	}
	FileIn in(bin_path);
	if (!in.IsOpen()) {
		rt.load_status = "model_bin_open_failed";
		return false;
	}
	in % rt.session;
	if (in.IsError()) {
		rt.load_status = "model_bin_load_failed";
		return false;
	}
	rt.loaded = true;
	rt.load_status = "ok";
	return true;
}

static void FillConvNetInput(const Image& patch, ConvNet::Volume& x, int out_w, int out_h) {
	const int src_w = patch.GetWidth();
	const int src_h = patch.GetHeight();
	double scale_x = (double)src_w / out_w;
	double scale_y = (double)src_h / out_h;
	int out_d = x.GetDepth();
	for (int y = 0; y < out_h; y++) {
		for (int xx = 0; xx < out_w; xx++) {
			int sx = min((int)(xx * scale_x), src_w - 1);
			int sy = min((int)(y * scale_y), src_h - 1);
			const RGBA& p = patch[sy][sx];
			double gray = (double)Grayscale(p) / 255.0 - 0.5;
			for (int d = 0; d < out_d; d++)
				x.Set(xx, y, d, gray);
		}
	}
}

static Vector<String> ParseLabelFilter(const PyValue& v) {
	Vector<String> out;
	if (v.GetType() == PY_LIST || v.GetType() == PY_TUPLE) {
		for (int i = 0; i < v.GetCount(); i++) {
			String s = TrimBoth(v.GetItem(i).ToString());
			if (!s.IsEmpty())
				out.Add(s);
		}
	}
	return out;
}

static String FormatTopAlternatives(const Vector<String>& cls, const ConvNet::Volume& outv, int n = 3) {
	Vector<int> idx;
	for (int i = 0; i < outv.GetLength(); i++)
		idx.Add(i);
	Sort(idx, [&](int a, int b) { return outv.Get(a) > outv.Get(b); });
	String out;
	for (int i = 0; i < min(n, idx.GetCount()); i++) {
		if (i) out << ",";
		int k = idx[i];
		String lb = (k >= 0 && k < cls.GetCount()) ? cls[k] : AsString(k);
		out << lb << ":" << Format("%.6f", outv.Get(k));
	}
	return out;
}

static PyValue builtin_classify_convnet(const Vector<PyValue>& args, void*) {
	PyValue out = PyValue::Dict();
	if (!g_image || args.GetCount() < 5) {
		out.SetItem(PyValue("label"), PyValue(""));
		out.SetItem(PyValue("confidence"), PyValue(0.0));
		out.SetItem(PyValue("method"), PyValue("convnet"));
		out.SetItem(PyValue("status"), PyValue("bad_args"));
		out.SetItem(PyValue("alternatives"), PyValue(""));
		return out;
	}
	String tag = ToLower(TrimBoth(args[0].ToString()));
	if (tag != "suit" && tag != "rank")
		tag = "rank";
	if (g_script)
		g_script->TraceApi("classify_convnet(" + tag + ")");
	Rect r;
	if (!ParseRectArgs(args, 1, r)) {
		out.SetItem(PyValue("label"), PyValue(""));
		out.SetItem(PyValue("confidence"), PyValue(0.0));
		out.SetItem(PyValue("method"), PyValue("convnet_" + tag));
		out.SetItem(PyValue("status"), PyValue("bad_rect"));
		out.SetItem(PyValue("alternatives"), PyValue(""));
		return out;
	}
	Image patch = CropRectSafe(*g_image, r);
	if (patch.IsEmpty()) {
		out.SetItem(PyValue("label"), PyValue(""));
		out.SetItem(PyValue("confidence"), PyValue(0.0));
		out.SetItem(PyValue("method"), PyValue("convnet_" + tag));
		out.SetItem(PyValue("status"), PyValue("empty_patch"));
		out.SetItem(PyValue("alternatives"), PyValue(""));
		return out;
	}
	ConvNetModelRuntime& rt = SelectConvNetRuntime(tag);
	if (!LoadConvNetModel(tag, rt)) {
		out.SetItem(PyValue("label"), PyValue(""));
		out.SetItem(PyValue("confidence"), PyValue(0.0));
		out.SetItem(PyValue("method"), PyValue("convnet_" + tag));
		out.SetItem(PyValue("status"), PyValue(rt.load_status));
		out.SetItem(PyValue("alternatives"), PyValue(""));
		return out;
	}
	ConvNet::Volume x(rt.width, rt.height, 3, 0.0);
	FillConvNetInput(patch, x, rt.width, rt.height);
	ConvNet::Net& net = rt.session.GetNetwork();
	ConvNet::Volume& pred = net.Forward(x, false);
	Vector<String> filter;
	if (args.GetCount() >= 6)
		filter = ParseLabelFilter(args[5]);
	Index<String> allowed;
	for (const String& s : filter)
		allowed.FindAdd(s);
	int best_i = -1;
	double best_v = -1;
	for (int i = 0; i < pred.GetLength(); i++) {
		String lb = (i < rt.classes.GetCount() ? rt.classes[i] : AsString(i));
		if (allowed.GetCount() && allowed.Find(lb) < 0)
			continue;
		double v = pred.Get(i);
		if (best_i < 0 || v > best_v) {
			best_i = i;
			best_v = v;
		}
	}
	if (best_i < 0) {
		for (int i = 0; i < pred.GetLength(); i++) {
			double v = pred.Get(i);
			if (best_i < 0 || v > best_v) {
				best_i = i;
				best_v = v;
			}
		}
	}
	String best_label = (best_i >= 0 && best_i < rt.classes.GetCount() ? rt.classes[best_i] : "");
	out.SetItem(PyValue("label"), PyValue(best_label));
	out.SetItem(PyValue("confidence"), PyValue(best_v < 0 ? 0.0 : best_v));
	out.SetItem(PyValue("method"), PyValue("convnet_" + tag));
	out.SetItem(PyValue("status"), PyValue(best_i >= 0 ? "ok" : "not_found"));
	out.SetItem(PyValue("alternatives"), PyValue(FormatTopAlternatives(rt.classes, pred, 3)));
	return out;
}

static PyValue builtin_read_text_backend(const Vector<PyValue>& args, void*) {
	if (!g_image || args.GetCount() < 2)
		return PyValue::None();
	TimeStop ts;
	String backend = ToLower(args[0].ToString());
	g_script->TraceApi("read_text_backend(" + backend + ")");
	Rect r;
	if (!ParseRectArgs(args, 1, r))
		return PyValue::None();
	Image patch = CropRectSafe(*g_image, r);
	if (patch.IsEmpty())
		return PyValue::None();
	String text;
	double confidence = 0.0;
	String method = backend;
	String status = "ok";
	if (backend == "tesseract" || (backend.GetCount() > 10 && backend.Left(10) == "tesseract_")) {
		String whitelist;
		int psm = -1;
		bool keep_lines = false;
		bool use_unsharp = false;
		if (backend == "tesseract_money") {
			whitelist = "0123456789$ ";
			psm = 7;
		}
		else if (backend == "tesseract_line") {
			psm = 7;
		}
		else if (backend == "tesseract_log") {
			psm = 4;
			keep_lines = true;
			use_unsharp = true;
		}
		else if (backend == "tesseract_card_index") {
			whitelist = "0123456789JQKA";
			psm = 10;
		}
		else if (backend == "tesseract_card_suit") {
			psm = 10;
		}
		TessRuntime& tess = GetTessRuntime();
		if (!tess.loaded) {
			status = tess.init_error.IsEmpty() ? "ocr_unavailable" : tess.init_error;
		}
		else {
			text = tess.Read(patch, whitelist, psm, keep_lines, use_unsharp);
			if (text.IsEmpty()) {
				Image p2 = Scale2xNearest(patch);
				text = tess.Read(p2, whitelist, psm, keep_lines, use_unsharp);
				if (!text.IsEmpty())
					status = "ok_upscaled";
			}
			confidence = text.IsEmpty() ? 0.0 : min(1.0, 0.2 + 0.05 * text.GetCount());
			if (text.IsEmpty())
				status = "empty";
			else if (status.IsEmpty())
				status = "ok";
		}
	}
	else if (backend == "convnet_text") {
		text = String();
		confidence = 0.0;
		status = "disabled";
	}
	else if (backend == "hybrid") {
		text = String();
		confidence = 0.0;
		method = "none";
		status = "not_implemented";
	}
	else {
		status = "unknown_backend";
	}
	PyValue out = PyValue::Dict();
	out.SetItem(PyValue("text"), PyValue(text));
	out.SetItem(PyValue("confidence"), PyValue(confidence));
	out.SetItem(PyValue("method"), PyValue(method));
	out.SetItem(PyValue("status"), PyValue(status));
	out.SetItem(PyValue("elapsed_us"), PyValue((int64)ts.Elapsed()));
	return out;
}

static PyValue builtin_openai_repair_text(const Vector<PyValue>& args, void*) {
	String raw = args.GetCount() > 0 ? args[0].ToString() : String();
	String example = args.GetCount() > 1 ? args[1].ToString() : String();
	String context = args.GetCount() > 2 ? args[2].ToString() : String();
	String text;
	double confidence = 0.0;
	String status;
	String err;
	bool ok = OpenAIRepairText(raw, example, context, text, confidence, status, err);
	PyValue out = PyValue::Dict();
	out.SetItem(PyValue("text"), PyValue(text));
	out.SetItem(PyValue("confidence"), PyValue(confidence));
	out.SetItem(PyValue("status"), PyValue(status));
	out.SetItem(PyValue("ok"), PyValue(ok));
	if (!err.IsEmpty())
		out.SetItem(PyValue("error"), PyValue(err));
	return out;
}

static PyValue builtin_get_strategy_advice(const Vector<PyValue>& args, void* user_data) {
	GameScript* gs = (GameScript*)user_data;
	if (!gs || args.GetCount() < 4) return PyValue::None();
	const PyValue& hole_val = args[0];
	const PyValue& board_val = args[1];
	int pot = (int)args[2].AsInt();
	const PyValue& history_val = args[3];
	Vector<int> hole;
	for (int i = 0; i < hole_val.GetCount(); i++) hole.Add((int)hole_val.GetItem(i).AsInt());
	Vector<int> board;
	for (int i = 0; i < board_val.GetCount(); i++) board.Add((int)board_val.GetItem(i).AsInt());
	Vector<byte> history;
	for (int i = 0; i < history_val.GetCount(); i++) history.Add((byte)history_val.GetItem(i).AsInt());
	static void* eval_ptr = nullptr;
	static void* strategy_ptr = nullptr;
	if (!strategy_ptr) InitStrategy(eval_ptr, strategy_ptr);
	Vector<double> probs;
	String advice = GetStrategyAdvice(hole, board, pot, history, strategy_ptr, probs);
	PyValue res = PyValue::Dict();
	res.SetItem(PyValue("advice"), PyValue(advice));
	PyValue probs_list = PyValue::List();
	for (double d : probs) probs_list.Add(PyValue(d));
	res.SetItem(PyValue("probs"), probs_list);
	return res;
}

static PyValue builtin_get_rule_ocr(const Vector<PyValue>& args, void* user_data) {
	if (!g_script || !g_image || args.GetCount() < 1) return PyValue::None();
	String rule = args[0].ToString();
	Rect r;
	double score = 0.0;
	int miss = 0;
	if (!g_script->TrackerGet(rule, r, score, miss)) {
		if (!g_script->FindBruteforce(rule, *g_image, nullptr, r, score))
			return PyValue::None();
	}
	TessRuntime& tess = GetTessRuntime();
	if (!tess.loaded) return PyValue::None();
	Image patch = CropRectSafe(*g_image, r);
	String text = tess.Read(patch);
	return PyValue(text);
}

static PyValue builtin_sg_parse_int(const Vector<PyValue>& args, void*) {
	if (args.GetCount() < 1) return PyValue(0);
	if (args[0].GetType() == PY_INT || args[0].GetType() == PY_FLOAT) return PyValue(args[0].AsInt());
	return PyValue(StrInt(args[0].ToString()));
}

static PyValue builtin_log(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() > 0) {
		String s = args[0].ToString();
		Cout() << "SCRIPT: " << s << "\n";
	}
	return PyValue::None();
}

static PyValue builtin_perform_action(const Vector<PyValue>& args, void* user_data) {
	GameScript* gs = (GameScript*)user_data;
	if (gs && gs->action_proxy && args.GetCount() > 0)
		gs->action_proxy((int)args[0].AsInt());
	return PyValue::None();
}

static PyValue builtin_get_game_type(const Vector<PyValue>& args, void*) {
	if (g_state) {
		switch(g_state->hand_type) {
			case GAME_TYPE_NLTH: return PyValue("nlth");
			case GAME_TYPE_PLO: return PyValue("plo");
			case GAME_TYPE_PLO5: return PyValue("plo5");
			case GAME_TYPE_HEARTS: return PyValue("hearts");
		}
	}
	return PyValue("nlth");
}

static PyValue builtin_get_hand_size(const Vector<PyValue>& args, void*) {
	if (g_state) {
		if (g_state->hand_type == GAME_TYPE_PLO) return PyValue((int64)4);
		if (g_state->hand_type == GAME_TYPE_PLO5) return PyValue((int64)5);
		if (g_state->hand_type == GAME_TYPE_HEARTS) return PyValue((int64)13);
	}
	return PyValue((int64)2);
}

static PyValue builtin_get_table_id(const Vector<PyValue>& args, void*) {
	if (g_state) return PyValue((int64)g_state->table_id);
	return PyValue((int64)0);
}

static PyValue builtin_get_tournament_mode(const Vector<PyValue>& args, void*) {
	if (g_state) return PyValue((int64)g_state->tournament_mode);
	return PyValue((int64)0);
}

static PyValue builtin_refresh_server_list(const Vector<PyValue>& args, void*) {
	int count = (args.GetCount() > 0) ? (int)args[0].AsInt() : 10;
	ServerListSim::GetGlobal().GenerateRandomList(count);
	return PyValue();
}

static PyValue builtin_get_server_count(const Vector<PyValue>& args, void*) {
	return PyValue((int64)ServerListSim::GetGlobal().GetCount());
}

static PyValue builtin_get_server_name(const Vector<PyValue>& args, void*) {
	int idx = (args.GetCount() > 0) ? (int)args[0].AsInt() : 0;
	if (idx >= 0 && idx < ServerListSim::GetGlobal().GetCount())
		return PyValue(ServerListSim::GetGlobal().GetGame(idx).info.name);
	return PyValue("");
}

static PyValue builtin_get_server_game_type(const Vector<PyValue>& args, void*) {
	int idx = (args.GetCount() > 0) ? (int)args[0].AsInt() : 0;
	if (idx >= 0 && idx < ServerListSim::GetGlobal().GetCount()) {
		switch(ServerListSim::GetGlobal().GetGame(idx).info.data.hand_type) {
			case GAME_TYPE_NLTH: return PyValue("nlth");
			case GAME_TYPE_PLO: return PyValue("plo");
			case GAME_TYPE_PLO5: return PyValue("plo5");
			case GAME_TYPE_HEARTS: return PyValue("hearts");
		}
	}
	return PyValue("");
}

static PyValue builtin_get_server_players(const Vector<PyValue>& args, void*) {
	int idx = (args.GetCount() > 0) ? (int)args[0].AsInt() : 0;
	if (idx >= 0 && idx < ServerListSim::GetGlobal().GetCount())
		return PyValue((int64)ServerListSim::GetGlobal().GetGame(idx).info.players.size());
	return PyValue((int64)0);
}

static PyValue builtin_get_server_max_players(const Vector<PyValue>& args, void*) {
	int idx = (args.GetCount() > 0) ? (int)args[0].AsInt() : 0;
	if (idx >= 0 && idx < ServerListSim::GetGlobal().GetCount())
		return PyValue((int64)ServerListSim::GetGlobal().GetGame(idx).info.data.maxNumberOfPlayers);
	return PyValue((int64)0);
}

static PyValue builtin_get_server_ping(const Vector<PyValue>& args, void*) {
	int idx = (args.GetCount() > 0) ? (int)args[0].AsInt() : 0;
	if (idx >= 0 && idx < ServerListSim::GetGlobal().GetCount())
		return PyValue((int64)ServerListSim::GetGlobal().GetGame(idx).ping);
	return PyValue((int64)0);
}

GameScript::GameScript() {
	vm.GetGlobals().GetAdd(PyValue("get_pot")) = PyValue::Function("get_pot", builtin_get_pot);
	vm.GetGlobals().GetAdd(PyValue("get_game_type")) = PyValue::Function("get_game_type", builtin_get_game_type);
	vm.GetGlobals().GetAdd(PyValue("get_hand_size")) = PyValue::Function("get_hand_size", builtin_get_hand_size);
	vm.GetGlobals().GetAdd(PyValue("get_table_id")) = PyValue::Function("get_table_id", builtin_get_table_id);
	vm.GetGlobals().GetAdd(PyValue("get_tournament_mode")) = PyValue::Function("get_tournament_mode", builtin_get_tournament_mode);
	vm.GetGlobals().GetAdd(PyValue("is_my_turn")) = PyValue::Function("is_my_turn", builtin_is_my_turn);
	vm.GetGlobals().GetAdd(PyValue("is_found")) = PyValue::Function("is_found", builtin_is_found);
	vm.GetGlobals().GetAdd(PyValue("get_stack")) = PyValue::Function("get_stack", builtin_get_stack);
	vm.GetGlobals().GetAdd(PyValue("get_stacks_count")) = PyValue::Function("get_stacks_count", builtin_get_stacks_count);
	vm.GetGlobals().GetAdd(PyValue("find_orb")) = PyValue::Function("find_orb", builtin_find_orb);
	vm.GetGlobals().GetAdd(PyValue("find_bruteforce")) = PyValue::Function("find_bruteforce", builtin_find_bruteforce);
	vm.GetGlobals().GetAdd(PyValue("set_window_rects")) = PyValue::Function("set_window_rects", builtin_set_window_rects);
	vm.GetGlobals().GetAdd(PyValue("set_window_rect")) = PyValue::Function("set_window_rect", builtin_set_window_rect);
	vm.GetGlobals().GetAdd(PyValue("get_rule_prop")) = PyValue::Function("get_rule_prop", builtin_get_rule_prop);
	vm.GetGlobals().GetAdd(PyValue("tracker_get")) = PyValue::Function("tracker_get", builtin_tracker_get);
	vm.GetGlobals().GetAdd(PyValue("tracker_set")) = PyValue::Function("tracker_set", builtin_tracker_set);
	vm.GetGlobals().GetAdd(PyValue("tracker_mark_miss")) = PyValue::Function("tracker_mark_miss", builtin_tracker_mark_miss);
	vm.GetGlobals().GetAdd(PyValue("tracker_clear")) = PyValue::Function("tracker_clear", builtin_tracker_clear);
	vm.GetGlobals().GetAdd(PyValue("tracker_should_reset")) = PyValue::Function("tracker_should_reset", builtin_tracker_should_reset);
	vm.GetGlobals().GetAdd(PyValue("clear_result_objects")) = PyValue::Function("clear_result_objects", builtin_clear_result_objects);
	vm.GetGlobals().GetAdd(PyValue("set_result_object")) = PyValue::Function("set_result_object", builtin_set_result_object);
	vm.GetGlobals().GetAdd(PyValue("set_result_prop")) = PyValue::Function("set_result_prop", builtin_set_result_prop);
	vm.GetGlobals().GetAdd(PyValue("set_result_semantic")) = PyValue::Function("set_result_semantic", builtin_set_result_semantic);
	vm.GetGlobals().GetAdd(PyValue("compare_patch_orb")) = PyValue::Function("compare_patch_orb", builtin_compare_patch_orb);
	vm.GetGlobals().GetAdd(PyValue("get_tracked_text")) = PyValue::Function("get_tracked_text", builtin_get_tracked_text);
	vm.GetGlobals().GetAdd(PyValue("set_tracked_text")) = PyValue::Function("set_tracked_text", builtin_set_tracked_text);
	vm.GetGlobals().GetAdd(PyValue("set_convnet_model_dir")) = PyValue::Function("set_convnet_model_dir", builtin_set_convnet_model_dir);
	vm.GetGlobals().GetAdd(PyValue("classify_convnet")) = PyValue::Function("classify_convnet", builtin_classify_convnet);
	vm.GetGlobals().GetAdd(PyValue("read_text_backend")) = PyValue::Function("read_text_backend", builtin_read_text_backend);
	vm.GetGlobals().GetAdd(PyValue("openai_repair_text")) = PyValue::Function("openai_repair_text", builtin_openai_repair_text);
	vm.GetGlobals().GetAdd(PyValue("get_rule_ocr")) = PyValue::Function("get_rule_ocr", builtin_get_rule_ocr);
	vm.GetGlobals().GetAdd(PyValue("sg_parse_int")) = PyValue::Function("sg_parse_int", builtin_sg_parse_int);
	vm.GetGlobals().GetAdd(PyValue("log")) = PyValue::Function("log", builtin_log, this);
	vm.GetGlobals().GetAdd(PyValue("perform_action")) = PyValue::Function("perform_action", builtin_perform_action, this);
	vm.GetGlobals().GetAdd(PyValue("get_strategy_advice")) = PyValue::Function("get_strategy_advice", builtin_get_strategy_advice, this);
	vm.GetGlobals().GetAdd(PyValue("refresh_server_list")) = PyValue::Function("refresh_server_list", builtin_refresh_server_list);
	vm.GetGlobals().GetAdd(PyValue("get_server_count")) = PyValue::Function("get_server_count", builtin_get_server_count);
	vm.GetGlobals().GetAdd(PyValue("get_server_name")) = PyValue::Function("get_server_name", builtin_get_server_name);
	vm.GetGlobals().GetAdd(PyValue("get_server_game_type")) = PyValue::Function("get_server_game_type", builtin_get_server_game_type);
	vm.GetGlobals().GetAdd(PyValue("get_server_players")) = PyValue::Function("get_server_players", builtin_get_server_players);
	vm.GetGlobals().GetAdd(PyValue("get_server_max_players")) = PyValue::Function("get_server_max_players", builtin_get_server_max_players);
	vm.GetGlobals().GetAdd(PyValue("get_server_ping")) = PyValue::Function("get_server_ping", builtin_get_server_ping);
}

GameScript::~GameScript() {
	vm.GetGlobals().Clear();
}

bool GameScript::Load(const String& code) {
	loaded = false;
	error = "";
	loaded_code.Clear();
	if (code.IsEmpty()) return false;
	try {
		Tokenizer tokenizer;
		tokenizer.SkipPythonComments(true);
		if (!tokenizer.Process(code, "script.py")) {
			error = "Tokenization failed";
			return false;
		}
		tokenizer.CombineTokens();
		ir.Clear();
		PyCompiler compiler(tokenizer.GetTokens());
		compiler.Compile(ir);
		vm.SetIR(ir);
		loaded = true;
		loaded_code = code;
	} catch (const Exc& e) {
		error = e;
	}
	return loaded;
}

bool GameScript::LoadAndInit(const String& code) {
	if (!Load(code))
		return false;
	text_trackers.Clear();
	try {
		vm.Run();
		loaded = true;
		ScriptLog(log_cb, "Platform script loaded");
	} catch (const Exc& e) {
		error = e;
		ScriptLog(log_cb, "Platform script error: " + error);
		loaded = false;
	}
	return loaded;
}

void GameScript::Clear() {
	vm.GetGlobals().Clear();
	ir.Clear();
	loaded = false;
	loaded_code.Clear();
	error.Clear();
	trackers.Clear();
	runtime_objects.Clear();
	text_trackers.Clear();
}

bool GameScript::LoadFile(const String& path) {
	return Load(Upp::LoadFile(path));
}

void GameScript::RunStep(const GameState& state) {
	if (!loaded) return;
	g_state = const_cast<GameState*>(&state);
	try {
		vm.Run();
	} catch (const Exc& e) {
		error = e;
		loaded = false;
	}
}

static PyValue MakeImageValue(const Image& img) {
	g_image = &img;
	g_image_ud.img = g_image;
	g_image_ud.Retain();
	return PyValue::UserDataNonOwning(&g_image_ud);
}

static bool GetDictInt(const PyValue& v, const char* key, int& out) {
	if (v.GetType() != PY_DICT) return false;
	PyValue val = v.GetItem(PyValue(key));
	if (val.GetType() == PY_INT || val.GetType() == PY_FLOAT) {
		out = (int)val.AsInt();
		return true;
	}
	return false;
}

static bool GetDictDouble(const PyValue& v, const char* key, double& out) {
	if (v.GetType() != PY_DICT) return false;
	PyValue val = v.GetItem(PyValue(key));
	if (val.GetType() == PY_INT || val.GetType() == PY_FLOAT) {
		out = val.AsDouble();
		return true;
	}
	return false;
}

static bool ParseInstance(const PyValue& v, Rect& r, double& score) {
	score = 1.0;
	if (v.GetType() == PY_DICT) {
		int x = 0, y = 0, w = 0, h = 0;
		int l = 0, t = 0, rr = 0, b = 0;
		bool has_xywh = GetDictInt(v, "x", x) && GetDictInt(v, "y", y) &&
		                GetDictInt(v, "w", w) && GetDictInt(v, "h", h);
		bool has_ltrb = GetDictInt(v, "left", l) && GetDictInt(v, "top", t) &&
		                GetDictInt(v, "right", rr) && GetDictInt(v, "bottom", b);
		if (has_xywh) {
			r = Rect(x, y, x + w, y + h);
		} else if (has_ltrb) {
			r = Rect(l, t, rr, b);
		} else {
			return false;
		}
		GetDictDouble(v, "score", score);
		return true;
	}
	if (v.GetType() == PY_LIST || v.GetType() == PY_TUPLE) {
		if (v.GetCount() < 4) return false;
		int x = (int)v.GetItem(0).AsInt();
		int y = (int)v.GetItem(1).AsInt();
		int w = (int)v.GetItem(2).AsInt();
		int h = (int)v.GetItem(3).AsInt();
		r = Rect(x, y, x + w, y + h);
		if (v.GetCount() >= 5) score = v.GetItem(4).AsDouble();
		return true;
	}
	return false;
}

bool GameScript::CallPlatformHooks(const Image& img) {
	if (!loaded || img.IsEmpty())
		return false;
	GameScript* prev_script = g_script;
	try {
		g_script = this;
		PyValue image_val = MakeImageValue(img);
		int q_find = vm.GetGlobals().Find(PyValue("find_instances"));
		if (q_find < 0) {
			ScriptLog(log_cb, "Platform script missing find_instances(image)");
			g_script = prev_script;
			return false;
		}
		int q_update = vm.GetGlobals().Find(PyValue("update_instances"));
		if (q_update < 0) {
			ScriptLog(log_cb, "Platform script missing update_instances(instances, image)");
			g_script = prev_script;
			return false;
		}
		PyValue instances = vm.Call(vm.GetGlobals()[q_find], Vector<PyValue>() << image_val);
		vm.Call(vm.GetGlobals()[q_update], Vector<PyValue>() << instances << image_val);
	} catch (const Exc& e) {
		error = e;
		ScriptLog(log_cb, "Platform script error: " + error);
		loaded = false;
		g_script = prev_script;
		return false;
	}
	g_script = prev_script;
	return true;
}

PyValue GameScript::RunCommonMain(const Image& img) {
	if (!loaded || img.IsEmpty())
		return PyValue::None();
	GameScript* prev_script = g_script;
	String prev_plat = current_platform;
	try {
		g_script = this;
		current_platform = "common";
		PyValue image_val = MakeImageValue(img);
		int q = vm.GetGlobals().Find(PyValue("common_main"));
		if (q >= 0) {
			PyValue res = vm.Call(vm.GetGlobals()[q], Vector<PyValue>() << image_val);
			g_script = prev_script;
			current_platform = prev_plat;
			return res;
		}
	} catch (const Exc& e) {
		error = e;
		ScriptLog(log_cb, "common_main error: " + error);
		g_script = prev_script;
		current_platform = prev_plat;
		return PyValue::None();
	}
	g_script = prev_script;
	current_platform = prev_plat;
	return PyValue::None();
}

bool GameScript::RunPlatformMain(const String& name, const PyValue& instance, const Image& img, const PyValue& common_state, Vector<Rect>& rects, Vector<double>& scores) {
	rects.Clear();
	scores.Clear();
	if (!loaded || img.IsEmpty())
		return false;
	GameScript* prev_script = g_script;
	String prev_plat = current_platform;
	try {
		g_script = this;
		current_platform = name;
		PyValue image_val = MakeImageValue(img);
		String ident = ScriptIdentFromName(name);
		int q = vm.GetGlobals().Find(PyValue("platform_main_" + ident));
		if (q < 0) {
			return CallNamedPlatformHooks(name, img, rects, scores);
		}
		PyValue instances = vm.Call(vm.GetGlobals()[q], Vector<PyValue>() << instance << image_val << common_state);
		if (instances.GetType() == PY_LIST || instances.GetType() == PY_TUPLE) {
			for (int i = 0; i < instances.GetCount(); i++) {
				Rect r;
				double s = 1.0;
				if (ParseInstance(instances.GetItem(i), r, s)) {
					rects.Add(r);
					scores.Add(s);
				}
			}
		}
	} catch (const Exc& e) {
		error = e;
		ScriptLog(log_cb, "platform_main_" + name + " error: " + error);
		g_script = prev_script;
		current_platform = prev_plat;
		return false;
	}
	g_script = prev_script;
	current_platform = prev_plat;
	return true;
}

bool GameScript::CallNamedPlatformHooks(const String& name, const Image& img, Vector<Rect>& rects, Vector<double>& scores) {
	rects.Clear();
	scores.Clear();
	if (!loaded || img.IsEmpty())
		return false;
	GameScript* prev_script = g_script;
	String prev_plat = current_platform;
	try {
		g_script = this;
		current_platform = name;
		PyValue image_val = MakeImageValue(img);
		String ident = ScriptIdentFromName(name);
		int q_find = vm.GetGlobals().Find(PyValue("find_instances_" + ident));
		int q_update = vm.GetGlobals().Find(PyValue("update_instances_" + ident));
		if (q_find < 0 || q_update < 0) {
			ScriptLog(log_cb, "Platform " + name + " missing hooks");
			g_script = prev_script;
			current_platform = prev_plat;
			return false;
		}
		PyValue instances = vm.Call(vm.GetGlobals()[q_find], Vector<PyValue>() << image_val);
		vm.Call(vm.GetGlobals()[q_update], Vector<PyValue>() << instances << image_val);
		if (instances.GetType() == PY_LIST || instances.GetType() == PY_TUPLE) {
			for (int i = 0; i < instances.GetCount(); i++) {
				Rect r;
				double s = 1.0;
				if (ParseInstance(instances.GetItem(i), r, s)) {
					rects.Add(r);
					scores.Add(s);
				}
			}
		}
	} catch (const Exc& e) {
		error = e;
		ScriptLog(log_cb, "Platform " + name + " hook error: " + error);
		loaded = false;
		g_script = prev_script;
		current_platform = prev_plat;
		return false;
	}
	g_script = prev_script;
	current_platform = prev_plat;
	return true;
}

int GameScript::RunPlatformTracking(const Image& img) {
	if (!loaded || img.IsEmpty())
		return 0;
	runtime_objects.Clear();
	GameScript* prev_script = g_script;
	try {
		g_script = this;
		PyValue image_val = MakeImageValue(img);
		int q_find = vm.GetGlobals().Find(PyValue("find_instances"));
		int q_update = vm.GetGlobals().Find(PyValue("update_instances"));
		if (q_find < 0 || q_update < 0) {
			ScriptLog(log_cb, "Platform script missing find_instances or update_instances");
			g_script = prev_script;
			return 0;
		}
		PyValue instances = vm.Call(vm.GetGlobals()[q_find], Vector<PyValue>() << image_val);
		vm.Call(vm.GetGlobals()[q_update], Vector<PyValue>() << instances << image_val);
		if (instances.GetType() == PY_LIST || instances.GetType() == PY_TUPLE) {
			g_script = prev_script;
			return instances.GetCount();
		}
		g_script = prev_script;
		return 0;
	} catch (const Exc& e) {
		error = e;
		ScriptLog(log_cb, "Platform script error: " + error);
		loaded = false;
		g_script = prev_script;
		return 0;
	}
}

bool GameScript::RunPlatformTracking(const Image& img, Vector<Rect>& rects, Vector<double>& scores) {
	rects.Clear();
	scores.Clear();
	if (!loaded || img.IsEmpty())
		return false;
	runtime_objects.Clear();
	GameScript* prev_script = g_script;
	try {
		g_script = this;
		PyValue image_val = MakeImageValue(img);
		int q_find = vm.GetGlobals().Find(PyValue("find_instances"));
		int q_update = vm.GetGlobals().Find(PyValue("update_instances"));
		if (q_find < 0 || q_update < 0) {
			ScriptLog(log_cb, "Platform script missing find_instances or update_instances");
			g_script = prev_script;
			return false;
		}
		PyValue instances = vm.Call(vm.GetGlobals()[q_find], Vector<PyValue>() << image_val);
		vm.Call(vm.GetGlobals()[q_update], Vector<PyValue>() << instances << image_val);
		if (instances.GetType() != PY_LIST && instances.GetType() != PY_TUPLE) {
			g_script = prev_script;
			return true;
		}
		for (int i = 0; i < instances.GetCount(); i++) {
			Rect r;
			double s = 1.0;
			if (ParseInstance(instances.GetItem(i), r, s)) {
				rects.Add(r);
				scores.Add(s);
			}
		}
	} catch (const Exc& e) {
		error = e;
		ScriptLog(log_cb, "Platform script error: " + error);
		loaded = false;
		g_script = prev_script;
		return false;
	}
	g_script = prev_script;
	return true;
}

void GameScript::TraceApi(const String& s) {
	if (!trace_api)
		return;
	String prefix = current_platform.IsEmpty() ? String() : "[" + current_platform + "] ";
	ScriptLog(log_cb, "api: " + prefix + s);
}

bool GameScript::TrackerGet(const String& name, Rect& rect, double& score, int& miss_count) const {
	int q = trackers.Find(name);
	if (q < 0)
		return false;
	const TrackerState& st = trackers[q];
	if (!st.has_rect || st.rect.IsEmpty())
		return false;
	rect = st.rect;
	score = st.score;
	miss_count = st.miss_count;
	return true;
}

void GameScript::TrackerSet(const String& name, const Rect& rect, double score) {
	if (name.IsEmpty() || rect.IsEmpty())
		return;
	TrackerState& st = trackers.GetAdd(name);
	st.rect = rect;
	st.score = score;
	st.miss_count = 0;
	st.has_rect = true;
}

int GameScript::TrackerMarkMiss(const String& name) {
	if (name.IsEmpty())
		return 0;
	TrackerState& st = trackers.GetAdd(name);
	st.miss_count++;
	return st.miss_count;
}

void GameScript::TrackerClear(const String& name) {
	int q = trackers.Find(name);
	if (q >= 0)
		trackers.Remove(q);
}

bool GameScript::TrackerShouldReset(const String& name, int miss_limit) const {
	int q = trackers.Find(name);
	if (q < 0)
		return true;
	const TrackerState& st = trackers[q];
	return st.miss_count >= max(1, miss_limit);
}

void GameScript::SetRuntimeObject(const String& name, const Rect& rect) {
	if (name.IsEmpty() || rect.IsEmpty())
		return;
	ScriptRuntimeObject& o = runtime_objects.GetAdd(name);
	o.name = name;
	o.rect = rect;
}

void GameScript::SetRuntimeObjectProp(const String& name, const String& key, const String& value) {
	if (name.IsEmpty() || key.IsEmpty())
		return;
	ScriptRuntimeObject& o = runtime_objects.GetAdd(name);
	o.name = name;
	o.props.GetAdd(key) = value;
}

void GameScript::ClearRuntimeObjects() {
	runtime_objects.Clear();
}

void GameScript::GetRuntimeObjects(Vector<ScriptRuntimeObject>& out) const {
	out.Clear();
	for (int i = 0; i < runtime_objects.GetCount(); i++) {
		ScriptRuntimeObject o;
		o.name = runtime_objects.GetKey(i);
		o.rect = runtime_objects[i].rect;
		o.props <<= runtime_objects[i].props;
		out.Add(pick(o));
	}
}

bool GameScript::CompareAndUpdateTextPatch(const String& name, const Image& patch, int good_match_threshold, int& good_matches, int& total_matches) {
	good_matches = 0;
	total_matches = 0;
	if (name.IsEmpty() || patch.IsEmpty())
		return false;
	TextTrackState& st = text_trackers.GetAdd(name);
	bool has_prev = !st.patch.IsEmpty();
	if (has_prev && st.patch.GetSize() == patch.GetSize()) {
		const RGBA* s1 = st.patch.Begin();
		const RGBA* s2 = patch.Begin();
		int len = patch.GetLength();
		int diff_pixels = 0;
		for (int i = 0; i < len; i++) {
			int dr = abs(s1[i].r - s2[i].r);
			int dg = abs(s1[i].g - s2[i].g);
			int db = abs(s1[i].b - s2[i].b);
			if (dr + dg + db > 30) diff_pixels++;
		}
		double diff_ratio = (double)diff_pixels / max(1, len);
		total_matches = 100;
		good_matches = (int)((1.0 - diff_ratio) * 100.0);
	}
	st.patch = patch;
	(void)good_match_threshold;
	return has_prev;
}

bool GameScript::GetTrackedTextValue(const String& name, String& text, double& confidence, String& method) const {
	int q = text_trackers.Find(name);
	if (q < 0)
		return false;
	const TextTrackState& st = text_trackers[q];
	if (!st.has_value)
		return false;
	text = st.text;
	confidence = st.confidence;
	method = st.method;
	return true;
}

void GameScript::SetTrackedTextValue(const String& name, const String& text, double confidence, const String& method) {
	if (name.IsEmpty())
		return;
	TextTrackState& st = text_trackers.GetAdd(name);
	st.text = text;
	st.confidence = confidence;
	st.method = method;
	st.has_value = true;
}

void GameScript::ClearTrackedTextValues() {
	for (int i = 0; i < text_trackers.GetCount(); i++) {
		text_trackers[i].has_value = false;
		text_trackers[i].text.Clear();
		text_trackers[i].confidence = 0.0;
		text_trackers[i].method.Clear();
	}
}

void GameScript::SetOpenAIKeyFile(const String& path) {
	g_openai_key_file = TrimBoth(path);
}

void GameScript::SetOpenAIModel(const String& model) {
	g_openai_model = TrimBoth(model);
}

}
