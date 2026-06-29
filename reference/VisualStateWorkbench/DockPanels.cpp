#include "MainWindow.h"

// ---------------------------------------------------------------------------
// RegionPropsPanel

RegionPropsPanel::RegionPropsPanel()
{
	id_lbl_.SetLabel("ID: —");
	rect_lbl_.SetLabel("Rect: —");
	fp_lbl_.SetLabel("Fingerprint: —");
	action_lbl_.SetLabel("Action: —");

	Add(id_lbl_.HSizePos(4, 4).TopPos(4, 20));
	Add(rect_lbl_.HSizePos(4, 4).TopPos(28, 20));
	Add(action_lbl_.HSizePos(4, 4).TopPos(52, 20));
	Add(fp_lbl_.HSizePos(4, 4).TopPos(76, 20));
}

void RegionPropsPanel::SetRegion(const String& id, const VsmRegionNode* node)
{
	if(!node) { Clear(); return; }
	id_lbl_.SetLabel("ID: " + id);
	String rect = Format("Rect: (%d,%d) ", node->x, node->y) + IntStr(node->w) + "x" + IntStr(node->h);
	rect_lbl_.SetLabel(rect);
	action_lbl_.SetLabel("Action: " + node->action);
	fp_lbl_.SetLabel("Fingerprint: " +
	                  (node->fingerprint.hash.IsEmpty() ? String("—") : node->fingerprint.hash));
}

void RegionPropsPanel::Clear()
{
	id_lbl_.SetLabel("ID: —");
	rect_lbl_.SetLabel("Rect: —");
	fp_lbl_.SetLabel("Fingerprint: —");
	action_lbl_.SetLabel("Action: —");
}

// ---------------------------------------------------------------------------
// ReplayTimelinePanel

ReplayTimelinePanel::ReplayTimelinePanel()
{
	step_btn_.SetLabel("Step");
	run_btn_.SetLabel("Run All");
	reset_btn_.SetLabel("Reset");
	progress_lbl_.SetLabel("0 / 0");

	step_btn_.WhenAction  = [=] { WhenStep(); };
	run_btn_.WhenAction   = [=] { WhenRunAll(); };
	reset_btn_.WhenAction = [=] { WhenReset(); };

	Add(reset_btn_.LeftPos(4, 70).TopPos(4, 24));
	Add(step_btn_.LeftPos(78, 70).TopPos(4, 24));
	Add(run_btn_.LeftPos(152, 70).TopPos(4, 24));
	Add(progress_lbl_.HSizePos(230, 4).TopPos(4, 24));
}

void ReplayTimelinePanel::SetProgress(int pos, int total)
{
	progress_lbl_.SetLabel(Format("%d / %d", pos, total));
}

// ---------------------------------------------------------------------------
// AnnotationEditorPanel

AnnotationEditorPanel::AnnotationEditorPanel()
{
	list_.AddColumn("ID", 100);
	list_.AddColumn("Name");
	list_.AddColumn("Parent", 80);
	list_.WhenSel = [=] { OnSel(); };

	name_lbl_.SetLabel("Name:");
	parent_lbl_.SetLabel("Parent:");
	rect_lbl_.SetLabel("Rect x/y/w/h:");

	create_btn_.SetLabel("New");
	delete_btn_.SetLabel("Delete");
	save_btn_.SetLabel("Apply");

	create_btn_.WhenAction = [=] { OnCreate(); };
	delete_btn_.WhenAction = [=] { OnDelete(); };
	save_btn_.WhenAction   = [=] { OnSave(); };

	// List
	Add(list_.HSizePos(4,4).TopPos(4,120));

	// Fields
	Add(name_lbl_.LeftPos(4,40).TopPos(130,20));
	Add(name_edit_.HSizePos(48,4).TopPos(130,20));
	Add(parent_lbl_.LeftPos(4,44).TopPos(156,20));
	Add(parent_edit_.HSizePos(52,4).TopPos(156,20));
	Add(rect_lbl_.HSizePos(4,4).TopPos(182,20));
	Add(x_edit_.LeftPos(4,36).TopPos(206,20));
	Add(y_edit_.LeftPos(44,36).TopPos(206,20));
	Add(w_edit_.LeftPos(84,36).TopPos(206,20));
	Add(h_edit_.LeftPos(124,36).TopPos(206,20));

	// Buttons
	Add(create_btn_.LeftPos(4,60).BottomPos(4,24));
	Add(delete_btn_.LeftPos(68,60).BottomPos(4,24));
	Add(save_btn_.RightPos(4,60).BottomPos(4,24));
}

