#include "Shell.h"

NAMESPACE_UPP

VfsPath::VfsPath() {
	
}

VfsPath::VfsPath(const String& s) {
	Set(s);
}

VfsPath::VfsPath(const VfsPath& path) {
	str = path.str;
	parts <<= path.parts;
}

VfsPath::VfsPath(const Vector<String>& path) {
	parts <<= path;
	StrFromParts();
}

VfsPath::VfsPath(VfsPath&& path) {
	str = pick(path.str);
	parts = pick(path.parts);
}

VfsPath& VfsPath::operator=(const VfsPath& path) {
	str = path.str;
	parts <<= path.parts;
	return *this;
}

void VfsPath::Set(String path) {
	this->str = path;
	
	parts = Split(path , INTERNAL_SEPS);
	
	#if INTERNAL_POSIX
	if (this->str.IsEmpty())
		str = "/";
	#endif
	
	#if INTERNAL_COMB || INTERNAL_CPM
	if (parts.GetCount()) {
		auto& p = parts[0];
		int c = p.GetCount();
		if (c && p[c-1] == ':')
			p = p.Left(c-1);
	}
	#endif
}

void VfsPath::Set(const Vector<String>& parts) {
	this->parts <<= parts;
	StrFromParts();
}

void VfsPath::StrFromParts() {
	String s;
	#if INTERNAL_POSIX
	if (parts.IsEmpty())
		s = "/";
	else
		for (const String& p: parts)
			s << "/" << p;
	#elif INTERNAL_COMB
	int c = parts.GetCount();
	if (c > 0) {
		s << parts[0] << ":";
		for(int i = 1; i < c; i++)
			s << "/" << parts[i];
	}
	#else
	int c = parts.GetCount();
	if (c > 0) {
		s << parts[0] << ":";
		for(int i = 1; i < c; i++)
			s << "\\" << parts[i];
	}
	#endif
	str = s;
}

void VfsPath::Set(const VfsPath& path, int begin, int end) {
	parts.Clear();
	for(int i = begin; i < path.parts.GetCount() && i < end; i++) {
		parts << path.parts[i];
	}
	StrFromParts();
}

bool VfsPath::IsLeft(const VfsPath& path) const {
	if (path.parts.GetCount() > parts.GetCount())
		return false;
	int c = path.parts.GetCount();
	for(int i = 0; i < c; i++)
		if (parts[i] != path.parts[i])
			return false;
	return true;
}

bool VfsPath::IsSame(const VfsPath& path, int this_begin, int other_begin, int len) const {
	ASSERT(this->parts.GetCount() >= this_begin + len);
	if (path.parts.GetCount() < other_begin + len)
		return false;
	for(int i = 0; i < len; i++) {
		int a = this_begin + i;
		int b = other_begin + i;
		if (this->parts[a] != path.parts[b])
			return false;
	}
	return true;
}

String VfsPath::AsSysPath() const {
	String s;
	#ifdef flagWIN32
	if (parts.GetCount())
		s << parts[0] << ":";
	for(int i = 1; i < parts.GetCount(); i++)
		s << "\\" << parts[i];
	#else
	for(int i = 0; i < parts.GetCount(); i++)
		s << "/" << parts[i];
	#endif
	return s;
}

const String& VfsPath::Get() const {return str;}
const Vector<String>& VfsPath::Parts() const {return parts;}
VfsPath::operator String() const {return str;}

int VfsPath::GetPartCount() const {
	return parts.GetCount();
}

bool VfsPath::IsEmpty() const {
	return parts.IsEmpty();
}

String VfsPath::TopPart() const {
	if (parts.IsEmpty())
		return String();
	else
		return parts.Top();
}

bool VfsPath::Normalize() {
	bool any_changes = false;
	while (1) {
		bool changes = false;
		for(int i = 0; i < parts.GetCount(); i++) {
			String& p = parts[i];
			if (p == "..") {
				if (!i) {
					str.Clear();
					parts.Clear();
					return false;
				}
				parts.Remove(i-1,2);
				changes = true;
			}
		}
		if (!changes) break;
		any_changes = true;
	}
	if (any_changes)
		StrFromParts();
	return true;
}

bool VfsPath::IsValidFullPath() const {
	return IsFullInternalDirectory(str);
}

void VfsPath::Append(const VfsPath& p) {
	parts.Append(p.parts);
	StrFromParts();
}

void VfsPath::RemoveLast() {
	parts.SetCount(parts.GetCount()-1);
}

String operator+(const char* s, const VfsPath& vfs) {
	return String(s) + vfs.Get();
}

String operator+(const VfsPath& vfs, const char* s) {
	return vfs.Get() + String(s);
}

bool IsFullInternalDirectory(const String& path) {
	#if defined flagINTERNAL_POSIX
	return path.GetCount() > 0 && path[0] == '/';
	#else
	if (path.IsEmpty())
		return true;
	for(int i = 0; i < path.GetCount(); i++) {
		int chr = path[i];
		if (chr == ':')
			return true;
		else if (chr == '/' || chr == '\\')
			return false;
	}
	return false;
	#endif
}

String AppendInternalFileName(const String& a, const String& b) {
	#if INTERNAL_POSIX || INTERNAL_COMB
	return AppendUnixFileName(a, b);
	#else
	return AppendCpmFileName(a, b);
	#endif
}

String NormalizeInternalPath(const String& path) {
	VfsPath vfs = path;
	vfs.Normalize();
	return vfs;
}







bool SystemFS::GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) {
	String sys_path = rel_path.AsSysPath();
	if (!DirectoryExists(sys_path)) {
		last_error = "no directory '" + sys_path + "'";
		return false;
	}
	FindFile ff;
	if (ff.Search(AppendFileName(sys_path, "*"))) do {
		String name = ff.GetName();
		if (name == "." || name == "..") continue;
		VfsItem& item = items.Add();
		item.name = name;
		if (ff.IsDirectory())
			item.type = VFS_DIRECTORY;
		else if (ff.IsSymLink())
			item.type = VFS_SYMLINK;
		else
			item.type = VFS_FILE;
		item.type_str = GetFileExt(name);
		if (item.type_str.GetCount() && item.type_str[0] == '.')
			item.type_str = item.type_str.Mid(1);
	}
	while (ff.Next());
	return true;
}

VfsItemType SystemFS::CheckItem(const VfsPath& rel_path) {
	String sys_path = rel_path.AsSysPath();
	if (DirectoryExists(sys_path))
		return VFS_DIRECTORY;
	else if (FileExists(sys_path))
		return VFS_FILE;
	else
		return VFS_NULL;
}

END_UPP_NAMESPACE
