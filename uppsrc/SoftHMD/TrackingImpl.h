#ifndef _SoftHMD_TrackingImpl_h_
#define _SoftHMD_TrackingImpl_h_

NAMESPACE_HMD_BEGIN

class SoftHmdVisualTracker : public VisualTracker {
public:
	SoftHmdVisualTracker();
	void Reset() override;
	void PutFrame(const VisualFrame& frame) override;
	bool GetState(FusionState& out) const override;
	
	void SetUseBright(bool b);
	void SetUseDark(bool b);
	void SetSplitStereo(bool b);
	void SetPointLimit(int limit);
	void SetDistanceLimit(int limit);
	StereoTrackerStats GetStats() const;
	const Octree* GetPointcloud() const;
	bool HasPose() const;
	vec3 GetPosition() const;
	quat GetOrientation() const;
	
private:
	bool DecodeFrame(const VisualFrame& frame, Image& out) const;
	
	StereoTracker tracker;
	FusionState state;
	bool has_state = false;
};

class SoftHmdImuTracker : public ImuTracker {
public:
	void Reset() override;
	void PutSample(const ImuSample& sample) override;
	
	const ImuSample& GetLastSample() const { return last_sample; }
	bool HasSample() const { return has_sample; }
	
private:
	ImuSample last_sample;
	bool has_sample = false;
};

class SoftHmdFusion : public Fusion {
public:
	void Reset() override;
	void PutVisual(const VisualFrame& frame) override;
	void PutImu(const ImuSample& sample) override;
	bool GetState(FusionState& out) const override;
	
	SoftHmdVisualTracker& GetVisual() { return visual; }
	SoftHmdImuTracker& GetImu() { return imu; }
	
private:
	SoftHmdVisualTracker visual;
	SoftHmdImuTracker imu;
	FusionState state;
	bool has_state = false;
};

class SoftHmdRelocalizer : public Relocalizer {
public:
	void Reset() override {}
	bool Relocalize(const VisualFrame&, FusionState&) override { return false; }
};

NAMESPACE_HMD_END

#endif
