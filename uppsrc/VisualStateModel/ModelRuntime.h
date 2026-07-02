#ifndef _VisualStateModel_ModelRuntime_h_
#define _VisualStateModel_ModelRuntime_h_

namespace Upp {

typedef String VsmModelObjectId;

// ---------------------------------------------------------------------------
// Model data structures

struct VsmModelProperty : Moveable<VsmModelProperty> {
	String key, value_json;
	void Jsonize(JsonIO& json) { json("key",key)("value_json",value_json); }
};

struct VsmModelObject : Moveable<VsmModelObject> {
	VsmModelObjectId       id;
	String                 type;
	bool                   active = true;
	Vector<VsmModelProperty> properties;
	void Jsonize(JsonIO& json) {
		json("id",id)("type",type)("active",active)("properties",properties);
	}
	const String* GetProperty(const String& key) const;
	void SetProperty(const String& key, const String& value_json);
};

struct VsmModelState : Moveable<VsmModelState> {
	Vector<VsmModelObject> objects;
	void Jsonize(JsonIO& json) { json("objects",objects); }
	VsmModelObject*       FindObject(const VsmModelObjectId& id);
	const VsmModelObject* FindObject(const VsmModelObjectId& id) const;
};

struct VsmModelEvent : Moveable<VsmModelEvent> {
	String type; // "ocr_result", "template_match", "region_appeared", "region_disappeared"
	String ts;
	String source_region_id;
	String source_rule_id;
	String data_json; // serialised observation payload
	void Jsonize(JsonIO& json) {
		json("type",type)("ts",ts)("source_region_id",source_region_id)
		    ("source_rule_id",source_rule_id)("data_json",data_json);
	}
};

struct VsmModelTransition : Moveable<VsmModelTransition> {
	String object_id, property_key;
	String from_value, to_value;
	String trigger_event_type;
	String recorded_at;
	void Jsonize(JsonIO& json) {
		json("object_id",object_id)("property_key",property_key)
		    ("from_value",from_value)("to_value",to_value)
		    ("trigger_event_type",trigger_event_type)("recorded_at",recorded_at);
	}
};

// ---------------------------------------------------------------------------
// Rule types (minimal explicit set — no scripting)

enum VsmModelRuleType {
	VSM_MR_SET_PROP_FROM_OCR       = 0, // copy OCR text → object property
	VSM_MR_SET_PROP_FROM_TEMPLATE  = 1, // copy template label → object property
	VSM_MR_CREATE_OBJECT           = 2, // create/activate object when region appears
	VSM_MR_MARK_INACTIVE           = 3, // mark object inactive when region disappears
	VSM_MR_VALIDATE_PROP           = 4, // check property matches expected value
};

struct VsmModelRule : Moveable<VsmModelRule> {
	String rule_id;
	int    type = VSM_MR_SET_PROP_FROM_OCR;
	String object_id;          // target object
	String property_key;       // property to set/validate
	String expected_value;     // for VALIDATE
	String source_rule_id;     // which OCR/template rule feeds this
	String region_id;          // for CREATE/MARK_INACTIVE
	void Jsonize(JsonIO& json) {
		json("rule_id",rule_id)("type",type)("object_id",object_id)
		    ("property_key",property_key)("expected_value",expected_value)
		    ("source_rule_id",source_rule_id)("region_id",region_id);
	}
};

// ---------------------------------------------------------------------------
// Runtime result

struct VsmModelRuntimeResult : Moveable<VsmModelRuntimeResult> {
	Vector<VsmModelTransition> transitions;
	Vector<VsmDivergence>      divergences;
	Vector<String>             warnings;
};

// ---------------------------------------------------------------------------
// VsmModelRuntime

class VsmModelRuntime {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Load/save rule definitions
	bool LoadRules(const String& json_path);
	bool SaveRules(const String& json_path) const;

	// Direct rule manipulation
	void AddRule(const VsmModelRule& r) { rules_.Add(r); }
	const Vector<VsmModelRule>& GetRules() const { return rules_; }

	// Apply a model event to the current state.
	// Returns any transitions and divergences triggered.
	VsmModelRuntimeResult ApplyEvent(const VsmModelEvent& ev);

	// Access current state
	const VsmModelState&             GetState()       const { return state_;       }
	const Vector<VsmModelTransition>& GetHistory()    const { return history_;     }
	const Vector<VsmDivergence>&      GetDivergences() const { return divergences_; }

	void Reset() { state_ = VsmModelState(); history_.Clear(); divergences_.Clear(); }

	// Write divergences_ as a JSON array to path.
	bool SaveDivergenceReport(const String& path);

private:
	CoreLog                    log_;
	Vector<VsmModelRule>       rules_;
	VsmModelState              state_;
	Vector<VsmModelTransition> history_;
	Vector<VsmDivergence>      divergences_;

	void ApplySetPropFromOcr      (const VsmModelRule& r, const VsmModelEvent& ev,
	                               VsmModelRuntimeResult& out);
	void ApplySetPropFromTemplate (const VsmModelRule& r, const VsmModelEvent& ev,
	                               VsmModelRuntimeResult& out);
	void ApplyCreateObject        (const VsmModelRule& r, const VsmModelEvent& ev,
	                               VsmModelRuntimeResult& out);
	void ApplyMarkInactive        (const VsmModelRule& r, const VsmModelEvent& ev,
	                               VsmModelRuntimeResult& out);
	void ApplyValidateProp        (const VsmModelRule& r, const VsmModelEvent& ev,
	                               VsmModelRuntimeResult& out);

	VsmModelObject& GetOrCreateObject(const VsmModelObjectId& id, const String& type = "unknown");
	VsmModelTransition MakeTransition(const VsmModelRule& r, const String& from,
	                                   const String& to, const VsmModelEvent& ev);
};

} // namespace Upp

#endif
