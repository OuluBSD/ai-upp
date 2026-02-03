#include "Camera.h"

NAMESPACE_UPP

StereoPreviewCtrl::StereoPreviewCtrl() {
	left_view.SetEye(0);
	right_view.SetEye(1);
	left_view.SetTitle("Left");
	right_view.SetTitle("Right");
	left_view.WhenContextMenu = THISBACK(OnContextMenu);
	right_view.WhenContextMenu = THISBACK(OnContextMenu);

	split.Horz(left_view, right_view);
	split.SetPos(5000);
	Add(split.SizePos());
}

void StereoPreviewCtrl::SetInputImages(const Image& left, const Image& right) {
	use_engine = true;
	input_left = left;
	input_right = right;
	UpdateDisplay();
}

void StereoPreviewCtrl::SetDisplayImages(const Image& left, const Image& right, bool overlay) {
	use_engine = false;
	input_left = Image();
	input_right = Image();
	overlay_mode = overlay;
	left_view.SetImage(left);
	right_view.SetImage(right);
	UpdateOverlayLayout();
}

void StereoPreviewCtrl::ClearImages() {
	input_left = Image();
	input_right = Image();
	left_view.SetImage(Image());
	right_view.SetImage(Image());
}

void StereoPreviewCtrl::SetCalibration(const StereoCalibrationData& data) {
	engine.SetCalibration(data);
}

void StereoPreviewCtrl::ClearCalibration() {
	engine.ClearCalibration();
}

void StereoPreviewCtrl::SetSettings(const StereoPreviewSettings& s) {
	ApplySettings(s, false);
}

void StereoPreviewCtrl::SetProcessingScale(float scale) {
	StereoPreviewSettings s = engine.GetSettings();
	s.processing_scale = scale;
	ApplySettings(s, true);
}

void StereoPreviewCtrl::ApplySettings(const StereoPreviewSettings& s, bool notify) {
	engine.SetSettings(s);
	UpdateDisplay();
	if (notify && WhenSettingsChanged)
		WhenSettingsChanged(engine.GetSettings());
}

void StereoPreviewCtrl::UpdateDisplay() {
	if (!use_engine)
		return;
	StereoPreviewOutput out;
	engine.Process(input_left, input_right, out);
	overlay_mode = out.overlay_mode;
	left_view.SetImage(out.left);
	right_view.SetImage(out.right);
	UpdateOverlayLayout();
}

void StereoPreviewCtrl::UpdateOverlayLayout() {
	if (overlay_mode) {
		right_view.Hide();
		split.SetPos(10000);
	} else {
		right_view.Show();
		split.SetPos(5000);
	}
	Refresh();
}

void StereoPreviewCtrl::OnContextMenu(Point p, dword flags) {
	MenuBar menu;
	menu.Add("Overlay Eyes", [=] { ToggleOption(&StereoPreviewSettings::overlay_eyes); })
		.Check(engine.GetSettings().overlay_eyes);
	menu.Add("Swap Order", [=] { ToggleOption(&StereoPreviewSettings::overlay_swap); })
		.Check(engine.GetSettings().overlay_swap);
	menu.Add("Tint Overlay", [=] { ToggleOption(&StereoPreviewSettings::tint_overlay); })
		.Check(engine.GetSettings().tint_overlay);
	menu.Add("Rectified Overlay", [=] { ToggleOption(&StereoPreviewSettings::rectified_overlay); })
		.Check(engine.GetSettings().rectified_overlay);
	menu.Add("Show Difference", [=] { ToggleOption(&StereoPreviewSettings::show_difference); })
		.Check(engine.GetSettings().show_difference);
	menu.Add("Show Epipolar", [=] { ToggleOption(&StereoPreviewSettings::show_epipolar); })
		.Check(engine.GetSettings().show_epipolar);
	menu.Add("Show Crosshair", [=] { ToggleOption(&StereoPreviewSettings::show_crosshair); })
		.Check(engine.GetSettings().show_crosshair);
	menu.Add("Process Scale 100%", [=] { SetScaleOption(1.0f); })
		.Check(engine.GetSettings().processing_scale > 0.99f);
	menu.Add("Process Scale 50%", [=] { SetScaleOption(0.5f); })
		.Check(fabs(engine.GetSettings().processing_scale - 0.5f) < 0.01f);
	menu.Add("Process Scale 25%", [=] { SetScaleOption(0.25f); })
		.Check(fabs(engine.GetSettings().processing_scale - 0.25f) < 0.01f);
	menu.Execute();
}

void StereoPreviewCtrl::ToggleOption(bool StereoPreviewSettings::*field) {
	StereoPreviewSettings s = engine.GetSettings();
	s.*field = !(s.*field);
	ApplySettings(s, true);
}

void StereoPreviewCtrl::SetScaleOption(float scale) {
	StereoPreviewSettings s = engine.GetSettings();
	s.processing_scale = scale;
	ApplySettings(s, true);
}

END_UPP_NAMESPACE
