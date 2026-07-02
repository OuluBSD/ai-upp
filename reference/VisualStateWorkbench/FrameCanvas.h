#ifndef _VisualStateWorkbench_FrameCanvas_h_
#define _VisualStateWorkbench_FrameCanvas_h_

// FrameCanvas — shows frame placeholder with layered overlays.
// Fires WhenRegionSelected(region_id) when user clicks a region or annotation.
// Supports annotation authoring: drag-to-create, click-to-select, drag-to-move.

class FrameCanvas : public Ctrl {
public:
	typedef FrameCanvas CLASSNAME;

	FrameCanvas();

	// Session
	void SetSession(const VsmSession* session) { session_ = session; }

	// Overlay data (all optional)
	void SetChangedRegions(const Vector<VsmChangedRect>& regions);
	void SetAnnotationLayer(VsmAnnotationLayer* layer) { ann_layer_ = layer; Refresh(); }
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

	int  GetSelectedAnnotation() const { return selected_ann_; }

	// Authoring events
	Event<String> WhenRegionSelected;    // fires with annotation_id or "region-N"
	Event<>       WhenAnnotationCreated; // fires after drag-to-create commits
	Event<>       WhenAnnotationMoved;   // fires after drag-to-move commits

	virtual void Paint(Draw& w) override;
	virtual void LeftDown(Point p, dword keyflags) override;
	virtual void MouseMove(Point p, dword keyflags) override;
	virtual void LeftUp(Point p, dword keyflags) override;

private:
	enum DragMode { DRAG_NONE, DRAG_CREATE, DRAG_MOVE };

	const VsmSession*                    session_      = nullptr;
	VsmAnnotationLayer*                  ann_layer_    = nullptr;
	const Vector<VsmTemplateMatchResult>* tmpl_results_ = nullptr;
	const Vector<VsmOcrResult>*          ocr_results_  = nullptr;

	Vector<VsmChangedRect> regions_;
	int selected_region_ = -1;
	int selected_ann_    = -1;

	bool show_regions_     = true;
	bool show_annotations_ = true;
	bool show_template_    = true;
	bool show_ocr_         = true;

	// Drag state
	DragMode drag_mode_    = DRAG_NONE;
	Point    drag_start_   = Point(0, 0);
	Point    drag_cur_     = Point(0, 0);
	Point    drag_offset_  = Point(0, 0); // offset from ann top-left to mouse when moving

	static const int kTopOffset = 24; // pixels reserved for session info text
	static const int kMinSize   = 8;  // minimum annotation rect size

	Rect CanvasToAnnotation(Rect r) const;
	Rect AnnotationToCanvas(int ax, int ay, int aw, int ah) const;
	Rect DragRect() const;

	int HitTestRegion(Point p) const;
	int HitTestAnnotation(Point p) const;
	void DrawRegionOverlay(Draw& w) const;
	void DrawAnnotationOverlay(Draw& w) const;
	void DrawTemplateOverlay(Draw& w) const;
	void DrawOcrOverlay(Draw& w) const;
	void DrawDragPreview(Draw& w) const;
};

#endif
