#include "MluiCtrls.h"

NAMESPACE_UPP

// ============================================================
// SetGeometryDialog
// ============================================================

SetGeometryDialog::SetGeometryDialog() {
	Title("Set Geometry");
	SetRect(0, 0, 420, 340);

	Add(mode_bbox.SetLabel("Bounding box").LeftPos(10, 120).TopPos(10, 20));
	Add(mode_poly.SetLabel("Polygon").LeftPos(140, 100).TopPos(10, 20));
	mode_bbox.Set(1);

	// BBox row labels + editors
	Add(lbl_x.SetText("X:").LeftPos(10, 25).TopPos(44, 20));
	Add(edit_x.LeftPos(35, 70).TopPos(42, 22));
	Add(lbl_y.SetText("Y:").LeftPos(120, 25).TopPos(44, 20));
	Add(edit_y.LeftPos(145, 70).TopPos(42, 22));
	Add(lbl_w.SetText("W:").LeftPos(230, 25).TopPos(44, 20));
	Add(edit_w.LeftPos(255, 70).TopPos(42, 22));
	Add(lbl_h.SetText("H:").LeftPos(340, 25).TopPos(44, 20));
	Add(edit_h.LeftPos(360, 50).TopPos(42, 22));

	// Polygon editor
	Add(lbl_poly.SetText("Points (x1,y1  x2,y2 ...):").LeftPos(10, 220).TopPos(44, 20));
	Add(edit_poly.LeftPos(10, 395).TopPos(66, 180));

	Add(lbl_hint.LeftPos(10, 395).TopPos(252, 40));
	lbl_hint.SetText("Hint: separate points with spaces or newlines.\n"
	                 "Comma between x and y: \"10,20  30,40\"");

	Add(btn_ok.RightPos(90, 80).BottomPos(8, 25));
	Add(btn_cancel.RightPos(5,  80).BottomPos(8, 25));
	btn_ok.SetLabel("OK"); btn_ok << [=] { AcceptBreak(IDOK); };
	btn_cancel.SetLabel("Cancel"); btn_cancel << [=] { RejectBreak(IDCANCEL); };

	mode_bbox << [=] { OnModeChange(); };
	mode_poly << [=] { OnModeChange(); };
	edit_x.WhenAction << [=] { OnBBoxChange(); };
	edit_y.WhenAction << [=] { OnBBoxChange(); };
	edit_w.WhenAction << [=] { OnBBoxChange(); };
	edit_h.WhenAction << [=] { OnBBoxChange(); };

	OnModeChange();
}

void SetGeometryDialog::OnModeChange() {
	bool bbox = mode_bbox.Get();
	lbl_x.Show(bbox); edit_x.Show(bbox);
	lbl_y.Show(bbox); edit_y.Show(bbox);
	lbl_w.Show(bbox); edit_w.Show(bbox);
	lbl_h.Show(bbox); edit_h.Show(bbox);
	lbl_poly.Show(!bbox); edit_poly.Show(!bbox);
	lbl_hint.Show(!bbox);
}

void SetGeometryDialog::OnBBoxChange() {
	// could update a live preview in future
}

void SetGeometryDialog::SetFromBBox(const Rectf& r) {
	mode_bbox.Set(1); OnModeChange();
	edit_x.SetData(r.left);
	edit_y.SetData(r.top);
	edit_w.SetData(r.Width());
	edit_h.SetData(r.Height());
}

void SetGeometryDialog::SetFromPolygon(const Vector<Pointf>& poly) {
	mode_poly.Set(1); OnModeChange();
	String s;
	for(int i = 0; i < poly.GetCount(); i++) {
		if(i) s << "\n";
		s << Format("%.4g,%.4g", poly[i].x, poly[i].y);
	}
	edit_poly.SetData(s);
}

void SetGeometryDialog::SetImageSize(int w, int h) {
	img_w = w; img_h = h;
}

