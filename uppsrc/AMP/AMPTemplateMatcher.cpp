#include "AMP.h"

NAMESPACE_UPP

static int AbsInt(int value)
{
	return value < 0 ? -value : value;
}

static int TemplatePixelScore(int frame_rgb, int atlas_rgb, int frame_gray,
	                          int atlas_gray, AmpTemplatePreprocessing mode,
	                          bool& supported)
{
	if(mode == AMP_TEMPLATE_RGB)
		return AbsInt((int)AmpRgbRed(frame_rgb) - AmpRgbRed(atlas_rgb)) +
		       AbsInt((int)AmpRgbGreen(frame_rgb) - AmpRgbGreen(atlas_rgb)) +
		       AbsInt((int)AmpRgbBlue(frame_rgb) - AmpRgbBlue(atlas_rgb));
	if(mode == AMP_TEMPLATE_GRAY || mode == AMP_TEMPLATE_OTSU)
		return AbsInt(frame_gray - atlas_gray);
	supported = false;
	return 0;
}

bool MatchAmpTemplatePixelsCpu(const AmpTemplatePixelBuffer& frame,
	                           const AmpTemplatePixelBuffer& atlas,
	                           const AmpTemplateAtlasManifest& manifest,
	                           int threshold, AmpTemplateMatchResult& result,
	                           String& error)
{
	if(!frame.IsValid() || !atlas.IsValid() ||
	   atlas.width != manifest.atlas_width ||
	   atlas.height != manifest.atlas_height) {
		error = "invalid dual-channel pixel dimensions";
		return false;
	}
	if(threshold < 0) {
		error = "threshold must not be negative";
		return false;
	}
	result = AmpTemplateMatchResult();
	for(int entry_index = 0; entry_index < manifest.entries.GetCount(); entry_index++) {
		const AmpTemplateAtlasEntry& entry = manifest.entries[entry_index];
		AmpTemplatePreprocessing mode;
		if(!ParseAmpTemplatePreprocessing(entry.preprocessing, mode, error))
			return false;
		AmpTemplateMatchHit& hit = result.entries.Add();
		hit.id = entry.id;
		hit.entry_index = entry_index;
		for(int y = 0; y + entry.height <= frame.height; y++)
			for(int x = 0; x + entry.width <= frame.width; x++) {
				result.candidate_count++;
				int score = 0;
				bool supported = true;
				for(int ty = 0; ty < entry.height && supported; ty++)
					for(int tx = 0; tx < entry.width; tx++)
						score += TemplatePixelScore(
							frame.rgb[(y + ty) * frame.width + x + tx],
							atlas.rgb[(entry.y + ty) * atlas.width + entry.x + tx],
							frame.gray[(y + ty) * frame.width + x + tx],
							atlas.gray[(entry.y + ty) * atlas.width + entry.x + tx],
							mode, supported);
				if(!supported) {
					error = Format("unsupported preprocessing for entry: %s", ~entry.id);
					return false;
				}
				if(score < hit.score) {
					hit.score = score;
					hit.x = x;
					hit.y = y;
				}
			}
		hit.accepted = hit.score <= threshold;
		if(hit.accepted) {
			result.accepted++;
			if(hit.score < result.winner_score) {
				result.winner_score = hit.score;
				result.winner_index = entry_index;
			}
		}
		else
			result.rejected++;
	}
	for(int value : frame.rgb)
		result.checksum = (result.checksum * 31 + value) & 0x7fffffff;
	return true;
}

static bool CheckMatchInput(const Vector<int>& frame, int frame_width, int frame_height,
	                         const Vector<int>& atlas,
	                         const AmpTemplateAtlasManifest& manifest, int threshold,
	                         String& error)
{
	if(frame_width <= 0 || frame_height <= 0 || frame.GetCount() != frame_width * frame_height) {
		error = "invalid frame dimensions";
		return false;
	}
	if(atlas.GetCount() != manifest.atlas_width * manifest.atlas_height) {
		error = "invalid atlas dimensions";
		return false;
	}
	if(threshold < 0) {
		error = "threshold must not be negative";
		return false;
	}
	return true;
}

