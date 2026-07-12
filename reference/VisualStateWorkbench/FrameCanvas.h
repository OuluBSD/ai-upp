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

	// Frame navigation
	void SetFrame(int frame) { current_frame_ = frame; Refresh(); }
	int  GetCurrentFrame() const { return current_frame_; }

	// Real per-frame image (task 0131). Drawn at (0, kTopOffset) at 1:1 scale,
	// UNDER the overlay layers, so overlay rects (which add kTopOffset) line up
	// pixel-exact with the frame. An empty Image() restores the old
	// dark-placeholder look (sample/imported sessions never set one, so their
	// appearance and drag-to-create coordinate system are unchanged).
	void SetFrameImage(const Image& img) { frame_image_ = img; Refresh(); }
	bool HasFrameImage() const { return !frame_image_.IsEmpty(); }

	// Optional info line shown at the top-left. When set, it replaces the
	// session_-derived line (used by TexasHoldem sessions, which are not backed
	// by a VsmSession). Empty string restores the default behaviour.
	void SetInfoText(const String& s) { info_text_ = s; Refresh(); }

	// Overlay data (all optional)
	void SetChangedRegions(const Vector<VsmChangedRect>& regions);
	void SetAnnotationLayer(VsmAnnotationLayer* layer) { ann_layer_ = layer; Refresh(); }
	void SetTemplateResults(const Vector<VsmTemplateMatchResult>* results) { tmpl_results_ = results; Refresh(); }
	void SetOcrResults(const Vector<VsmOcrResult>* results) { ocr_results_ = results; Refresh(); }

	// Layout-binding overlay (task 0132). `model` supplies every `.form`
	// element/sub-slot candidate rect (drawn faintly, elements vs sub-slots in
	// distinct colors); `bindings` are the current frame's matched
	// region->candidate bindings (their matched candidate rect is emphasized
	// with its role label). Both are borrowed pointers owned by MainWindow;
	// pass nullptr to clear. All are drawn only when ShowLayout() is on and are
	// visually distinct from the plain changed-region overlay.
	void SetLayoutModel(const VsmSessionLayoutModel* model) { layout_model_ = model; Refresh(); }
	void SetLayoutBindings(const Vector<VsmLayoutBinding>* bindings) { layout_bindings_ = bindings; Refresh(); }

	// Highlight the changed region / its matched layout candidate for the given
	// region index (the same "region-N" index WhenRegionSelected fires). -1
	// clears. Reused by MainWindow so a click in the Layout Bindings panel and a
	// click on the canvas drive the SAME selection state.
	void SelectRegion(int region_index) { selected_region_ = region_index; Refresh(); }

	// Overlay toggles (persist via AppRegistry in MainWindow)
	bool ShowRegions()      const { return show_regions_;    }
	bool ShowAnnotations()  const { return show_annotations_; }
	bool ShowTemplate()     const { return show_template_;   }
	bool ShowOcr()          const { return show_ocr_;        }
	bool ShowLayout()       const { return show_layout_;     }

	void SetShowRegions     (bool b) { show_regions_      = b; Refresh(); }
	void SetShowAnnotations (bool b) { show_annotations_  = b; Refresh(); }
	void SetShowTemplate    (bool b) { show_template_     = b; Refresh(); }
	void SetShowOcr         (bool b) { show_ocr_          = b; Refresh(); }
	void SetShowLayout      (bool b) { show_layout_       = b; Refresh(); }
	void SetShowEmptyStatePlaceholder(bool b) { show_empty_state_placeholder_ = b; Refresh(); }

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
	int                                  current_frame_ = -1;
	Image                                frame_image_;   // real per-frame image (optional)
	String                               info_text_;     // overrides session_ info line when set
	VsmAnnotationLayer*                  ann_layer_    = nullptr;
	const Vector<VsmTemplateMatchResult>* tmpl_results_ = nullptr;
	const Vector<VsmOcrResult>*          ocr_results_  = nullptr;
	const VsmSessionLayoutModel*         layout_model_    = nullptr;
	const Vector<VsmLayoutBinding>*      layout_bindings_ = nullptr;

	Vector<VsmChangedRect> regions_;
	int selected_region_ = -1;
	int selected_ann_    = -1;

	bool show_regions_                 = true;
	bool show_annotations_             = true;
	bool show_template_                = true;
	bool show_ocr_                     = true;
	bool show_layout_                  = true;
	bool show_empty_state_placeholder_ = false;

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
	void DrawLayoutOverlay(Draw& w) const;
	void DrawDragPreview(Draw& w) const;
};

#endif