bool SetGeometryDialog::ParsePolyText(const String& text, Vector<Pointf>& out) const {
	out.Clear();
	// Accept: "x,y" pairs separated by whitespace or semicolons
	String t = text;
	for(int ci = 0; ci < t.GetCount(); ci++) {
		char c = t[ci];
		if(c == ';' || c == '\n' || c == '\r') t.Set(ci, ' ');
	}
	CParser p(t);
	while(!p.IsEof()) {
		double x, y;
		if(p.IsDouble()) x = p.ReadDouble(); else { p.SkipTerm(); continue; }
		if(p.IsChar(',')) p.PassChar(',');
		if(p.IsDouble()) y = p.ReadDouble(); else break;
		out.Add(Pointf(x, y));
	}
	return out.GetCount() >= 2;
}

Vector<Pointf> SetGeometryDialog::GetPolygon() const {
	if(mode_bbox.Get()) {
		double x = edit_x.GetData(), y = edit_y.GetData();
		double w = edit_w.GetData(), h = edit_h.GetData();
		Vector<Pointf> r;
		r.Add(Pointf(x,     y));
		r.Add(Pointf(x + w, y));
		r.Add(Pointf(x + w, y + h));
		r.Add(Pointf(x,     y + h));
		return r;
	} else {
		Vector<Pointf> out;
		ParsePolyText(edit_poly.GetData().ToString(), out);
		return out;
	}
}

Rectf SetGeometryDialog::GetBBox() const {
	Vector<Pointf> poly = GetPolygon();
	if(poly.IsEmpty()) return Rectf(0, 0, 0, 0);
	double x0 = poly[0].x, y0 = poly[0].y, x1 = x0, y1 = y0;
	for(const auto& p : poly) {
		x0 = min(x0, p.x); y0 = min(y0, p.y);
		x1 = max(x1, p.x); y1 = max(y1, p.y);
	}
	return Rectf(x0, y0, x1, y1);
}

// ============================================================
// SlotMetaPanel
// ============================================================

SlotMetaPanel::SlotMetaPanel() {
	int y = 0;
	auto row = [&](Label& lbl, const char* txt, Ctrl& c, int lw = 80, int ch = 22) {
		Add(lbl.SetText(txt).LeftPos(5, lw).TopPos(y + 2, 20));
		Add(c.LeftPos(5 + lw, 180).TopPos(y, ch));
		y += ch + 4;
	};

	row(lbl_id,       "Slot ID:",   edit_id);
	row(lbl_label,    "Label:",     edit_label);
	row(lbl_category, "Category:",  drop_category);
	row(lbl_hint,     "Hint:",      edit_hint, 80, 44);
	y += 22; // extra space after hint

	Add(chk_required.SetLabel("Required").HSizePos(5, 5).TopPos(y, 20));
	y += 24;
	Add(chk_multiple.SetLabel("Allow multiple instances").HSizePos(5, 5).TopPos(y, 20));
	y += 28;

	Add(lbl_bbox.SetText("BBox hint (normalized 0..1):").HSizePos(5, 5).TopPos(y, 20));
	y += 22;
	Add(lbl_bx.SetText("X:").LeftPos(5,  20).TopPos(y + 2, 18));
	Add(edit_bx      .LeftPos(25, 60).TopPos(y, 22));
	Add(lbl_by.SetText("Y:").LeftPos(95, 20).TopPos(y + 2, 18));
	Add(edit_by      .LeftPos(115, 60).TopPos(y, 22));
	Add(lbl_bw.SetText("W:").LeftPos(185, 20).TopPos(y + 2, 18));
	Add(edit_bw      .LeftPos(205, 60).TopPos(y, 22));
	Add(lbl_bh.SetText("H:").LeftPos(275, 20).TopPos(y + 2, 18));
	Add(edit_bh      .LeftPos(295, 60).TopPos(y, 22));
	y += 28;

	Add(lbl_meta.SetText("Metadata:").HSizePos(5, 5).TopPos(y, 20));
	y += 22;
	Add(meta_list.HSizePos(5, 5).TopPos(y, 80));
	meta_list.AddColumn("Key", 100).Edit(meta_edit_key);
	meta_list.AddColumn("Value", 140).Edit(meta_edit_val);
	meta_list.SetLineCy(20);
	meta_list.Appending().Removing();
	y += 84;
	Add(btn_meta_add.SetLabel("Add").LeftPos(5,  60).TopPos(y, 22));
	Add(btn_meta_del.SetLabel("Remove").LeftPos(70, 70).TopPos(y, 22));

	auto fire = [=] { FireChange(); };
	edit_id.WhenAction     << fire;
	edit_label.WhenAction  << fire;
	drop_category.WhenAction << fire;
	edit_hint.WhenAction   << fire;
	chk_required.WhenAction << fire;
	chk_multiple.WhenAction << fire;
	edit_bx.WhenAction << fire; edit_by.WhenAction << fire;
	edit_bw.WhenAction << fire; edit_bh.WhenAction << fire;

	btn_meta_add << [=] {
		int row = meta_list.GetCount();
		meta_list.Add("key", "value");
		meta_list.SetCursor(row);
		meta_list.StartEdit(0);
		FireChange();
	};
	btn_meta_del << [=] {
		if(meta_list.IsCursor()) { meta_list.Remove(meta_list.GetCursor()); FireChange(); }
	};
}

