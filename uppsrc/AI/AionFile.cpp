#include <AI/AI.h>
#include <ide/ide.h>

NAMESPACE_UPP

void AionFile::PostSave()
{
	if(!post_saving) {
		post_saving = true;
		PostCallback(THISBACK1(Save, false));
	}
}

void AionFile::Clear() { files.Clear(); }

void AionFile::SetPath(String path)
{
	this->path = path;
	dir = GetFileDirectory(path);
}

void AionFile::Load()
{
	Clear();
	if(FileExists(path)) {
		LoadFromJsonFile(*this, path);
	}
}

void AionFile::Save(bool forced)
{
	post_saving = false;
	if(!path.IsEmpty()) {
		// The hash value must be both 32-bit and 64-bit compatible,
		// so use the sha1 hasher instead of the fast hasher
		String current_sha1 = GetHashSha1();
		if (current_sha1 != saved_hash || forced) {
			saved_hash = current_sha1;
			RealizeDirectory(GetFileDirectory(path));
			StoreAsJsonFile(*this, path, true);
		}
	}
}

String AionFile::GetHashSha1() {
	Sha1Stream s;
	s % *this;
	return s.FinishString();
}

void AionFile::Serialize(Stream& s)
{
	byte version = 1;
	s % version;
	
	if (version >= 1)
		s % saved_hash % files;
}

void AionFile::Jsonize(JsonIO& json)
{
	json
		("saved_hash", saved_hash)
		("files", files)
		;
}

bool MakeRelativePath(const String& includes_, const String& dir, String& best_ai_dir, String& best_rel_dir)
{
	bool found = false;
	Vector<String> ai_dirs = GetAiDirsRaw();
	if (!ai_dirs.IsEmpty()) {
		int def_cand_parts = INT_MAX;
		String ai_dir = ai_dirs.Top();
		String includes = includes_;
		MergeWith(includes, ";", GetClangInternalIncludes());
		for(const String& s : Split(includes, ';')) {
		#ifdef PLATFORM_WIN32 // we need to ignore internal VC++ headers
			static VectorMap<String, bool> use;
			int q = use.Find(s);
			if(q < 0) {
				q = use.GetCount();
				use.Add(s, !FileExists(AppendFileName(s, "vcruntime.h")));
			}
			if(use[q])
		#endif
			{
				if (dir.Find(s) == 0) {
					String rel_dir = dir.Mid(s.GetCount());
					int cand_parts = Split(rel_dir, DIR_SEPS).GetCount();
					// Prefer the shortest directory
					if (cand_parts < def_cand_parts) {
						best_ai_dir = ai_dir;
						best_rel_dir = rel_dir;
						found = true;
					}
				}
			}
		}
	}
	return found;
}

AiFileInfo& AionFile::RealizePath(const String& includes, const String& path)
{
	String rel_file = NormalizePath(path, dir);
	int a = rel_file.Find(dir);
	if(a == 0)
		rel_file = rel_file.Mid(dir.GetCount());
	else {
		String def_dir = GetFileDirectory(path);
		Vector<String> upp_dirs = FindParentUppDirectories(def_dir);
		if (!upp_dirs.IsEmpty()) {
			String& upp_dir = upp_dirs[0];
			ASSERT(path.Find(upp_dir) == 0);
			rel_file = path.Mid(upp_dir.GetCount());
		}
		else {
			String ai_dir, rel_dir;
			if (MakeRelativePath(includes, def_dir, ai_dir, rel_dir)) {
				String filename = GetFileName(path);
				rel_file = AppendFileName(rel_dir, filename);
			}
		}
	}
	#ifdef flagWIN32
	rel_file.Replace(DIR_SEPS, "/");
	#endif
	if (rel_file.GetCount() && rel_file[0] == '/')
		rel_file = rel_file.Mid(1);
	
	lock.Enter();
	int i = files.Find(rel_file);
	if(i >= 0) {
		AiFileInfo& o = files[i];
		lock.Leave();
		return o;
	}
	AiFileInfo& o = files.Add(rel_file);
	lock.Leave();
	return o;
}

AiFileInfo& AionFile::RealizePath0(const String& includes, const String& path)
{
	String rel_file = NormalizePath(path, dir);
	int a = rel_file.Find(dir);
	if(a == 0)
		rel_file = rel_file.Mid(dir.GetCount());
	else {
		String ai_dir, rel_dir;
		if (MakeRelativePath(includes, GetFileDirectory(path), ai_dir, rel_dir)) {
			String filename = GetFileName(path);
			rel_file = AppendFileName(rel_dir, filename);
		}
	}
	int i = files.Find(rel_file);
	if(i >= 0)
		return files[i];
	return files.Add(rel_file);
}

