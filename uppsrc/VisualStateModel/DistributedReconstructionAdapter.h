#ifndef _VisualStateModel_DistributedReconstructionAdapter_h_
#define _VisualStateModel_DistributedReconstructionAdapter_h_

namespace Upp {

struct VsmDistributedObservation : Moveable<VsmDistributedObservation> {
	String stream;
	String identity;
	int participant = -1;
	DistributedActionKind kind = DISTRIBUTED_ACTION_PASSIVE;
	int64 sequence = -1;
	int64 timestamp = -1;
	bool amount_known = false;
	double amount = -1;
	String source;
};

class VsmDistributedReconstructionAdapter {
	DistributedReconstructionService service_;
	Vector<String> diagnostics_;

public:
	void Begin(const String& stream, const DistributedStateSnapshot& before);
	bool Observe(const VsmDistributedObservation& observation);
	DistributedServiceResult Complete(const String& stream,
	                                  const DistributedStateSnapshot& after);
	bool GetAuthoritative(const String& stream, DistributedStateSnapshot& out) const;
	const Vector<String>& GetDiagnostics() const { return diagnostics_; }
};

} // namespace Upp

#endif