void SlotMetaPanel::SetCategories(const Vector<String>& names) {
	String cur = drop_category.GetCount() > 0 ? drop_category.GetData().ToString() : String();
	drop_category.Clear();
	drop_category.Add("", "(none)");
	for(const String& n : names) drop_category.Add(n, n);
	drop_category.SetData(cur); // SetData selects by key; no-op if not found
}

void SlotMetaPanel::Load(const MluiScriptSlot& s) {
	edit_id.SetData(s.slot_id);
	edit_label.SetData(s.label);
	drop_category.SetData(s.category);
	edit_hint.SetData(s.hint);
	chk_required.Set(s.required);
	chk_multiple.Set(s.allow_multiple);
	edit_bx.SetData(s.bbox_hint.left);
	edit_by.SetData(s.bbox_hint.top);
	edit_bw.SetData(s.bbox_hint.Width());
	edit_bh.SetData(s.bbox_hint.Height());
	meta_list.Clear();
	for(int i = 0; i < s.metadata.GetCount(); i++)
		meta_list.Add(s.metadata.GetKey(i), s.metadata[i]);
}

void SlotMetaPanel::Save(MluiScriptSlot& s) const {
	s.slot_id      = edit_id.GetData().ToString();
	s.label        = edit_label.GetData().ToString();
	s.category     = drop_category.GetData().ToString();
	s.hint         = edit_hint.GetData().ToString();
	s.required     = chk_required.Get();
	s.allow_multiple = chk_multiple.Get();
	double bx = edit_bx.GetData(), by = edit_by.GetData();
	double bw = edit_bw.GetData(), bh = edit_bh.GetData();
	s.bbox_hint    = Rectf(bx, by, bx + bw, by + bh);
	s.metadata.Clear();
	for(int i = 0; i < meta_list.GetCount(); i++)
		s.metadata.Add(meta_list.Get(i, 0).ToString(), meta_list.Get(i, 1).ToString());
}

void SlotMetaPanel::Clear() {
	edit_id.Clear(); edit_label.Clear(); edit_hint.Clear();
	drop_category.SetIndex(0);
	chk_required.Set(0); chk_multiple.Set(0);
	edit_bx.SetData(0); edit_by.SetData(0);
	edit_bw.SetData(0); edit_bh.SetData(0);
	meta_list.Clear();
}

// ============================================================
// MluiScriptEditor
// ============================================================

