#ifndef _AMP_AMPTemplateAtlas_h_
#define _AMP_AMPTemplateAtlas_h_

struct AmpTemplateAtlasEntry : Moveable<AmpTemplateAtlasEntry> {
	String id;
	String kind;
	String scale;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	String preprocessing;
	int rotation_min = 0;
	int rotation_max = 0;
	int rotation_step = 0;
	float threshold = 0;
};

struct AmpTemplateAtlasManifest : Moveable<AmpTemplateAtlasManifest> {
	String format = "amp-template-atlas-v1";
	String atlas_name;
	int atlas_width = 0;
	int atlas_height = 0;
	Vector<AmpTemplateAtlasEntry> entries;

	bool Validate(String& error) const;
	bool Save(const String& path) const;
	bool Load(const String& path, String& error);
};

#endif
