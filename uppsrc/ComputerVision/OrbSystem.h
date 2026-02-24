#ifndef _ComputerVision_OrbSystem_h_
#define _ComputerVision_OrbSystem_h_

struct ColorLine : Moveable<ColorLine> {
	Point a, b;
	Color clr;
	byte alpha = 255;
};

class OrbSystem {
	DMatrix<byte> lev0_img;
	DMatrix<byte> lev_img;
	DMatrix<byte> pattern_preview;
	DMatrix<byte> match_mask;
	DMatrix<byte> train_img;
	DMatrix<float> homo3x3;
	Vector<Keypoint> screen_corners;
	
	Vector<BinDescriptor> screen_descriptors;
	Vector<Vector<BinDescriptor>> pattern_descriptors;
	
	Vector<Vector<Keypoint>> pattern_corners;
	Vector<Keypoint> lev_corners, lev_descr;
	Vector<Keypoint> corners, pattern_xy, screen_xy;
	Vector<KeypointMatch> matches;
	
	int num_train_levels = 10;
	Orb o;
	Yape06 y;
	Homography2D mm_kernel;
	RansacParams ransac_param;
	MotionEstimator<Homography2D> mot;
	
	int    blur_size = 9;
	int    lap_thres = 2;
	int    eigen_thres = 2;
	int    keypoint_match_threshold = 95;
	double ratio_test = 0.8;
	
	void render_matches(const Vector<KeypointMatch>& matches);
	void render_pattern_shape();
	void RenderCorners(const ByteMat& bg, const ByteMat* mini_img, const Vector<Keypoint>& corners, ByteMat& out);
	void OutputFromGray(const ByteMat& gray);
	
public:
	static const int u_max[];

	void SetInput(Image i);
	void InitDefault();
	void Process();
	
	void TrainPattern();
	int  DetectKeypoints(DescriptorImage& output, int max_allowed);
	int  DetectKeypoints(const ByteMat& img, Vector<Keypoint>& corners, int max_allowed);
	double IcAngle(const ByteMat& img, int px, int py);
	int  FindTransform(Vector<KeypointMatch>& matches);
	int  MatchPattern();
	void TCorners(const Vector<float>& M, int w, int h);

	void SetKeypointMatchThreshold(int t) { keypoint_match_threshold = t; }
	int  GetKeypointMatchThreshold() const { return keypoint_match_threshold; }
	
	void SetRatioTest(double r)           { ratio_test = r; }
	double GetRatioTest() const             { return ratio_test; }

	int  GetLastGoodMatches() const       { return last_good_matches; }
	int  GetLastMatchCount() const        { return last_match_count; }
	const Vector<Pointf>& GetLastCorners() const { return last_corners; }
	const Vector<float>& GetHomo() const  { return homo3x3.data; }
	
	ByteMat input, output, tmp0, tmp1, tmp2;
	Size sz;
	Vector<ColorLine> lines;

	Vector<Pointf> last_corners;
	int last_good_matches = 0;
	int last_match_count = 0;
};

#endif
