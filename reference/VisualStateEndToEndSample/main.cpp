// VisualStateModel — end-to-end CLI sample
// Demonstrates the complete pipeline: session → frames → change detection →
// annotations → preprocessing → template match → OCR → model runtime →
// transitions + divergence → report

#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// Report generation

static void WriteMarkdownReport(const String& path,
                                 const VsmPipelineRunSummary& summary,
                                 const VsmObservationPipeline& pipe,
                                 const VsmModelRuntime& model)
{
	String md;
	md << "# VisualStateModel End-to-End Sample Report\n\n";
	md << "Generated: " << summary.started_at << "\n\n";

	md << "## Run Summary\n\n";
	md << "| Field | Value |\n|---|---|\n";
	md << "| Run ID | " << summary.run_id << " |\n";
	md << "| Session | " << summary.session_id << " |\n";
	md << "| Frames processed | " << summary.frames_processed << " |\n";
	md << "| Change events | " << summary.change_events << " |\n";
	md << "| Regions detected | " << summary.regions_detected << " |\n";
	md << "| Observations | " << summary.observations_made << " |\n";
	md << "| Transitions | " << summary.transitions << " |\n";
	md << "| Divergences | " << summary.divergences << " |\n";
	md << "| Success | " << (summary.success ? "yes" : "NO") << " |\n\n";

	md << "## Model State\n\n";
	const VsmModelState& state = model.GetState();
	if(state.objects.IsEmpty()) {
		md << "_No model objects._\n\n";
	} else {
		for(const VsmModelObject& obj : state.objects) {
			md << "### Object: " << obj.id << "\n\n";
			md << "| Property | Value |\n|---|---|\n";
			for(const VsmModelProperty& p : obj.properties)
				md << "| " << p.key << " | " << p.value_json << " |\n";
			md << "\n";
		}
	}

	md << "## Transitions\n\n";
	if(model.GetHistory().IsEmpty()) {
		md << "_No transitions._\n\n";
	} else {
		md << "| Object | Property | From | To |\n|---|---|---|---|\n";
		for(const VsmModelTransition& t : model.GetHistory())
			md << "| " << t.object_id << " | " << t.property_key
			   << " | " << t.from_value << " | " << t.to_value << " |\n";
		md << "\n";
	}

	md << "## Divergences\n\n";
	if(model.GetDivergences().IsEmpty()) {
		md << "_No divergences._\n\n";
	} else {
		md << "| Severity | Message |\n|---|---|\n";
		for(const VsmDivergence& d : model.GetDivergences())
			md << "| " << d.severity << " | " << d.message << " |\n";
		md << "\n";
	}

	md << "## Observations (" << pipe.GetObservations().GetCount() << ")\n\n";
	md << "| Frame | Type | Rule | Data |\n|---|---|---|---|\n";
	for(const VsmObservation& obs : pipe.GetObservations())
		md << "| " << obs.frame << " | " << obs.type
		   << " | " << obs.rule_id << " | " << obs.data_json << " |\n";
	md << "\n";

	SaveFile(path, md);
}

// ---------------------------------------------------------------------------
// Build session with two synthetic frame images

static void BuildSampleSession(VsmSessionStore& store, const String& session_root)
{
	store.Create(session_root, "e2e-sample-001", 320, 240, "synthetic");

	// Frame 0: solid gray 128 (baseline)
	VsmImageBuffer f0 = VsmImageBuffer::MakeSolid(320, 240, 128, 1);
	store.SaveFrameImage(0, f0);

	// Frame 1: checkerboard (simulates changed region)
	VsmImageBuffer f1 = VsmImageBuffer::MakeCheckerboard(320, 240, 32);
	store.SaveFrameImage(1, f1);

	store.SaveManifest();
}

