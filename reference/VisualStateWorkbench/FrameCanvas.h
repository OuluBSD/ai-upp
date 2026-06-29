#ifndef _VisualStateWorkbench_FrameCanvas_h_
#define _VisualStateWorkbench_FrameCanvas_h_

// FrameCanvas — shows frame placeholder with layered overlays.
// Fires WhenRegionSelected(region_id) when user clicks a region or annotation.

class FrameCanvas : public Ctrl {
public:
	typedef FrameCanvas CLASSNAME;

	FrameCanvas();

	// Session
	void SetSession(const VsmSession* session) { session_ = session; }

	// Overlay data (all optional)
	void SetChangedRegions(const Vector<VsmChangedRect>& regions);
	void SetAnnotationLayer(const VsmAnnotationLayer* layer) { ann_layer_ = layer; Refresh(); }
	void SetTemplateResults(const Vector<VsmTemplateMatchResult>* results) { tmpl_results_ = results; Refresh(); }
	void SetOcrResults(const Vector<VsmOcrResult>* results) { ocr_results_ = results; Refresh(); }

	// Overlay toggles (persist via AppRegistry in MainWindow)
	bool ShowRegions()      const { return show_regions_;    }
	bool ShowAnnotations()  const { return show_annotations_; }
	bool ShowTemplate()     const { return show_template_;   }
	bool ShowOcr()          const { return show_ocr_;        }

	void SetShowRegions     (bool b) { show_regions_      = b; Refresh(); }
	void SetShowAnnotations (bool b) { show_annotations_  = b; Refresh(); }
	void SetShowTemplate    (bool b) { show_template_     = b; Refresh(); }
	void SetShowOcr         (bool b) { show_ocr_          = b; Refresh(); }

	Event<String> WhenRegionSelected; // fires with region_id / annotation_id (may be empty)

	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword keyflags) override;

private:
	const VsmSession*                    session_      = nullptr;
	const VsmAnnotationLayer*            ann_layer_    = nullptr;
	const Vector<VsmTemplateMatchResult>* tmpl_results_ = nullptr;
	const Vector<VsmOcrResult>*          ocr_results_  = nullptr;

	Vector<VsmChangedRect> regions_;
	int selected_region_ = -1;
	int selected_ann_    = -1;

	bool show_regions_     = true;
	bool show_annotations_ = true;
	bool show_template_    = true;
	bool show_ocr_         = true;

	static const int kTopOffset = 24; // pixels reserved for session info text

	int HitTestRegion(Point p) const;
	int HitTestAnnotation(Point p) const;
	void DrawRegionOverlay(Draw& w) const;
	void DrawAnnotationOverlay(Draw& w) const;
	void DrawTemplateOverlay(Draw& w) const;
	void DrawOcrOverlay(Draw& w) const;
};

#endif
