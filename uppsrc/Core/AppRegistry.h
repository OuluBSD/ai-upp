#ifndef _Core_AppRegistry_h_
#define _Core_AppRegistry_h_

class AppRegistry {
public:
	enum BlobMode {
		BLOB_AUTO,
		BLOB_INLINE_BASE64,
		BLOB_EXTERNAL_FILE,
	};

	AppRegistry();

	AppRegistry& Vendor(const String& id)   { vendor_ = id; return *this; }
	AppRegistry& AppId(const String& id)    { app_id_ = id; return *this; }
	AppRegistry& Profile(const String& id)  { profile_ = id; return *this; }

	String GetVendor()  const { return vendor_; }
	String GetAppId()   const { return app_id_; }
	String GetProfile() const { return profile_; }

	String GetConfigDir() const;
	String GetStateDir()  const;
	String GetCacheDir()  const;

	bool Load();
	bool Save() const;

	void  Set(const String& key, const Value& value);
	Value Get(const String& key, const Value& def = Null) const;

	template <class T>
	bool LoadJson(const String& key, T& obj) {
		String s = Get(key, Null);
		if(IsNull(s)) { LogMsg("LoadJson: key not found: " + key); return false; }
		LoadFromJson(obj, s);
		LogMsg("LoadJson: loaded key: " + key);
		return true;
	}

	template <class T>
	bool SaveJson(const String& key, const T& obj) {
		Set(key, StoreAsJson(obj));
		LogMsg("SaveJson: saved key: " + key);
		return true;
	}

	bool LoadBlob(const String& key, String& out) const;
	bool SaveBlob(const String& key, const String& data, BlobMode mode = BLOB_AUTO);

	String GetBlobPath(const String& key) const;

	const Vector<String>& GetLog() const { return log_; }
	void ClearLog()                       { log_.Clear(); }

private:
	String vendor_  = "AiUpp";
	String app_id_;
	String profile_ = "default";

	VectorMap<String, Value> values_;
	mutable Vector<String>   log_;

	static const int64 kInlineThreshold = 64 * 1024;

	String RegistryPath()   const;
	String StateDirForKey(const String& key) const;
	String BlobDirPath()    const;
	String SafeKey(const String& key) const;

	void LogMsg(const String& msg) const;
};

#endif
