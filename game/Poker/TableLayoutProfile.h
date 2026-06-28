#ifndef _GameCommon_TableLayoutProfile_h_
#define _GameCommon_TableLayoutProfile_h_

#include <Core/Core.h>

namespace Upp {

struct TexasTableLayout {
	static void SetProfile(const String& profile_name);
	static String GetProfile();
	static Vector<String> GetProfiles();
	static Size BaseSize();
	static Rect TableRect(Size sz);
	static Rect PotRect(Size sz);
	static Point PotTextPos(Size sz);
	static Point RoundTextPos(Size sz);
	static Rect BoardCardRect(Size sz, int index);
	static Rect PlayerRect(Size sz, int player_index);
	static Rect HeroCardRect(Size sz, int card_index);
};

}

#endif
