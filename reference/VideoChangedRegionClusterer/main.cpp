#include "VideoChangedRegionClusterer.h"

NAMESPACE_UPP

static String ResolvePath(const String& raw_path, const String& manifest_path) {
	String path = UnixPath(raw_path);
	if (FileExists(path)) return NormalizePath(path);
	
	// Try relative to manifest
	String manifest_dir = GetFileFolder(manifest_path);
	String rel_to_manifest = AppendFileName(manifest_dir, GetFileName(path));
	if (FileExists(rel_to_manifest)) return NormalizePath(rel_to_manifest);
	
	String rel_to_manifest_full = AppendFileName(manifest_dir, path);
	if (FileExists(rel_to_manifest_full)) return NormalizePath(rel_to_manifest_full);

	// Try matching via workspace root / common directories (like "tmp", "reference")
	int idx = path.Find("ai-upp");
	if (idx >= 0) {
		String sub = path.Mid(idx + 7); // skip "ai-upp/"
		String local_path = AppendFileName(GetCurrentDirectory(), sub);
		if (FileExists(local_path)) return NormalizePath(local_path);
		local_path = AppendFileName(manifest_dir, sub);
		if (FileExists(local_path)) return NormalizePath(local_path);
	}
	
	for (const char* dir_key : {"tmp", "reference", "uppsrc", "bazaar"}) {
		idx = path.Find(dir_key);
		if (idx >= 0) {
			String sub = path.Mid(idx);
			String local_path = AppendFileName(GetCurrentDirectory(), sub);
			if (FileExists(local_path)) return NormalizePath(local_path);
			
			String dir = manifest_dir;
			for (int depth = 0; depth < 4; depth++) {
				String try_path = AppendFileName(dir, sub);
				if (FileExists(try_path)) return NormalizePath(try_path);
				String parent = GetFileFolder(dir);
				if (parent == dir) break;
				dir = parent;
			}
		}
	}
	return raw_path; // fallback
}

static Vector<double> ImageToDoubleVector(const Image& img, int width, int height) {
	Image scaled_img = img;
	if (img.GetSize() != Size(width, height)) {
		scaled_img = Rescale(img, width, height);
	}
	
	Vector<double> res;
	res.SetCount(width * height, 0.0);
	
	for (int y = 0; y < height; y++) {
		const RGBA* row = scaled_img[y];
		for (int x = 0; x < width; x++) {
			// Grayscale conversion: 0.299R + 0.587G + 0.114B
			double gray = (0.299 * row[x].r + 0.587 * row[x].g + 0.114 * row[x].b) / 255.0;
			res[y * width + x] = gray;
		}
	}
	return res;
}

struct Point2D : Moveable<Point2D> {
	double x = 0.0;
	double y = 0.0;
	int sample_index = -1;
};

struct Cluster : Moveable<Cluster> {
	double cx = 0.0, cy = 0.0;
	Vector<int> sample_indices;
};

