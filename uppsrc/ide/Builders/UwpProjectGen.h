#ifndef _ide_Builders_UwpProjectGen_h_
#define _ide_Builders_UwpProjectGen_h_

String UwpTemplate(const unsigned char *data, int len);
Uuid   UwpGuidFromString(const String& seed);
String UwpGuidString(const String& seed);
String ReplaceUwpTokens(String text, const VectorMap<String, String>& tokens);
String MakeUwpRelativePath(const String& root, const String& path);
String ToWindowsPath(const String& path);
String XmlEscape(const String& s);
String JoinUwpList(const Vector<String>& items, const char *sep);
bool   IsCompileSourceFile(const String& path);
bool   IsHeaderFile(const String& path);
bool   IsCSharpFile(const String& path);
void   CollectUwpProjectFiles(Index<String>& files, const String& source, const String& target);
String MakeItemList(const Index<String>& files, const String& root, const String& tag);
String MakeFilteredItemList(const Index<String>& files, const String& root, const String& tag, const String& filter);
String MakeContentItemList(const Index<String>& files, const String& root);
String MakeFilteredContentItemList(const Index<String>& files, const String& root, const String& filter);
void   WriteUwpAssets(const String& assets_dir);
bool   StageUwpFile(const String& source, const String& target, bool& updated);

#endif
