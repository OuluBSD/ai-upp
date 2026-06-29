#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

bool VsmGroundTruthLoader::Load(const String& json_path, VsmSession& out)
{
	String raw = LoadFile(json_path);
	if(raw.IsEmpty()) {
		LogError(log_, "VsmGT", "Cannot read file: " + json_path);
		return false;
	}

	Value root = ParseJSON(raw);
	if(root.IsError()) {
		LogError(log_, "VsmGT", "JSON parse error in: " + json_path);
		return false;
	}

	out.schema = root["schema"];
	if(out.schema != 1) {
		LogWarn(log_, "VsmGT", Format("Unknown schema %d in %s", out.schema, json_path));
		out.load_warnings.Add(Format("Unknown schema %d", out.schema));
	}

	Value sess = root["session"];
	if(!sess.IsNull()) {
		out.session_id   = sess["id"].ToString();
		out.source_type  = sess["source_type"].ToString();
		out.frame_width  = sess["frame_width"];
		out.frame_height = sess["frame_height"];
		out.started_at   = sess["started_at"].ToString();
		out.ended_at     = sess["ended_at"].ToString();
		out.image_dir    = sess["image_dir"].ToString();
		out.crop_dir     = sess["crop_dir"].ToString();
	}

	Value events = root["events"];
	if(!events.IsNull()) {
		for(int i = 0; i < events.GetCount(); i++)
			ProcessEvent(events[i], out);
	}

	LogInfo(log_, "VsmGT", Format("Loaded %d events from %s", events.GetCount(), json_path));
	return true;
}

void VsmGroundTruthLoader::ProcessEvent(const Value& ev, VsmSession& out)
{
	String type = ev["type"].ToString();
	int    frame = ev["frame"].IsNull() ? -1 : (int)ev["frame"];
	String ts    = ev["ts"].ToString();

	if(type == "frame") {
		VsmFrameRef& ref = out.frames.Add();
		ref.frame      = frame;
		ref.ts         = ts;
		ref.image_file = ev["image_file"].ToString();
	}
	else if(type == "change") {
		VsmChangeEvent& ce = out.changes.Add();
		ce.frame = frame;
		ce.ts    = ts;
		Value regs = ev["regions"];
		for(int i = 0; i < regs.GetCount(); i++) {
			const Value& r = regs[i];
			VsmChangedRect& cr = ce.regions.Add();
			cr.x     = r["x"];
			cr.y     = r["y"];
			cr.w     = r["w"];
			cr.h     = r["h"];
			cr.score = r["score"];
		}
	}
	else if(type == "region") {
		VsmRegionNode& rn = out.regions.Add();
		rn.id        = ev["region_id"].ToString();
		rn.parent_id = ev["parent_id"].ToString();
		rn.action    = ev["action"].ToString();
		rn.label     = ev["label"].ToString();
		rn.frame     = frame;
		rn.ts        = ts;
		Value rect = ev["rect"];
		if(!rect.IsNull()) {
			rn.x = rect["x"]; rn.y = rect["y"];
			rn.w = rect["w"]; rn.h = rect["h"];
		}
		rn.fingerprint.hash = ev["fingerprint"].ToString();
	}
	else if(type == "ocr") {
		VsmOcrObservation& ocr = out.ocr_results.Add();
		ocr.frame         = frame;
		ocr.ts            = ts;
		ocr.region_id     = ev["region_id"].ToString();
		ocr.trigger_frame = ev["trigger_frame"].IsNull() ? frame : (int)ev["trigger_frame"];
		ocr.text          = ev["text"].ToString();
		ocr.confidence    = ev["confidence"];
		ocr.engine        = ev["engine"].ToString();
		ocr.crop_file     = ev["crop_file"].ToString();
	}
	else if(type == "template") {
		VsmTemplateObservation& tmpl = out.template_results.Add();
		tmpl.frame         = frame;
		tmpl.ts            = ts;
		tmpl.region_id     = ev["region_id"].ToString();
		tmpl.template_name = ev["template_name"].ToString();
		tmpl.score         = ev["score"];
		tmpl.crop_file     = ev["crop_file"].ToString();
		Value mr = ev["match_rect"];
		if(!mr.IsNull()) {
			tmpl.mx = mr["x"]; tmpl.my = mr["y"];
			tmpl.mw = mr["w"]; tmpl.mh = mr["h"];
		}
	}
	else if(type == "state_snapshot" || type == "transition") {
		VsmModelStateRef& msr = out.state_snapshots.Add();
		msr.frame      = frame;
		msr.ts         = ts;
		Value st = (type == "state_snapshot") ? ev["state"] : ev["to"];
		msr.state_json = AsJSON(st);
	}
	else if(type == "divergence") {
		VsmDivergence& div = out.divergences.Add();
		div.frame         = frame;
		div.ts            = ts;
		div.severity      = ev["severity"].ToString();
		div.message       = ev["message"].ToString();
		div.region_id     = ev["region_id"].ToString();
		div.expected_json = AsJSON(ev["expected"]);
		div.observed_json = AsJSON(ev["observed"]);
		LogWarn(log_, "VsmGT", Format("Divergence at frame %d: %s", frame, div.message));
	}
	else if(type == "warning") {
		String msg = ev["message"].ToString();
		out.load_warnings.Add(msg);
		LogWarn(log_, "VsmGT", msg);
	}
	else if(type == "error") {
		String msg = ev["message"].ToString();
		out.load_warnings.Add("ERROR: " + msg);
		LogError(log_, "VsmGT", msg);
	}
	// missing_frame, annotation, replay_checkpoint — skipped for now
}

