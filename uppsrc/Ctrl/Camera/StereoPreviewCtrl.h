#ifndef _Ctrl_Camera_StereoPreviewCtrl_h_
#define _Ctrl_Camera_StereoPreviewCtrl_h_

class StereoPreviewCtrl : public ParentCtrl {
public:
	typedef StereoPreviewCtrl CLASSNAME;
	StereoPreviewCtrl();

	void SetInputImages(const Image& left, const Image& right);
	void SetDisplayImages(const Image& left, const Image& right, bool overlay_mode = false);
	void ClearImages();

	void SetCalibration(const StereoCalibrationData& data);
	void ClearCalibration();
	bool HasCalibration() const { return engine.HasCalibration(); }

	void SetSettings(const StereoPreviewSettings& s);
	const StereoPreviewSettings& GetSettings() const { return engine.GetSettings(); }

	void SetProcessingScale(float scale);

	PreviewCtrl& LeftPlot() { return left_view; }
	PreviewCtrl& RightPlot() { return right_view; }

	Event<const StereoPreviewSettings&> WhenSettingsChanged;

private:
	PreviewCtrl left_view;
	PreviewCtrl right_view;
	Splitter split;
	StereoPreviewEngine engine;
	bool use_engine = true;
	bool overlay_mode = false;
	Image input_left;
	Image input_right;

	void ApplySettings(const StereoPreviewSettings& s, bool notify);
	void UpdateDisplay();
	void OnContextMenu(Point p, dword flags);
	void BuildMenu(Bar& bar);
	void ToggleOption(bool StereoPreviewSettings::*field);
	void SetScaleOption(float scale);
	void UpdateOverlayLayout();
};

#endif