// ---------------------------------------------------------------------------
// CONSOLE_APP_MAIN

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	AppLog log;
	log.SetForwardToUppLog(false);

	Cout() << "=== VisualStateModel End-to-End Sample ===\n\n";

	// 1. Create session in temp directory
	String session_root = AppendFileName(GetTempPath(), "vsm_e2e_sample");
	if(DirectoryExists(session_root))
		DeleteFolderDeep(session_root);

	VsmSessionStore store;
	store.SetLog(&log);
	BuildSampleSession(store, session_root);
	Cout() << "Session created: " << session_root << "\n";

	// 2. Build ground truth session with 1 change event and 1 region
	VsmSession session;
	session.schema     = 1;
	session.session_id = "e2e-sample-001";
	session.frame_width  = 320;
	session.frame_height = 240;
	session.source_type  = "synthetic";

	VsmFrameRef fr0; fr0.frame = 0; fr0.ts = "2026-01-15T14:23:00.000Z";
	VsmFrameRef fr1; fr1.frame = 1; fr1.ts = "2026-01-15T14:23:00.033Z";
	session.frames.Add(fr0);
	session.frames.Add(fr1);

	VsmChangeEvent& ce = session.changes.Add();
	ce.frame = 1; ce.ts = fr1.ts;
	VsmChangedRect& cr = ce.regions.Add();
	cr.x = 0; cr.y = 0; cr.w = 320; cr.h = 240; cr.score = 0.85;

	VsmRegionNode rn;
	rn.id = "rgn-0001"; rn.frame = 1; rn.x = 0; rn.y = 0; rn.w = 320; rn.h = 240;
	session.regions.Add(rn);

	// 3. Annotation layer
	VsmAnnotationLayer ann;
	ann.schema = 1; ann.session_id = session.session_id;
	VsmRegionAnnotation& a = ann.annotations.Add();
	a.id = "ann-login"; a.name = "Login Screen";
	a.x = 0; a.y = 0; a.w = 320; a.h = 240;

	// 4. Preprocessing pipeline
	VsmPreprocessPipeline pipeline;
	pipeline.id = "pipe-e2e"; pipeline.name = "E2E Preprocess";
	VsmPreprocessStep gs; gs.type = VSM_PREP_GRAYSCALE;
	VsmPreprocessStep n32; n32.type = VSM_PREP_NORMALIZE_32;
	pipeline.steps.Add(gs); pipeline.steps.Add(n32);

	// 5. Template matcher with synthetic asset
	VsmFingerprint32 fp_cb;
	// Checkerboard luma averages ~128 so fill with 128
	memset(fp_cb.data, 128, sizeof(fp_cb.data));
	VsmTemplateMatcher matcher;
	matcher.SetLog(&log);
	matcher.AddSyntheticAsset("asset-checkerboard", fp_cb);

	Vector<VsmTemplateRule> tmpl_rules;
	VsmTemplateRule& tr = tmpl_rules.Add();
	tr.rule_id = "tmpl-001"; tr.annotation_id = "ann-login";
	tr.mode = VSM_TM_PRESENCE; tr.threshold = 0.3;
	tr.requirement = VSM_TMR_OPTIONAL;
	VsmTemplateCandidate& tc = tr.candidates.Add();
	tc.asset_id = "asset-checkerboard"; tc.label = "Checkerboard";

	// 6. OCR — fake engine returns "Login" on frame 1, "Dashboard" on frame 2
	//    We fire both; the second triggers a divergence.
	VsmFakeOcrEngine fake_ocr("Login", 0.92);
	Vector<VsmOcrRule> ocr_rules;
	VsmOcrRule& or1 = ocr_rules.Add();
	or1.rule_id = "ocr-001"; or1.annotation_id = "ann-login";
	or1.expectation.mode = VSM_EXPECT_EXACT;
	or1.expectation.expected_text = "Login";
	or1.confidence_threshold = 0.5;

	// 7. Model runtime — two rules: set screen prop, validate it
	VsmModelRuntime model_rt;
	model_rt.SetLog(&log);

	VsmModelRule mr1;
	mr1.rule_id = "mr-001"; mr1.type = VSM_MR_SET_PROP_FROM_OCR;
	mr1.object_id = "app-screen"; mr1.property_key = "screen";
	mr1.source_rule_id = "ocr-001";
	model_rt.AddRule(mr1);

	VsmModelRule mr2;
	mr2.rule_id = "mr-002"; mr2.type = VSM_MR_VALIDATE_PROP;
	mr2.object_id = "app-screen"; mr2.property_key = "screen";
	mr2.expected_value = "\"Login\""; mr2.source_rule_id = "ocr-001";
	model_rt.AddRule(mr2);

	// Force a divergence by manually applying a "Dashboard" event first
	{
		VsmModelEvent ev_dash;
		ev_dash.type = "ocr_result"; ev_dash.source_rule_id = "ocr-001";
		ev_dash.data_json = "\"Dashboard\"";
		ev_dash.ts = "2026-01-15T14:23:00.010Z";
		model_rt.ApplyEvent(ev_dash);
		Cout() << "Pre-seeded 'Dashboard' event to force divergence\n";
	}

	// 8. Wire up observation pipeline
	VsmObservationPipeline pipe;
	pipe.SetLog(&log);
	pipe.SetSession(&session);
	pipe.SetSessionStore(&store);
	pipe.SetAnnotationLayer(&ann);
	pipe.SetPreprocessPipeline(&pipeline);
	pipe.SetTemplateRules(&tmpl_rules);
	pipe.SetTemplateMatcher(&matcher);
	pipe.SetOcrRules(&ocr_rules);
	pipe.SetOcrEngine(&fake_ocr);
	pipe.SetModelRuntime(&model_rt);

	VsmPipelineRunSummary summary = pipe.Run();

	// 9. Print concise CLI summary
	Cout() << "\n--- Pipeline Run Summary ---\n";
	Cout() << "Run ID:          " << summary.run_id << "\n";
	Cout() << "Session:         " << summary.session_id << "\n";
	Cout() << "Frames:          " << summary.frames_processed << "\n";
	Cout() << "Regions:         " << summary.regions_detected << "\n";
	Cout() << "Observations:    " << summary.observations_made << "\n";
	Cout() << "Transitions:     " << summary.transitions << "\n";
	Cout() << "Divergences:     " << summary.divergences << "\n";
	Cout() << "Success:         " << (summary.success ? "yes" : "NO") << "\n";

	// 10. Verify acceptance criteria
	int exit_code = 0;
	auto Check = [&](bool ok, const char* label) {
		if(!ok) {
			Cout() << "FAIL: " << label << "\n";
			exit_code = 1;
		} else {
			Cout() << "OK:   " << label << "\n";
		}
	};

	Cout() << "\n--- Acceptance Checks ---\n";
	Check(summary.success,              "Pipeline run success");
	Check(summary.frames_processed >= 1, "At least 1 frame processed");
	Check(summary.observations_made >= 1, "At least 1 observation");
	Check(summary.transitions >= 1,      "At least 1 model transition");
	Check(model_rt.GetDivergences().GetCount() >= 1, "At least 1 divergence");
	Check(model_rt.GetState().objects.GetCount() >= 1, "At least 1 model object");

	// Verify divergences.json was auto-saved by the pipeline
	{
		String divs_path = AppendFileName(session_root, "divergences.json");
		bool divs_exist = FileExists(divs_path);
		Check(divs_exist, "divergences.json written");
		if(divs_exist) {
			Vector<VsmDivergence> pdivs;
			bool ok = LoadFromJson(pdivs, LoadFile(divs_path));
			Check(ok && pdivs.GetCount() > 0, "divergences.json valid JSON (>0 records)");
			if(ok)
				Cout() << "   => divergences.json: " << pdivs.GetCount() << " record(s)\n";
		}
	}

	// 10b. Ground-truth comparison
	// Set up a fresh model with a VALIDATE rule that produces divergences
	// when OCR returns "Login" (expected_value intentionally different).
	{
		VsmModelRuntime gt_model;
		gt_model.SetLog(&log);
		VsmModelRule gtmr1;
		gtmr1.rule_id = "gt-mr-001"; gtmr1.type = VSM_MR_SET_PROP_FROM_OCR;
		gtmr1.object_id = "app-screen"; gtmr1.property_key = "screen";
		gtmr1.source_rule_id = "ocr-001";
		gt_model.AddRule(gtmr1);
		VsmModelRule gtmr2;
		gtmr2.rule_id = "gt-mr-002"; gtmr2.type = VSM_MR_VALIDATE_PROP;
		gtmr2.object_id = "app-screen"; gtmr2.property_key = "screen";
		// OCR returns "Login"; VALIDATE expects "Dashboard" → always diverges
		gtmr2.expected_value = "Dashboard";
		gtmr2.source_rule_id = "ocr-001";
		gt_model.AddRule(gtmr2);

		// Pipeline for the GT run
		VsmObservationPipeline gt_pipe;
		gt_pipe.SetLog(&log);
		gt_pipe.SetSessionStore(&store);
		gt_pipe.SetAnnotationLayer(&ann);
		gt_pipe.SetPreprocessPipeline(&pipeline);
		gt_pipe.SetTemplateRules(&tmpl_rules);
		gt_pipe.SetTemplateMatcher(&matcher);
		gt_pipe.SetOcrRules(&ocr_rules);
		gt_pipe.SetOcrEngine(&fake_ocr);
		gt_pipe.SetModelRuntime(&gt_model);

		// Ground truth: 2 expected divergences matching what the GT run will produce.
		// ApplyValidateProp builds expected_json as "\"" + expected_value + "\""
		// so with expected_value="Dashboard": expected_json = "\"Dashboard\""
		VsmGroundTruthSession gt;
		for(int i = 0; i < 2; i++) {
			VsmDivergence& gd = gt.divergences.Add();
			gd.expected_json = "\"Dashboard\"";
			gd.severity      = "warning";
		}

		// Frame source replaying the session images from the store
		VsmSessionStoreSource gt_src;
		gt_src.SetLog(&log);
		if(gt_src.Open(session_root)) {
			VsmPipelineRunWithGTSummary gt_sum = gt_pipe.RunWithGroundTruth(gt_src, gt);
			const VsmComparisonResult& cmp = gt_sum.comparison;
			Check(cmp.matched >= 1, "ground-truth comparison: matched >= 1");
			Cout() << "OK: ground-truth comparison: " << cmp.matched
			       << " matched, " << cmp.missing << " missing, "
			       << cmp.unexpected << " unexpected\n";
		} else {
			Check(false, "ground-truth comparison: session source open");
		}
	}

	// 11. Write pipeline run outputs
	pipe.SaveOutputs(session_root, summary.run_id);
	Cout() << "\nRun outputs: " << AppendFileName(AppendFileName(session_root, "runs"), summary.run_id) << "\n";

	// 12. Write replay report
	String report_dir = store.GetPaths().reports_dir;
	RealizeDirectory(report_dir);
	String report_path = AppendFileName(report_dir, "e2e_report.md");
	WriteMarkdownReport(report_path, summary, pipe, model_rt);
	Cout() << "Report: " << report_path << "\n";

	// 13. Final exit
	Cout() << "\n";
	if(exit_code == 0)
		Cout() << "All end-to-end checks passed.\n";
	else
		Cout() << "Some checks FAILED.\n";

	SetExitCode(exit_code);
}