// ---------------------------------------------------------------------------

String VsmMakeSampleJson()
{
	return
		"{"
		"\"schema\":1,"
		"\"producer\":{\"name\":\"VisualStateModel\",\"version\":\"0.1.0\","
		              "\"created_at\":\"2026-01-15T14:23:00.000Z\"},"
		"\"session\":{"
		  "\"id\":\"demo-001\","
		  "\"source_type\":\"synthetic\","
		  "\"frame_width\":320,"
		  "\"frame_height\":240,"
		  "\"started_at\":\"2026-01-15T14:23:00.000Z\","
		  "\"image_dir\":\"frames/\","
		  "\"crop_dir\":\"crops/\"},"
		"\"events\":["
		  "{\"type\":\"frame\",\"frame\":0,\"ts\":\"2026-01-15T14:23:00.000Z\"},"
		  "{\"type\":\"frame\",\"frame\":1,\"ts\":\"2026-01-15T14:23:00.033Z\"},"
		  "{\"type\":\"change\",\"frame\":1,\"ts\":\"2026-01-15T14:23:00.033Z\","
		   "\"regions\":[{\"x\":10,\"y\":20,\"w\":80,\"h\":40,\"score\":0.91}]},"
		  "{\"type\":\"region\",\"frame\":1,\"ts\":\"2026-01-15T14:23:00.033Z\","
		   "\"region_id\":\"rgn-0001\",\"action\":\"created\","
		   "\"rect\":{\"x\":10,\"y\":20,\"w\":80,\"h\":40},"
		   "\"parent_id\":\"\",\"fingerprint\":\"sha1:a3f8c1d2e\"},"
		  "{\"type\":\"state_snapshot\",\"frame\":2,\"ts\":\"2026-01-15T14:23:00.066Z\","
		   "\"state\":{\"screen\":\"Login\"}},"
		  "{\"type\":\"divergence\",\"frame\":3,\"ts\":\"2026-01-15T14:23:00.100Z\","
		   "\"severity\":\"warning\","
		   "\"expected\":{\"screen\":\"Dashboard\"},"
		   "\"observed\":{\"screen\":\"Login\"},"
		   "\"message\":\"Expected Dashboard after action\"}"
		"]}";
}

} // namespace Upp
