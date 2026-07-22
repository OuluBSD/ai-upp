#ifndef _AMP_AMPTemplateMatcher_h_
#define _AMP_AMPTemplateMatcher_h_

struct AmpTemplateMatchHit : Moveable<AmpTemplateMatchHit> {
	String id;
	int entry_index = -1;
	int x = -1;
	int y = -1;
	int score = INT_MAX;
	bool accepted = false;
};

struct AmpTemplateMatchResult : Moveable<AmpTemplateMatchResult> {
	Vector<AmpTemplateMatchHit> entries;
	int winner_index = -1;
	int winner_score = INT_MAX;
	int accepted = 0;
	int rejected = 0;
	int candidate_count = 0;
	int checksum = 0;
};

bool MatchAmpTemplatesCpu(const Vector<int>& frame, int frame_width, int frame_height,
                      const Vector<int>& atlas, const AmpTemplateAtlasManifest& manifest,
                      int threshold, AmpTemplateMatchResult& result, String& error);

bool MatchAmpTemplatesAmp(const Vector<int>& frame, int frame_width, int frame_height,
                      const Vector<int>& atlas, const AmpTemplateAtlasManifest& manifest,
                      int threshold, const String& device_path,
                      AmpTemplateMatchResult& result, String& error);

#endif
