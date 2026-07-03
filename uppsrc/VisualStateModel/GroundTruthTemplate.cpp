#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

VsmGroundTruthTemplateResult VsmGroundTruthTemplateGenerator::Generate(
	const VsmGroundTruthTemplateOptions& opts)
{
	VsmGroundTruthTemplateResult result;
	result.output_path = opts.output_path;

	// --- Step 1: Load manifest from the session directory ---
	VsmSessionStore store;
	if(!store.Open(opts.session_dir)) {
		LogError(log_, "VsmGT", "Cannot open session directory: " + opts.session_dir);
		return result;
	}

	const VsmSessionManifest& manifest = store.GetManifest();
	int frame_count = manifest.frames.GetCount();

	// --- Step 2: Get current UTC time ---
	Time t = GetUtcTime();
	String created_at = Format("%04d-%02d-%02dT%02d:%02d:%02d.000Z",
	                            t.year, t.month, t.day, t.hour, t.minute, t.second);

	// --- Step 3: Build the JSON structure manually as a string ---
	StringBuffer json_buf;

	json_buf << "{\n";
	json_buf << "  \"schema\": 1,\n";
	json_buf << "  \"producer\": {\n";
	json_buf << "    \"name\": \"VsmGroundTruthTemplateGenerator\",\n";
	json_buf << "    \"version\": \"0.1.0\",\n";
	json_buf << "    \"created_at\": \"" << created_at << "\"\n";
	json_buf << "  },\n";
	json_buf << "  \"session\": {\n";
	json_buf << "    \"id\": \"" << manifest.session_id << "\",\n";
	json_buf << "    \"source_type\": \"" << manifest.source_type << "\",\n";
	json_buf << "    \"frame_width\": " << manifest.frame_width << ",\n";
	json_buf << "    \"frame_height\": " << manifest.frame_height << ",\n";
	json_buf << "    \"started_at\": \"" << manifest.created_at << "\",\n";
	json_buf << "    \"ended_at\": \"" << manifest.created_at << "\",\n";
	json_buf << "    \"image_dir\": \"frames/\",\n";
	json_buf << "    \"crop_dir\": \"crops/\"\n";
	json_buf << "  },\n";
	json_buf << "  \"events\": [\n";

	// Add frame events
	for(int i = 0; i < frame_count; i++) {
		if(i > 0) json_buf << ",\n";
		json_buf << "    {\n";
		json_buf << "      \"type\": \"frame\",\n";
		json_buf << "      \"frame\": " << i << ",\n";
		json_buf << "      \"ts\": \"" << manifest.created_at << "\",\n";
		json_buf << "      \"image_file\": \"" << manifest.frames[i].relative_path << "\"\n";
		json_buf << "    }";
	}

	// Add exactly one example divergence entry
	if(frame_count > 0) json_buf << ",\n";
	json_buf << "    {\n";
	json_buf << "      \"type\": \"divergence\",\n";
	json_buf << "      \"frame\": 0,\n";
	json_buf << "      \"ts\": \"" << manifest.created_at << "\",\n";
	json_buf << "      \"severity\": \"warning\",\n";
	json_buf << "      \"message\": \"EXAMPLE - replace or remove\",\n";
	json_buf << "      \"region_id\": \"\",\n";
	json_buf << "      \"expected\": {},\n";
	json_buf << "      \"observed\": {}\n";
	json_buf << "    }\n";

	json_buf << "  ]\n";
	json_buf << "}\n";

	String json = json_buf;

	// --- Step 4: Write the template to output path ---
	if(!SaveFile(opts.output_path, json)) {
		LogError(log_, "VsmGT", "Cannot write template JSON: " + opts.output_path);
		return result;
	}

	LogInfo(log_, "VsmGT", Format("Generated template with %d frames", frame_count));

	result.success      = true;
	result.frame_count  = frame_count;
	result.session_id   = manifest.session_id;
	return result;
}

} // namespace Upp
