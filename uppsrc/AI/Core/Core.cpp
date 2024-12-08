#include "Core.h"

#define IMAGECLASS TextImgs
#define IMAGEFILE <AI/Core/Images.iml>
#include <Draw/iml_source.h>

NAMESPACE_UPP


String GetGenderString(int i) {
	switch (i) {
		case GENDER_CHILD:		return "child";
		case GENDER_AUTHORITY:	return "authority";
		case GENDER_MALE:		return "male";
		case GENDER_FEMALE:		return "female";
		case GENDER_SELLER:		return "seller";
		case GENDER_BUYER:		return "buyer";
		case GENDER_MARKETER:	return "marketer";
		case GENDER_CONSUMER:	return "consumer";
		default: return String();
	}
}

Vector<String> GetGenders() {
	Vector<String> v;
	for(int i = 0; i < GENDER_COUNT; i++)
		v << GetGenderString(i);
	return v;
}

int FindGender(const String& s) {
	String ls = ToLower(TrimBoth(s));
	if (ls.IsEmpty()) return -1;
	if (IsDigit(ls[0]) || ls[0] == '-') {
		int i = ScanInt(ls);
		if (i < 0 || i >= GENDER_COUNT) return -1;
		return i;
	}
	for(int i = 0; i < GENDER_COUNT; i++)
		if (GetGenderString(i) == ls)
			return i;
	return -1;
}

END_UPP_NAMESPACE