bool MatchAmpTemplatesCpu(const Vector<int>& frame, int frame_width, int frame_height,
	                      const Vector<int>& atlas, const AmpTemplateAtlasManifest& manifest,
	                      int threshold, AmpTemplateMatchResult& result, String& error)
{
	if(!CheckMatchInput(frame, frame_width, frame_height, atlas, manifest, threshold, error))
		return false;
	result = AmpTemplateMatchResult();
	for(int entry_index = 0; entry_index < manifest.entries.GetCount(); entry_index++) {
		const AmpTemplateAtlasEntry& entry = manifest.entries[entry_index];
		AmpTemplateMatchHit& hit = result.entries.Add();
		hit.id = entry.id;
		hit.entry_index = entry_index;
		for(int y = 0; y + entry.height <= frame_height; y++)
			for(int x = 0; x + entry.width <= frame_width; x++) {
				result.candidate_count++;
				int score = 0;
				for(int ty = 0; ty < entry.height; ty++)
					for(int tx = 0; tx < entry.width; tx++) {
						int frame_value = frame[(y + ty) * frame_width + x + tx];
						int atlas_value = atlas[(entry.y + ty) * manifest.atlas_width + entry.x + tx];
						score += AbsInt(frame_value - atlas_value);
					}
				if(score < hit.score) {
					hit.score = score;
					hit.x = x;
					hit.y = y;
				}
			}
		hit.accepted = hit.score <= threshold;
		if(hit.accepted) {
			result.accepted++;
			if(hit.score < result.winner_score) {
				result.winner_score = hit.score;
				result.winner_index = entry_index;
			}
		}
		else
			result.rejected++;
	}
	for(int value : frame)
		result.checksum = (result.checksum * 31 + value) & 0x7fffffff;
	return true;
}