static Vector<Cluster> RunKMeans(const Vector<Point2D>& points, int K) {
	Vector<Cluster> clusters;
	if (points.IsEmpty() || K <= 0) return clusters;
	if (K > points.GetCount()) K = points.GetCount();
	
	clusters.SetCount(K);
	
	// Seed first centroid randomly
	int first_idx = Random(points.GetCount());
	clusters[0].cx = points[first_idx].x;
	clusters[0].cy = points[first_idx].y;
	
	// Initialize other centroids using max-distance
	for (int k = 1; k < K; k++) {
		double max_dist_sq = -1.0;
		int best_idx = 0;
		for (int i = 0; i < points.GetCount(); i++) {
			double min_dist_sq = DBL_MAX;
			for (int j = 0; j < k; j++) {
				double dx = points[i].x - clusters[j].cx;
				double dy = points[i].y - clusters[j].cy;
				double d2 = dx * dx + dy * dy;
				if (d2 < min_dist_sq) min_dist_sq = d2;
			}
			if (min_dist_sq > max_dist_sq) {
				max_dist_sq = min_dist_sq;
				best_idx = i;
			}
		}
		clusters[k].cx = points[best_idx].x;
		clusters[k].cy = points[best_idx].y;
	}
	
	bool changed = true;
	int iter = 0;
	while (changed && iter < 300) {
		changed = false;
		iter++;
		
		for (int k = 0; k < K; k++) clusters[k].sample_indices.Clear();
		
		// Assign to nearest
		for (int i = 0; i < points.GetCount(); i++) {
			double min_dist_sq = DBL_MAX;
			int best_k = 0;
			for (int k = 0; k < K; k++) {
				double dx = points[i].x - clusters[k].cx;
				double dy = points[i].y - clusters[k].cy;
				double d2 = dx * dx + dy * dy;
				if (d2 < min_dist_sq) {
					min_dist_sq = d2;
					best_k = k;
				}
			}
			clusters[best_k].sample_indices.Add(i);
		}
		
		// Recompute
		for (int k = 0; k < K; k++) {
			if (clusters[k].sample_indices.IsEmpty()) {
				int r_idx = Random(points.GetCount());
				clusters[k].cx = points[r_idx].x;
				clusters[k].cy = points[r_idx].y;
				changed = true;
				continue;
			}
			
			double sx = 0.0, sy = 0.0;
			for (int idx : clusters[k].sample_indices) {
				sx += points[idx].x;
				sy += points[idx].y;
			}
			double new_cx = sx / clusters[k].sample_indices.GetCount();
			double new_cy = sy / clusters[k].sample_indices.GetCount();
			if (abs(new_cx - clusters[k].cx) > 1e-6 || abs(new_cy - clusters[k].cy) > 1e-6) {
				clusters[k].cx = new_cx;
				clusters[k].cy = new_cy;
				changed = true;
			}
		}
	}
	return clusters;
}

// Extract real OCR text for a candidate. Must only ever return either an
// empty string or text that actually came from an OCR string field - never
// a stringified composite Value (ValueMap/ValueArray AsString() produces a
// generic "{ key: val, ... }" dump that is byte-identical across unrelated
// candidates whenever the underlying structure matches, e.g. every
// "ocr": {"status":"unavailable","reason":"no_matching_semantic_ocr_result"}
// candidate would otherwise serialize to the exact same placeholder text.
// That previously caused the OCR-text-equality union-find rule below to
// spuriously merge candidates from different tables/frames/seats that
// simply shared the same "no OCR result" status, defeating per-table-slot
// grouping. Only String-typed values are ever accepted as OCR text.
static String GetOcrText(const ValueMap& candidate) {
	String ocr_text;
	for (const char* key : {"text", "ocr_text"}) {
		if (candidate.Find(key) >= 0 && IsString(candidate[key])) {
			ocr_text = AsString(candidate[key]);
			if (!ocr_text.IsEmpty()) return ocr_text;
		}
	}
	if (candidate.Find("source") >= 0 && IsValueMap(candidate["source"])) {
		ValueMap src = candidate["source"];
		for (const char* key : {"text", "ocr_text"}) {
			if (src.Find(key) >= 0 && IsString(src[key])) {
				ocr_text = AsString(src[key]);
				if (!ocr_text.IsEmpty()) return ocr_text;
			}
		}
		// Real schema: source.ocr = {"status":"available"|"unavailable",
		// "results":[{"text":..., "preprocessed_text":..., ...}, ...]}.
		// Only pull actual per-result text strings; a status of
		// "unavailable" (or an empty results array) must yield "" here,
		// not a stringified status/reason placeholder.
		if (src.Find("ocr") >= 0 && IsValueMap(src["ocr"])) {
			ValueMap ocr_map = src["ocr"];
			if (AsString(ocr_map.Get("status", "")) == "available" &&
			    ocr_map.Find("results") >= 0 && IsValueArray(ocr_map["results"])) {
				ValueArray results = ocr_map["results"];
				String combined;
				for (int i = 0; i < results.GetCount(); i++) {
					if (!IsValueMap(results[i])) continue;
					ValueMap r = results[i];
					String t;
					for (const char* key : {"text", "preprocessed_text", "original_text"}) {
						if (r.Find(key) >= 0 && IsString(r[key])) {
							t = AsString(r[key]);
							if (!t.IsEmpty()) break;
						}
					}
					if (!t.IsEmpty()) {
						if (!combined.IsEmpty()) combined << " | ";
						combined << t;
					}
				}
				if (!combined.IsEmpty()) return combined;
			}
		}
		if (src.Find("contains_text") >= 0 && IsValueMap(src["contains_text"])) {
			ValueMap ct = src["contains_text"];
			if (ct.Find("text") >= 0 && IsString(ct["text"])) {
				ocr_text = AsString(ct["text"]);
				if (!ocr_text.IsEmpty()) return ocr_text;
			}
		}
	}
	return "";
}

