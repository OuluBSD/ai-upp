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