bool MatchAmpTemplatePixelsAmp(const AmpTemplatePixelBuffer& frame,
	                           const AmpTemplatePixelBuffer& atlas,
	                           const AmpTemplateAtlasManifest& manifest,
	                           int threshold, const String& device_path,
	                           AmpTemplateMatchResult& result, String& error)
{
	if(!frame.IsValid() || !atlas.IsValid() ||
	   atlas.width != manifest.atlas_width ||
	   atlas.height != manifest.atlas_height) {
		error = "invalid dual-channel pixel dimensions";
		return false;
	}
	if(threshold < 0) {
		error = "threshold must not be negative";
		return false;
	}
	Vector<int> candidate_entry, candidate_x, candidate_y;
	Vector<int> entry_x, entry_y, entry_width, entry_height, entry_mode;
	for(int entry_index = 0; entry_index < manifest.entries.GetCount(); entry_index++) {
		const AmpTemplateAtlasEntry& entry = manifest.entries[entry_index];
		AmpTemplatePreprocessing mode;
		if(!ParseAmpTemplatePreprocessing(entry.preprocessing, mode, error))
			return false;
		entry_x.Add(entry.x);
		entry_y.Add(entry.y);
		entry_width.Add(entry.width);
		entry_height.Add(entry.height);
		entry_mode.Add((int)mode);
		for(int y = 0; y + entry.height <= frame.height; y++)
			for(int x = 0; x + entry.width <= frame.width; x++) {
				candidate_entry.Add(entry_index);
				candidate_x.Add(x);
				candidate_y.Add(y);
			}
	}
#ifndef HAVE_SYSTEM_AMP
	return MatchAmpTemplatePixelsCpu(frame, atlas, manifest, threshold, result, error);
#else
	if(device_path.IsEmpty()) {
		error = "native AMP device path is required";
		return false;
	}
	Vector<int> scores;
	scores.SetCount(candidate_entry.GetCount());
	concurrency::array_view<int, 1> frame_rgb_view(frame.rgb.GetCount(),
	                                               const_cast<int*>(frame.rgb.Begin()));
	concurrency::array_view<int, 1> frame_gray_view(frame.gray.GetCount(),
	                                                const_cast<int*>(frame.gray.Begin()));
	concurrency::array_view<int, 1> atlas_rgb_view(atlas.rgb.GetCount(),
	                                               const_cast<int*>(atlas.rgb.Begin()));
	concurrency::array_view<int, 1> atlas_gray_view(atlas.gray.GetCount(),
	                                                const_cast<int*>(atlas.gray.Begin()));
	concurrency::array_view<int, 1> candidate_entry_view(candidate_entry.GetCount(), candidate_entry.Begin());
	concurrency::array_view<int, 1> candidate_x_view(candidate_x.GetCount(), candidate_x.Begin());
	concurrency::array_view<int, 1> candidate_y_view(candidate_y.GetCount(), candidate_y.Begin());
	concurrency::array_view<int, 1> entry_x_view(entry_x.GetCount(), entry_x.Begin());
	concurrency::array_view<int, 1> entry_y_view(entry_y.GetCount(), entry_y.Begin());
	concurrency::array_view<int, 1> entry_width_view(entry_width.GetCount(), entry_width.Begin());
	concurrency::array_view<int, 1> entry_height_view(entry_height.GetCount(), entry_height.Begin());
	concurrency::array_view<int, 1> entry_mode_view(entry_mode.GetCount(), entry_mode.Begin());
	concurrency::array_view<int, 1> score_view(scores.GetCount(), scores.Begin());
	int atlas_width = atlas.width;
	int frame_width = frame.width;
	parallel_for_each(concurrency::accelerator(device_path.ToWString().ToStd()).default_view,
	                 score_view.extent,
	                 [=](concurrency::index<1> index) PARALLEL_AMP {
		int candidate = index[0];
		int entry_index = candidate_entry_view[candidate];
		int score = 0;
		int mode = entry_mode_view[entry_index];
		for(int ty = 0; ty < entry_height_view[entry_index]; ty++)
			for(int tx = 0; tx < entry_width_view[entry_index]; tx++) {
				int frame_index = (candidate_y_view[candidate] + ty) * frame_width +
				                  candidate_x_view[candidate] + tx;
				int atlas_index = (entry_y_view[entry_index] + ty) * atlas_width +
				                  entry_x_view[entry_index] + tx;
				if(mode == AMP_TEMPLATE_RGB) {
					int a = frame_rgb_view[frame_index];
					int b = atlas_rgb_view[atlas_index];
					int d = (a & 255) - (b & 255);
					score += d < 0 ? -d : d;
					d = ((a >> 8) & 255) - ((b >> 8) & 255);
					score += d < 0 ? -d : d;
					d = ((a >> 16) & 255) - ((b >> 16) & 255);
					score += d < 0 ? -d : d;
				}
				else {
					int d = frame_gray_view[frame_index] - atlas_gray_view[atlas_index];
					score += d < 0 ? -d : d;
				}
			}
		score_view[candidate] = score;
	});
	score_view.synchronize();
	result = AmpTemplateMatchResult();
	result.candidate_count = candidate_entry.GetCount();
	for(int entry_index = 0; entry_index < manifest.entries.GetCount(); entry_index++) {
		AmpTemplateMatchHit& hit = result.entries.Add();
		hit.id = manifest.entries[entry_index].id;
		hit.entry_index = entry_index;
		for(int candidate = 0; candidate < candidate_entry.GetCount(); candidate++)
			if(candidate_entry[candidate] == entry_index && scores[candidate] < hit.score) {
				hit.score = scores[candidate];
				hit.x = candidate_x[candidate];
				hit.y = candidate_y[candidate];
			}
		hit.accepted = hit.score <= threshold;
		if(hit.accepted) {
			result.accepted++;
			if(hit.score < result.winner_score) {
				result.winner_score = hit.score;
				result.winner_index = entry_index;
			}
		}
		else
			result.rejected++;
	}
	for(int value : frame.rgb)
		result.checksum = (result.checksum * 31 + value) & 0x7fffffff;
	return true;
#endif
}