bool DoTrainAutoencoder(const String& dataset_path, const String& model_path, int epochs) {
	if (!FileExists(dataset_path)) {
		Cerr() << "ERROR: dataset manifest file does not exist: " << dataset_path << "\n";
		return false;
	}
	
	String content = LoadFile(dataset_path);
	Value root = ParseJSON(content);
	if (IsError(root) || !IsValueMap(root)) {
		Cerr() << "ERROR: Failed to parse dataset manifest as JSON.\n";
		return false;
	}
	
	ValueMap root_map = root;
	ValueArray samples = root_map.Get("samples", ValueArray());
	if (samples.IsEmpty()) {
		Cerr() << "ERROR: No samples found in dataset manifest.\n";
		return false;
	}
	
	Vector<Vector<double>> training_vectors;
	for (int i = 0; i < samples.GetCount(); i++) {
		ValueMap sample = samples[i];
		String path;
		ValueArray variants = sample.Get("image_variants", ValueArray());
		for (int j = 0; j < variants.GetCount(); j++) {
			String try_path = ResolvePath(AsString(variants[j]), dataset_path);
			if (FileExists(try_path)) {
				path = try_path;
				break;
			}
		}
		if (path.IsEmpty()) {
			String crop_path = AsString(sample["crop_path"]);
			if (!crop_path.IsEmpty()) {
				String try_path = ResolvePath(crop_path, dataset_path);
				if (FileExists(try_path)) path = try_path;
			}
		}
		
		if (path.IsEmpty()) continue;
		
		Image img = StreamRaster::LoadFileAny(path);
		if (img.IsEmpty()) {
			Cerr() << "Warning: Failed to load image " << path << "\n";
			continue;
		}
		
		training_vectors.Add(ImageToDoubleVector(img, 28, 28));
	}
	
	if (training_vectors.IsEmpty()) {
		Cerr() << "ERROR: No valid crop images loaded for training.\n";
		return false;
	}
	
	Cout() << "Loaded " << training_vectors.GetCount() << " training images.\n";
	
	String net_config_json = 
		"[\n"
		"  {\"type\":\"input\", \"input_width\":28, \"input_height\":28, \"input_depth\":1},\n"
		"  {\"type\":\"fc\", \"neuron_count\":50, \"activation\":\"tanh\"},\n"
		"  {\"type\":\"fc\", \"neuron_count\":2},\n"
		"  {\"type\":\"fc\", \"neuron_count\":50, \"activation\":\"tanh\"},\n"
		"  {\"type\":\"regression\", \"neuron_count\":784},\n"
		"  {\"type\":\"adadelta\", \"batch_size\":50, \"l2_decay\":0.001}\n"
		"]\n";
		
	ConvNet::Session session;
	if (!session.MakeLayers(net_config_json)) {
		Cerr() << "ERROR: Failed to construct network layers.\n";
		return false;
	}
	
	ConvNet::SessionData& sd = session.Data();
	sd.BeginDataResult(784, training_vectors.GetCount(), 28, 28, 1, 0);
	for (int i = 0; i < training_vectors.GetCount(); i++) {
		Vector<double>& target_in = sd.Get(i);
		Vector<double>& target_out = sd.GetResult(i);
		const Vector<double>& source_vec = training_vectors[i];
		for (int j = 0; j < 784; j++) {
			target_in[j] = source_vec[j];
			target_out[j] = source_vec[j];
		}
	}
	sd.EndData();
	
	Cout() << "Starting Autoencoder training for " << epochs << " epochs...\n";
	session.SetMaxTrainIters(epochs);
	session.WhenIterationInterval = Callback1<int>([&](int iter) {
		Cout() << "  Epoch " << iter << "/" << epochs << " - loss: " << session.GetLossAverage() << "\n";
	}, 1);
	
	session.StartTraining();
	while (session.IsTraining()) {
		Sleep(50);
	}
	Cout() << "Training complete.\n";
	
	RealizeDirectory(GetFileFolder(model_path));
	FileOut fout(model_path);
	if (!fout.IsOpen()) {
		Cerr() << "ERROR: Failed to open model file for writing: " << model_path << "\n";
		return false;
	}
	fout % session;
	fout.Close();
	
	Cout() << "Saved trained autoencoder to " << model_path << "\n";
	return true;
}

