#ifndef _CtrlLib_Mlui_h_
#define _CtrlLib_Mlui_h_

struct MluiSnapshotOptions {
	bool include_hidden = false;
	bool include_layout = true;
	bool include_visible_text = true;
	bool include_visible_text_ratio = true;
	bool include_focus = true;
};

struct MluiFocusAction : Moveable<MluiFocusAction> {
	String id;
	String label;
	String hint;
	bool   enabled = true;
};

struct MluiFocusPage {
	String id;
	String title;
	String description;

	Vector<String> linked_ctrls;
	Vector<MluiFocusAction> actions;
	VectorMap<String, Value> static_context;

	VectorMap<String, Value> runtime_values;
	VectorMap<String, String> runtime_value_desc;
	VectorMap<String, String> runtime_ctrls;
	VectorMap<String, String> runtime_ctrl_desc;
	VectorMap<String, bool> runtime_action_enabled;
	VectorMap<String, String> runtime_action_desc;

	VectorMap<String, Function<Value(const ValueMap&)>> action_handlers;

	MluiFocusPage& Add(Ctrl& c);
	MluiFocusPage& Action(const String& action_id, const String& label, const String& hint, bool enabled = true);
	MluiFocusPage& ActionHandler(const String& action_id, Function<Value(const ValueMap&)> handler);
	MluiFocusPage& Context(const String& key, const Value& value);

	MluiFocusPage& ClearRuntime();
	MluiFocusPage& AddValue(const String& key, const Value& value, const String& description);
	MluiFocusPage& AddCtrl(const String& key, const Ctrl& ctrl, const String& description);
	MluiFocusPage& AddState(const String& key, const Value& value, const String& description);
	MluiFocusPage& AddAction(const String& action_id, bool enabled, const String& description);
};

MluiFocusPage& RegisterMluiFocusPage(const String& id, const String& title, const String& description);
MluiFocusPage& GetMluiFocusPage(const String& id);
bool           HasMluiFocusPage(const String& id);
Value          BuildMluiFocusList();
Value          BuildMluiFocusPage(const String& id);
Value          BuildMluiFocusTree(const String& id = String(), int depth = 2);
Value          SearchMluiFocus(const String& query, const String& page_filter = String(), int limit = 64);
bool           PerformMluiFocusAction(const String& page_id, const String& action_id, const ValueMap& args,
                                      Value& out_result, String& out_error);

namespace MLUI {

using FocusAction = MluiFocusAction;
using FocusPage = MluiFocusPage;

inline FocusPage& RegisterFocusPage(const String& id, const String& title, const String& description)
{
	return RegisterMluiFocusPage(id, title, description);
}

inline FocusPage& GetFocusPage(const String& id)
{
	return GetMluiFocusPage(id);
}

inline bool HasFocusPage(const String& id)
{
	return HasMluiFocusPage(id);
}

inline Value BuildFocusList()
{
	return BuildMluiFocusList();
}

inline Value BuildFocusPage(const String& id)
{
	return BuildMluiFocusPage(id);
}

inline Value BuildFocusTree(const String& id = String(), int depth = 2)
{
	return BuildMluiFocusTree(id, depth);
}

inline Value SearchFocus(const String& query, const String& page_filter = String(), int limit = 64)
{
	return SearchMluiFocus(query, page_filter, limit);
}

inline bool PerformFocusAction(const String& page_id, const String& action_id, const ValueMap& args,
                               Value& out_result, String& out_error)
{
	return PerformMluiFocusAction(page_id, action_id, args, out_result, out_error);
}

}

#ifndef MLUI_USE_VAR
#define MLUI_USE_VAR(page, var, description) \
	(page).AddValue(#var, Value(var), (description))
#endif

#ifndef MLUI_USE_CTRL
#define MLUI_USE_CTRL(page, ctrl, description) \
	(page).AddCtrl(#ctrl, (ctrl), (description))
#endif

#ifndef MLUI_USE_STATE
#define MLUI_USE_STATE(page, key, value, description) \
	(page).AddState((key), Value(value), (description))
#endif

#ifndef MLUI_USE_ACTION
#define MLUI_USE_ACTION(page, action_id, enabled, description) \
	(page).AddAction((action_id), (enabled), (description))
#endif

Value  BuildMluiSnapshot(const MluiSnapshotOptions& options = MluiSnapshotOptions());
String HandleMluiJsonRequest(const String& request_json);
void   StartMluiRuntime();
void   RegisterMluiRuntimeStarter();

#endif
