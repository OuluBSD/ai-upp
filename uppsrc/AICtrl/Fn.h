#ifndef _AI_TextCtrl_Fn_h_
#define _AI_TextCtrl_Fn_h_

NAMESPACE_UPP

void SetColoredListValue(ArrayCtrl& list, int row, int col, const String& txt, Color clr, bool blend=true);
String CacheImageFile(String dir, hash_t h);
String ThumbnailImageFile(String dir, hash_t h);
String FullImageFile(String dir, hash_t h);
Image RescaleToFit(const Image& img, int smaller_side_length);

struct AiComponentCtrl : ComponentCtrl {
	const Index<String>& GetTypeclasses() const;
	const Vector<ContentType>& GetContents() const;
	const Vector<String>& GetContentParts() const;
};

END_UPP_NAMESPACE

#endif
