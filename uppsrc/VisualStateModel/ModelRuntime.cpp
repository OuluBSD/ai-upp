#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmModelObject helpers

const String* VsmModelObject::GetProperty(const String& key) const
{
	for(const VsmModelProperty& p : properties)
		if(p.key == key) return &p.value_json;
	return nullptr;
}

void VsmModelObject::SetProperty(const String& key, const String& value_json)
{
	for(VsmModelProperty& p : properties)
		if(p.key == key) { p.value_json = value_json; return; }
	VsmModelProperty& np = properties.Add();
	np.key        = key;
	np.value_json = value_json;
}

// ---------------------------------------------------------------------------
// VsmModelState helpers

VsmModelObject* VsmModelState::FindObject(const VsmModelObjectId& id)
{
	for(VsmModelObject& o : objects)
		if(o.id == id) return &o;
	return nullptr;
}

const VsmModelObject* VsmModelState::FindObject(const VsmModelObjectId& id) const
{
	for(const VsmModelObject& o : objects)
		if(o.id == id) return &o;
	return nullptr;
}

// ---------------------------------------------------------------------------
// VsmModelRuntime — persistence

bool VsmModelRuntime::LoadRules(const String& json_path)
{
	String json = LoadFile(json_path);
	if(json.IsEmpty()) {
		LogError(log_, "VsmModel", "Cannot read rules: " + json_path);
		return false;
	}
	if(!LoadFromJson(rules_, json)) {
		LogError(log_, "VsmModel", "Cannot parse rules: " + json_path);
		return false;
	}
	LogInfo(log_, "VsmModel", Format("Loaded %d rules from %s",
	                                  rules_.GetCount(), json_path));
	return true;
}

bool VsmModelRuntime::SaveRules(const String& json_path) const
{
	return SaveFile(json_path, StoreAsJson(rules_, true));
}

// ---------------------------------------------------------------------------
// VsmModelRuntime — Apply event

VsmModelRuntimeResult VsmModelRuntime::ApplyEvent(const VsmModelEvent& ev)
{
	VsmModelRuntimeResult result;
	for(const VsmModelRule& r : rules_) {
		// Route by event type
		bool applies = false;
		switch(r.type) {
		case VSM_MR_SET_PROP_FROM_OCR:
			applies = (ev.type == "ocr_result" && r.source_rule_id == ev.source_rule_id);
			break;
		case VSM_MR_SET_PROP_FROM_TEMPLATE:
			applies = (ev.type == "template_match" && r.source_rule_id == ev.source_rule_id);
			break;
		case VSM_MR_CREATE_OBJECT:
			applies = (ev.type == "region_appeared" && r.region_id == ev.source_region_id);
			break;
		case VSM_MR_MARK_INACTIVE:
			applies = (ev.type == "region_disappeared" && r.region_id == ev.source_region_id);
			break;
		case VSM_MR_VALIDATE_PROP:
			applies = (ev.type == "ocr_result" || ev.type == "template_match") &&
			          (r.source_rule_id == ev.source_rule_id);
			break;
		}
		if(!applies) continue;

		switch(r.type) {
		case VSM_MR_SET_PROP_FROM_OCR:      ApplySetPropFromOcr(r, ev, result);      break;
		case VSM_MR_SET_PROP_FROM_TEMPLATE: ApplySetPropFromTemplate(r, ev, result); break;
		case VSM_MR_CREATE_OBJECT:          ApplyCreateObject(r, ev, result);        break;
		case VSM_MR_MARK_INACTIVE:          ApplyMarkInactive(r, ev, result);        break;
		case VSM_MR_VALIDATE_PROP:          ApplyValidateProp(r, ev, result);        break;
		}
	}
	// Persist transitions and divergences
	for(const VsmModelTransition& t : result.transitions)
		history_.Add(t);
	for(const VsmDivergence& d : result.divergences)
		divergences_.Add(d);
	return result;
}

VsmModelObject& VsmModelRuntime::GetOrCreateObject(const VsmModelObjectId& id,
                                                    const String& type)
{
	VsmModelObject* obj = state_.FindObject(id);
	if(obj) return *obj;
	VsmModelObject& no = state_.objects.Add();
	no.id     = id;
	no.type   = type;
	no.active = true;
	LogInfo(log_, "VsmModel", "Created object '" + id + "' type=" + type);
	return no;
}

