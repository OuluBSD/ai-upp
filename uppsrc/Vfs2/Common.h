#ifndef _Vfs2_Common_h_
#define _Vfs2_Common_h_

struct PkgFile : Moveable<PkgFile> {
	int pkg = -1, file = -1;
	
	PkgFile() {}
	PkgFile(PkgFile&& k) {*this = k;}
	PkgFile(const PkgFile& k) {*this = k;}
	PkgFile(int pkg, int file) : pkg(pkg), file(file) {}
	void operator=(const PkgFile& k) {file = k.file; pkg = k.pkg;}
	bool operator()(const PkgFile& a, const PkgFile& b) const {
		if (a.pkg != b.pkg) return a.pkg < b.pkg;
		return a.file < b.file;
	}
	hash_t GetHashValue() const {return CombineHash(pkg, file);}
	bool operator==(const PkgFile& p) const {return pkg == p.pkg && file == p.file;}
	String ToString() const {return Format("[%d:%d]", pkg, file);}
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

#endif
