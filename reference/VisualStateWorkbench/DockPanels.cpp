#include "MainWindow.h"

// ---------------------------------------------------------------------------
// Shared helpers for the Run actions on OcrRulePanel/TemplateRulePanel/
// PipelineEditorPanel — all three test the real current frame of the active
// session (pushed in via SetCurrentFrame()) instead of self-seeded or
// synthetic data.

namespace {

// Shown by all three panels' Run actions when no real current frame is
// available yet (no session active, or nothing stepped to a frame yet).
const char* const kNoActiveFrameMsg = "No active frame";

// VsmImageBuffer (uppsrc/VisualStateModel/ImageBuffer.h) holds its pixels in
// a Upp::Vector<byte>, which only supports move (no copy-assign/copy-ctor) --
// so each panel needs its own deep copy of the frame MainWindow pushes in,
// rather than an (impossible) plain '=' from a const& source.
VsmImageBuffer CloneImageBuffer(const VsmImageBuffer& src)
{
	VsmImageBuffer out;
	out.width    = src.width;
	out.height   = src.height;
	out.channels = src.channels;
	out.pixels.SetCount(src.pixels.GetCount());
	if(!src.pixels.IsEmpty())
		memcpy(out.pixels.begin(), src.pixels.begin(), src.pixels.GetCount());
	return out;
}

// Convert the headless VsmImageBuffer (grayscale/RGB/RGBA, as read from
// VsmSessionStoreSource/VsmSessionStore) into the RGBA VsmFrameImage shape
// the preprocessing/template/OCR executors expect. Mirrors the same
// grayscale-to-RGBA conversion already done in
// VsmObservationPipeline::GetRegionCrop() (uppsrc/VisualStateModel/PipelineRunner.cpp).
VsmFrameImage ToFrameImage(const VsmImageBuffer& img)
{
	VsmFrameImage out;
	if(img.IsEmpty())
		return out;
	out.Set(img.width, img.height, nullptr);
	for(int y = 0; y < img.height; y++) {
		for(int x = 0; x < img.width; x++) {
			byte* dst = out.data + (y * img.width + x) * 4;
			byte  gv  = img.Get(x, y, 0);
			dst[0] = dst[1] = dst[2] = gv;
			dst[3] = 255;
		}
	}
	return out;
}

} // anonymous namespace

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
// ModelStatePanel

ModelStatePanel::ModelStatePanel()
{
	objects_list_.AddColumn("ID");
	objects_list_.AddColumn("Type", 70);
	objects_list_.AddColumn("Active", 45);

	props_list_.AddColumn("Key");
	props_list_.AddColumn("Value");

	transitions_list_.AddColumn("Object");
	transitions_list_.AddColumn("Property", 70);
	transitions_list_.AddColumn("→ Value");
	transitions_list_.AddColumn("Event", 80);

	divergences_list_.AddColumn("Frame",    60);
	divergences_list_.AddColumn("Region",   90);
	divergences_list_.AddColumn("Severity", 60);
	divergences_list_.AddColumn("Message");
	divergences_list_.WhenSel = [=] { OnDivergenceSel(); };

	divergence_detail_.SetReadOnly();

	run_sample_btn_.SetLabel("Run Sample Events");

	objects_area_.Add(objects_list_.HSizePos(0,0).TopPos(0,120));
	objects_area_.Add(props_list_.HSizePos(0,0).TopPos(124,100));

	transitions_area_.Add(transitions_list_.SizePos());

	divergences_split_.Vert();
	divergences_split_.Add(divergences_list_);
	divergences_split_.Add(divergence_detail_);
	divergences_area_.Add(divergences_split_.SizePos());

	tabs_.Add(objects_area_.SizePos(), "Objects");
	tabs_.Add(transitions_area_.SizePos(), "Transitions");
	tabs_.Add(divergences_area_.SizePos(), "Divergences");

	Add(tabs_.HSizePos(4,4).TopPos(4,240));
	Add(run_sample_btn_.HSizePos(4,4).BottomPos(4,24));

	run_sample_btn_.WhenAction = [=] { OnRunSample(); };
}

void ModelStatePanel::SetRuntime(VsmModelRuntime* rt)
{
	rt_ = rt;
	Refresh();
}

