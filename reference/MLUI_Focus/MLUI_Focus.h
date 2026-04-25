#ifndef _MLUI_Focus_MLUI_Focus_h_
#define _MLUI_Focus_MLUI_Focus_h_

#include <CtrlLib/CtrlLib.h>
#include <CtrlLib/GuiAutomation.h>

NAMESPACE_UPP

namespace MLUIRef {

struct FocusPageDef : Moveable<FocusPageDef> {
	String id;
	String title;
	String summary;
	Vector<String> route_ids;
	Vector<String> component_ids;
	Vector<String> action_ids;
	VectorMap<String, Value> runtime_values;
	VectorMap<String, String> runtime_value_desc;
	VectorMap<String, String> runtime_ctrls;
	VectorMap<String, String> runtime_ctrl_desc;
	VectorMap<String, bool> runtime_action_enabled;
	VectorMap<String, String> runtime_action_desc;

	FocusPageDef& Route(const String& route_id);
	FocusPageDef& Component(const String& component_id);
	FocusPageDef& Action(const String& action_id);
	FocusPageDef& ClearRuntime();
	FocusPageDef& AddValue(const String& key, const Value& value, const String& description);
	FocusPageDef& AddCtrl(const String& key, const Ctrl& ctrl, const String& description);
	FocusPageDef& AddState(const String& key, const Value& value, const String& description);
	FocusPageDef& AddAction(const String& action_id, bool enabled, const String& description);
};

struct FocusRouteDef : Moveable<FocusRouteDef> {
	String id;
	String path;
	String page_id;
	String summary;
	bool   is_default = false;
};

struct FocusSiteMapDef : Moveable<FocusSiteMapDef> {
	String id;
	String title;
	Vector<String> route_ids;

	FocusSiteMapDef& Route(const String& route_id);
};

struct FocusLinkDef : Moveable<FocusLinkDef> {
	String id;
	String from_route;
	String to_route;
	String label;
	String condition;
};

struct FocusQueryDef : Moveable<FocusQueryDef> {
	String id;
	String route_id;
	String param;
	String default_value;
	String summary;
};

struct FocusComponentDef : Moveable<FocusComponentDef> {
	String id;
	String page_id;
	String title;
	String summary;
	Vector<String> payload_keys;

	FocusComponentDef& Key(const String& payload_key);
};

struct FocusFormFieldDef : Moveable<FocusFormFieldDef> {
	String id;
	String type;
	bool   required = false;
	String summary;
};

struct FocusFormDef : Moveable<FocusFormDef> {
	String id;
	String page_id;
	String action_id;
	Vector<FocusFormFieldDef> fields;

	FocusFormDef& Field(const String& id, const String& type, bool required, const String& summary);
};

struct FocusActionContractDef : Moveable<FocusActionContractDef> {
	String id;
	String page_id;
	String summary;
	String input_schema;
	String output_schema;
	Vector<String> side_effects;

	FocusActionContractDef& SideEffect(const String& effect);
};

struct FocusStateDef : Moveable<FocusStateDef> {
	String id;
	String page_id;
	VectorMap<String, Value> values;

	FocusStateDef& Set(const String& key, const Value& value);
};

struct FocusHistoryEntry : Moveable<FocusHistoryEntry> {
	String from_route;
	String to_route;
	String action_id;
	Time   stamp;
};

struct FocusDiffOp : Moveable<FocusDiffOp> {
	String component_id;
	String op;
	String path;
	Value  value;
	Time   stamp;
};

struct FocusPermissionDef : Moveable<FocusPermissionDef> {
	String id;
	String action_id;
	bool   enabled = true;
	String disabled_reason;
};

struct FocusDevtoolsEntry : Moveable<FocusDevtoolsEntry> {
	String channel;
	String level;
	String message;
	Time   stamp;
};

FocusPageDef&           RegisterFocusPage(const String& id, const String& title, const String& summary);
FocusPageDef&           GetFocusPage(const String& id);
FocusRouteDef&          RegisterFocusRoute(const String& id, const String& path, const String& page_id, const String& summary, bool is_default = false);
FocusSiteMapDef&        RegisterFocusSiteMap(const String& id, const String& title);
FocusLinkDef&           RegisterFocusLink(const String& id, const String& from_route_id, const String& to_route_id, const String& label, const String& condition = String());
FocusQueryDef&          RegisterFocusQuery(const String& id, const String& route_id, const String& param, const String& default_value, const String& summary);
FocusComponentDef&      RegisterFocusComponent(const String& id, const String& page_id, const String& title, const String& summary);
FocusFormDef&           RegisterFocusForm(const String& id, const String& page_id, const String& action_id);
FocusActionContractDef& RegisterFocusActionContract(const String& id, const String& page_id, const String& summary, const String& input_schema, const String& output_schema);
FocusStateDef&          RegisterFocusState(const String& id, const String& page_id);
FocusStateDef&          GetFocusState(const String& id);

void                    PushFocusHistory(const String& from_route, const String& to_route, const String& action_id);
void                    PushFocusDiff(const String& component_id, const String& op, const String& path, const Value& value);
void                    SetFocusPermission(const String& action_id, bool enabled, const String& disabled_reason = String());
void                    PushFocusDevtools(const String& channel, const String& level, const String& message);

void                    EmitAllFocusArtifacts(AutomationVisitor& av);

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

class MLUIFocusReferenceApp : public TopWindow {
public:
	typedef MLUIFocusReferenceApp CLASSNAME;

	MLUIFocusReferenceApp();
	virtual bool Access(Visitor& v) override;

private:
	void PopulateBoard();
	void OnBoardSelection();
	void OnStatusChange();
	void OnAssigneeChange();
	void RefreshEditorFromSelection();

	Splitter   split;
	ArrayCtrl  board;
	ParentCtrl editor;

	Label      route_label;
	EditString route_value;
	Label      issue_label;
	EditString issue_key;
	Label      title_label;
	EditString issue_title;
	Label      status_label;
	DropList   status;
	Label      assignee_label;
	EditString assignee;
	Option     blocked;
	DocEdit    notes;
};

END_UPP_NAMESPACE

#endif
