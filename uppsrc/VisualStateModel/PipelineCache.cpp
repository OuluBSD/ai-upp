#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmCacheKey

String VsmCacheKey::Compute() const
{
	String s = asset_id + "|" + pipeline_id + "|" + rule_id + "|"
	         + rule_type + "|" + engine_version + "|" + IntStr(schema_version);
	return MD5String(s);
}

// ---------------------------------------------------------------------------
// VsmPipelineCache

bool VsmPipelineCache::Open(const String& cache_dir)
{
	RealizeDirectory(cache_dir);
	path_    = AppendFileName(cache_dir, "vsm_cache.json");
	is_open_ = true;

	if(!FileExists(path_)) {
		LogInfo(log_, "Cache", "New cache at: " + path_);
		return true;
	}
	// Load existing
	String raw = LoadFile(path_);
	if(raw.IsEmpty()) return true;

	Value arr = ParseJSON(raw);
	if(arr.IsError()) {
		LogWarn(log_, "Cache", "Cannot parse cache, starting fresh: " + path_);
		return true;
	}
	for(int i = 0; i < arr.GetCount(); i++) {
		VsmCacheEntry e;
		LoadFromJsonValue(e, arr[i]);
		entries_.Add(e.key_hash, e);
	}
	LogInfo(log_, "Cache", Format("Loaded %d entries from: " + path_, entries_.GetCount()));
	return true;
}

bool VsmPipelineCache::Save() const
{
	if(!is_open_) return false;
	Vector<VsmCacheEntry> vec;
	for(const VsmCacheEntry& e : entries_)
		vec.Add(e);
	String json = StoreAsJson(vec, true);
	if(!SaveFile(path_, json)) {
		LogError(log_, "Cache", "Cannot save cache: " + path_);
		return false;
	}
	return true;
}

bool VsmPipelineCache::Clear()
{
	entries_.Clear();
	hits_ = misses_ = 0;
	if(FileExists(path_))
		FileDelete(path_);
	LogInfo(log_, "Cache", "Cache cleared");
	return true;
}

bool VsmPipelineCache::Get(const VsmCacheKey& key, String& data_json_out) const
{
	String hash = key.Compute();
	int idx = entries_.Find(hash);
	if(idx < 0) { misses_++; return false; }
	data_json_out = entries_[idx].data_json;
	hits_++;
	return true;
}

void VsmPipelineCache::Put(const VsmCacheKey& key, const String& data_json)
{
	String hash = key.Compute();
	VsmCacheEntry e;
	e.key_hash   = hash;
	e.data_json  = data_json;
	e.created_at = MakeTimestamp();

	int idx = entries_.Find(hash);
	if(idx >= 0) entries_[idx] = e;
	else         entries_.Add(hash, e);
}

bool VsmPipelineCache::HasHash(const String& hash) const
{
	return entries_.Find(hash) >= 0;
}

String VsmPipelineCache::MakeTimestamp()
{
	Time t = GetUtcTime();
	return Format("%04d-%02d-%02dT%02d:%02d:%02d.000Z",
	              t.year, t.month, t.day, t.hour, t.minute, t.second);
}

} // namespace Upp
