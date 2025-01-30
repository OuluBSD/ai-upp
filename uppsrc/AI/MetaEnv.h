#ifndef _AI_MetaSrcPkg_h_
#define _AI_MetaSrcPkg_h_

#define DEBUG_METANODE_DTOR 0
#define USE_META_BIN 1

#if USE_META_BIN
	#define META_FILENAME "Meta.bin"
	#define META_EXISTS_FN FileExists
#else
	#define META_FILENAME "Meta.d"
	#define META_EXISTS_FN DirectoryExists
#endif

struct FileAnnotation;

NAMESPACE_UPP

struct NodeVisitor {
	JsonIO* json = 0;
	Stream* stream = 0;
	VersionControlSystem* vcs = 0;
	CombineHash hash;
	int mode = -1;
	int file_ver = -1;
	bool skip = false;
	enum {MODE_JSON, MODE_STREAM, MODE_HASH, MODE_VCS};
	bool storing = false;
	String error;
	
	typedef NodeVisitor CLASSNAME;
	NodeVisitor(JsonIO& j) {json = &j; mode = MODE_JSON; storing = j.IsStoring();}
	NodeVisitor(Stream& s) {stream = &s; mode = MODE_STREAM; storing = s.IsStoring();}
	NodeVisitor(VersionControlSystem& v) {vcs = &v; mode = MODE_VCS; storing = vcs->IsStoring();}
	NodeVisitor(hash_t) {mode = MODE_HASH; storing = true;}
	template <class T> void DoHash(T& o) {hash.Do(o);}
	bool IsLoading() const {return !storing;}
	bool IsStoring() const {return storing;}
	bool IsHashing() const {return mode == MODE_HASH;}
	bool IsError() const {return !error.IsEmpty();}
	void SetError(String s) {error = s;}
	
	
	template<class T>
	NodeVisitor& Visit(const char* key, T& o) {
		if      (mode == MODE_STREAM) o.Visit(*this);
		else if (mode == MODE_JSON) json->Var(key, o, THISBACK(VisitJsonItem<T>));
		else if (mode == MODE_HASH) o.Visit(*this);
		else if (mode == MODE_VCS) {
			vcs->BeginObject(key);
			o.Visit(*this);
			vcs->End();
		}
		return *this;
	}
	
	
	
	// VisitVector functions are for Vector<T> or Array<T> etc.
	
	template<class T>
	void VisitVectorSerialize(T& o) {
		Ver(1)(1);
		int count = o.GetCount();
		(*stream) / count;
		if (!storing)
			o.SetCount(count);
		for (auto& v : o)
			v.Visit(*this);
	}
	template<class T>
	void VisitJsonItem(JsonIO& j, T& o) {
		NodeVisitor v(j);
		o.Visit(v);
	}
	template<class T>
	void VisitVectorJson(const char* key, T& o) {
		json->Array(key, o, THISBACK(VisitJsonItem<typename T::value_type>));
	}
	template<class T>
	void VisitVectorHash(T& o) {
		hash.Put(o.GetCount());
		for (auto& v : o)
			v.Visit(*this);
	}
	template<class T>
	void VisitVectorVcs(String key, T& o) {
		vcs->BeginVector(key, o);
		int i = 0;
		for (auto& v : o) {
			vcs->BeginAt(i++);
			v.Visit(*this);
			vcs->End();
		}
		vcs->End();
	}
	
	template<class T>
	NodeVisitor& VisitVector(const char* key, T& o) {
		if      (mode == MODE_STREAM) VisitVectorSerialize<T>(o);
		else if (mode == MODE_JSON) VisitVectorJson<T>(key, o);
		else if (mode == MODE_HASH) VisitVectorHash<T>(o);
		else if (mode == MODE_VCS) VisitVectorVcs<T>(key, o);
		return *this;
	}
	
	
	
	// VisitVectorVector functions are for Vector<Vector<T>> or Array<Array<T>> etc.
	
