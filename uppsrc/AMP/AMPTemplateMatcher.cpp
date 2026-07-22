#include "AMP.h"

NAMESPACE_UPP

static int AbsInt(int value)
{
	return value < 0 ? -value : value;
}

bool MatchAmpTemplatesCpu(const Vector<int>& frame, int frame_width, int frame_height,
	                      const Vector<int>& atlas, const AmpTemplateAtlasManifest& manifest,
	                      int threshold, AmpTemplateMatchResult& result, String& error)
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

END_UPP_NAMESPACE
