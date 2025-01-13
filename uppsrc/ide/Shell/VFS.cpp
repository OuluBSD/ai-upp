#include "Shell.h"

NAMESPACE_UPP

VfsPath::VfsPath() {
	
}

VfsPath::VfsPath(const String& s) {
	Set(s);
}

void VfsPath::Set(String path) {
	this->str = path;
	parts = Split(path , "/");
}

void VfsPath::Set(const VfsPath& path, int begin, int end) {
	str.Clear();
	parts.Clear();
	for(int i = begin; i < path.parts.GetCount() && i < end; i++) {
		parts << path.parts[i];
	}
	str = Join(parts, "/");
}

bool VfsPath::IsLeft(const VfsPath& path) const {
	if (parts.GetCount() > path.parts.GetCount())
		return false;
	int c = parts.GetCount();
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





bool SystemFS::GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) {
	String sys_path = rel_path.AsSysPath();
	if (!DirectoryExists(sys_path)) {
		last_error = "Directory doesn't exist '" + sys_path + "'";
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

END_UPP_NAMESPACE