MluiScriptEditor::MluiScriptEditor() {
	Title("MLUI Script Editor"); Sizeable().Zoomable();
	SetRect(0, 0, 860, 680);

	// ---- file bar ----
	Add(lbl_file.SetText("Script:").LeftPos(5, 50).TopPos(8, 20));
	Add(edit_file.LeftPos(55, 500).TopPos(6, 22));
	Add(btn_browse.SetLabel("...").LeftPos(560, 30).TopPos(6, 22));
	Add(btn_load  .SetLabel("Load").LeftPos(596, 60).TopPos(6, 22));
	Add(btn_save  .SetLabel("Save").LeftPos(661, 60).TopPos(6, 22));

	// ---- script header ----
	Add(lbl_name.SetText("Name:").LeftPos(5, 50).TopPos(34, 20));
	Add(edit_name.LeftPos(55, 300).TopPos(32, 22));
	Add(lbl_desc.SetText("Description:").LeftPos(370, 85).TopPos(34, 20));
	Add(edit_desc.LeftPos(460, 385).TopPos(32, 22));

	// ---- slot list (left) + slot panel (right) ----
	Add(split.VSizePos(62, 160).HSizePos(5, 5));
	split.Horz(slot_list, slot_panel);
	split.SetPos(3000); // 30% left

	// ---- category manager (below splitter, right side) ----
	Add(lbl_cats_hdr.SetText("Script Categories:").RightPos(5, 580).BottomPos(118, 18));
	Add(cat_list.RightPos(5, 580).BottomPos(60, 56));
	cat_list.AddColumn("Name"); cat_list.SetLineCy(18);
	Add(edit_new_cat.RightPos(70, 500).BottomPos(35, 22));
	Add(btn_add_cat.SetLabel("+").RightPos(5, 60).BottomPos(35, 22));
	Add(btn_del_cat.SetLabel("-").RightPos(5, 580).BottomPos(35, 22));

	slot_list.AddColumn("Slot ID", 120);
	slot_list.AddColumn("Label", 120);
	slot_list.AddColumn("Category", 80);
	slot_list.AddColumn("Req", 35);
	slot_list.WhenSel = THISBACK(OnSlotSel);

	// ---- bottom buttons ----
	Add(btn_add         .SetLabel("Add Slot")            .LeftPos(5,   90).BottomPos(8, 25));
	Add(btn_remove      .SetLabel("Remove")              .LeftPos(100, 70).BottomPos(8, 25));
	Add(btn_up          .SetLabel("Up")                  .LeftPos(175, 50).BottomPos(8, 25));
	Add(btn_down        .SetLabel("Down")                .LeftPos(230, 60).BottomPos(8, 25));
	Add(btn_copy_hints  .SetLabel("Copy hints from image").LeftPos(300, 150).BottomPos(8, 25));
	Add(btn_apply       .SetLabel("Apply to image")       .LeftPos(460, 120).BottomPos(8, 25));
	Add(btn_close       .SetLabel("Close")                .RightPos(5,  80).BottomPos(8, 25));

	btn_browse      << THISBACK(OnBrowse);
	btn_load        << THISBACK(OnLoad);
	btn_save        << THISBACK(OnSave);
	btn_add         << THISBACK(OnAddSlot);
	btn_remove      << THISBACK(OnRemoveSlot);
	btn_up          << THISBACK(OnMoveUp);
	btn_down        << THISBACK(OnMoveDown);
	btn_copy_hints  << THISBACK(OnCopyHints);
	btn_apply       << THISBACK(OnApply);
	btn_close       << [=] { Close(); };

	btn_add_cat << [=] {
		String n = edit_new_cat.GetData().ToString().IsEmpty()
		           ? edit_new_cat.GetData().ToString() : edit_new_cat.GetData().ToString();
		n = edit_new_cat.GetData().ToString();
		if(n.IsEmpty()) return;
		bool found = false;
		for(const String& s : script_categories_) if(s == n) { found = true; break; }
		if(!found) {
			script_categories_.Add(n);
			cat_list.Add(n);
			RefreshCategoryDroplist();
		}
		edit_new_cat.Clear();
	};
	btn_del_cat << [=] {
		if(!cat_list.IsCursor()) return;
		int i = cat_list.GetCursor();
		script_categories_.Remove(i);
		cat_list.Remove(i);
		RefreshCategoryDroplist();
	};

	slot_panel.WhenChange = THISBACK(FlushCurrentSlot);
}

