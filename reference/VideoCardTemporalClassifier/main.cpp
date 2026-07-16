#include "VideoCardTemporalClassifier.h"

NAMESPACE_UPP

struct ClassifierOptions {
	String dataset;
	String model;
	bool train = false;
	bool predict = false;
	String net_config;
	int epochs = 50;
	double confidence_threshold = 0.9;
	bool help = false;
};

ClassifierOptions ParseArgs(const Vector<String>& args) {
	ClassifierOptions o;
	for (int i = 0; i < args.GetCount(); i++) {
		if (args[i] == "--dataset" && i + 1 < args.GetCount()) {
			o.dataset = args[++i];
		} else if (args[i] == "--model" && i + 1 < args.GetCount()) {
			o.model = args[++i];
		} else if (args[i] == "--train") {
			o.train = true;
		} else if (args[i] == "--predict") {
			o.predict = true;
		} else if (args[i] == "--net-config" && i + 1 < args.GetCount()) {
			o.net_config = args[++i];
		} else if (args[i] == "--epochs" && i + 1 < args.GetCount()) {
			o.epochs = StrInt(args[++i]);
		} else if (args[i] == "--confidence-threshold" && i + 1 < args.GetCount()) {
			o.confidence_threshold = ScanDouble(args[++i]);
		} else if (args[i] == "--help" || args[i] == "-h") {
			o.help = true;
		}
	}
	return o;
}

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

static Vector<double> ImageToDoubleVector(const Image& img, int width, int height, int depth) {
	Image scaled_img = img;
	if (img.GetSize() != Size(width, height)) {
		scaled_img = Rescale(img, width, height);
	}
	
	Vector<double> res;
	res.SetCount(width * height * depth, 0.0);
	
	for (int y = 0; y < height; y++) {
		const RGBA* row = scaled_img[y];
		for (int x = 0; x < width; x++) {
			if (depth == 1) {
				// Grayscale conversion: 0.299R + 0.587G + 0.114B
				double gray = (0.299 * row[x].r + 0.587 * row[x].g + 0.114 * row[x].b) / 255.0;
				res[y * width + x] = gray;
			} else if (depth == 3) {
				int base_idx = (y * width + x) * 3;
				res[base_idx + 0] = row[x].r / 255.0;
				res[base_idx + 1] = row[x].g / 255.0;
				res[base_idx + 2] = row[x].b / 255.0;
			} else {
				int base_idx = (y * width + x) * depth;
				if (depth >= 1) res[base_idx + 0] = row[x].r / 255.0;
				if (depth >= 2) res[base_idx + 1] = row[x].g / 255.0;
				if (depth >= 3) res[base_idx + 2] = row[x].b / 255.0;
				if (depth >= 4) res[base_idx + 3] = row[x].a / 255.0;
			}
		}
	}
	return res;
}

