#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmPipelineDiagnostics

void VsmPipelineDiagnostics::AddInfo(const String& src, const String& msg)
{
	VsmPipelineDiagnostic& d = entries.Add();
	d.level = "info"; d.source = src; d.message = msg;
}

void VsmPipelineDiagnostics::AddWarning(const String& src, const String& msg)
{
	VsmPipelineDiagnostic& d = entries.Add();
	d.level = "warning"; d.source = src; d.message = msg;
}

void VsmPipelineDiagnostics::AddError(const String& src, const String& msg)
{
	VsmPipelineDiagnostic& d = entries.Add();
	d.level = "error"; d.source = src; d.message = msg;
}

// ---------------------------------------------------------------------------
// VsmObservationPipeline helpers

const VsmRegionAnnotation* VsmObservationPipeline::FindAnnotation(const VsmChangedRect& rect) const
{
	if(!ann_layer_) return nullptr;
	// Simple overlap search: annotation rect overlaps changed rect
	for(const VsmRegionAnnotation& a : ann_layer_->annotations) {
		int ox1 = max(rect.x, a.x),            oy1 = max(rect.y, a.y);
		int ox2 = min(rect.x + rect.w, a.x + a.w), oy2 = min(rect.y + rect.h, a.y + a.h);
		if(ox2 > ox1 && oy2 > oy1) return &a;
	}
	return nullptr;
}

VsmFrameImage VsmObservationPipeline::GetRegionCrop(const VsmChangedRect& rect, int frame_idx)
{
	// Try to load real frame image from store and crop it
	if(store_) {
		VsmImageBuffer img;
		if(store_->LoadFrameImage(frame_idx, img) && !img.IsEmpty()) {
			// Extract sub-region crop as grayscale
			int cx = max(0, rect.x), cy = max(0, rect.y);
			int cw = min(rect.w, img.width  - cx);
			int ch_h = min(rect.h, img.height - cy);
			if(cw > 0 && ch_h > 0) {
				VsmFrameImage crop;
				crop.Set(cw, ch_h, nullptr);
				// Convert image pixels → RGBA crop
				for(int y = 0; y < ch_h; y++) {
					for(int x = 0; x < cw; x++) {
						byte* dst = crop.data + (y * cw + x) * 4;
						int   ich = img.channels;
						byte  gv  = img.Get(cx + x, cy + y, 0);
						dst[0] = gv; dst[1] = gv; dst[2] = gv; dst[3] = 255;
					}
				}
				return crop;
			}
		}
	}
	// Fallback: create synthetic crop from score-derived gray value
	VsmFrameImage crop;
	int w = max(1, rect.w), h = max(1, rect.h);
	crop.Set(w, h, nullptr);
	byte gv = (byte)(int)(rect.score * 200 + 40);
	for(int i = 0; i < w * h; i++) {
		byte* p = crop.data + i * 4;
		p[0] = gv; p[1] = gv; p[2] = gv; p[3] = 255;
	}
	return crop;
}

void VsmObservationPipeline::RunTemplateRules(const VsmFrameImage& crop,
                                              const VsmRegionAnnotation* ann,
                                              const VsmChangedRect& rect,
                                              int frame_idx, const String& ts)
{
	if(!matcher_ || !tmpl_rules_) return;
	String ann_id = ann ? ann->id : String();
	for(const VsmTemplateRule& rule : *tmpl_rules_) {
		if(!ann_id.IsEmpty() && rule.annotation_id != ann_id) continue;
		VsmTemplateMatchResult res = matcher_->Match(crop, rule);
		// Build data JSON
		String data = "{\"matched\":" + String(res.matched ? "true" : "false")
		            + ",\"score\":" + DblStr(res.score)
		            + ",\"label\":\"" + res.matched_label + "\"}";
		VsmObservation obs;
		obs.type          = "template_match";
		obs.region_id     = rect.x >= 0 ? "rgn-auto" : String(); // no stable id here
		obs.annotation_id = ann_id;
		obs.rule_id       = rule.rule_id;
		obs.data_json     = data;
		obs.ts            = ts;
		obs.frame         = frame_idx;
		EmitObservation(pick(obs));
	}
}