void MluiScriptEditor::RefreshCategoryDroplist() {
	cat_list.Clear();
	for(const String& n : script_categories_) cat_list.Add(n);
	slot_panel.SetCategories(script_categories_);
}

void MluiScriptEditor::SetCategoryNames(const Vector<String>& names) {
	category_names_ <<= names;
	for(const String& n : names) {
		bool found = false;
		for(const String& s : script_categories_) if(s == n) { found = true; break; }
		if(!found) script_categories_.Add(n);
	}
	RefreshCategoryDroplist();
}

void MluiScriptEditor::SetReferenceAnnotations(const Vector<RefAnnotEntry>& entries,
                                               int img_w, int img_h,
                                               const String& img_path) {
	ref_annots_.Clear();
	ref_img_w_ = img_w; ref_img_h_ = img_h;
	for(const auto& e : entries)
		if(!e.slot_id.IsEmpty() && e.bbox.Width() > 0 && e.bbox.Height() > 0)
			ref_annots_.Add(e);

	if(!img_path.IsEmpty() && !script_.reference_image.IsSet()) {
		script_.reference_image.file_path = img_path;
		script_.reference_image.width  = img_w;
		script_.reference_image.height = img_h;
	}
}

void MluiScriptEditor::SetCurrentImageSize(int w, int h) {
	cur_img_w_ = w; cur_img_h_ = h;
}

void MluiScriptEditor::LoadScript(const MluiScript& s) {
	loading_ = true;
	script_ = s;
	edit_name.SetData(s.name);
	edit_desc.SetData(s.description);
	// Merge slot categories into script_categories_
	for(const auto& slot : s.slots) {
		if(!slot.category.IsEmpty()) {
			bool found = false;
			for(const String& c : script_categories_) if(c == slot.category) { found = true; break; }
			if(!found) script_categories_.Add(slot.category);
		}
	}
	RefreshCategoryDroplist();
	RefreshSlotList();
	if(slot_list.GetCount() > 0) { slot_list.SetCursor(0); OnSlotSel(); }
	loading_ = false;
}

MluiScript MluiScriptEditor::GetScript() const {
	MluiScript s = script_;
	s.name        = edit_name.GetData().ToString();
	s.description = edit_desc.GetData().ToString();
	return s;
}

void MluiScriptEditor::RefreshSlotList() {
	int cur = slot_list.IsCursor() ? slot_list.GetCursor() : -1;
	slot_list.Clear();
	for(const auto& slot : script_.slots) {
		slot_list.Add(slot.slot_id, slot.label, slot.category,
		              slot.required ? "yes" : "");
	}
	if(cur >= 0 && cur < slot_list.GetCount()) slot_list.SetCursor(cur);
}

int MluiScriptEditor::CurrentSlotIndex() const {
	return slot_list.IsCursor() ? slot_list.GetCursor() : -1;
}

void MluiScriptEditor::FlushCurrentSlot() {
	if(loading_) return;
	int i = CurrentSlotIndex();
	if(i < 0 || i >= script_.slots.GetCount()) return;
	slot_panel.Save(script_.slots[i]);
	// Refresh list row text without changing cursor
	auto& s = script_.slots[i];
	slot_list.Set(i, 0, s.slot_id);
	slot_list.Set(i, 1, s.label);
	slot_list.Set(i, 2, s.category);
	slot_list.Set(i, 3, s.required ? "yes" : "");
}

void MluiScriptEditor::OnSlotSel() {
	int i = CurrentSlotIndex();
	if(i < 0 || i >= script_.slots.GetCount()) { slot_panel.Clear(); return; }
	loading_ = true;
	slot_panel.Load(script_.slots[i]);
	loading_ = false;
}

void MluiScriptEditor::OnAddSlot() {
	FlushCurrentSlot();
	MluiScriptSlot s;
	s.slot_id = Format("slot_%d", script_.slots.GetCount() + 1);
	s.label   = "New Slot";
	script_.slots.Add(s);
	RefreshSlotList();
	slot_list.SetCursor(script_.slots.GetCount() - 1);
	OnSlotSel();
}