bool DoTrain(const ClassifierOptions& o) {
	if (!FileExists(o.dataset)) {
		Cerr() << "ERROR: dataset manifest file does not exist: " << o.dataset << "\n";
		return false;
	}
	
	// Load dataset_manifest.json
	String content = LoadFile(o.dataset);
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
	
	// Collect training data: samples that have non-null, non-empty labels
	Index<String> classes_index;
	Vector<ValueMap> labeled_samples;
	for (int i = 0; i < samples.GetCount(); i++) {
		ValueMap sample = samples[i];
		Value label_val = sample["label"];
		if (!label_val.IsNull() && !label_val.IsVoid()) {
			String label = AsString(label_val);
			if (!label.IsEmpty()) {
				labeled_samples.Add(sample);
				classes_index.FindAdd(label);
			}
		}
	}
	
	if (labeled_samples.IsEmpty()) {
		Cerr() << "ERROR: No labeled samples found for training.\n";
		return false;
	}
	
	if (classes_index.GetCount() < 2) {
		Cerr() << "ERROR: Training requires at least 2 distinct classes. Found: " << classes_index.GetCount() << "\n";
		return false;
	}
	
	Cout() << "Found " << labeled_samples.GetCount() << " labeled samples belonging to " << classes_index.GetCount() << " classes:\n";
	for (int i = 0; i < classes_index.GetCount(); i++) {
		Cout() << "  [" << i << "]: " << classes_index[i] << "\n";
	}
	
	// Prepare the net config string
	String net_config_json;
	if (!o.net_config.IsEmpty()) {
		if (!FileExists(o.net_config)) {
			Cerr() << "ERROR: net-config file does not exist: " << o.net_config << "\n";
			return false;
		}
		net_config_json = LoadFile(o.net_config);
	} else {
		// Default simple CNN
		net_config_json = Format(
			"[\n"
			"  {\"type\":\"input\", \"input_width\":32, \"input_height\":32, \"input_depth\":1},\n"
			"  {\"type\":\"conv\", \"width\":5, \"height\":5, \"filter_count\":12, \"stride\":1, \"pad\":2, \"activation\":\"relu\"},\n"
			"  {\"type\":\"pool\", \"width\":2, \"height\":2, \"stride\":2},\n"
			"  {\"type\":\"conv\", \"width\":5, \"height\":5, \"filter_count\":24, \"stride\":1, \"pad\":2, \"activation\":\"relu\"},\n"
			"  {\"type\":\"pool\", \"width\":2, \"height\":2, \"stride\":2},\n"
			"  {\"type\":\"softmax\", \"class_count\": %d},\n"
			"  {\"type\":\"adadelta\", \"batch_size\":16, \"l2_decay\":0.001}\n"
			"]\n",
			classes_index.GetCount()
		);
	}
	
	// Parse and adjust class count in softmax/svm layers of the loaded config if needed
	Value layers_val = ParseJSON(net_config_json);
	if (IsValueArray(layers_val)) {
		ValueArray layers = layers_val;
		for (int i = 0; i < layers.GetCount(); i++) {
			if (IsValueMap(layers[i])) {
				ValueMap layer = layers[i];
				String type = layer.Get("type", "");
				if (type == "softmax" || type == "svm") {
					layer.Set("class_count", classes_index.GetCount());
					layers.Set(i, layer);
				}
			}
		}
		net_config_json = AsJSON(layers);
	}
	
	// Initialize Session
	ConvNet::Session session;
	if (!session.MakeLayers(net_config_json)) {
		Cerr() << "ERROR: Failed to construct network layers from configuration.\n";
		return false;
	}
	
	// Retrieve input layer specs
	ConvNet::LayerBase* input_layer = session.GetInput();
	if (!input_layer) {
		Cerr() << "ERROR: Input layer was not initialized correctly.\n";
		return false;
	}
	int width = input_layer->input_width;
	int height = input_layer->input_height;
	int depth = input_layer->input_depth;
	
	Cout() << "Network input dimensions: " << width << "x" << height << "x" << depth << "\n";
	
	// Load and convert images
	Vector<Vector<double>> training_vectors;
	Vector<int> training_labels;
	
	for (int i = 0; i < labeled_samples.GetCount(); i++) {
		ValueMap sample = labeled_samples[i];
		String label = AsString(sample["label"]);
		int label_idx = classes_index.Find(label);
		ASSERT(label_idx >= 0);
		
		// Find first valid image path from variants or crop_path
		String path;
		ValueArray variants = sample.Get("image_variants", ValueArray());
		for (int j = 0; j < variants.GetCount(); j++) {
			String try_path = ResolvePath(AsString(variants[j]), o.dataset);
			if (FileExists(try_path)) {
				path = try_path;
				break;
			}
		}
		if (path.IsEmpty()) {
			String crop_path = AsString(sample["crop_path"]);
			if (!crop_path.IsEmpty()) {
				String try_path = ResolvePath(crop_path, o.dataset);
				if (FileExists(try_path)) path = try_path;
			}
		}
		
		if (path.IsEmpty()) {
			Cerr() << "Warning: No valid image found for sample ID " << AsString(sample["id"]) << ". Skipping.\n";
			continue;
		}
		
		Image img = StreamRaster::LoadFileAny(path);
		if (img.IsEmpty()) {
			Cerr() << "Warning: Failed to load image " << path << ". Skipping.\n";
			continue;
		}
		
		training_vectors.Add(ImageToDoubleVector(img, width, height, depth));
		training_labels.Add(label_idx);
	}
	
	if (training_vectors.IsEmpty()) {
		Cerr() << "ERROR: Failed to load any valid images for training.\n";
		return false;
	}
	
	Cout() << "Successfully loaded " << training_vectors.GetCount() << " training samples.\n";
	
	// Populate SessionData
	ConvNet::SessionData& sd = session.Data();
	sd.BeginDataClass(classes_index.GetCount(), training_vectors.GetCount(), width, height, depth, 0);
	for (int i = 0; i < classes_index.GetCount(); i++) {
		sd.SetClass(i, classes_index[i]);
	}
	for (int i = 0; i < training_vectors.GetCount(); i++) {
		Vector<double>& target_vec = sd.Get(i);
		const Vector<double>& source_vec = training_vectors[i];
		int cnt = source_vec.GetCount();
		target_vec.SetCount(cnt);
		for (int j = 0; j < cnt; j++) {
			target_vec[j] = source_vec[j];
		}
		sd.SetLabel(i, training_labels[i]);
	}
	sd.EndData();
	
	// Train the network
	Cout() << "Starting training for " << o.epochs << " epochs...\n";
	session.SetMaxTrainIters(o.epochs);
	session.WhenIterationInterval = Callback1<int>([&](int iter) {
		Cout() << "  Epoch " << iter << "/" << o.epochs << " - loss: " << session.GetLossAverage() 
		       << " - accuracy: " << session.GetTrainingAccuracyAverage() << "\n";
	}, 1);
	
	session.StartTraining();
	while (session.IsTraining()) {
		Sleep(50);
	}
	
	Cout() << "Training completed.\n";
	
	// Save model to binary file
	RealizeDirectory(GetFileFolder(o.model));
	FileOut fout(o.model);
	if (!fout.IsOpen()) {
		Cerr() << "ERROR: Could not open model file for writing: " << o.model << "\n";
		return false;
	}
	
	fout % session;
	fout.Close();
	
	Cout() << "Model successfully saved to " << o.model << "\n";
	return true;
}