	template<class T>
	void VisitVectorVectorSerialize(T& o) {
		Ver(1)(1);
		int count = o.GetCount();
		(*stream) / count;
		if (!storing)
			o.SetCount(count);
		for (auto& v : o)
			VisitVectorSerialize(v);
	}
	template<class T>
	void VisitVectorVectorItem(JsonIO& j, T& o) {
		JsonizeArray(j, o, THISBACK(VisitJsonItem<typename T::value_type>));
	}
	template<class T>
	void VisitVectorVectorJson(const char* key, T& o) {
		json->Array(key, o, THISBACK(VisitVectorVectorItem<typename T::value_type>));
	}
	template<class T>
	void VisitVectorVectorHash(T& o) {
		hash.Put(o.GetCount());
		for (auto& v : o)
			VisitVectorHash(v);
	}
	template<class T>
	void VisitVectorVectorVcs(String key, T& o) {
		vcs->BeginVector(key, o);
		int i = 0;
		for (auto& v : o) {
			vcs->BeginAt(i++);
			VisitVectorHash(v);
			vcs->End();
		}
		vcs->End();
	}
	
	template<class T>
	NodeVisitor& VisitVectorVector(const char* key, T& o) {
		if      (mode == MODE_STREAM) VisitVectorVectorSerialize<T>(o);
		else if (mode == MODE_JSON) VisitVectorVectorJson<T>(key, o);
		else if (mode == MODE_HASH) VisitVectorVectorHash<T>(o);
		else if (mode == MODE_VCS) VisitVectorVectorVcs<T>(key, o);
		return *this;
	}
	
	
	
	// VisitMap functions are for those VectorMaps & ArrayMaps,
	// which has standard Jsonize function for Key class (no Visit(MetaNode&))
	
	template<class T>
	void VisitMapSerialize(T& o) {
		using KeyType = decltype(o.PopKey());
		Ver(1)(1);
		int count = o.GetCount();
		(*stream) / count;
		if (storing) {
			for (auto it : ~o) {
				*stream / const_cast<KeyType&>(it.key);
				it.value.Visit(*this);
			}
		}
		else {
			o.Clear();
			for(int i = 0; i < count; i++) {
				KeyType key;
				*stream / key;
				o.Add(key).Visit(*this);
			}
		}
	}
	template <class T>
	void VisitFromJsonValue(T& var, const Value& x)
	{
		JsonIO io(x);
		NodeVisitor vis(io);
		var.Visit(vis);
	}
	template <class T>
	Value VisitAsJsonValue(const T& var)
	{
		JsonIO io;
		NodeVisitor vis(io);
		const_cast<T&>(var).Visit(vis);
		return io.GetResult();
	}
	
	template<class T>
	void VisitMapJson(String key, T& map) {
		using K = decltype(map.PopKey());
		using V = typename T::value_type;
		if (!storing) {
			map.Clear();
			const Value& va = this->json->Get()[key];
			map.Reserve(va.GetCount());
			for(int i = 0; i < va.GetCount(); i++) {
				K key;
				V value;
				LoadFromJsonValue(key, va[i]["key"]);
				VisitFromJsonValue(value, va[i]["value"]);
				map.Add(key, pick(value));
			}
		}
		else  {
			Vector<Value> va;
			va.SetCount(map.GetCount());
			for(int i = 0; i < map.GetCount(); i++) {
				ValueMap item;
				JsonIO json;
				item.Add("key", StoreAsJsonValue(map.GetKey(i)));
				item.Add("value", VisitAsJsonValue(map[i]));
				va[i] = item;
			}
			this->json->Set(key, ValueArray(pick(va)));
		}
	}
	template<class T>
	void VisitMapHash(T& o) {
		hash.Put(o.GetCount());
		for (auto& v : o)
			v.Visit(*this);
	}
	template<class T>
	void VisitMapVcs(String key, T& o) {
		vcs->BeginMap(key, o);
		int i = 0;
		for (auto v : ~o) {
			if (storing)
				vcs->BeginKeyStore(i++, v.key);
			else
				vcs->BeginAt(i++);
			v.value.Visit(*this);
			vcs->End();
		}
		vcs->End();
	}
	template<class T>
	NodeVisitor& VisitMap(const char* key, T& o) {
		if      (mode == MODE_STREAM) VisitMapSerialize<T>(o);
		else if (mode == MODE_JSON) VisitMapJson<T>(key, o);
		else if (mode == MODE_HASH) VisitMapHash<T>(o);
		else if (mode == MODE_VCS) VisitMapVcs<T>(key, o);
		return *this;
	}
	
	
	
	// VisitMapKV functions are for those VectorMaps & ArrayMaps,
	// which has custom Key class with Visit(MetaNode&) instead of Jsonzie(JsonIO&)
	
