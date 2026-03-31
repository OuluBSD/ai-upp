#ifndef _MluiScript_MluiScript_h_
#define _MluiScript_MluiScript_h_

#include <Core/Core.h>

NAMESPACE_UPP

// ============================================================
// MluiScriptSlot
//
// One named region/role that should exist in an image annotated
// with this script.  bbox_hint uses normalized coordinates
// (0.0 = left/top edge, 1.0 = right/bottom edge of the image)
// so the hint stays valid across different image sizes.
// A value of Rectf(0,0,0,0) means "no hint set yet".
//
// allow_multiple: true when this UI element appears several
// times in one image (e.g. a repeating list row).
//
// Slot assignment is recorded on AnnotationObject by storing
// the key "mlui_slot_id" in metadata_keys / metadata_values.
// ============================================================

struct MluiScriptSlot : Moveable<MluiScriptSlot> {
	String slot_id;           // machine name, unique within script: "header_bar"
	String label;             // human-readable: "Header Bar"
	String category;          // matches Category::name in the project
	String hint;              // natural-language description for agents/humans
	Rectf  bbox_hint          = Rectf(0, 0, 0, 0); // normalized 0..1; (0,0,0,0) = unset
	bool   required           = false;
	bool   allow_multiple     = false;
	VectorMap<String, String> metadata; // extra key/value pairs

	MluiScriptSlot() {}
	MluiScriptSlot(const MluiScriptSlot& o) { *this = o; }
	MluiScriptSlot& operator=(const MluiScriptSlot& o) {
		slot_id = o.slot_id; label = o.label; category = o.category;
		hint = o.hint; bbox_hint = o.bbox_hint;
		required = o.required; allow_multiple = o.allow_multiple;
		metadata <<= o.metadata;
		return *this;
	}

	void Jsonize(JsonIO& jio) {
		jio("slot_id", slot_id)("label", label)("category", category)
		   ("hint", hint)("bbox_hint", bbox_hint)
		   ("required", required)("allow_multiple", allow_multiple);
		// metadata serialized as parallel arrays for simplicity
		Vector<String> mk, mv;
		if(jio.IsStoring()) {
			mk <<= metadata.GetKeys();
			mv <<= metadata.GetValues();
		}
		jio("metadata_keys", mk)("metadata_values", mv);
		if(jio.IsLoading()) {
			metadata.Clear();
			for(int i = 0; i < min(mk.GetCount(), mv.GetCount()); i++)
				metadata.Add(mk[i], mv[i]);
		}
	}
};

// ============================================================
// MluiScriptReferenceImage
//
// Records which image was used when the bbox_hints were set,
// and its pixel dimensions (so hints can be re-normalized if
// the reference changes).
// ============================================================

struct MluiScriptReferenceImage : Moveable<MluiScriptReferenceImage> {
	String file_path;
	int    width  = 0;
	int    height = 0;

	bool IsSet() const { return !file_path.IsEmpty() && width > 0 && height > 0; }

	void Jsonize(JsonIO& jio) {
		jio("file_path", file_path)("width", width)("height", height);
	}
};

// ============================================================
// MluiScript
//
// Template that declares what named regions (slots) an image
// of a particular UI layout should contain.  One .mlui file
// corresponds to one layout archetype.  A dataset image may
// reference zero or more .mlui files (via MluiScriptLink on
// ImageEntry).
//
// File extension: .mlui
// File format:    JSON (UTF-8)
// ============================================================

struct MluiScript : Moveable<MluiScript> {
	String format      = "AnnotationEditorScript";
	int    version     = 1;
	String name;
	String description;
	String author;
	Time   created;
	MluiScriptReferenceImage reference_image;
	Vector<MluiScriptSlot>   slots;

	// --- future: embedding for coarse layout recognition ---
	// Vector<double> reference_embedding;

	MluiScript() {}
	MluiScript(const MluiScript& o) { *this = o; }
	MluiScript& operator=(const MluiScript& o) {
		format = o.format; version = o.version; name = o.name;
		description = o.description; author = o.author; created = o.created;
		reference_image = o.reference_image;
		slots <<= o.slots;
		return *this;
	}

	void Jsonize(JsonIO& jio) {
		jio("format", format)("version", version)
		   ("name", name)("description", description)
		   ("author", author)("created", created)
		   ("reference_image", reference_image)
		   ("slots", slots);
	}

	// Returns index of slot with given slot_id, or -1.
	int  FindSlot(const String& slot_id) const;
	bool HasSlot(const String& slot_id) const { return FindSlot(slot_id) >= 0; }

	// Convenience: get a slot by id (asserts it exists).
	const MluiScriptSlot& GetSlot(const String& slot_id) const;
	MluiScriptSlot&       GetSlot(const String& slot_id);
};

// ============================================================
// MluiScriptLink
//
// Stored on ImageEntry (or Dataset) to associate one .mlui
// file with an image.  An image may carry several links.
// ============================================================

struct MluiScriptLink : Moveable<MluiScriptLink> {
	String script_path;   // absolute or project-relative path to .mlui file
	double confidence = 1.0; // 1.0 = manually assigned; <1 = model guess

	MluiScriptLink() {}
	MluiScriptLink(const MluiScriptLink& o) { *this = o; }
	MluiScriptLink& operator=(const MluiScriptLink& o) {
		script_path = o.script_path; confidence = o.confidence; return *this;
	}

	void Jsonize(JsonIO& jio) {
		jio("script_path", script_path)("confidence", confidence);
	}
};

// ============================================================
// Load / Save helpers
// ============================================================

bool LoadMluiScript(MluiScript& s, const String& path);
bool SaveMluiScript(const MluiScript& s, const String& path);

// ============================================================
// Metadata key convention (stored on AnnotationObject)
// ============================================================
// metadata_keys[i] == MLUI_SLOT_ID_KEY  →  slot_id in metadata_values[i]
// metadata_keys[i] == MLUI_SCRIPT_KEY   →  script_path in metadata_values[i]

inline const char* MluiSlotIdKey()  { return "mlui_slot_id"; }
inline const char* MluiScriptKey()  { return "mlui_script"; }

END_UPP_NAMESPACE

#endif