bool DoExtractFeatures(const String& dataset_path, const String& model_path, int K, const String& out_json_path) {
	if (!FileExists(dataset_path)) {
		Cerr() << "ERROR: dataset manifest file does not exist: " << dataset_path << "\n";
		return false;
	}
	if (!FileExists(model_path)) {
		Cerr() << "ERROR: model file does not exist: " << model_path << "\n";
		return false;
	}
	
	ConvNet::Session session;
	FileIn fin(model_path);
	if (!fin.IsOpen()) {
		Cerr() << "ERROR: Failed to open model for reading: " << model_path << "\n";
		return false;
	}
	fin % session;
	fin.Close();
	
	// Locate bottleneck layer (neuron count / output_depth == 2)
	int bottleneck_idx = -1;
	for (int i = 0; i < session.GetLayerCount(); i++) {
		const ConvNet::LayerBase& layer = session.GetLayer(i);
		if (layer.output_depth == 2 || layer.neuron_count == 2) {
			bottleneck_idx = i;
			break;
		}
	}
	if (bottleneck_idx == -1) {
		bottleneck_idx = 3; // fallback to index 3
		if (bottleneck_idx >= session.GetLayerCount()) {
			bottleneck_idx = session.GetLayerCount() - 1;
		}
	}
	Cout() << "Using bottleneck layer index: " << bottleneck_idx << "\n";
	
	String content = LoadFile(dataset_path);
	Value root = ParseJSON(content);
	if (IsError(root) || !IsValueMap(root)) {
		Cerr() << "ERROR: Failed to parse dataset manifest as JSON.\n";
		return false;
	}
	
	ValueMap root_map = root;
	ValueArray samples = root_map.Get("samples", ValueArray());
	if (samples.IsEmpty()) {
		Cerr() << "ERROR: No samples found in dataset manifest.\n";
		return false;
	}
	
	Vector<Point2D> points;
	for (int i = 0; i < samples.GetCount(); i++) {
		ValueMap sample = samples[i];
		String path;
		ValueArray variants = sample.Get("image_variants", ValueArray());
		for (int j = 0; j < variants.GetCount(); j++) {
			String try_path = ResolvePath(AsString(variants[j]), dataset_path);
			if (FileExists(try_path)) {
				path = try_path;
				break;
			}
		}
		if (path.IsEmpty()) {
			String crop_path = AsString(sample["crop_path"]);
			if (!crop_path.IsEmpty()) {
				String try_path = ResolvePath(crop_path, dataset_path);
				if (FileExists(try_path)) path = try_path;
			}
		}
		
		if (path.IsEmpty()) continue;
		
		Image img = StreamRaster::LoadFileAny(path);
		if (img.IsEmpty()) continue;
		
		Vector<double> input_vec = ImageToDoubleVector(img, 28, 28);
		session.Predict(input_vec); // forward pass
		
		ConvNet::LayerBase& bottleneck_layer = session.GetLayer(bottleneck_idx);
		double f0 = bottleneck_layer.output_activation.Get(0);
		double f1 = bottleneck_layer.output_activation.Get(1);
		
		Point2D pt;
		pt.x = f0;
		pt.y = f1;
		pt.sample_index = i;
		points.Add(pt);
	}
	
	if (points.IsEmpty()) {
		Cerr() << "ERROR: No valid images processed for feature extraction.\n";
		return false;
	}
	
	Cout() << "Extracted features for " << points.GetCount() << " images.\n";
	
	// Perform K-Means
	Vector<Cluster> clusters = RunKMeans(points, K);
	
	// Assign cluster labels
	Vector<int> cluster_assignments;
	cluster_assignments.SetCount(points.GetCount(), 0);
	for (int k = 0; k < clusters.GetCount(); k++) {
		for (int idx : clusters[k].sample_indices) {
			cluster_assignments[idx] = k;
		}
	}
	
	// Format JSON
	ValueArray out_samples;
	for (int i = 0; i < points.GetCount(); i++) {
		int orig_idx = points[i].sample_index;
		ValueMap orig_sample = samples[orig_idx];
		
		ValueMap m;
		if (orig_sample.Find("id") >= 0) m("id", orig_sample["id"]);
		if (orig_sample.Find("crop_path") >= 0) m("crop_path", orig_sample["crop_path"]);
		else if (orig_sample.Find("image_variants") >= 0 && IsValueArray(orig_sample["image_variants"])) {
			ValueArray vars = orig_sample["image_variants"];
			if (vars.GetCount() > 0) m("crop_path", vars[0]);
		}
		
		ValueArray features_arr;
		features_arr.Add(points[i].x);
		features_arr.Add(points[i].y);
		m("features", features_arr);
		m("cluster", cluster_assignments[i]);
		
		out_samples.Add(m);
	}
	
	ValueArray centroids_arr;
	for (int k = 0; k < clusters.GetCount(); k++) {
		ValueMap cent;
		cent("cluster", k);
		cent("cx", clusters[k].cx);
		cent("cy", clusters[k].cy);
		centroids_arr.Add(cent);
	}
	
	ValueMap root_out;
	root_out("samples", out_samples);
	root_out("centroids", centroids_arr);
	
	RealizeDirectory(GetFileFolder(out_json_path));
	if (!SaveFile(out_json_path, AsJSON(root_out, true))) {
		Cerr() << "ERROR: Failed to save clusters to " << out_json_path << "\n";
		return false;
	}
	
	Cout() << "Extracted features and saved clusters to " << out_json_path << "\n";
	return true;
}

