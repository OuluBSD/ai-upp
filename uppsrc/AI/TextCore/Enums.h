#ifndef _AI_TextCore_Enums_h_
#define _AI_TextCore_Enums_h_

NAMESPACE_UPP

enum {
	SCORE_IDEA,
	SCORE_EMOTION,
	SCORE_HOOK,
	SCORE_SHARE,
	SCORE_VALUE,
	SCORE_COMEDY,
	SCORE_SEX,
	SCORE_POLITICS,
	SCORE_LOVE,
	SCORE_SOCIAL,

	SCORE_COUNT
};

// TODO rename these to DB_DATA_SRC_TYPE or something
enum {
	DB_SONG,
	DB_TWITTER,
	DB_BLOG,
	DB_DIALOG,
	DB_STORYBOARD,
	DB_CODE,
	
	DB_COUNT
};

inline String GetAppModeString(int appmode) {
	switch (appmode) {
		case DB_SONG:			return "Song";
		case DB_TWITTER:		return "Twitter";
		case DB_BLOG:			return "Blog";
		case DB_DIALOG:			return "Dialog";
		case DB_STORYBOARD:		return "Storyboard";
		case DB_CODE:			return "Code";
		default: break;
	}
	return "<error>";
}

enum {
	STRESS_NONE,
	STRESS_PRIMARY,
	STRESS_SECONDARY,
};

enum {
#define ATTR_ITEM(e, g, i0, i1) e,
ATTR_LIST
#undef ATTR_ITEM
ATTR_COUNT
};

END_UPP_NAMESPACE

#endif
