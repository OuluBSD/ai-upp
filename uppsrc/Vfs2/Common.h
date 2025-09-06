#ifndef _Vfs2_Common_h_
#define _Vfs2_Common_h_

struct PkgFile : Moveable<PkgFile> {
	hash_t pkg_hash = 0, file_hash = 0;
	
	PkgFile() {}
	PkgFile(PkgFile&& k) {*this = k;}
	PkgFile(const PkgFile& k) {*this = k;}
	PkgFile(hash_t pkg_hash, hash_t file_hash) : pkg_hash(pkg_hash), file_hash(file_hash) {}
	void operator=(const PkgFile& k) {file_hash = k.file_hash; pkg_hash = k.pkg_hash;}
	bool operator()(const PkgFile& a, const PkgFile& b) const {
		if (a.pkg_hash != b.pkg_hash) return a.pkg_hash < b.pkg_hash;
		return a.file_hash < b.file_hash;
	}
	hash_t GetHashValue() const {return CombineHash(pkg_hash, file_hash);}
	bool operator==(const PkgFile& p) const {return pkg_hash == p.pkg_hash && file_hash == p.file_hash;}
	String ToString() const {return Format("[%x:%x]", (int64)pkg_hash, (int64)file_hash);}
};

struct CodeArgs {
	typedef enum : int {
		SCOPE_COMMENTS,
		FUNCTIONALITY,
		
		FN_COUNT
	} Fn;
	typedef std::underlying_type<Fn>::type FnType;
	Fn fn;
	VectorMap<String, String> data;
	Vector<String> code;
	String lang;

	void Clear() {data.Clear(); code.Clear(); lang="";}
	void Jsonize(JsonIO& json) { json("fn", reinterpret_cast<FnType&>(fn))("data",data)("code",code)("lang",lang); }

	String Get() const { return StoreAsJson(*this); }
	void Put(const String& s) { LoadFromJson(*this, s); }
};

String GetRelSrcPath(const String& rel_path);

#endif