void ModelStatePanel::Refresh()
{
	objects_list_.Clear();
	props_list_.Clear();
	transitions_list_.Clear();
	divergences_list_.Clear();
	if(!rt_) return;

	const VsmModelState& st = rt_->GetState();
	for(const VsmModelObject& obj : st.objects) {
		objects_list_.Add(obj.id, obj.type, obj.active ? "yes" : "no");
		if(objects_list_.GetCursor() < 0 && &obj == &st.objects[0]) {
			for(const VsmModelProperty& p : obj.properties)
				props_list_.Add(p.key, p.value_json);
		}
	}
	for(const VsmModelTransition& t : rt_->GetHistory())
		transitions_list_.Add(t.object_id, t.property_key, t.to_value, t.trigger_event_type);
	for(const VsmDivergence& d : rt_->GetDivergences())
		divergences_list_.Add(d.frame,
		                      d.region_id.IsEmpty() ? String("—") : d.region_id,
		                      d.severity, d.message);
	divergence_detail_.Clear();
}

void ModelStatePanel::OnDivergenceSel()
{
	int row = divergences_list_.GetCursor();
	if(!rt_ || row < 0) {
		divergence_detail_.Clear();
		return;
	}
	const Vector<VsmDivergence>& divs = rt_->GetDivergences();
	if(row >= divs.GetCount()) return;
	const VsmDivergence& d = divs[row];

	String txt = "expected:\n" + (d.expected_json.IsEmpty() ? String("(none)") : d.expected_json)
	           + "\n\nobserved:\n" + (d.observed_json.IsEmpty() ? String("(none)") : d.observed_json);
	divergence_detail_.Set(txt);

	if(d.frame >= 0)
		WhenJumpToFrame(d.frame);
}

void ModelStatePanel::OnRunSample()
{
	if(!rt_) return;
	rt_->Reset();

	// Inject two events from seed rules
	VsmModelEvent ev1;
	ev1.type           = "ocr_result";
	ev1.source_rule_id = "ocr-001";
	ev1.data_json      = "\"Login\"";
	ev1.ts             = "2026-01-15T14:23:00.000Z";
	rt_->ApplyEvent(ev1);

	VsmModelEvent ev2;
	ev2.type           = "ocr_result";
	ev2.source_rule_id = "ocr-001";
	ev2.data_json      = "\"Dashboard\"";
	ev2.ts             = "2026-01-15T14:23:05.000Z";
	rt_->ApplyEvent(ev2);

	Refresh();
}

// ---------------------------------------------------------------------------
// OcrRulePanel

OcrRulePanel::OcrRulePanel()
{
	rules_list_.AddColumn("Rule");
	rules_list_.AddColumn("Expected");
	results_list_.AddColumn("Rule");
	results_list_.AddColumn("Text");
	results_list_.AddColumn("Conf.", 45);
	results_list_.AddColumn("Status", 60);

	expected_lbl_.SetLabel("Expected text:");
	status_lbl_.SetLabel("—");
	add_btn_.SetLabel("+ Rule");
	remove_btn_.SetLabel("Remove");
	run_btn_.SetLabel("Run (Fake)");

	add_btn_.WhenAction    = [=] { OnAdd(); };
	remove_btn_.WhenAction = [=] { OnRemove(); };
	run_btn_.WhenAction    = [=] { OnRun(); };

	Add(rules_list_.HSizePos(4,4).TopPos(4, 80));
	Add(expected_lbl_.LeftPos(4,90).TopPos(88,20));
	Add(expected_edit_.HSizePos(98,4).TopPos(88,20));
	Add(add_btn_.LeftPos(4,60).TopPos(114,22));
	Add(remove_btn_.LeftPos(68,60).TopPos(114,22));
	Add(run_btn_.RightPos(4,80).TopPos(114,22));
	Add(results_list_.HSizePos(4,4).TopPos(142,100));
	Add(status_lbl_.HSizePos(4,4).BottomPos(4,20));
}

void OcrRulePanel::SetRules(Vector<VsmOcrRule>* rules)
{
	rules_ = rules;
	// Seed the next_ocr_id_ counter from the max existing numeric suffix,
	// so newly added rules won't collide with already-loaded rules.
	next_ocr_id_ = 0;
	if(rules_) {
		for(const VsmOcrRule& r : *rules_) {
			if(r.rule_id.StartsWith("ocr-")) {
				int id = ScanInt(r.rule_id.Mid(4));
				if(id >= next_ocr_id_)
					next_ocr_id_ = id + 1;
			}
		}
	}
	RebuildRules();
}

void OcrRulePanel::AddResult(const VsmOcrResult& result, const VsmOcrComparison& cmp)
{
	static const char* kSev[] = {"OK","WARN","ERROR"};
	int sev = max(0, min(2, cmp.severity));
	results_list_.Add(result.rule_id, result.text,
	                  FormatDouble(result.confidence, 2), kSev[sev]);
	status_lbl_.SetLabel("Last: " + cmp.message);
}