void AnnotationEditorPanel::SetLayer(VsmAnnotationLayer* layer)
{
	layer_ = layer;
	RebuildList();
}

void AnnotationEditorPanel::RebuildList()
{
	list_.Clear();
	if(!layer_) return;
	for(const VsmRegionAnnotation& a : layer_->annotations)
		list_.Add(a.id, a.name, a.parent_id.IsEmpty() ? "—" : a.parent_id);
}

void AnnotationEditorPanel::OnSel()
{
	if(!layer_) return;
	int row = list_.GetCursor();
	if(row < 0 || row >= layer_->annotations.GetCount()) {
		FillFields(nullptr);
		return;
	}
	FillFields(&layer_->annotations[row]);
}

void AnnotationEditorPanel::FillFields(const VsmRegionAnnotation* a)
{
	if(!a) {
		name_edit_.Clear();
		parent_edit_.Clear();
		x_edit_ = 0; y_edit_ = 0; w_edit_ = 100; h_edit_ = 40;
		return;
	}
	name_edit_.SetData(a->name);
	parent_edit_.SetData(a->parent_id);
	x_edit_ = a->x; y_edit_ = a->y; w_edit_ = a->w; h_edit_ = a->h;
}

void AnnotationEditorPanel::ApplyFields()
{
	if(!layer_) return;
	int row = list_.GetCursor();
	if(row < 0 || row >= layer_->annotations.GetCount()) return;
	VsmRegionAnnotation& a = layer_->annotations[row];
	a.name      = name_edit_.GetData().ToString();
	a.parent_id = parent_edit_.GetData().ToString();
	a.x = (int)x_edit_.GetData();
	a.y = (int)y_edit_.GetData();
	a.w = (int)w_edit_.GetData();
	a.h = (int)h_edit_.GetData();
}

void AnnotationEditorPanel::OnCreate()
{
	if(!layer_) return;
	VsmRegionAnnotation& a = layer_->annotations.Add();
	a.id   = Format("ann-%06d", (int)layer_->annotations.GetCount());
	a.name = "New Annotation";
	a.x = 0; a.y = 0; a.w = 100; a.h = 40;
	RebuildList();
	list_.SetCursor(layer_->annotations.GetCount() - 1);
	FillFields(&layer_->annotations.Top());
	WhenLayerChanged();
}

void AnnotationEditorPanel::OnDelete()
{
	if(!layer_) return;
	int row = list_.GetCursor();
	if(row < 0 || row >= layer_->annotations.GetCount()) return;
	layer_->annotations.Remove(row);
	RebuildList();
	FillFields(nullptr);
	WhenLayerChanged();
}

void AnnotationEditorPanel::OnSave()
{
	ApplyFields();
	RebuildList();
	WhenLayerChanged();
}

// ---------------------------------------------------------------------------
// SessionInfoPanel

SessionInfoPanel::SessionInfoPanel()
{
	id_lbl_.SetLabel("Session: —");
	source_lbl_.SetLabel("Source: —");
	size_lbl_.SetLabel("Size: —");
	created_lbl_.SetLabel("Created: —");
	format_lbl_.SetLabel("Format: —");
	assets_lbl_.SetLabel("Assets: —");

	Add(id_lbl_.HSizePos(4, 4).TopPos(4, 20));
	Add(source_lbl_.HSizePos(4, 4).TopPos(28, 20));
	Add(size_lbl_.HSizePos(4, 4).TopPos(52, 20));
	Add(created_lbl_.HSizePos(4, 4).TopPos(76, 20));
	Add(format_lbl_.HSizePos(4, 4).TopPos(100, 20));
	Add(assets_lbl_.HSizePos(4, 4).TopPos(124, 20));
}

void SessionInfoPanel::SetManifest(const VsmSessionManifest& m)
{
	id_lbl_.SetLabel("Session: " + m.session_id);
	source_lbl_.SetLabel("Source: " + m.source_type);
	size_lbl_.SetLabel(Format("Size: %dx%d", m.frame_width, m.frame_height));
	created_lbl_.SetLabel("Created: " + m.created_at);
	format_lbl_.SetLabel("Format: " + m.image_format);
	assets_lbl_.SetLabel(Format("Frames: %d  Crops: %d",
	                             m.frames.GetCount(), m.crops.GetCount()));
}

void SessionInfoPanel::Clear()
{
	id_lbl_.SetLabel("Session: —");
	source_lbl_.SetLabel("Source: —");
	size_lbl_.SetLabel("Size: —");
	created_lbl_.SetLabel("Created: —");
	format_lbl_.SetLabel("Format: —");
	assets_lbl_.SetLabel("Assets: —");
}