String GetAiPathCandidate(const String& includes_, String dir)
{
	Vector<String> ai_dirs = GetAiDirsRaw();
	Vector<String> upp_dirs = GetUppDirs();
	String dummy_cand, def_cand, any_ai_cand, preferred_ai_cand;
	int def_cand_parts = INT_MAX;
	dummy_cand = dir + DIR_SEPS + "AI.json";
	if (!ai_dirs.IsEmpty()) {
		for (const String& upp_dir : upp_dirs) {
			if (dir.Find(upp_dir) != 0) continue;
			String rel_path = dir.Mid(upp_dir.GetCount());
			for (const String& ai_dir : ai_dirs) {
				String ai_dir_cand = AppendFileName(ai_dir, rel_path);
				String path = AppendFileName(ai_dir_cand, "AI.json");
				if (any_ai_cand.IsEmpty())
					any_ai_cand = path;
				if (preferred_ai_cand.IsEmpty() && FileExists(path))
					preferred_ai_cand = path;
			}
		}
	}
	if (!preferred_ai_cand.IsEmpty())
		return preferred_ai_cand;
	else if (!any_ai_cand.IsEmpty())
		return any_ai_cand;
	
	if (!ai_dirs.IsEmpty()) {
		String ai_dir, rel_dir;
		if (MakeRelativePath(includes_, dir, ai_dir, rel_dir)) {
			String abs_dir = AppendFileName(ai_dir, rel_dir);
			def_cand = AppendFileName(abs_dir, "AI.json");
		}
	}
	
	if (!def_cand.IsEmpty())
		return def_cand;
	else
		return dummy_cand;
}

Vector<String> FindParentUppDirectories(const String& sub_dir) {
	Vector<String> results;
	Vector<String> parts = Split(sub_dir, DIR_SEPS);
	for(int i = 0; i < parts.GetCount(); i++) {
		int c = parts.GetCount() - i;
		if(!c)
			continue;
		String parent_dir;
		for(int j = 0; j < c; j++) {
			// a posix path always begins with the root /
			#ifndef flagPOSIX
			if(!parent_dir.IsEmpty())
			#endif
				parent_dir << DIR_SEPS;
			parent_dir << parts[j];
		}
		String topname = parts[c - 1];
		String upp_path = parent_dir + DIR_SEPS + topname + ".upp";
		if(!FileExists(upp_path))
			continue;
		results << parent_dir;
	}
	return results;
}

String AionIndex::ResolveAionFilePath(const String& includes, String path)
{
	String def_dir = GetFileDirectory(path);
	Vector<String> upp_dirs = FindParentUppDirectories(def_dir);
	for (String& upp_dir : upp_dirs)
		return GetAiPathCandidate(includes, upp_dir);
	return GetAiPathCandidate(includes, def_dir);
}

AionFile& AionIndex::ResolveFile(const String& includes, String path)
{
	String aion_path = ResolveAionFilePath(includes, path);
	lock.EnterRead();
	int i = files.Find(aion_path);
	if(i >= 0) {
		AionFile& f = files[i];
		lock.LeaveRead();
		return f;
	}
	lock.LeaveRead();
	lock.EnterWrite();
	AionFile& f = files.Add(aion_path);
	f.SetPath(aion_path);
	f.Load();
	lock.LeaveWrite();
	return f;
}

AiFileInfo& AionIndex::ResolveFileInfo(const String& includes, String path)
{
	return ResolveFile(includes, path).RealizePath(includes, path);
}

AionIndex& AiIndex() { return Single<AionIndex>(); }

void AionIndex::Load(const String& includes, const String& path, FileAnnotation& fa)
{
	AionFile& af = ResolveFile(includes, path);
	af.Load(includes, path, fa);
}

void AionFile::Load(const String& includes, const String& path, FileAnnotation& fa)
{
	lock.Enter();
	if(IsEmpty())
		Load();
	AiFileInfo& afi = RealizePath0(includes, path);
	afi.UpdateLinks(fa);
	lock.Leave();
}

void AionIndex::Store(const String& includes, const String& path, FileAnnotation& fa)
{
	AionFile& af = ResolveFile(includes, path);
	af.Store(includes, path, fa);
}

void AionFile::Store(const String& includes, const String& path, FileAnnotation& fa)
{
	AiFileInfo& afi = RealizePath(includes, path);
	lock.Enter();
	afi.UpdateLinks(fa);
	Save();
	lock.Leave();
}

END_UPP_NAMESPACE