bool DoOcrCluster(const String& candidates_path, const String& out_ocr_path) {
	if (!FileExists(candidates_path)) {
		Cerr() << "ERROR: candidates file does not exist: " << candidates_path << "\n";
		return false;
	}
	
	Value root = ParseJSON(LoadFile(candidates_path));
	if (IsError(root)) {
		Cerr() << "ERROR: Failed to parse candidates file as JSON.\n";
		return false;
	}
	
	ValueArray candidates;
	if (IsValueMap(root)) {
		ValueMap root_map = root;
		if (root_map.Find("candidates") >= 0) candidates = root_map["candidates"];
		else if (root_map.Find("groups") >= 0) candidates = root_map["groups"];
	} else if (IsValueArray(root)) {
		candidates = root;
	}
	
	if (candidates.IsEmpty()) {
		Cerr() << "ERROR: No candidates found in file.\n";
		return false;
	}
	
	int N = candidates.GetCount();
	Vector<int> parent;
	parent.SetCount(N);
	for (int i = 0; i < N; i++) parent[i] = i;
	
	auto Find = [&](int i, auto& self) -> int {
		if (parent[i] == i) return i;
		return parent[i] = self(parent[i], self);
	};
	
	auto Union = [&](int i, int j) {
		int root_i = Find(i, Find);
		int root_j = Find(j, Find);
		if (root_i != root_j) {
			parent[root_i] = root_j;
		}
	};
	
	for (int i = 0; i < N; i++) {
		ValueMap cand_i = candidates[i];
		double cx_i = 0.0, cy_i = 0.0;
		if (cand_i.Find("rect") >= 0) {
			ValueMap rect = cand_i["rect"];
			cx_i = (double)rect.Get("x", 0.0) + (double)rect.Get("w", 0.0) / 2.0;
			cy_i = (double)rect.Get("y", 0.0) + (double)rect.Get("h", 0.0) / 2.0;
		}
		String ocr_i = GetOcrText(cand_i);
		String clean_i = TrimBoth(ToLower(ocr_i));
		
		for (int j = i + 1; j < N; j++) {
			ValueMap cand_j = candidates[j];
			double cx_j = 0.0, cy_j = 0.0;
			if (cand_j.Find("rect") >= 0) {
				ValueMap rect = cand_j["rect"];
				cx_j = (double)rect.Get("x", 0.0) + (double)rect.Get("w", 0.0) / 2.0;
				cy_j = (double)rect.Get("y", 0.0) + (double)rect.Get("h", 0.0) / 2.0;
			}
			String ocr_j = GetOcrText(cand_j);
			String clean_j = TrimBoth(ToLower(ocr_j));
			
			double dx = cx_i - cx_j;
			double dy = cy_i - cy_j;
			double dist = sqrt(dx*dx + dy*dy);
			
			bool connected = false;
			if (dist <= 40.0) {
				connected = true;
			} else if (!clean_i.IsEmpty() && clean_i == clean_j) {
				connected = true;
			}
			
			if (connected) {
				Union(i, j);
			}
		}
	}
	
	// Gather clusters
	VectorMap<int, ValueArray> grouped_clusters;
	for (int i = 0; i < N; i++) {
		int root = Find(i, Find);
		grouped_clusters.GetAdd(root).Add(candidates[i]);
	}
	
	ValueArray clusters_arr;
	for (int i = 0; i < grouped_clusters.GetCount(); i++) {
		ValueMap cl;
		cl("cluster_id", i);
		cl("candidates", grouped_clusters[i]);
		clusters_arr.Add(cl);
	}
	
	ValueMap root_out;
	root_out("clusters", clusters_arr);
	root_out("cluster_count", grouped_clusters.GetCount());
	
	RealizeDirectory(GetFileFolder(out_ocr_path));
	if (!SaveFile(out_ocr_path, AsJSON(root_out, true))) {
		Cerr() << "ERROR: Failed to save OCR layout clusters to " << out_ocr_path << "\n";
		return false;
	}
	
	Cout() << "OCR layout clustering complete. Found " << grouped_clusters.GetCount()
	       << " clusters. Output written to " << out_ocr_path << "\n";
	return true;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN {
	String dataset_path = "tmp/dataset_manifest.json";
	String model_path = "tmp/autoencoder.bin";
	String candidates_path = "tmp/video_card_candidates.json";
	String out_features_path = "tmp/autoencoder_clusters.json";
	String out_ocr_path = "tmp/ocr_layout_clusters.json";
	int K = 5;
	int epochs = 50;
	
	bool train_autoencoder = false;
	bool extract_features = false;
	bool ocr_cluster = false;
	
	const Vector<String>& args = CommandLine();
	for (int i = 0; i < args.GetCount(); i++) {
		if (args[i] == "--train-autoencoder") {
			train_autoencoder = true;
		} else if (args[i] == "--extract-features") {
			extract_features = true;
		} else if (args[i] == "--ocr-cluster") {
			ocr_cluster = true;
		} else if (args[i] == "--dataset" && i + 1 < args.GetCount()) {
			dataset_path = args[++i];
		} else if (args[i] == "--model" && i + 1 < args.GetCount()) {
			model_path = args[++i];
		} else if (args[i] == "--candidates" && i + 1 < args.GetCount()) {
			candidates_path = args[++i];
		} else if (args[i] == "--k" && i + 1 < args.GetCount()) {
			K = StrInt(args[++i]);
		} else if (args[i] == "--epochs" && i + 1 < args.GetCount()) {
			epochs = StrInt(args[++i]);
		} else if (args[i] == "--help" || args[i] == "-h") {
			Cout() << "VideoChangedRegionClusterer options:\n"
			       << "  --train-autoencoder       Train the autoencoder model\n"
			       << "  --extract-features        Extract features and cluster crops\n"
			       << "  --ocr-cluster             Cluster candidates by OCR and space\n"
			       << "  --dataset <path>          Path to dataset_manifest.json\n"
			       << "  --model <path>            Path to save/load model (default: tmp/autoencoder.bin)\n"
			       << "  --candidates <path>       Path to video_card_candidates.json\n"
			       << "  --k <value>               Number of K-Means clusters (default: 5)\n"
			       << "  --epochs <value>          Number of training epochs (default: 50)\n";
			return;
		}
	}
	
	if (!train_autoencoder && !extract_features && !ocr_cluster) {
		Cerr() << "ERROR: No action specified. Use --help to see options.\n";
		SetExitCode(1);
		return;
	}
	
	if (train_autoencoder) {
		if (!DoTrainAutoencoder(dataset_path, model_path, epochs)) {
			SetExitCode(1);
			return;
		}
	}
	
	if (extract_features) {
		if (!DoExtractFeatures(dataset_path, model_path, K, out_features_path)) {
			SetExitCode(1);
			return;
		}
	}
	
	if (ocr_cluster) {
		if (!DoOcrCluster(candidates_path, out_ocr_path)) {
			SetExitCode(1);
			return;
		}
	}
}
