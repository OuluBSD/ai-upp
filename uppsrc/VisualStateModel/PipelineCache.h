#ifndef _VisualStateModel_PipelineCache_h_
#define _VisualStateModel_PipelineCache_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Cache key: uniquely identifies one computation based on its inputs.
// All fields that affect the output should be part of the key.

struct VsmCacheKey : Moveable<VsmCacheKey> {
	String asset_id;       // frame or crop asset relative path
	String pipeline_id;    // preprocessing pipeline id
	String rule_id;        // template or OCR rule id
	String rule_type;      // "template", "ocr", "fingerprint", "preprocess"
	String engine_version; // OCR engine name+version (for OCR results)
	int    schema_version = 1;

	String Compute() const; // returns MD5 hex string of concatenated fields
	void Jsonize(JsonIO& json) {
		json("asset_id",asset_id)("pipeline_id",pipeline_id)
		    ("rule_id",rule_id)("rule_type",rule_type)
		    ("engine_version",engine_version)("schema_version",schema_version);
	}
};

// ---------------------------------------------------------------------------
// Cache entry: stores one computation result.

struct VsmCacheEntry : Moveable<VsmCacheEntry> {
	String key_hash;    // MD5 of VsmCacheKey
	String data_json;   // serialized result
	String created_at;  // ISO timestamp
	void Jsonize(JsonIO& json) {
		json("key_hash",key_hash)("data_json",data_json)("created_at",created_at);
	}
};

// ---------------------------------------------------------------------------
// VsmPipelineCache: file-backed key→entry store.
//
// Storage layout: session/cache/vsm_cache.json
// Only metadata is stored in the JSON index. Large binary blobs (if any)
// would go to external files, but the current implementation serializes
// results as compact JSON strings.

class VsmPipelineCache {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Open cache from directory (creates if missing)
	bool Open(const String& cache_dir);
	// Flush to disk
	bool Save() const;
	// Delete all entries and remove the cache file
	bool Clear();

	bool   IsOpen() const { return is_open_; }

	// Lookup by key — returns false on miss
	bool   Get(const VsmCacheKey& key, String& data_json_out) const;
	// Store a result
	void   Put(const VsmCacheKey& key, const String& data_json);
	// Check by hash
	bool   HasHash(const String& hash) const;

	int    GetCount()     const { return entries_.GetCount(); }
	int    GetHits()      const { return hits_;   }
	int    GetMisses()    const { return misses_;  }
	void   ResetStats()         { hits_ = misses_ = 0; }

	String GetPath()  const { return path_; }

private:
	bool               is_open_ = false;
	String             path_;
	VectorMap<String, VsmCacheEntry> entries_; // keyed by hash
	mutable int        hits_   = 0;
	mutable int        misses_ = 0;
	mutable CoreLog    log_;

	static String MakeTimestamp();
};

} // namespace Upp

#endif
