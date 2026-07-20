#ifndef _VisualStateModel_RecognizedSidecar2_h_
#define _VisualStateModel_RecognizedSidecar2_h_

struct VsmRecognizedSidecar2HandResult : Moveable<VsmRecognizedSidecar2HandResult> {
	DistributedLegalityReport legality;
	bool authoritative = false;
};

class VsmRecognizedSidecar2 {
	VsmDistributedReconstructionAdapter adapter_;
	Vector<DistributedSidecar2Line> lines_;
	String stream_;
	int hand_ = -1;
	int64 next_sequence_ = 0;
	int hand_marker_ = -1;
	bool open_ = false;

	static DistributedStateSnapshot MakeSnapshot(const String& phase,
	                                             const bool active[6],
	                                             const double committed[6],
	                                             double total);

public:
	void BeginHand(const String& stream, int hand, int64 timestamp_seconds,
	              const DistributedStateSnapshot& before,
	              const String& event_text);
	bool AddEvent(int64 timestamp_seconds, const String& event_text);
	bool AddAction(int64 timestamp_seconds, int participant,
	               DistributedActionKind kind, bool amount_known, double amount,
	               const String& event_text);
	VsmRecognizedSidecar2HandResult EndHand(int64 timestamp_seconds,
	                                        const DistributedStateSnapshot& after);
	void EndOpenHand(int64 timestamp_seconds, const DistributedStateSnapshot& after);
	String Generate() const;
	const Vector<DistributedSidecar2Line>& GetLines() const { return lines_; }
};

#endif