void MluiScriptEditor::OnRemoveSlot() {
	int i = CurrentSlotIndex();
	if(i < 0) return;
	if(!PromptYesNo("Remove slot \"" + script_.slots[i].slot_id + "\"?")) return;
	script_.slots.Remove(i);
	RefreshSlotList();
	if(script_.slots.GetCount() > 0)
		slot_list.SetCursor(min(i, script_.slots.GetCount() - 1));
	OnSlotSel();
}

void MluiScriptEditor::OnMoveUp() {
	int i = CurrentSlotIndex();
	if(i <= 0) return;
	FlushCurrentSlot();
	Swap(script_.slots[i], script_.slots[i - 1]);
	RefreshSlotList();
	slot_list.SetCursor(i - 1);
}

void MluiScriptEditor::OnMoveDown() {
	int i = CurrentSlotIndex();
	if(i < 0 || i >= script_.slots.GetCount() - 1) return;
	FlushCurrentSlot();
	Swap(script_.slots[i], script_.slots[i + 1]);
	RefreshSlotList();
	slot_list.SetCursor(i + 1);
}

void MluiScriptEditor::OnBrowse() {
	FileSel fs;
	fs.Type("MLUI Script (*.mlui)", "*.mlui");
	fs.DefaultExt("mlui");
	// Browse is used both for load and save contexts — use open dialog (non-destructive)
	if(fs.ExecuteOpen()) edit_file.SetData(fs.Get());
}

void MluiScriptEditor::OnLoad() {
	String path = edit_file.GetData().ToString();
	if(path.IsEmpty()) { OnBrowse(); path = edit_file.GetData().ToString(); }
	if(path.IsEmpty()) return;
	MluiScript s;
	if(!LoadMluiScript(s, path)) { PromptOK("Failed to load: " + path); return; }
	LoadScript(s);
}

void MluiScriptEditor::OnSave() {
	FlushCurrentSlot();
	String path = edit_file.GetData().ToString();
	if(path.IsEmpty()) { OnBrowse(); path = edit_file.GetData().ToString(); }
	if(path.IsEmpty()) return;
	MluiScript s = GetScript();
	if(!SaveMluiScript(s, path)) PromptOK("Failed to save: " + path);
}

void MluiScriptEditor::OnCopyHints() {
	// For each slot in the script, look for a reference annotation whose
	// slot_id matches (or whose bbox we can use as a hint).
	if(ref_img_w_ <= 0 || ref_img_h_ <= 0) {
		PromptOK("No reference image annotations available.\n"
		         "Open and annotate an image first, then re-open this dialog.");
		return;
	}

	int updated = 0;
	for(auto& slot : script_.slots) {
		for(const auto& ra : ref_annots_) {
			// Match by slot_id if set; otherwise skip (don't guess)
			if(ra.slot_id.IsEmpty() || ra.slot_id != slot.slot_id) continue;
			// Normalize bbox to 0..1
			slot.bbox_hint = Rectf(
				ra.bbox.left   / ref_img_w_,
				ra.bbox.top    / ref_img_h_,
				ra.bbox.right  / ref_img_w_,
				ra.bbox.bottom / ref_img_h_);
			updated++;
			break;
		}
	}

	// Update reference_image record
	if(!ref_annots_.IsEmpty()) {
		// reference_image already set via SetReferenceAnnotations
	}

	RefreshSlotList();
	// Reload current slot panel so bbox hint fields update
	OnSlotSel();
	PromptOK(Format("Updated bbox hints for %d slot(s).\n\n"
	                "Tip: save the script to persist the hints.", updated));
}

void MluiScriptEditor::OnApply() {
	if(!WhenApply) return;
	if(cur_img_w_ <= 0 || cur_img_h_ <= 0) {
		PromptOK("No image is currently open.\nOpen an image before applying the script.");
		return;
	}
	FlushCurrentSlot();
	// Deliver the script + image dimensions; caller builds AnnotationObject stubs.
	WhenApply(GetScript(), cur_img_w_, cur_img_h_);
}

END_UPP_NAMESPACE