VsmModelTransition VsmModelRuntime::MakeTransition(const VsmModelRule& r,
                                                    const String& from,
                                                    const String& to,
                                                    const VsmModelEvent& ev)
{
	VsmModelTransition t;
	t.object_id           = r.object_id;
	t.property_key        = r.property_key;
	t.from_value          = from;
	t.to_value            = to;
	t.trigger_event_type  = ev.type;
	t.recorded_at         = ev.ts;
	return t;
}

void VsmModelRuntime::ApplySetPropFromOcr(const VsmModelRule& r, const VsmModelEvent& ev,
                                           VsmModelRuntimeResult& out)
{
	VsmModelObject& obj = GetOrCreateObject(r.object_id, "ui_element");
	const String* old   = obj.GetProperty(r.property_key);
	String old_val      = old ? *old : String();
	// data_json is the OCR text (plain string in double-quotes for JSON)
	String new_val      = ev.data_json;
	obj.SetProperty(r.property_key, new_val);
	out.transitions.Add(MakeTransition(r, old_val, new_val, ev));
	LogInfo(log_, "VsmModel", Format("SetPropOCR '%s'.%s = %s",
	                                  r.object_id, r.property_key, new_val));
}

void VsmModelRuntime::ApplySetPropFromTemplate(const VsmModelRule& r,
                                                const VsmModelEvent& ev,
                                                VsmModelRuntimeResult& out)
{
	VsmModelObject& obj = GetOrCreateObject(r.object_id, "ui_element");
	const String* old   = obj.GetProperty(r.property_key);
	String old_val      = old ? *old : String();
	String new_val      = ev.data_json;
	obj.SetProperty(r.property_key, new_val);
	out.transitions.Add(MakeTransition(r, old_val, new_val, ev));
	LogInfo(log_, "VsmModel", Format("SetPropTemplate '%s'.%s = %s",
	                                  r.object_id, r.property_key, new_val));
}

void VsmModelRuntime::ApplyCreateObject(const VsmModelRule& r, const VsmModelEvent& ev,
                                         VsmModelRuntimeResult& out)
{
	GetOrCreateObject(r.object_id, "ui_region");
	out.transitions.Add(MakeTransition(r, "inactive", "active", ev));
}

void VsmModelRuntime::ApplyMarkInactive(const VsmModelRule& r, const VsmModelEvent& ev,
                                         VsmModelRuntimeResult& out)
{
	VsmModelObject* obj = state_.FindObject(r.object_id);
	if(obj && obj->active) {
		obj->active = false;
		out.transitions.Add(MakeTransition(r, "active", "inactive", ev));
		LogInfo(log_, "VsmModel", "Marked '" + r.object_id + "' inactive");
	}
}

void VsmModelRuntime::ApplyValidateProp(const VsmModelRule& r, const VsmModelEvent& ev,
                                         VsmModelRuntimeResult& out)
{
	const VsmModelObject* obj = state_.FindObject(r.object_id);
	if(!obj) {
		out.warnings.Add("ValidateProp: object not found: " + r.object_id);
		return;
	}
	const String* val = obj->GetProperty(r.property_key);
	String actual     = val ? *val : String();
	if(actual != r.expected_value) {
		VsmDivergence& div  = out.divergences.Add();
		div.ts              = ev.ts;
		div.severity        = "warning";
		div.region_id       = ev.source_region_id;
		div.message         = Format("Property '%s' expected %s got %s",
		                              r.property_key, r.expected_value, actual);
		div.expected_json   = "\"" + r.expected_value + "\"";
		div.observed_json   = "\"" + actual + "\"";
		LogWarn(log_, "VsmModel", div.message);
	} else {
		LogInfo(log_, "VsmModel", Format("ValidateProp '%s'.%s = %s OK",
		                                  r.object_id, r.property_key, actual));
	}
}

// ---------------------------------------------------------------------------
// VsmModelRuntime::SaveDivergenceReport

bool VsmModelRuntime::SaveDivergenceReport(const String& path)
{
	String json = StoreAsJson(divergences_, true);
	if(!SaveFile(path, json)) {
		LogError(log_, "ModelRuntime", "Cannot write divergences: " + path);
		return false;
	}
	LogInfo(log_, "ModelRuntime", Format("Divergences saved: %d record(s) → %s",
	                                      divergences_.GetCount(), path));
	return true;
}

} // namespace Upp
