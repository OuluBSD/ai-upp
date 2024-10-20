#include <AI/AI.h>
#include <ide/ide.h>


NAMESPACE_UPP


void AionFile::PostSave() {
	if (!post_saving) {
		post_saving = true;
		PostCallback(THISBACK(Save));
	}
}

void AionFile::Clear() {
	source_files.Clear();
}

void AionFile::Load(String path) {
	this->path = path;
	Clear();
	if (FileExists(path)) {
		lock.EnterWrite();
		LoadFromJsonFile(*this, path);
		lock.LeaveWrite();
	}
}

void AionFile::Save() {
	post_saving = false;
	if (!path.IsEmpty()) {
		lock.EnterWrite();
		StoreAsJsonFile(*this, path);
		lock.LeaveWrite();
	}
}

void AionFile::Jsonize(JsonIO& json) {
	json	("source_files", source_files)
			;
}

String& AionFile::RealizePathString(const String& path) {
	lock.EnterRead();
	int i = source_files.Find(path);
	if (i >= 0) {
		String& o = source_files[i];
		lock.LeaveRead();
		return o;
	}
	lock.LeaveRead();
	lock.EnterWrite();
	String& o = source_files.Add(path);
	lock.LeaveWrite();
	return o;
}

void AionFile::Store(String path, FileAnnotation& f) {
	String json = StoreAsString((AiFileInfo&)f);
	RealizePathString(path) = json;
	PostSave();
}

void AionFile::Load(String path, FileAnnotation& f) {
	String json = RealizePathString(path);
	if (!json.IsEmpty())
		LoadFromString((AiFileInfo&)f, json);
}

String AionIndex::ResolveAionFilePath(String path) {
	Vector<String> parts = Split(GetFileDirectory(path), DIR_SEPS);
	for(int i = 0; i < parts.GetCount(); i++) {
		int c = parts.GetCount()-i;
		if (!c) continue;
		String s;
		for(int j = 0; j < c; j++) {
			if (!s.IsEmpty()) s << DIR_SEPS;
			s << parts[j];
		}
		String upp_path = s + DIR_SEPS + parts[c-1] + ".upp";
		if (!FileExists(upp_path))
			continue;
		s << DIR_SEPS << "AI.json";
		return s;
	}
	return AppendFileName(GetFileDirectory(path), "AI.json");
}

AionFile& AionIndex::ResolveFile(String path) {
	String aion_path = ResolveAionFilePath(path);
	lock.EnterRead();
	int i = files.Find(aion_path);
	if (i >= 0) {
		AionFile& f = files[i];
		lock.LeaveRead();
		return f;
	}
	lock.LeaveRead();
	lock.EnterWrite();
	AionFile& f = files.Add(aion_path);
	f.path = aion_path;
	lock.LeaveWrite();
	return f;
}

AionIndex& AiIndex() {
	return Single<AionIndex>();
}

END_UPP_NAMESPACE

