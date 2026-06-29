#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

bool VsmReplaySession::Load(const String& json_path)
{
	session_.frames.Clear();
	session_.changes.Clear();
	session_.regions.Clear();
	session_.ocr_results.Clear();
	session_.template_results.Clear();
	session_.state_snapshots.Clear();
	session_.divergences.Clear();
	session_.load_warnings.Clear();
	if(!loader_.Load(json_path, session_))
		return false;
	BuildStepEvents();
	LogInfo(log_, "VsmReplay", Format("Loaded session '%s' — %d step events",
	                                   session_.session_id, step_events_.GetCount()));
	return true;
}

bool VsmReplaySession::LoadFromString(const String& json)
{
	String tmp_path = AppendFileName(GetTempPath(), "vsm_replay_tmp.json");
	if(!SaveFile(tmp_path, json)) {
		LogError(log_, "VsmReplay", "Cannot write temp file for in-memory load");
		return false;
	}
	bool ok = Load(tmp_path);
	FileDelete(tmp_path);
	return ok;
}

bool VsmReplaySession::CanStep() const
{
	return event_pos_ < step_events_.GetCount();
}

bool VsmReplaySession::Step()
{
	if(!CanStep()) return false;
	ProcessStepEvent(step_events_[event_pos_++]);
	return true;
}

void VsmReplaySession::RunAll()
{
	while(Step())
		;
}

int VsmReplaySession::GetTotalEvents() const
{
	return step_events_.GetCount();
}

void VsmReplaySession::BuildStepEvents()
{
	step_events_.Clear();
	event_pos_ = 0;

	auto Add = [&](const char* type, int idx, int frame) {
		StepEvent& se = step_events_.Add();
		se.type  = type;
		se.index = idx;
		se.frame = frame;
	};

	for(int i = 0; i < session_.frames.GetCount(); i++)
		Add("frame", i, session_.frames[i].frame);
	for(int i = 0; i < session_.changes.GetCount(); i++)
		Add("change", i, session_.changes[i].frame);
	for(int i = 0; i < session_.regions.GetCount(); i++)
		Add("region", i, session_.regions[i].frame);
	for(int i = 0; i < session_.ocr_results.GetCount(); i++)
		Add("ocr", i, session_.ocr_results[i].frame);
	for(int i = 0; i < session_.template_results.GetCount(); i++)
		Add("template", i, session_.template_results[i].frame);
	for(int i = 0; i < session_.state_snapshots.GetCount(); i++)
		Add("state", i, session_.state_snapshots[i].frame);
	for(int i = 0; i < session_.divergences.GetCount(); i++)
		Add("divergence", i, session_.divergences[i].frame);

	Sort(step_events_, [](const StepEvent& a, const StepEvent& b) {
		return a.frame < b.frame;
	});
}

void VsmReplaySession::ProcessStepEvent(const StepEvent& se)
{
	if(se.type == "frame") {
		const VsmFrameRef& f = session_.frames[se.index];
		LogInfo(log_, "VsmReplay", Format("Frame %d ts=%s", f.frame, f.ts));
	}
	else if(se.type == "change") {
		const VsmChangeEvent& ce = session_.changes[se.index];
		LogInfo(log_, "VsmReplay", Format("Change frame=%d regions=%d",
		                                   ce.frame, ce.regions.GetCount()));
	}
	else if(se.type == "region") {
		const VsmRegionNode& rn = session_.regions[se.index];
		LogInfo(log_, "VsmReplay", Format("Region %s action=%s frame=%d",
		                                   rn.id, rn.action, rn.frame));
	}
	else if(se.type == "state") {
		const VsmModelStateRef& msr = session_.state_snapshots[se.index];
		LogInfo(log_, "VsmReplay", Format("State snapshot frame=%d: %s",
		                                   msr.frame, msr.state_json));
	}
	else if(se.type == "divergence") {
		const VsmDivergence& div = session_.divergences[se.index];
		LogWarn(log_, "VsmReplay", Format("Divergence frame=%d [%s]: %s",
		                                   div.frame, div.severity, div.message));
	}
}

} // namespace Upp
