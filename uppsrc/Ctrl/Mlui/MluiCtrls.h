#ifndef _Ctrl_Mlui_MluiCtrls_h_
#define _Ctrl_Mlui_MluiCtrls_h_

#include <CtrlLib/CtrlLib.h>
#include <Core/MluiScript/MluiScript.h>

NAMESPACE_UPP

// ============================================================
// SetGeometryDialog
//
// Lets the user specify or modify the geometry of an
// AnnotationObject without drawing it on canvas.  Supports:
//   - Bounding box (x, y, w, h in pixels)
//   - Polygon (paste/type point list: "x1,y1  x2,y2 ...")
//
// Usage:
//   SetGeometryDialog dlg;
//   dlg.SetImageSize(img.GetWidth(), img.GetHeight());
//   dlg.SetFromBBox(obj.bbox);          // pre-fill when editing
//   if(dlg.Run() == IDOK) {
//       obj.polygons.Clear();
//       obj.polygons.Add(dlg.GetPolygon());
//       obj.UpdateBBox();
//   }
// ============================================================

class SetGeometryDialog : public TopWindow {
public:
	typedef SetGeometryDialog CLASSNAME;
	SetGeometryDialog();

	// Pre-fill from an existing bounding box.
	void SetFromBBox(const Rectf& r);

	// Pre-fill from an existing polygon.
	void SetFromPolygon(const Vector<Pointf>& poly);

	// Set image dimensions for validation and normalization hints.
	void SetImageSize(int w, int h);

	// Returns the geometry as a polygon (4-point rect for bbox mode,
	// N-point polygon for polygon mode).
	// Returns empty vector on parse failure.
	Vector<Pointf> GetPolygon() const;

	// Returns the axis-aligned bounding box of the result.
	Rectf GetBBox() const;

private:
	void OnModeChange();
	void OnBBoxChange();

	bool ParsePolyText(const String& text, Vector<Pointf>& out) const;

	Option    mode_bbox, mode_poly;
	Label     lbl_bbox, lbl_x, lbl_y, lbl_w, lbl_h;
	EditDouble edit_x, edit_y, edit_w, edit_h;
	Label     lbl_poly;
	DocEdit   edit_poly;
	Label     lbl_hint;
	Button    btn_ok, btn_cancel;

	int img_w = 0, img_h = 0;
};

// ============================================================
// SlotMetaPanel
//
// Embedded panel for editing a single MluiScriptSlot's fields.
// Used inside MluiScriptEditor (right pane).
// ============================================================

class SlotMetaPanel : public ParentCtrl {
public:
	typedef SlotMetaPanel CLASSNAME;
	SlotMetaPanel();

	void   SetCategories(const Vector<String>& cat_names);
	void   Load(const MluiScriptSlot& slot);
	void   Save(MluiScriptSlot& slot) const;
	void   Clear();

	Callback WhenChange;

private:
	void FireChange() { WhenChange(); }

	Label      lbl_id, lbl_label, lbl_category, lbl_hint;
	Label      lbl_bbox, lbl_bx, lbl_by, lbl_bw, lbl_bh;
	EditString edit_id, edit_label, edit_hint;
	DropList   drop_category;
	EditDouble edit_bx, edit_by, edit_bw, edit_bh;
	Option     chk_required, chk_multiple;
	Label      lbl_meta;
	ArrayCtrl  meta_list;
	Button     btn_meta_add, btn_meta_del;
};

// ============================================================
// MluiScriptEditor
//
// Full dialog for creating and editing .mlui script files.
//
// Layout:
//   [toolbar: file path | Browse | Load | Save]
//   [left: slot list]  [right: SlotMetaPanel]
//   [bottom: Add Slot | Remove | Move Up | Move Down |
//            Copy hints from image | Apply to image | Close]
//
// The "Copy hints from current image" action requires the
// caller to provide current annotation data via:
//   editor.SetReferenceAnnotations(objects, img_w, img_h, img_path)
//
// The "Apply to image" action produces a list of new objects
// that the caller should insert into the active ImageEntry:
//   editor.WhenApply = [=](const Vector<AnnotationObject>& objs) { ... }
// ============================================================

class MluiScriptEditor : public TopWindow {
public:
	typedef MluiScriptEditor CLASSNAME;
	MluiScriptEditor();

	// Provide the category names from the current project so the
	// slot editor can offer a drop-list.
	void SetCategoryNames(const Vector<String>& names);

	// Provide current image bounding boxes so "Copy hints from image"
	// can derive normalized bbox_hints.
	// Each entry is {slot_id, normalized_bbox}.  If slot_id is empty the
	// entry is skipped (caller must pre-resolve slot_ids).
	struct RefAnnotEntry : Moveable<RefAnnotEntry> { String slot_id; Rectf bbox; };
	void SetReferenceAnnotations(const Vector<RefAnnotEntry>& entries,
	                             int img_w, int img_h,
	                             const String& img_path);

	// Load an existing script into the editor.
	void LoadScript(const MluiScript& s);

	// Return a copy of the script as currently edited.
	MluiScript GetScript() const;

	// Called when the user clicks "Apply to image".
	// Delivers the current script together with the target image size so
	// the caller (AnnotationEditor) can build AnnotationObject stubs itself.
	// This keeps MluiCtrls free of Dataset.h types.
	Function<void(const MluiScript& script, int img_w, int img_h)> WhenApply;

	// Tell the editor the pixel dimensions of the image currently open
	// so Apply can report them to WhenApply.
	void SetCurrentImageSize(int w, int h);

private:
	void OnBrowse();
	void OnLoad();
	void OnSave();
	void OnSlotSel();
	void OnAddSlot();
	void OnRemoveSlot();
	void OnMoveUp();
	void OnMoveDown();
	void OnCopyHints();
	void OnApply();

	void RefreshSlotList();
	void FlushCurrentSlot();   // write SlotMetaPanel back to current slot
	int  CurrentSlotIndex() const;

	// ---- file bar ----
	Label      lbl_file;
	EditString edit_file;
	Button     btn_browse, btn_load, btn_save;

	// ---- script header ----
	Label      lbl_name, lbl_desc;
	EditString edit_name, edit_desc;

	// ---- main splitter ----
	Splitter   split;
	ArrayCtrl  slot_list;
	SlotMetaPanel slot_panel;

	// ---- bottom buttons ----
	Button     btn_add, btn_remove, btn_up, btn_down;
	Button     btn_copy_hints, btn_apply, btn_close;

	// ---- data ----
	MluiScript script_;
	Vector<String> category_names_;

	Vector<RefAnnotEntry> ref_annots_;
	int ref_img_w_ = 0, ref_img_h_ = 0;

	// current image size for Apply
	int cur_img_w_ = 0, cur_img_h_ = 0;

	bool loading_ = false; // suppress change callbacks during load
};

END_UPP_NAMESPACE

#endif