bool DoPredict(const ClassifierOptions& o) {
	if (!FileExists(o.dataset)) {
		Cerr() << "ERROR: dataset manifest file does not exist: " << o.dataset << "\n";
		return false;
	}
	if (!FileExists(o.model)) {
		Cerr() << "ERROR: model file does not exist: " << o.model << "\n";
		return false;
	}
	
	// Load Session from model
	ConvNet::Session session;
	FileIn fin(o.model);
	if (!fin.IsOpen()) {
		Cerr() << "ERROR: Could not open model file for reading: " << o.model << "\n";
		return false;
	}
	fin % session;
	fin.Close();
	
	ConvNet::LayerBase* input_layer = session.GetInput();
	if (!input_layer) {
		Cerr() << "ERROR: Invalid or corrupt model file (input layer not found).\n";
		return false;
	}
	int width = input_layer->input_width;
	int height = input_layer->input_height;
	int depth = input_layer->input_depth;
	
	ConvNet::SessionData& sd = session.Data();
	int cls_count = sd.GetClassCount();
	if (cls_count == 0) {
		Cerr() << "ERROR: No classes registered in model.\n";
		return false;
	}
	
	// Load dataset manifest
	String content = LoadFile(o.dataset);
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
	
	Cout() << "Running inference on " << samples.GetCount() << " samples...\n";
	
	ValueArray updated_samples;
	int prediction_success_count = 0;
	
	for (int i = 0; i < samples.GetCount(); i++) {
		ValueMap sample = samples[i];
		
		// Load all available image variants for voting
		Vector<String> valid_paths;
		ValueArray variants = sample.Get("image_variants", ValueArray());
		for (int j = 0; j < variants.GetCount(); j++) {
			String try_path = ResolvePath(AsString(variants[j]), o.dataset);
			if (FileExists(try_path)) {
				valid_paths.Add(try_path);
			}
		}
		if (valid_paths.IsEmpty()) {
			String crop_path = AsString(sample["crop_path"]);
			if (!crop_path.IsEmpty()) {
				String try_path = ResolvePath(crop_path, o.dataset);
				if (FileExists(try_path)) valid_paths.Add(try_path);
			}
		}
		
		if (valid_paths.IsEmpty()) {
			Cerr() << "Warning: No valid images found for sample ID " << AsString(sample["id"]) << ". Leaving unchanged.\n";
			updated_samples.Add(sample);
			continue;
		}
		
		// Sum predictions across all valid variants (temporal voting/smoothing)
		Vector<double> sum_probs;
		sum_probs.SetCount(cls_count, 0.0);
		int valid_predictions = 0;
		
		for (const String& path : valid_paths) {
			Image img = StreamRaster::LoadFileAny(path);
			if (img.IsEmpty()) continue;
			
			Vector<double> input_vec = ImageToDoubleVector(img, width, height, depth);
			Vector<double> probs = session.Predict(input_vec);
			if (probs.GetCount() == cls_count) {
				for (int c = 0; c < cls_count; c++) {
					sum_probs[c] += probs[c];
				}
				valid_predictions++;
			}
		}
		
		if (valid_predictions == 0) {
			Cerr() << "Warning: Failed to predict any variants for sample ID " << AsString(sample["id"]) << ". Leaving unchanged.\n";
			updated_samples.Add(sample);
			continue;
		}
		
		// Average the class probabilities
		for (int c = 0; c < cls_count; c++) {
			sum_probs[c] /= valid_predictions;
		}
		
		// Find best class
		int best_idx = 0;
		double max_prob = -1.0;
		for (int c = 0; c < cls_count; c++) {
			if (sum_probs[c] > max_prob) {
				max_prob = sum_probs[c];
				best_idx = c;
			}
		}
		
		// Calculate margin (difference between best and second best)
		Vector<double> sorted_probs;
		sorted_probs.Append(sum_probs);
		Sort(sorted_probs);
		double margin = sorted_probs.Top();
		if (sorted_probs.GetCount() >= 2) {
			margin = sorted_probs.Top() - sorted_probs[sorted_probs.GetCount() - 2];
		}
		
		String predicted_label = sd.GetClass(best_idx);
		
		// Update sample fields
		ValueMap updated_sample = sample;
		updated_sample.Set("label", predicted_label);
		updated_sample.Set("confidence", max_prob);
		updated_sample.Set("confidence_margin", margin);
		
		// Decide needs_review status
		String review_status = AsString(sample["review_status"]);
		bool needs_review = true;
		if (review_status == "approved" || max_prob >= o.confidence_threshold) {
			needs_review = false;
		}
		updated_sample.Set("needs_review", needs_review);
		
		updated_samples.Add(updated_sample);
		prediction_success_count++;
	}
	
	// Update manifest's samples array and model status
	root_map.Set("samples", updated_samples);
	
	ValueMap model_info;
	model_info.Set("status", "ready");
	model_info.Set("runtime_required", true);
	model_info.Set("path", o.model);
	root_map.Set("model", model_info);
	
	// Save updated manifest
	String updated_json = AsJSON(root_map, true);
	if (!SaveFile(o.dataset, updated_json)) {
		Cerr() << "ERROR: Failed to save updated dataset manifest to: " << o.dataset << "\n";
		return false;
	}
	
	Cout() << "Successfully updated dataset manifest " << o.dataset << " with " 
	       << prediction_success_count << " predictions.\n";
	return true;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN {
	const Vector<String>& args = CommandLine();
	ClassifierOptions o = ParseArgs(args);
	
	if (o.help || o.dataset.IsEmpty() || o.model.IsEmpty() || (!o.train && !o.predict)) {
		Cout() << "Reviewed-Card Temporal Classifier U++ Utility\n"
		       << "Usage: VideoCardTemporalClassifier --dataset <manifest_file> --model <model_file> [options]\n\n"
		       << "Options:\n"
		       << "  --train                       Train the network on labeled samples\n"
		       << "  --predict                     Predict labels and update the manifest\n"
		       << "  --net-config <file>           Optional path to net configuration JSON file\n"
		       << "  --epochs <N>                  Number of epochs to train (default: 50)\n"
		       << "  --confidence-threshold <val>  Threshold for needs_review=false (default: 0.9)\n"
		       << "  --help, -h                    Show this help message\n";
		return;
	}
	
	if (o.train) {
		Cout() << "Mode: TRAINING\n";
		if (!DoTrain(o)) {
			Cerr() << "Training failed.\n";
			SetExitCode(1);
			return;
		}
	}
	
	if (o.predict) {
		Cout() << "Mode: PREDICTION\n";
		if (!DoPredict(o)) {
			Cerr() << "Prediction failed.\n";
			SetExitCode(1);
			return;
		}
	}
	
	Cout() << "Operation completed successfully.\n";
}