	template<class T>
	void VisitMapKVSerialize(T& o) {
		using KeyType = decltype(o.PopKey());
		Ver(1)(1);
		int count = o.GetCount();
		(*stream) / count;
		if (storing) {
			for (auto it : ~o) {
				const_cast<KeyType&>(it.key).Visit(*this);
				it.value.Visit(*this);
			}
		}
		else {
			o.Clear();
			for(int i = 0; i < count; i++) {
				KeyType key;
				key.Visit(*this);
				o.Add(key).Visit(*this);
			}
		}
	}
	template<class T>
	void VisitMapKVItem(JsonIO& j, T& o) {
		NodeVisitor v(j);
		o.Visit(v);
	}
	template<class T>
	void VisitMapKVJson(String key, T& map) {
		using K = decltype(map.PopKey());
		using V = typename T::value_type;
		if (!storing) {
			map.Clear();
			const Value& va = this->json->Get()[key];
			map.Reserve(va.GetCount());
			for(int i = 0; i < va.GetCount(); i++) {
				K key;
				V value;
				VisitFromJsonValue(key, va[i]["key"]);
				VisitFromJsonValue(value, va[i]["value"]);
				map.Add(key, pick(value));
			}
		}
		else  {
			Vector<Value> va;
			va.SetCount(map.GetCount());
			for(int i = 0; i < map.GetCount(); i++) {
				ValueMap item;
				JsonIO json;
				item.Add("key", VisitAsJsonValue(map.GetKey(i)));
				item.Add("value", VisitAsJsonValue(map[i]));
				va[i] = item;
			}
			this->json->Set(key, ValueArray(pick(va)));
		}
	}
	template<class T>
	void VisitMapKVHash(T& o) {
		hash.Put(o.GetCount());
		for (auto& v : o)
			v.Visit(*this);
	}
	template<class T>
	void VisitMapKVVcs(const String& key, T& o) {
		using KeyType = decltype(o.PopKey());
		vcs->BeginMapKV<T>(key, o);
		int i = 0;
		for (auto v : ~o) {
			vcs->BeginKV(i++); {
				if (storing) {
					vcs->BeginKeyVisit();{
						const_cast<KeyType&>(v.key).Visit(*this);}
					vcs->End();
				}
				vcs->BeginValueVisit();{
					v.value.Visit(*this);}
				vcs->End();}
			vcs->End();
		}
		vcs->End();
	}
	template<class T>
	NodeVisitor& VisitMapKV(const char* key, T& o) {
		if      (mode == MODE_STREAM) VisitMapKVSerialize<T>(o);
		else if (mode == MODE_JSON) VisitMapKVJson<T>(key, o);
		else if (mode == MODE_HASH) VisitMapKVHash<T>(o);
		else if (mode == MODE_VCS) VisitMapKVVcs<T>(key, o);
		return *this;
	}
	
	
	
	
	NodeVisitor& Ver(int version) {
		if (mode != MODE_STREAM) return *this;
		*stream % version;
		file_ver = version;
		skip = false;
		return *this;
	}
	NodeVisitor& operator()(int version) {
		if (mode != MODE_STREAM) return *this;
		skip = file_ver < version;
		return *this;
	}
	template <class T> NodeVisitor& operator()(const char* key, T& o) {
		if (skip) return *this;
		switch (mode) {
			case MODE_JSON: (*json)(key,o); return *this;
			case MODE_STREAM: *stream % o; return *this;
			case MODE_HASH: DoHash<T>(o); return *this;
			case MODE_VCS: vcs->Do(key, o); return *this;
			default: return *this;
		}
	}
	
	
	template<class T> NodeVisitor& operator()(const char* key, T& o, int) {return VisitVector(key, o);}
	template<class T> NodeVisitor& operator()(const char* key, T& o, int, int) {return VisitMap(key, o);}
	template<class T> NodeVisitor& operator()(const char* key, T& o, int, int, int) {return VisitKVMap(key, o);}
	template<class T> NodeVisitor& operator()(const char* key, T& o, int, int, int, int) {return VisitVectorVector(key, o);}
	template<class T> NodeVisitor& operator()(const char* key, T& o, int, int, int, int, int) {return Visit(key, o);}
	
	#define VISIT_VECTOR 0
	#define VISIT_MAP 0,0
	#define VISIT_MAP_KV 0,0,0
	#define VISIT_VECTOR_VECTOR 0,0,0,0
	#define VISIT_NODE 0,0,0,0,0
};