void OcrRulePanel::ClearResults()
{
	results_list_.Clear();
	status_lbl_.SetLabel("—");
}

void OcrRulePanel::SetCurrentFrame(const VsmImageBuffer& img)
{
	current_frame_ = CloneImageBuffer(img);
}

void OcrRulePanel::RebuildRules()
{
	rules_list_.Clear();
	if(!rules_) return;
	for(const VsmOcrRule& r : *rules_)
		rules_list_.Add(r.rule_id, r.expectation.expected_text);
}

void OcrRulePanel::OnAdd()
{
	if(!rules_) return;
	VsmOcrRule& r         = rules_->Add();
	r.rule_id             = Format("ocr-%06d", next_ocr_id_);
	next_ocr_id_++;
	r.expectation.mode    = VSM_EXPECT_EXACT;
	r.expectation.expected_text = expected_edit_.GetData().ToString();
	r.confidence_threshold = 0.5;
	RebuildRules();
}

void OcrRulePanel::OnRemove()
{
	if(!rules_) return;
	int row = rules_list_.GetCursor();
	if(row >= 0 && row < rules_->GetCount())
		rules_->Remove(row);
	RebuildRules();
}

void OcrRulePanel::OnRun()
{
	if(!rules_ || rules_->IsEmpty()) { status_lbl_.SetLabel("No rules"); return; }
	if(current_frame_.IsEmpty()) { status_lbl_.SetLabel(kNoActiveFrameMsg); return; }
	int row = rules_list_.GetCursor();
	if(row < 0) row = 0;
	const VsmOcrRule& rule = (*rules_)[row];

	VsmFrameImage img = ToFrameImage(current_frame_);

	// There is no real OCR engine implementation in this codebase — only
	// VsmFakeOcrEngine (uppsrc/VisualStateModel/OcrLayer.h), whose Execute()
	// ignores the image entirely and always returns a fixed configured
	// string. Seeding it with the rule's own expected text (as before) can
	// never fail. Instead, seed it with a deterministic fingerprint hash of
	// the REAL current frame's content (VsmRegionMemory::ExtractFingerprint +
	// VsmFingerprint32::ComputeHash(), the same fingerprinting already used
	// elsewhere in the headless model) — an "md5:..." hash string that will
	// not coincidentally equal a rule's human-readable expected text, so the
	// comparison can genuinely report a mismatch.
	VsmFingerprint32 fp;
	VsmRegionMemory::ExtractFingerprint(img, 0, 0, img.width, img.height, fp);
	VsmFakeOcrEngine fake(fp.ComputeHash());
	VsmOcrExecutor exec;
	exec.SetEngine(&fake);

	VsmOcrRequest req;
	req.rule_id   = rule.rule_id;
	req.region_id = rule.annotation_id;
	req.status    = VSM_OCR_PENDING;

	VsmOcrResult res   = exec.RunRequest(img, req);
	VsmOcrComparison c = exec.Compare(res, rule);
	AddResult(res, c);
}

// ---------------------------------------------------------------------------
// TemplateRulePanel

TemplateRulePanel::TemplateRulePanel()
{
	rules_list_.AddColumn("Rule ID");
	rules_list_.AddColumn("Mode", 60);
	rules_list_.AddColumn("Thr.", 50);
	results_list_.AddColumn("Rule");
	results_list_.AddColumn("Matched", 55);
	results_list_.AddColumn("Score", 55);
	results_list_.AddColumn("Label");

	add_btn_.SetLabel("+ Rule");
	remove_btn_.SetLabel("Remove");
	run_btn_.SetLabel("Run");
	status_lbl_.SetLabel("—");
	mode_lbl_.SetLabel("Mode:");
	req_lbl_.SetLabel("Req:");

	mode_drop_.Add(0, "Presence");
	mode_drop_.Add(1, "Multi-opt");
	mode_drop_.SetIndex(0);

	req_drop_.Add(0, "Optional");
	req_drop_.Add(1, "Required");
	req_drop_.SetIndex(0);

	add_btn_.WhenAction    = [=] { OnAdd(); };
	remove_btn_.WhenAction = [=] { OnRemove(); };
	run_btn_.WhenAction    = [=] { OnRun(); };

	Add(rules_list_.HSizePos(4,4).TopPos(4, 80));
	Add(mode_lbl_.LeftPos(4,34).TopPos(88,20));
	Add(mode_drop_.LeftPos(42,80).TopPos(88,20));
	Add(req_lbl_.LeftPos(126,28).TopPos(88,20));
	Add(req_drop_.LeftPos(158,80).TopPos(88,20));
	Add(add_btn_.LeftPos(4,60).TopPos(114,22));
	Add(remove_btn_.LeftPos(68,60).TopPos(114,22));
	Add(run_btn_.RightPos(4,80).TopPos(114,22));
	Add(results_list_.HSizePos(4,4).TopPos(142,100));
	Add(status_lbl_.HSizePos(4,4).BottomPos(4,20));
}

