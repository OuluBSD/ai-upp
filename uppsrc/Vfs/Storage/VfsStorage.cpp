#include "VfsStorage.h"

#include <Vfs/Core/VfsValue.h>

NAMESPACE_UPP

namespace {

static constexpr int kVfsFragmentVersion = 1;

struct FragmentDocument : Moveable<FragmentDocument> {
	int      version = kVfsFragmentVersion;
	hash_t   pkg_hash = 0;
	hash_t   file_hash = 0;
	VfsValue root;

	void Jsonize(JsonIO& io) {
		io
			("version", version)
			("pkg_hash", pkg_hash)
			("file_hash", file_hash)
			("root", root);
	}
};

static void PropagateFragmentHashes(VfsValue& node, hash_t pkg_hash, hash_t file_hash) {
	if (pkg_hash)
		node.pkg_hash = pkg_hash;
	if (file_hash)
		node.file_hash = file_hash;
	for (VfsValue& child : node.sub)
		PropagateFragmentHashes(child, pkg_hash, file_hash);
}

static void FillDocumentFromFragment(const VfsValue& fragment, FragmentDocument& doc) {
	doc.pkg_hash = fragment.pkg_hash;
	doc.file_hash = fragment.file_hash;
	doc.root.Assign(nullptr, fragment);
	PropagateFragmentHashes(doc.root, doc.pkg_hash, doc.file_hash);
}

} // namespace

bool VfsSaveFragment(const String& path, const VfsValue& fragment) {
	try {
		if (!RealizePath(GetFileFolder(path)))
			return false;
		FragmentDocument doc;
		FillDocumentFromFragment(fragment, doc);
		String json = StoreAsJson(doc, true);
		return SaveFile(path, json);
	}
	catch (const Exc& e) {
		RLOG("VfsSaveFragment: failed to save '" << path << "': " << e);
	}
	catch (...) {
		RLOG("VfsSaveFragment: unknown error while saving '" << path << "'");
	}
	return false;
}

bool VfsLoadFragment(const String& path, VfsValue& out_fragment) {
	try {
		String data = LoadFile(path);
		FragmentDocument doc;
		if (!LoadFromJson(doc, ~data))
			return false;
		if (doc.version != 0 && doc.version != kVfsFragmentVersion) {
			RLOG("VfsLoadFragment: unsupported fragment version " << doc.version << " for '" << path << "'");
			return false;
		}
		// Allow payload to override header hashes if they are missing.
		if (!doc.pkg_hash)
			doc.pkg_hash = doc.root.pkg_hash;
		if (!doc.file_hash)
			doc.file_hash = doc.root.file_hash;
		out_fragment.Assign(nullptr, doc.root);
		PropagateFragmentHashes(out_fragment, doc.pkg_hash, doc.file_hash);
		return true;
	}
	catch (const Exc& e) {
		RLOG("VfsLoadFragment: failed to load '" << path << "': " << e);
	}
	catch (...) {
		RLOG("VfsLoadFragment: unknown error while loading '" << path << "'");
	}
	return false;
}

bool VfsLoadLegacy(const String& path, VfsValue& out_fragment) {
	// Placeholder: once the legacy binary/JSON reader migrates we can attempt it here.
	(void)path;
	(void)out_fragment;
	return false;
}

END_UPP_NAMESPACE