template <class T>
inline void VersionControlSystem::BeginMapKV(String key, T& o) {
	using KeyType = decltype(o.PopKey());
	Push(ST_MAP_KV, key);
	if (storing)
		RemoveIdxFolders(o.GetCount());
	else {
		o.Clear();
		String dir = GetCurrentDirectory();
		int count = ResolveCount();
		o.Reserve(count);
		for(int i = 0; i < count; i++) {
			BeginKeyVisit();
			KeyType key;
			NodeVisitor vis(*this);
			key.Visit(vis);
			o.Add(pick(key));
			End();
		}
	}
}

template <> inline void NodeVisitor::DoHash<Index<int>>(Index<int>& o) {hash.Do(o.GetKeys());}
template <> inline void NodeVisitor::DoHash<Index<String>>(Index<String>& o) {hash.Do(o.GetKeys());}

bool MakeRelativePath(const String& includes, const String& dir, String& best_ai_dir, String& best_rel_dir);
Vector<String> FindParentUppDirectories(const String& dir);

struct MetaNodeSubset;


// Don't change the order! It breaks MetaNode serialization and all user files
enum {
	METAKIND_BEGIN = 1000,
	
	METAKIND_DATABASE_SOURCE,
	METAKIND_PKG_ENV,
	METAKIND_CONTEXT,
	METAKIND_DB_REF,
	METAKIND_COMMENT,
	METAKIND_ECS_SPACE,
	
	METAKIND_EXTENSION_BEGIN = 2000,
	
	METAKIND_ECS_ENTITY,
	
	METAKIND_ECS_COMPONENT_BEGIN = 3000,
	METAKIND_ECS_COMPONENT_UNDEFINED = METAKIND_ECS_COMPONENT_BEGIN,
	
	METAKIND_ECS_COMPONENT_PROFILE = 3100,
	METAKIND_ECS_COMPONENT_OWNER,
	METAKIND_ECS_COMPONENT_UNUSED____0,
	METAKIND_ECS_COMPONENT_SCRIPT,
	METAKIND_ECS_COMPONENT_LYRICS,
	METAKIND_ECS_COMPONENT_SONG,
	METAKIND_ECS_COMPONENT_PERSPECTIVE,
	METAKIND_ECS_COMPONENT_ARTIST,
	METAKIND_ECS_COMPONENT_NOTEPAD,
	
	METAKIND_ECS_COMPONENT_RELEASE = 3200,
	METAKIND_ECS_COMPONENT_RELEASE_BRIEFING,
	METAKIND_ECS_COMPONENT_RELEASE_COVER_IMAGE,
	METAKIND_ECS_COMPONENT_PLATFORM_MANAGER,
	METAKIND_ECS_COMPONENT_SCRIPT_REASONING,
	METAKIND_ECS_COMPONENT_LEAD_TEMPLATE,
	METAKIND_ECS_COMPONENT_LEAD_PUBLISHER,
	METAKIND_ECS_COMPONENT_BIOGRAPHY,
	METAKIND_ECS_COMPONENT_BIOGRAPHY_PERSPECTIVES,
	METAKIND_ECS_COMPONENT_SCRIPT_VOICEOVER,
	
	METAKIND_ECS_COMPONENT_BIOGRAPHY_ANALYSIS = 3300,
	METAKIND_ECS_COMPONENT_IMG_LAYER,
	METAKIND_ECS_COMPONENT_IMG_GEN_LAYER,
	METAKIND_ECS_COMPONENT_IMG_ASPECT_FIXER_LAYER,
	METAKIND_ECS_COMPONENT_VIDEO_PROMPT_MAKER,
	METAKIND_ECS_COMPONENT_VIDEO_STORYBOARD,
	METAKIND_ECS_COMPONENT_LEAD_DATA,
	METAKIND_ECS_COMPONENT_PROJECT_WIZARD,
	METAKIND_ECS_COMPONENT_PROVIDER,
	
	METAKIND_ECS_COMPONENT_DECISION_MAKER = 3400,
	METAKIND_ECS_COMPONENT_ASSERTIONS,
	METAKIND_ECS_COMPONENT_AGGRESSIONS,
	METAKIND_ECS_COMPONENT_STRENGTH,
	METAKIND_ECS_COMPONENT_LEADERSHIP,
	METAKIND_ECS_COMPONENT_JUSTICE,
	METAKIND_ECS_COMPONENT_SENTIMENTALITY,
	METAKIND_ECS_COMPONENT_ADDICTION,
	METAKIND_ECS_COMPONENT_CARETAKER,
	