void TemplateRulePanel::SetRules(Vector<VsmTemplateRule>* rules)
{
	rules_ = rules;
	// Seed the next_rule_id_ counter from the max existing numeric suffix,
	// so newly added rules won't collide with already-loaded rules.
	next_rule_id_ = 0;
	if(rules_) {
		for(const VsmTemplateRule& r : *rules_) {
			if(r.rule_id.StartsWith("rule-")) {
				int id = ScanInt(r.rule_id.Mid(5));
				if(id >= next_rule_id_)
					next_rule_id_ = id + 1;
			}
		}
	}
	RebuildRules();
}

void TemplateRulePanel::AddMatchResult(const VsmTemplateMatchResult& res)
{
	results_list_.Add(res.rule_id,
	                  res.matched ? "YES" : "no",
	                  FormatDouble(res.score, 3),
	                  res.matched_label.IsEmpty() ? "—" : res.matched_label);
}

void TemplateRulePanel::RebuildRules()
{
	rules_list_.Clear();
	if(!rules_) return;
	for(const VsmTemplateRule& r : *rules_)
		rules_list_.Add(r.rule_id,
		                r.mode == VSM_TM_PRESENCE ? "presence" : "multi",
		                FormatDouble(r.threshold, 2));
}

void TemplateRulePanel::OnAdd()
{
	if(!rules_) return;
	VsmTemplateRule& r = rules_->Add();
	r.rule_id     = Format("rule-%06d", next_rule_id_);
	next_rule_id_++;
	r.mode        = (int)mode_drop_.GetData();
	r.requirement = (int)req_drop_.GetData();
	r.threshold   = 0.8;
	RebuildRules();
}

void TemplateRulePanel::OnRemove()
{
	if(!rules_) return;
	int row = rules_list_.GetCursor();
	if(row >= 0 && row < rules_->GetCount())
		rules_->Remove(row);
	RebuildRules();
}

void TemplateRulePanel::SetCurrentFrame(const VsmImageBuffer& img)
{
	current_frame_ = CloneImageBuffer(img);
}

void TemplateRulePanel::OnRun()
{
	if(!rules_ || rules_->IsEmpty()) { status_lbl_.SetLabel("No rules"); return; }
	if(current_frame_.IsEmpty()) { status_lbl_.SetLabel(kNoActiveFrameMsg); return; }
	int row = rules_list_.GetCursor();
	if(row < 0) row = 0;
	const VsmTemplateRule& rule = (*rules_)[row];

	VsmFrameImage img = ToFrameImage(current_frame_);
	VsmTemplateMatcher matcher;
	VsmTemplateMatchResult res = matcher.Match(img, rule);
	AddMatchResult(res);
	status_lbl_.SetLabel(res.matched
	                      ? ("Last: matched (" + (res.matched_label.IsEmpty() ? String("—") : res.matched_label) + ")")
	                      : String("Last: no match"));
}

// ---------------------------------------------------------------------------
// PipelineEditorPanel

PipelineEditorPanel::PipelineEditorPanel()
{
	steps_list_.AddColumn("Step");
	steps_list_.AddColumn("Params");

	add_gray_btn_.SetLabel("Gray");
	add_inv_btn_.SetLabel("Invert");
	add_thresh_btn_.SetLabel("Thresh");
	add_norm_btn_.SetLabel("Norm32");
	remove_btn_.SetLabel("Remove");
	run_btn_.SetLabel("Run");
	result_lbl_.SetLabel("—");

	add_gray_btn_.WhenAction  = [=] { OnAdd(VSM_PREP_GRAYSCALE); };
	add_inv_btn_.WhenAction   = [=] { OnAdd(VSM_PREP_INVERT); };
	add_thresh_btn_.WhenAction= [=] { OnAdd(VSM_PREP_THRESHOLD); };
	add_norm_btn_.WhenAction  = [=] { OnAdd(VSM_PREP_NORMALIZE_32); };
	remove_btn_.WhenAction    = [=] { OnRemove(); };
	run_btn_.WhenAction       = [=] { OnRun(); };

	Add(steps_list_.HSizePos(4,4).TopPos(4, 100));
	Add(add_gray_btn_.LeftPos(4,44).TopPos(108,22));
	Add(add_inv_btn_.LeftPos(52,44).TopPos(108,22));
	Add(add_thresh_btn_.LeftPos(100,50).TopPos(108,22));
	Add(add_norm_btn_.LeftPos(154,50).TopPos(108,22));
	Add(remove_btn_.LeftPos(4,60).TopPos(134,22));
	Add(run_btn_.RightPos(4,50).TopPos(134,22));
	Add(result_lbl_.HSizePos(4,4).TopPos(162,40));
}