void VsmObservationPipeline::RunOcrRules(const VsmFrameImage& crop,
                                         const VsmRegionAnnotation* ann,
                                         const VsmChangedRect& rect,
                                         int frame_idx, const String& ts)
{
	if(!ocr_rules_) return;
	VsmOcrEngine* eng = ocr_engine_ ? ocr_engine_ : &default_fake_ocr_;
	VsmOcrExecutor exec;
	exec.SetLog(log_.GetSink());
	exec.SetEngine(eng);

	String ann_id = ann ? ann->id : String();
	for(const VsmOcrRule& rule : *ocr_rules_) {
		if(!ann_id.IsEmpty() && rule.annotation_id != ann_id) continue;
		VsmOcrRequest req;
		req.rule_id   = rule.rule_id;
		req.region_id = ann_id;
		req.frame     = frame_idx;
		req.ts        = ts;
		req.status    = VSM_OCR_PENDING;

		VsmOcrResult   result = exec.RunRequest(crop, req);
		VsmOcrComparison cmp  = exec.Compare(result, rule);

		String data = "{\"text\":\"" + result.text
		            + "\",\"confidence\":" + DblStr(result.confidence) + "}";

		VsmObservation obs;
		obs.type          = "ocr_result";
		obs.region_id     = ann_id;
		obs.annotation_id = ann_id;
		obs.rule_id       = rule.rule_id;
		obs.data_json     = data;
		obs.ts            = ts;
		obs.frame         = frame_idx;
		EmitObservation(pick(obs));

		if(cmp.severity == VSM_OCR_WARNING)
			diagnostics_.AddWarning("OCR", cmp.message);
	}
}

void VsmObservationPipeline::EmitObservation(VsmObservation&& obs)
{
	EmitModelEvent(obs);
	observations_.Add(pick(obs));
}

void VsmObservationPipeline::EmitModelEvent(const VsmObservation& obs)
{
	if(!model_rt_) return;
	VsmModelEvent ev;
	ev.type           = obs.type;
	ev.ts             = obs.ts;
	ev.source_region_id = obs.region_id;
	ev.source_rule_id   = obs.rule_id;
	// For OCR: data_json contains {"text":"...",...}; extract text as value
	if(obs.type == "ocr_result") {
		// Find "text":"VALUE" in data_json
		int i = obs.data_json.Find("\"text\":\"");
		if(i >= 0) {
			i += 8;
			int j = obs.data_json.Find("\"", i);
			if(j > i)
				ev.data_json = "\"" + obs.data_json.Mid(i, j - i) + "\"";
		}
	} else if(obs.type == "template_match") {
		// data_json = {"label":"..."}
		int i = obs.data_json.Find("\"label\":\"");
		if(i >= 0) {
			i += 9;
			int j = obs.data_json.Find("\"", i);
			if(j > i)
				ev.data_json = "\"" + obs.data_json.Mid(i, j - i) + "\"";
		}
	} else {
		ev.data_json = obs.data_json;
	}
	VsmModelRuntimeResult res = model_rt_->ApplyEvent(ev);
	// Diagnostics for divergences
	for(const VsmDivergence& d : res.divergences)
		diagnostics_.AddWarning("ModelRuntime", d.message);
}

void VsmObservationPipeline::ProcessRegion(const VsmChangedRect& rect,
                                           int frame_idx, const String& ts)
{
	const VsmRegionAnnotation* ann = FindAnnotation(rect);

	// Get crop (real or synthetic)
	VsmFrameImage crop = GetRegionCrop(rect, frame_idx);

	// Preprocess (if pipeline set)
	if(pipeline_) {
		VsmPreprocessExecutor pexec;
		pexec.SetLog(log_.GetSink());
		VsmFrameImage out;
		VsmPreprocessResultRef pres = pexec.Execute(crop, *pipeline_, out);
		if(pres.success && out.width > 0)
			crop = pick(out);
		for(const String& w : pres.warnings)
			diagnostics_.AddWarning("Preprocess", w);
	}

	RunTemplateRules(crop, ann, rect, frame_idx, ts);
	RunOcrRules(crop, ann, rect, frame_idx, ts);
}

// ---------------------------------------------------------------------------
// Run