	METAKIND_ECS_COMPONENT_SUPPORT_SYSTEM = 3500,
	METAKIND_ECS_COMPONENT_BEAUTY_STANDARD,
	METAKIND_ECS_COMPONENT_ATTRACTIVENESS,
	METAKIND_ECS_COMPONENT_WILLINGNESS,
	METAKIND_ECS_COMPONENT_FACTORY,
	METAKIND_ECS_COMPONENT_PRODUCT,
	METAKIND_ECS_COMPONENT_CONSUMER,
	METAKIND_ECS_COMPONENT_COMPOSITION,
	METAKIND_ECS_COMPONENT_SONG_IDEA,
	METAKIND_ECS_COMPONENT_LITIGATION,
	METAKIND_ECS_COMPONENT_LAWYER,
	METAKIND_ECS_COMPONENT_JUDGE,
	METAKIND_ECS_COMPONENT_LOBBYING,
	// TODO fill this from macros
	
	METAKIND_ECS_COMPONENT_END,
	
	
	METAKIND_ECS_VIRTUAL_BEGIN = 4000,
	METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER,
	METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_PART,
	METAKIND_ECS_VIRTUAL_IO_TRANSCRIPTION_VOICEOVER_GENERATE,
	
	METAKIND_EXTENSION_END = 5000,
	
	
	
};

inline bool IsEcsComponentKind(int kind) {
	return kind > METAKIND_ECS_COMPONENT_BEGIN && kind < METAKIND_ECS_COMPONENT_END;
}

struct MetaNode;
class ToolAppCtrl;

struct MetaNodeExt : Pte<MetaNodeExt> {
	MetaNode& node;
	
	MetaNodeExt(MetaNode& n) : node(n) {}
	virtual ~MetaNodeExt() {}
	virtual void Visit(NodeVisitor& s) = 0;
	virtual String GetName() const {return String();}
	hash_t GetHashValue() const;
	
	void CopyFrom(const MetaNodeExt& e);
	bool operator==(const MetaNodeExt& e) const;
	void Serialize(Stream& s);
	void Jsonize(JsonIO& json);
};

struct MetaExtCtrl : WidgetCtrl {
	Ptr<MetaNodeExt> ext;
	Event<> WhenEditorChange;
	Event<> WhenTitle;
	Ptr<ToolAppCtrl> owner;
	
	MetaNode& GetNode();
	const MetaNode& GetNode() const;
	MetaNodeExt& GetExt();
	String GetFilePath() const;
	VfsPath GetCursorPath() const override;
	
	template <class T> T& GetExt() {return dynamic_cast<T&>(*ext);}
};

struct MetaExtFactory {
	typedef MetaNodeExt* (*NewFn)(MetaNode&);
	typedef MetaExtCtrl* (*NewCtrl)();
	typedef bool (*IsFn)(const MetaNodeExt& e);
	
	struct Factory {
		int kind;
		int category;
		String name;
		String ctrl_name;
		NewFn new_fn = 0;
		NewCtrl new_ctrl_fn = 0;
		IsFn is_fn = 0;
	};
	
	
	template<class T> struct Functions {
		static MetaNodeExt* Create(MetaNode& owner) {MetaNodeExt* c = new T(owner); return c;}
		static bool IsNodeExt(const MetaNodeExt& e) {return dynamic_cast<const T*>(&e);}
	};
	template<class T> struct CtrlFunctions {
		static MetaExtCtrl* CreateCtrl() {return new T;}
	};
	static Array<Factory>& List() {static Array<Factory> f; return f;}
	
	template <class T> inline static void Register(String name) {
		static_assert(!std::is_base_of<::UPP::Ctrl, T>());
		Factory& f = List().Add();
		f.kind = T::GetKind();
		f.category = FindKindCategory(f.kind);
		f.name = name;
		f.new_fn = &Functions<T>::Create;
		f.is_fn = &Functions<T>::IsNodeExt;
	}
	
	template <class Comp, class Ctrl> static void RegisterCtrl(String ctrl_name) {
		static_assert(std::is_base_of<::UPP::Ctrl, Ctrl>());
		static_assert(!std::is_base_of<::UPP::Ctrl, Comp>());
		for (Factory& f : List()) {
			if (f.is_fn == &Functions<Comp>::IsNodeExt) {
				ASSERT_(!f.new_ctrl_fn, "Only one Ctrl per Extension is supported currently, and one is already registered");
				f.new_ctrl_fn = &CtrlFunctions<Ctrl>::CreateCtrl;
				f.ctrl_name = ctrl_name;
				return;
			}
		}
		Panic("No component found");
	}
	static MetaNodeExt* CreateKind(int kind, MetaNode& owner);
	static MetaNodeExt* CloneKind(int kind, const MetaNodeExt& e, MetaNode& owner);
	static MetaNodeExt* Clone(const MetaNodeExt& e, MetaNode& owner);
	static int FindKindFactory(int kind);
	static int FindKindCategory(int kind);
};

