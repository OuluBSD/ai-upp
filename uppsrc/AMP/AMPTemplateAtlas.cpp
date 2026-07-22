#include "AMP.h"

NAMESPACE_UPP

static Value AmpAtlasEntryToValue(const AmpTemplateAtlasEntry& entry)
{
	ValueMap value;
	value.Add("id", entry.id);
	value.Add("kind", entry.kind);
	value.Add("scale", entry.scale);
	value.Add("x", entry.x);
	value.Add("y", entry.y);
	value.Add("width", entry.width);
	value.Add("height", entry.height);
	value.Add("preprocessing", entry.preprocessing);
	value.Add("rotation_min", entry.rotation_min);
	value.Add("rotation_max", entry.rotation_max);
	value.Add("rotation_step", entry.rotation_step);
	value.Add("threshold", entry.threshold);
	return value;
}

bool AmpTemplateAtlasManifest::Validate(String& error) const
{
	if(atlas_width <= 0 || atlas_height <= 0) {
		error = "atlas dimensions must be positive";
		return false;
	}
	Index<String> ids;
	for(int i = 0; i < entries.GetCount(); i++) {
		const AmpTemplateAtlasEntry& entry = entries[i];
		if(entry.id.IsEmpty() || entry.kind.IsEmpty() || entry.scale.IsEmpty()) {
			error = Format("entry %d has incomplete identity", i);
			return false;
		}
		if(ids.Find(entry.id) >= 0) {
			error = Format("duplicate entry id: %s", ~entry.id);
			return false;
		}
		ids.Add(entry.id);
		if(entry.x < 0 || entry.y < 0 || entry.width <= 0 || entry.height <= 0 ||
		   entry.x + entry.width > atlas_width || entry.y + entry.height > atlas_height) {
			error = Format("entry %s is outside atlas bounds", ~entry.id);
			return false;
		}
		if(entry.rotation_step < 0 || entry.rotation_min > entry.rotation_max) {
			error = Format("entry %s has invalid rotation metadata", ~entry.id);
			return false;
		}
	}
	return true;
}

bool AmpTemplateAtlasManifest::Save(const String& path) const
{
	String error;
	if(!Validate(error)) {
		COUTLOG(Format("amp_atlas_manifest_invalid error=%s", ~error));
		return false;
	}
	ValueArray array;
	for(const AmpTemplateAtlasEntry& entry : entries)
		array.Add(AmpAtlasEntryToValue(entry));
	ValueMap root;
	root.Add("format", format);
	root.Add("atlas_name", atlas_name);
	root.Add("atlas_width", atlas_width);
	root.Add("atlas_height", atlas_height);
	root.Add("entries", array);
	bool ok = SaveFile(path, AsJSON(root, true));
	COUTLOG(Format("amp_atlas_manifest_saved path=%s entries=%d status=%s", ~path,
	               entries.GetCount(), ok ? "pass" : "fail"));
	return ok;
}

bool AmpTemplateAtlasManifest::Load(const String& path, String& error)
{
	Value root_value = ParseJSON(LoadFile(path));
	if(root_value.IsError() || !root_value.Is<ValueMap>()) {
		error = "atlas manifest root is not a JSON object";
		return false;
	}
	ValueMap root = root_value;
	format = AsString(root.Get("format", Value()));
	atlas_name = AsString(root.Get("atlas_name", Value()));
	atlas_width = (int)root.Get("atlas_width", 0);
	atlas_height = (int)root.Get("atlas_height", 0);
	entries.Clear();
	ValueArray array = root.Get("entries", ValueArray());
	for(int i = 0; i < array.GetCount(); i++) {
		if(!array[i].Is<ValueMap>()) {
			error = Format("entry %d is not a JSON object", i);
			return false;
		}
		ValueMap value = array[i];
		AmpTemplateAtlasEntry& entry = entries.Add();
		entry.id = AsString(value.Get("id", Value()));
		entry.kind = AsString(value.Get("kind", Value()));
		entry.scale = AsString(value.Get("scale", Value()));
		entry.x = (int)value.Get("x", 0);
		entry.y = (int)value.Get("y", 0);
		entry.width = (int)value.Get("width", 0);
		entry.height = (int)value.Get("height", 0);
		entry.preprocessing = AsString(value.Get("preprocessing", Value()));
		entry.rotation_min = (int)value.Get("rotation_min", 0);
		entry.rotation_max = (int)value.Get("rotation_max", 0);
		entry.rotation_step = (int)value.Get("rotation_step", 0);
		entry.threshold = (float)value.Get("threshold", 0.0);
	}
	if(!Validate(error))
		return false;
	COUTLOG(Format("amp_atlas_manifest_loaded path=%s entries=%d", ~path, entries.GetCount()));
	return true;
}

END_UPP_NAMESPACE
