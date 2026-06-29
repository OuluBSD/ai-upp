#include "Core/Core.h"

namespace Upp {

AppRegistry::AppRegistry()
{}

// ---------------------------------------------------------------------------
// Platform path helpers

static String sAiUppDir(const String& base, const String& vendor,
                         const String& app_id, const String& profile)
{
	return AppendFileName(AppendFileName(AppendFileName(base, vendor), app_id), profile);
}

String AppRegistry::GetConfigDir() const
{
	String base;
#ifdef PLATFORM_WIN32
	base = GetAppDataFolder();
	if(base.IsEmpty()) base = GetHomeDirFile(".config");
#else
	base = GetEnv("XDG_CONFIG_HOME");
	if(base.IsEmpty()) base = AppendFileName(GetHomeDirectory(), ".config");
#endif
	return sAiUppDir(base, vendor_, app_id_, profile_);
}

String AppRegistry::GetStateDir() const
{
	String base;
#ifdef PLATFORM_WIN32
	base = GetEnv("LOCALAPPDATA");
	if(base.IsEmpty()) base = GetAppDataFolder();
#else
	base = GetEnv("XDG_STATE_HOME");
	if(base.IsEmpty()) base = AppendFileName(GetHomeDirectory(), ".local/state");
#endif
	return sAiUppDir(base, vendor_, app_id_, profile_);
}

String AppRegistry::GetCacheDir() const
{
	String base;
#ifdef PLATFORM_WIN32
	String local = GetEnv("LOCALAPPDATA");
	if(local.IsEmpty()) local = GetAppDataFolder();
	base = AppendFileName(local, "cache");
#else
	base = GetEnv("XDG_CACHE_HOME");
	if(base.IsEmpty()) base = AppendFileName(GetHomeDirectory(), ".cache");
#endif
	return sAiUppDir(base, vendor_, app_id_, profile_);
}

// ---------------------------------------------------------------------------

String AppRegistry::RegistryPath() const
{
	return AppendFileName(GetConfigDir(), "registry.json");
}

String AppRegistry::SafeKey(const String& key) const
{
	String out;
	for(int i = 0; i < key.GetCount(); i++) {
		char c = key[i];
		out.Cat(IsAlNum(c) || c == '_' || c == '-' ? c : '_');
	}
	return out;
}

String AppRegistry::BlobDirPath() const
{
	return AppendFileName(GetStateDir(), "blob");
}

void AppRegistry::LogMsg(const String& msg) const
{
	log_.Add(String("[AppRegistry] ") + msg);
}

// ---------------------------------------------------------------------------
// Load / Save

bool AppRegistry::Load()
{
	String path = RegistryPath();
	LogMsg(String("Load: ") + path);
	String s = LoadFile(path);
	if(s.IsEmpty()) {
		LogMsg("Load: not found, using defaults");
		return false;
	}
	Value v = ParseJSON(s);
	if(v.IsError()) {
		LogMsg("Load: JSON parse error, using defaults");
		return false;
	}
	ValueMap vm(v);
	for(int i = 0; i < vm.GetCount(); i++) {
		String key = vm.GetKey(i);
		values_.GetAdd(key) = vm.GetValue(i);
	}
	LogMsg(Format("Load: OK, %d keys", values_.GetCount()));
	return true;
}

bool AppRegistry::Save() const
{
	String path = RegistryPath();
	RealizeDirectory(GetFileFolder(path));
	ValueMap vm;
	for(int i = 0; i < values_.GetCount(); i++)
		vm.Add(values_.GetKey(i), values_[i]);
	bool ok = SaveFile(path, AsJSON(Value(vm), true));
	LogMsg(ok ? String("Save: OK -> ") + path : String("Save: FAILED -> ") + path);
	return ok;
}

// ---------------------------------------------------------------------------
// Scalar get/set

void AppRegistry::Set(const String& key, const Value& value)
{
	values_.GetAdd(key) = value;
}

Value AppRegistry::Get(const String& key, const Value& def) const
{
	int i = values_.Find(key);
	return i >= 0 ? values_[i] : def;
}

// ---------------------------------------------------------------------------
// Blob

String AppRegistry::GetBlobPath(const String& key) const
{
	return AppendFileName(BlobDirPath(), SafeKey(key) + ".bin");
}

bool AppRegistry::SaveBlob(const String& key, const String& data, BlobMode mode)
{
	bool use_inline = (mode == BLOB_INLINE_BASE64) ||
	                  (mode == BLOB_AUTO && data.GetCount() <= kInlineThreshold);
	if(use_inline) {
		Set(String("blob:") + key, Base64Encode(data));
		LogMsg(Format("SaveBlob: inline key=%s size=%d", key, data.GetCount()));
		return true;
	}
	String path = GetBlobPath(key);
	RealizeDirectory(GetFileFolder(path));
	bool ok = SaveFile(path, data);
	if(ok) {
		Set(String("blobfile:") + key, path);
		LogMsg(String("SaveBlob: external key=") + key + " path=" + path);
	} else {
		LogMsg(String("SaveBlob: FAILED external key=") + key + " path=" + path);
	}
	return ok;
}

bool AppRegistry::LoadBlob(const String& key, String& out) const
{
	Value v = Get(String("blob:") + key, Null);
	if(!IsNull(v)) {
		out = Base64Decode(String(v));
		LogMsg(String("LoadBlob: inline key=") + key);
		return true;
	}
	v = Get(String("blobfile:") + key, Null);
	if(!IsNull(v)) {
		String path = String(v);
		out = LoadFile(path);
		if(out.IsVoid()) {
			LogMsg(String("LoadBlob: missing file key=") + key + " path=" + path);
			return false;
		}
		LogMsg(String("LoadBlob: external key=") + key);
		return true;
	}
	LogMsg(String("LoadBlob: not found key=") + key);
	return false;
}

}