#define INITIALIZER_COMPONENT(x) INITIALIZER(x) {MetaExtFactory::Register<x>(#x);}
#define INITIALIZER_COMPONENT_CTRL(comp,ctrl) INITIALIZER(ctrl) {MetaExtFactory::RegisterCtrl<comp,ctrl>(#ctrl);}

struct MetaNode : Pte<MetaNode> {
	Array<MetaNode> sub;
	int kind = -1;
	String id, type;
	hash_t type_hash = 0;
	Point begin = Null;
	Point end = Null;
	hash_t filepos_hash = 0;
	int file = -1;
	bool is_ref = false;
	bool is_definition = false;
	hash_t serial = 0;
	bool is_disabled = false;
	One<MetaNodeExt> ext;
	
	// Temp
	int pkg = -1;
	bool only_temporary = false;
	Ptr<MetaNode> owner;
	Ptr<MetaNode> type_ptr;
	#if DEBUG_METANODE_DTOR
	bool trace_kill = false;
	#endif
	
	MetaNode() {}
	MetaNode(MetaNode* owner, const MetaNode& n) {Assign(owner, n);}
	~MetaNode();
	void Destroy();
	void Assign(MetaNode* owner, const MetaNode& n) {this->owner = owner; CopySubFrom(n); CopyFieldsFrom(n);}
	void Assign(MetaNode* owner, const ClangNode& n);
	void CopyFrom(const MetaNode& n);
	void CopyFieldsFrom(const MetaNode& n, bool forced_downgrade=false);
	void CopySubFrom(const MetaNode& n);
	void FindDifferences(const MetaNode& n, Vector<String>& diffs, int max_diffs=30) const;
	String GetKindString() const;
	static String GetKindString(int i);
	MetaNode& GetAdd(String id, String type, int kind);
	MetaNode& Add(const MetaNode& n);
	MetaNode& Add(MetaNode* n);
	MetaNode& Add();
	MetaNode& Add(int kind, String id=String());
	MetaNode* Detach(MetaNode* n);
	MetaNode* Detach(int i);
	void Remove(MetaNode* n);
	void Remove(int i);
	String GetTreeString(int depth=0) const;
	int Find(int kind, const String& id) const;
	int Find(const String& id) const;
	hash_t GetTotalHash() const;
	hash_t GetSourceHash(bool* total_hash_diffs=0) const;
	void Visit(NodeVisitor& vis);
	void FixParent() {for (auto& s : sub) s.owner = this;}
	void PointPkgTo(MetaNodeSubset& other, int pkg_id);
	void PointPkgTo(MetaNodeSubset& other, int pkg_id, int file_id);
	void CopyPkgTo(MetaNode& other, int pkg_id) const;
	void CopyPkgTo(MetaNode& other, int pkg_id, int file_id) const;
	bool HasPkgDeep(int pkg_id) const;
	bool HasPkgFileDeep(int pkg_id, int file_id) const;
	void SetPkgDeep(int pkg_id);
	void SetFileDeep(int file_id);
	void SetPkgFileDeep(int pkg_id, int file_id);
	void SetTempDeep();
	Vector<MetaNode*> FindAllShallow(int kind);
	Vector<const MetaNode*> FindAllShallow(int kind) const;
	void FindAllDeep(int kind, Vector<MetaNode*>& out);
	void FindAllDeep(int kind, Vector<const MetaNode*>& out) const;
	bool IsFieldsSame(const MetaNode& n) const;
	bool IsSourceKind() const;
	bool IsStructKind() const;
	int GetRegularCount() const;
	//bool IsClassTemplateDefinition() const;
	String GetBasesString() const;
	String GetNestString() const;
	bool OwnerRecursive(const MetaNode& n) const;
	bool ContainsDeep(const MetaNode& n) const;
	void RemoveAllShallow(int kind);
	void RemoveAllDeep(int kind);
	void GetTypeHashes(Index<hash_t>& type_hashes) const;
	void RealizeSerial();
	void FixSerialDeep();
	Vector<Ptr<MetaNodeExt>> GetAllExtensions();
	VfsPath GetPath() const;
	void DeepChk();
	void Chk();
	
