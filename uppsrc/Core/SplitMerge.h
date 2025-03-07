Vector<String> Split(int maxcount, const char *s, const char * (*text_filter)(const char *), bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(int maxcount, const char *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(int maxcount, const char *s, int chr, bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(int maxcount, const char *s, const char *text, bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(const char *s, const char * (*text_filter)(const char *), bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(const char *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(const char *s, int chr, bool ignoreempty = true, bool ignoreescaped = false);
Vector<String> Split(const char *s, const char *text, bool ignoreempty = true, bool ignoreescaped = false);

Vector<WString> Split(int maxcount, const wchar *s, const wchar * (*text_filter)(const wchar *), bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(int maxcount, const wchar *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(int maxcount, const wchar *s, int chr, bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(int maxcount, const wchar *s, const wchar *text, bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(const wchar *s, const wchar * (*text_filter)(const wchar *), bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(const wchar *s, int (*filter)(int), bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(const wchar *s, int chr, bool ignoreempty = true, bool ignoreescaped = false);
Vector<WString> Split(const wchar *s, const wchar *text, bool ignoreempty = true, bool ignoreescaped = false);

String  Join(const Vector<String>& im, const String& delim, bool ignoreempty = false);
WString Join(const Vector<WString>& im, const WString& delim, bool ignoreempty = false);

template <typename... Args>
bool SplitTo(const char *s, int delim, bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const char *s, int delim, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
bool SplitTo(const char *s, int (*delim)(int), bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const char *s, int (*delim)(int), Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
bool SplitTo(const char *s, const char *delim, bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const char *s, const char *delim, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
void MergeWith(String& dest, const char *delim, const Args& ...args)
{
	foreach_arg([&](const String& arg) {
		if(arg.GetCount()) {
			if(dest.GetCount())
				dest << delim;
			dest << arg;
		}
	}, args...);
}

template <typename... Args>
String Merge(const char *delim, const Args& ...args)
{
	String r;
	MergeWith(r, delim, args...);
	return r;
}

template <typename... Args>
bool SplitTo(const wchar *s, int delim, bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, int delim, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, int (*delim)(int), bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, int (*delim)(int), Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, const wchar *delim, bool ignoreempty, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim, ignoreempty), args...);
}

template <typename... Args>
bool SplitTo(const wchar *s, const wchar *delim, Args& ...args)
{
	return scatter(Split(sizeof...(args), s, delim), args...);
}

template <typename... Args>
void MergeWith(WString& dest, const wchar *delim, const Args& ...args)
{
	foreach_arg([&](const WString& arg) {
		if(arg.GetCount()) {
			if(dest.GetCount())
				dest << delim;
			dest << arg;
		}
	}, args...);
}

template <typename... Args>
WString Merge(const wchar *delim, const Args& ...args)
{
	WString r;
	MergeWith(r, delim, args...);
	return r;
}