bool MatchAmpTemplatesAmp(const Vector<int>& frame, int frame_width, int frame_height,
	                      const Vector<int>& atlas, const AmpTemplateAtlasManifest& manifest,
	                      int threshold, const String& device_path,
	                      AmpTemplateMatchResult& result, String& error)
{
	if(!CheckMatchInput(frame, frame_width, frame_height, atlas, manifest, threshold, error))
		return false;
#ifdef HAVE_SYSTEM_AMP
	if(device_path.IsEmpty()) {
		error = "native AMP device path is required";
		return false;
	}
#endif
	Vector<int> candidate_entry, candidate_x, candidate_y;
	Vector<int> entry_x, entry_y, entry_width, entry_height;
	for(int entry_index = 0; entry_index < manifest.entries.GetCount(); entry_index++) {
		const AmpTemplateAtlasEntry& entry = manifest.entries[entry_index];
		entry_x.Add(entry.x);
		entry_y.Add(entry.y);
		entry_width.Add(entry.width);
		entry_height.Add(entry.height);
		for(int y = 0; y + entry.height <= frame_height; y++)
			for(int x = 0; x + entry.width <= frame_width; x++) {
				candidate_entry.Add(entry_index);
				candidate_x.Add(x);
				candidate_y.Add(y);
			}
	}
	Vector<int> scores;
	scores.SetCount(candidate_entry.GetCount());
	concurrency::array_view<int, 1> frame_view(frame.GetCount(), const_cast<int*>(frame.Begin()));
	concurrency::array_view<int, 1> atlas_view(atlas.GetCount(), const_cast<int*>(atlas.Begin()));
	concurrency::array_view<int, 1> entry_view(candidate_entry.GetCount(), candidate_entry.Begin());
	concurrency::array_view<int, 1> x_view(candidate_x.GetCount(), candidate_x.Begin());
	concurrency::array_view<int, 1> y_view(candidate_y.GetCount(), candidate_y.Begin());
	concurrency::array_view<int, 1> entry_x_view(entry_x.GetCount(), entry_x.Begin());
	concurrency::array_view<int, 1> entry_y_view(entry_y.GetCount(), entry_y.Begin());
	concurrency::array_view<int, 1> entry_width_view(entry_width.GetCount(), entry_width.Begin());
	concurrency::array_view<int, 1> entry_height_view(entry_height.GetCount(), entry_height.Begin());
	concurrency::array_view<int, 1> score_view(scores.GetCount(), scores.Begin());
	int atlas_width = manifest.atlas_width;
	int frame_width_copy = frame_width;
	parallel_for_each(
#ifdef HAVE_SYSTEM_AMP
		concurrency::accelerator(device_path.ToWString().ToStd()).default_view,
#endif
		score_view.extent,
		[=](concurrency::index<1> index) PARALLEL_AMP {
			int candidate = index[0];
			int entry_index = entry_view[candidate];
			int score = 0;
			for(int ty = 0; ty < entry_height_view[entry_index]; ty++)
				for(int tx = 0; tx < entry_width_view[entry_index]; tx++) {
					int a = frame_view[(y_view[candidate] + ty) * frame_width_copy + x_view[candidate] + tx];
					int b = atlas_view[(entry_y_view[entry_index] + ty) * atlas_width + entry_x_view[entry_index] + tx];
					int d = a - b;
					score += d < 0 ? -d : d;
				}
			score_view[candidate] = score;
	});
	score_view.synchronize();
	result = AmpTemplateMatchResult();
	result.candidate_count = candidate_entry.GetCount();
	for(int entry_index = 0; entry_index < manifest.entries.GetCount(); entry_index++) {
		AmpTemplateMatchHit& hit = result.entries.Add();
		hit.id = manifest.entries[entry_index].id;
		hit.entry_index = entry_index;
		for(int candidate = 0; candidate < candidate_entry.GetCount(); candidate++)
			if(candidate_entry[candidate] == entry_index && scores[candidate] < hit.score) {
				hit.score = scores[candidate];
				hit.x = candidate_x[candidate];
				hit.y = candidate_y[candidate];
			}
		hit.accepted = hit.score <= threshold;
		if(hit.accepted) {
			result.accepted++;
			if(hit.score < result.winner_score) {
				result.winner_score = hit.score;
				result.winner_index = entry_index;
			}
		}
		else
			result.rejected++;
	}
	for(int value : frame)
		result.checksum = (result.checksum * 31 + value) & 0x7fffffff;
	return true;
}

END_UPP_NAMESPACE