VsmPipelineRunSummary VsmObservationPipeline::Run()
{
	observations_.Clear();
	diagnostics_  = VsmPipelineDiagnostics();

	VsmPipelineRunSummary summary;
	if(!session_) {
		diagnostics_.AddError("Pipeline", "No session set");
		return summary;
	}

	summary.session_id = session_->session_id;
	Time t = GetUtcTime();
	summary.started_at = Format("%04d-%02d-%02dT%02d:%02d:%02d.000Z",
	                             t.year, t.month, t.day, t.hour, t.minute, t.second);
	summary.run_id = "run-" + IntStr((int)GetTickCount());

	LogInfo(log_, "Pipeline", "Starting run '" + summary.run_id + "' on session '" + summary.session_id + "'");

	// Determine unique frame indices from change events
	Index<int> seen_frames;
	for(const VsmChangeEvent& ce : session_->changes)
		seen_frames.FindAdd(ce.frame);
	summary.frames_processed = seen_frames.GetCount();
	summary.change_events    = session_->changes.GetCount();

	// Main loop: iterate change events
	for(const VsmChangeEvent& ce : session_->changes) {
		summary.regions_detected += ce.regions.GetCount();
		for(const VsmChangedRect& rect : ce.regions)
			ProcessRegion(rect, ce.frame, ce.ts);
	}

	// Emit region_appeared / region_disappeared for stable region nodes
	if(model_rt_) {
		for(const VsmRegionNode& rn : session_->regions) {
			{
				VsmObservation obs;
				obs.type      = "region_appeared";
				obs.region_id = rn.id;
				obs.frame     = rn.frame;
				obs.ts        = session_->changes.GetCount() > 0
				                ? session_->changes[0].ts : summary.started_at;
				EmitObservation(pick(obs));
			}
		}
	}

	summary.observations_made = observations_.GetCount();
	if(model_rt_) {
		summary.transitions  = model_rt_->GetHistory().GetCount();
		summary.divergences  = model_rt_->GetDivergences().GetCount();
	}
	summary.success = true;

	LogInfo(log_, "Pipeline", Format("Run complete: %d obs, %d transitions, %d divergences",
	                                  summary.observations_made,
	                                  summary.transitions, summary.divergences));
	return summary;
}

// ---------------------------------------------------------------------------
// RunFromSource

VsmPipelineRunSummary VsmObservationPipeline::RunFromSource(VsmFrameSource& src)
{
	observations_.Clear();
	diagnostics_ = VsmPipelineDiagnostics();

	VsmPipelineRunSummary summary;
	if(!src.IsReady()) {
		diagnostics_.AddError("Pipeline", "Frame source not ready: " + src.GetLastError());
		return summary;
	}

	summary.session_id = src.GetSourceInfo();
	Time t = GetUtcTime();
	summary.started_at = Format("%04d-%02d-%02dT%02d:%02d:%02d.000Z",
	                             t.year, t.month, t.day, t.hour, t.minute, t.second);
	summary.run_id = "run-src-" + IntStr((int)GetTickCount());

	LogInfo(log_, "Pipeline", "RunFromSource: " + src.GetSourceInfo());

	int frame_idx = 0;
	VsmImageBuffer img;
	int64 ts_ms = 0;

	while(src.ReadFrame(img, ts_ms)) {
		// Persist frame to session store if wired up
		if(store_)
			store_->SaveFrameImage(frame_idx, img, ts_ms);

		// Generate a full-frame changed rect for the annotation/rules pass
		VsmChangedRect rect;
		rect.x = 0; rect.y = 0;
		rect.w = img.width; rect.h = img.height;
		rect.score = 1.0;

		String ts = IntStr64(ts_ms) + "ms";
		ProcessRegion(rect, frame_idx, ts);

		summary.frames_processed++;
		summary.change_events++;
		summary.regions_detected++;
		frame_idx++;
	}

	if(!src.GetLastError().IsEmpty())
		diagnostics_.AddWarning("FrameSource", src.GetLastError());

	summary.observations_made = observations_.GetCount();
	if(model_rt_) {
		summary.transitions = model_rt_->GetHistory().GetCount();
		summary.divergences = model_rt_->GetDivergences().GetCount();
	}
	summary.success = true;

	LogInfo(log_, "Pipeline",
	        Format("RunFromSource complete: %d frames, %d obs, %d transitions",
	               summary.frames_processed, summary.observations_made, summary.transitions));
	return summary;
}

// ---------------------------------------------------------------------------
// SaveOutputs

bool VsmObservationPipeline::SaveOutputs(const String& session_root, const String& run_id)
{
	String run_dir = AppendFileName(AppendFileName(session_root, "runs"), run_id);
	RealizeDirectory(run_dir);

	// observations.json
	if(!SaveFile(AppendFileName(run_dir, "observations.json"),
	             StoreAsJson(observations_, true))) {
		LogError(log_, "Pipeline", "Cannot write observations.json");
		return false;
	}

	// diagnostics.json
	if(!SaveFile(AppendFileName(run_dir, "diagnostics.json"),
	             StoreAsJson(diagnostics_, true))) {
		LogError(log_, "Pipeline", "Cannot write diagnostics.json");
		return false;
	}

	return true;
}

} // namespace Upp