	template <class T>
	T& Add() {
		MetaNode& s = Add();
		T* o = new T(s);
		s.ext = o;
		s.owner = this;
		s.pkg = pkg;
		s.file = file;
		s.kind = T::GetKind();
		return *o;
	}
	
	template <class T>
	Vector<Ptr<T>> FindAll() {
		Vector<Ptr<T>> v;
		for (auto& s : sub) {
			if (s.ext) {
				T* o = dynamic_cast<T*>(&*s.ext);
				if (o) v.Add(o);
			}
		}
		return v;
	}
	
	template <class T>
	T* Find(int kind=-1) {
		bool chk_kind = kind >= 0;
		for (auto& s : sub) {
			if (chk_kind && s.kind != kind) continue;
			if (s.ext) {
				T* o = dynamic_cast<T*>(&*s.ext);
				ASSERT(!chk_kind || o);
				if (o) return o;
			}
		}
		return 0;
	}
	
	template <class T> T& GetExt() {
		return dynamic_cast<T&>(*ext);
	}
};

struct MetaNodeSubset {
	Array<MetaNodeSubset> sub;
	Ptr<MetaNode> n;
	
	MetaNodeSubset() {}
	//MetaNodeSubset(MetaNode& n) : n(&n) {}
	void Clear() {sub.Clear(); n = 0;}
};

struct MetaSrcPkg;

struct MetaSrcFile : Moveable<MetaSrcFile> {
	int id = -1;
	String saved_hash;
	hash_t highest_seen_serial = 0;
	VectorMap<hash_t,String> seen_types;
	One<MetaNode> temp;
	
	MetaSrcPkg* pkg = 0;
	String full_path, rel_path;
	mutable Mutex lock;
	bool managed_file = false; // if file only exists as MetaNode json
	bool keep_file_ids = false;
	bool env_file = false;
	
	MetaSrcFile() {}
	void Visit(NodeVisitor& vis);
	bool IsDirTree() const {return GetFileExt(full_path) == ".d";}
	bool IsBinary() const {return GetFileExt(full_path) == ".bin";}
	bool IsECS() const {return GetFileExt(full_path) == ".ecs";}
	bool IsEnv() const {return GetFileExt(full_path) == ".env";}
	bool IsExt(String s) const {return GetFileExt(full_path) == s;}
	bool Store(bool forced=false);
	String StoreJson();
	bool Load();
	bool LoadJson(String json);
	void RefreshSeenTypes();
	MetaNode& GetTemp();
	MetaNode& CreateTemp();
	void ClearTemp();
	void OnSeenTypes();
	void OnSerialCounter();
	String GetTitle() const {return GetFileTitle(full_path);}
	void KeepFileIndices(bool b=true) {keep_file_ids = b;}
	void ManageFile(bool b=true) {managed_file = b;}
	void MakeTempFromEnv(bool all_files=false);
	String UpdateStoring();
	void UpdateLoading();
	
	#if 0
	MetaSrcFile(const MetaSrcFile& f) {*this = f;}
	MetaSrcFile(MetaSrcFile&& f) /*: ai_items(pick(f.ai_items))*/ {}
	void operator=(const MetaSrcFile& s);
	void UpdateLinks(FileAnnotation& ann);
	#endif
};

struct MetaSrcPkg {
	typedef MetaSrcPkg CLASSNAME;
	
	Array<MetaSrcFile> files;
	
	String dir;
	int id = -1;
	
	MetaSrcPkg() {}
	MetaSrcPkg(MetaSrcPkg&& f) {*this = f;}
	MetaSrcPkg(const MetaSrcPkg& f) {*this = f;}
	void operator=(const MetaSrcPkg& f);
	void Init();
	bool Load();
	bool Store(bool forced);
	MetaSrcFile& GetAddFile(const String& full_path);
	MetaSrcFile& GetMetaFile();
	//void SetPath(String full_path, String upp_dir);
	String GetTitle() const;
	String GetRelativePath(const String& path) const;
	String GetFullPath(const String& rel_path) const;
	String GetFullPath(int file_i) const;
	void Visit(NodeVisitor& vis);
	int FindFile(String path) const;
	String GetDirectory() const {return dir;}
private:
	bool post_saving = false;
	Mutex lock;
	mutable String title;
};

class ClangTypeResolver;
struct MetaNode;
struct Entity;

typedef enum : byte {
	MERGEMODE_OVERWRITE_OLD,
	MERGEMODE_KEEP_OLD,
	MERGEMODE_UPDATE_SUBSET,
} MergeMode;

