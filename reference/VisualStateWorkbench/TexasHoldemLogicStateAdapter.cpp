#include "TexasHoldemLogicStateAdapter.h"

namespace Upp {

const VsmLogicCompareRecordOut* VsmSessionLogicModel::RecordForFrame(int frame_id) const
{
	if(!loaded || frame_id < 0 || frame_id >= records.GetCount())
		return nullptr;
	return &records[frame_id];
}

VsmSessionLogicModel VsmBuildSessionLogicModel(const VsmTexasHoldemSession& session,
                                               const String& form_path)
{
	VsmSessionLogicModel model;
	model.form_path = form_path;

	if(session.IsEmpty()) {
		model.error = "Session not loaded";
		return model;
	}

	String error;
	Vector<VsmLogicCompareRecordOut> records;
	bool ok = VsmDeriveSessionLogicStates(session.root, form_path, records, error);
	if(!ok) {
		model.error = error;
		return model;
	}

	model.records = pick(records);
	model.loaded  = true;
	return model;
}

} // namespace Upp
