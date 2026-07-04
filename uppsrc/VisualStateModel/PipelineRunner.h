#ifndef _VisualStateModel_PipelineRunner_h_
#define _VisualStateModel_PipelineRunner_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Pipeline run outputs

struct VsmObservation : Moveable<VsmObservation> {
	String type;          // "template_match", "ocr_result", "region_appeared", "region_disappeared"
	String region_id;
	String annotation_id;
	String rule_id;
	String data_json;
	String ts;
	int    frame = -1;
	void Jsonize(JsonIO& json) {
		json("type",type)("region_id",region_id)("annotation_id",annotation_id)
		    ("rule_id",rule_id)("data_json",data_json)("ts",ts)("frame",frame);
	}
};

struct VsmPipelineRunSummary : Moveable<VsmPipelineRunSummary> {
	String run_id;
	String session_id;
	String started_at;
	int    frames_processed  = 0;
	int    change_events     = 0;
	int    regions_detected  = 0;
	int    observations_made = 0;
	int    transitions       = 0;
	int    divergences       = 0;
	bool   success           = false;
	void Jsonize(JsonIO& json) {
		json("run_id",run_id)("session_id",session_id)("started_at",started_at)
		    ("frames_processed",frames_processed)("change_events",change_events)
		    ("regions_detected",regions_detected)("observations_made",observations_made)
		    ("transitions",transitions)("divergences",divergences)("success",success);
	}
};

struct VsmPipelineDiagnostic : Moveable<VsmPipelineDiagnostic> {
	String level;   // "info", "warning", "error"
	String source;
	String message;
	void Jsonize(JsonIO& json) { json("level",level)("source",source)("message",message); }
};

struct VsmPipelineDiagnostics : Moveable<VsmPipelineDiagnostics> {
	Vector<VsmPipelineDiagnostic> entries;
	void AddInfo   (const String& src, const String& msg);
	void AddWarning(const String& src, const String& msg);
	void AddError  (const String& src, const String& msg);
	void Jsonize(JsonIO& json) { json("entries", entries); }
};

// Run summary extended with ground-truth comparison result
struct VsmPipelineRunWithGTSummary : VsmPipelineRunSummary {
	VsmComparisonResult comparison;
};

// ---------------------------------------------------------------------------
// VsmObservationPipeline

class VsmObservationPipeline {
public:
	void SetLog(AppLog* sink)                                   { log_.SetSink(sink); }
	void SetSession(const VsmSession* session)                  { session_     = session;   }
	void SetSessionStore(VsmSessionStore* store)                { store_       = store;     }
	void SetAnnotationLayer(const VsmAnnotationLayer* layer)    { ann_layer_   = layer;     }
	void SetPreprocessPipeline(const VsmPreprocessPipeline* p)  { pipeline_    = p;         }
	void SetTemplateRules(const Vector<VsmTemplateRule>* rules) { tmpl_rules_  = rules;     }
	void SetTemplateMatcher(VsmTemplateMatcher* matcher)        { matcher_     = matcher;   }
	void SetOcrRules(const Vector<VsmOcrRule>* rules)           { ocr_rules_   = rules;     }
	void SetOcrEngine(VsmOcrEngine* eng)                        { ocr_engine_  = eng;       }
	void SetModelRuntime(VsmModelRuntime* runtime)              { model_rt_    = runtime;   }

	// Run the full pipeline over the session (change-event based).
	VsmPipelineRunSummary Run();

	// Alternate entry: drive pipeline frame-by-frame from a VsmFrameSource.
	// Each frame generates a full-frame VsmChangedRect for the annotation/rules pass.
	// Saves frames to store_ if set.
	VsmPipelineRunSummary RunFromSource(VsmFrameSource& src);

	// Alternate entry: drive a semi-live, event-stepped source (see
	// VsmSteppedFrameSource, FrameSource.h). Loops Step()+ReadFrame() while
	// HasMoreSteps() is true; per-frame processing is identical to
	// RunFromSource().
	VsmPipelineRunSummary RunFromSteppedSource(VsmSteppedFrameSource& src);

	// Run from source then compare observed divergences against ground truth.
	// Saves comparison_result.json to session store if configured.
	VsmPipelineRunWithGTSummary RunWithGroundTruth(VsmFrameSource& src,
	                                               VsmGroundTruthSession& gt);

	const Vector<VsmObservation>&  GetObservations()  const { return observations_;  }
	const VsmPipelineDiagnostics&  GetDiagnostics()   const { return diagnostics_;   }

	// Write outputs to <session_root>/runs/<run_id>/
	bool SaveOutputs(const String& session_root, const String& run_id);

private:
	CoreLog                          log_;
	const VsmSession*                session_    = nullptr;
	VsmSessionStore*                 store_      = nullptr;
	const VsmAnnotationLayer*        ann_layer_  = nullptr;
	const VsmPreprocessPipeline*     pipeline_   = nullptr;
	const Vector<VsmTemplateRule>*   tmpl_rules_ = nullptr;
	VsmTemplateMatcher*              matcher_    = nullptr;
	const Vector<VsmOcrRule>*        ocr_rules_  = nullptr;
	VsmOcrEngine*                    ocr_engine_ = nullptr;
	VsmModelRuntime*                 model_rt_   = nullptr;

	Vector<VsmObservation>  observations_;
	VsmPipelineDiagnostics  diagnostics_;

	VsmFakeOcrEngine default_fake_ocr_;

	// Per-region processing
	void ProcessRegion(const VsmChangedRect& rect, int frame_idx, const String& ts);
	// Get or create a crop image for this region
	VsmFrameImage GetRegionCrop(const VsmChangedRect& rect, int frame_idx);
	// Find annotation matching region rect (by overlap)
	const VsmRegionAnnotation* FindAnnotation(const VsmChangedRect& rect) const;
	// Run template rules for annotation
	void RunTemplateRules(const VsmFrameImage& crop, const VsmRegionAnnotation* ann,
	                      const VsmChangedRect& rect, int frame_idx, const String& ts);
	// Run OCR rules for annotation
	void RunOcrRules(const VsmFrameImage& crop, const VsmRegionAnnotation* ann,
	                 const VsmChangedRect& rect, int frame_idx, const String& ts);
	// Emit model event and apply to runtime
	void EmitModelEvent(const VsmObservation& obs);
	// Emit observation and record it
	void EmitObservation(VsmObservation&& obs);
	// Shared per-frame processing body for RunFromSource/RunFromSteppedSource
	void ProcessSourceFrame(const VsmImageBuffer& img, int frame_idx,
	                        int64 ts_ms, VsmPipelineRunSummary& summary);
};

} // namespace Upp

#endif
