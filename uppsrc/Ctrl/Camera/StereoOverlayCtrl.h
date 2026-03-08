#ifndef _Ctrl_Camera_StereoOverlayCtrl_h_
#define _Ctrl_Camera_StereoOverlayCtrl_h_

class StereoOverlayCtrl : public Ctrl {
public:
	typedef StereoOverlayCtrl CLASSNAME;

	StereoOverlayCtrl();

	void SetImages(const Image& bright, const Image& dark);
	void SetOverlay(bool is_bright, const StereoOverlayData& overlay);
	void ClearOverlay(bool is_bright);
	void SetStats(bool is_bright, const StereoTrackerStatsData& stats);

	void SetShowDescriptors(bool b) { show_descriptors = b; }
	void SetShowDescriptorIds(bool b) { show_descriptor_ids = b; }
	void SetShowMatchLines(bool b) { show_match_lines = b; }
	void SetShowMatchIds(bool b) { show_match_ids = b; }
	void SetShowStatsOverlay(bool b) { show_stats_overlay = b; }
	void SetShowSplitView(bool b);

	void SetLabels(const String& bright, const String& dark);
	void SetDrawLabel(bool b) { view.SetDrawLabel(b); }

private:
	CameraStereoView view;
	StereoOverlayData bright_overlay;
	StereoOverlayData dark_overlay;
	StereoTrackerStatsData bright_stats;
	StereoTrackerStatsData dark_stats;
	bool has_bright_overlay = false;
	bool has_dark_overlay = false;
	bool show_descriptors = true;
	bool show_descriptor_ids = false;
	bool show_match_lines = false;
	bool show_match_ids = false;
	bool show_stats_overlay = true;
	bool show_split_view = true;

	void DrawOverlay(Draw& w, const Rect& r, const Image& img, const StereoOverlayData& overlay,
	                 const StereoTrackerStatsData& stats, const char* label,
	                 const Color& point_color, const Color& match_color) const;
	void OnOverlay(Draw& w, const Rect& r, const Image& img, int channel);
};

#endif