const char* PipelineEditorPanel::StepName(int type)
{
	switch(type) {
	case VSM_PREP_GRAYSCALE:    return "Grayscale";
	case VSM_PREP_INVERT:       return "Invert";
	case VSM_PREP_THRESHOLD:    return "Threshold";
	case VSM_PREP_NORMALIZE_32: return "Normalize32";
	case VSM_PREP_OTSU:         return "Otsu (deferred)";
	case VSM_PREP_EDGE_DETECT:  return "EdgeDetect (deferred)";
	}
	return "Unknown";
}

void PipelineEditorPanel::SetPipeline(VsmPreprocessPipeline* pipeline)
{
	pipeline_ = pipeline;
	RebuildList();
}

void PipelineEditorPanel::SetCurrentFrame(const VsmImageBuffer& img)
{
	current_frame_ = CloneImageBuffer(img);
}

void PipelineEditorPanel::RebuildList()
{
	steps_list_.Clear();
	if(!pipeline_) return;
	for(const VsmPreprocessStep& s : pipeline_->steps)
		steps_list_.Add(StepName(s.type),
		                Format("thr=%d tw=%d th=%d", s.params.threshold_value,
		                       s.params.target_w, s.params.target_h));
}

void PipelineEditorPanel::OnAdd(int type)
{
	if(!pipeline_) return;
	VsmPreprocessStep s; s.type = type;
	pipeline_->steps.Add(s);
	RebuildList();
	WhenPipelineChanged();
}

void PipelineEditorPanel::OnRemove()
{
	if(!pipeline_) return;
	int row = steps_list_.GetCursor();
	if(row < 0 || row >= pipeline_->steps.GetCount()) return;
	pipeline_->steps.Remove(row);
	RebuildList();
	WhenPipelineChanged();
}

void PipelineEditorPanel::OnRun()
{
	if(!pipeline_) { result_lbl_.SetLabel("No pipeline"); return; }
	if(current_frame_.IsEmpty()) { result_lbl_.SetLabel(kNoActiveFrameMsg); return; }

	// Run against the real current frame of the active session (pushed via
	// SetCurrentFrame()) instead of a synthetic gradient.
	VsmFrameImage img = ToFrameImage(current_frame_);
	VsmPreprocessExecutor exec;
	VsmFrameImage out;
	VsmPreprocessResultRef res = exec.Execute(img, *pipeline_, out);
	String msg = Format("steps=%d warnings=%d output=%dx%d",
	                     res.steps_run, res.warnings.GetCount(),
	                     out.width, out.height);
	result_lbl_.SetLabel(msg);
	WhenPipelineChanged();
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

	saved_status_lbl_.SetLabel("");  // Initially empty; updated by NotifySaveResult()

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

	// Buttons and saved status
	Add(create_btn_.LeftPos(4,60).BottomPos(4,24));
	Add(delete_btn_.LeftPos(68,60).BottomPos(4,24));
	Add(save_btn_.RightPos(66,60).BottomPos(4,24));
	Add(saved_status_lbl_.RightPos(4,60).BottomPos(4,24));
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
		x_edit_.Clear();
		y_edit_.Clear();
		w_edit_.Clear();
		h_edit_.Clear();
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
	if(!layer_) return;
	int row = list_.GetCursor();
	if(row < 0 || row >= layer_->annotations.GetCount()) return;
	ApplyFields();
	RebuildList();
	WhenLayerChanged();
}

void AnnotationEditorPanel::NotifySaveResult(bool success, bool path_empty)
{
	if(path_empty) {
		saved_status_lbl_.SetLabel("not saved — no path");
	} else if(success) {
		saved_status_lbl_.SetLabel("Saved");
	} else {
		saved_status_lbl_.SetLabel("save failed");
	}
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