struct MetaEnvironment : VFS {
	struct FilePos : Moveable<FilePos> {
		Vector<Ptr<MetaNode>> hash_nodes;
	};
	struct Type : Moveable<Type> {
		Vector<Ptr<MetaNode>> hash_nodes;
		String seen_type;
	};
	VectorMap<hash_t,FilePos> filepos;
	VectorMap<hash_t,Type> types;
	hash_t serial_counter = 0;
	SpinLock serial_lock;
	
	Array<MetaSrcPkg> pkgs;
	MetaNode root;
	RWMutex lock;
	
	MetaEnvironment();
	hash_t NewSerial();
	hash_t CurrentSerial() const {return serial_counter;}
	int FindPkg(String dir) const;
	MetaSrcPkg& GetAddPkg(String path);
	String ResolveMetaSrcPkgPath(const String& includes, String path, String& ret_upp_dir);
	MetaSrcFile& ResolveFile(const String& includes, const String& path);
	//MetaSrcFile& ResolveFileInfo(const String& includes, String path);
	MetaSrcFile& Load(const String& includes, const String& path);
	bool LoadFileRoot(const String& includes, const String& path, bool manage_file);
	bool LoadFileRootVisit(const String& includes, const String& path, NodeVisitor& vis, bool manage_file, MetaNode** file_node=0);
	//void Store(const String& includes, const String& path, FileAnnotation& fa);
	void Store(String& includes, const String& path, ClangNode& n);
	void SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id);
	void SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id, int file_id);
	void SplitNode(const MetaNode& root, MetaNode& other, int pkg_id);
	void SplitNode(const MetaNode& root, MetaNode& other, int pkg_id, int file_id);
	String GetFilepath(int pkg_id, int file_id) const;
	static bool IsMergeable(CXCursorKind kind);
	static bool IsMergeable(int kind);
	bool MergeNode(MetaNode& root, const MetaNode& other, MergeMode mode);
	bool MergeVisit(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode);
	bool MergeVisitPartMatching(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode);
	void RefreshNodePtrs(MetaNode& n);
	void MergeVisitPost(MetaNode& n);
	MetaNode* FindDeclaration(const MetaNode& n);
	MetaNode* FindTypeDeclaration(hash_t type_hash);
	Vector<MetaNode*> FindDeclarationsDeep(const MetaNode& n);
	bool MergeResolver(ClangTypeResolver& ctr);
	hash_t RealizeTypePath(const String& path);
	MetaNode& RealizeFileNode(int pkg, int file, int kind);
	void OnLoadFile(MetaSrcFile& file);
	MetaNode* FindNodeEnv(Entity& n);
	void UpdateWorkspace(Workspace& wspc);
	Vector<MetaNode*> FindAllEnvs();
	MetaNode* LoadDatabaseSourceVisit(MetaSrcFile& file, String path, NodeVisitor& vis);
	bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) override;
	VfsItemType CheckItem(const VfsPath& rel_path) override;
};

MetaEnvironment& MetaEnv();





template <class T>
bool VisitFromJson(T& var, const char *json)
{
	try {
		Value jv = ParseJSON(json);
		if(jv.IsError())
			return false;
		JsonIO io(jv);
		NodeVisitor vis(io);
		var.Visit(vis);
	}
	catch(ValueTypeError) {
		return false;
	}
	catch(JsonizeError) {
		return false;
	}
	return true;
}

template <class T>
bool VisitFromJsonFile(T& var, const char *file = NULL)
{
	return VisitFromJson(var, LoadFile(sJsonFile(file)));
}

template <class T>
String VisitToJson(T& var)
{
	try {
		JsonIO io;
		NodeVisitor vis(io);
		var.Visit(vis);
		Value val = io.GetResult();
		return AsJSON(val);
	}
	catch (...) {
		return String();
	}
}

template <class T>
bool DoVisitToJson(T& var, String& res)
{
	try {
		JsonIO io;
		NodeVisitor vis(io);
		var.Visit(vis);
		if (vis.IsError())
			return false;
		Value val = io.GetResult();
		res = AsJSON(val);
	}
	catch (...) {
		return false;
	}
	return true;
}

template <class T>
bool VisitToJsonFile(T& var, const char *file = NULL)
{
	try {
		String json = VisitToJson(var);
		FileOut s(file);
		s << json;
		s.Close();
	}
	catch (...) {
		return false;
	}
	return true;
}

END_UPP_NAMESPACE

#endif
