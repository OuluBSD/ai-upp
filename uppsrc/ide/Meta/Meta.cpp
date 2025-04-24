#include "Meta.h"

NAMESPACE_UPP

bool MakeRelativePath(const String& includes_, const String& dir, String& best_ai_dir,
                      String& best_rel_dir)
{
	bool found = false;
	Vector<String> ai_dirs = GetAiDirsRaw();
	if(!ai_dirs.IsEmpty()) {
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
				if(dir.Find(s) == 0) {
					String rel_dir = dir.Mid(s.GetCount());
					int cand_parts = Split(rel_dir, DIR_SEPS).GetCount();
					// Prefer the shortest directory
					if(cand_parts < def_cand_parts) {
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

void Assign(MetaNode& mn, MetaNode* owner, const ClangNode& n)
{
	mn.owner = owner;
	int c = n.sub.GetCount();
	mn.sub.SetCount(c);
	for(int i = 0; i < c; i++)
		UPP::Assign(mn.sub[i], &mn, n.sub[i]);
	mn.kind = n.kind;
	mn.id = n.id;
	mn.type = n.type;
	mn.type_hash = n.type_hash;
	mn.begin = n.begin;
	mn.end = n.end;
	mn.filepos_hash = n.filepos_hash;
	mn.is_ref = n.is_ref;
	mn.is_definition = n.is_definition;
}


void Store(MetaEnvironment& env, String& includes, const String& path, ClangNode& cn)
{
	ClangTypeResolver ctr;
	if(!ctr.Process(cn)) {
		LOG("MetaEnvironment::Store: error: clang type resolving failed: " + ctr.GetError());
		return;
	}
	if(!env.MergeResolver(ctr)) {
		LOG("MetaEnvironment::Store: error: merging resolver failed");
		return;
	}

	cn.TranslateTypeHash(ctr.GetTypeTranslation());

	// LOG(n.GetTreeString());
	MetaSrcFile& file = env.ResolveFile(includes, path);
	MetaSrcPkg& pkg = *file.pkg;
	MetaNode n;
	UPP::Assign(n, 0, cn);
	n.SetPkgDeep(pkg.id);
	n.SetFileDeep(file.id);
	n.RealizeSerial();
	if(!env.MergeNode(env.root, n, MERGEMODE_OVERWRITE_OLD))
		return;

	pkg.Store(false);
}

void UpdateWorkspace(MetaEnvironment& env, Workspace& wspc) {
	for(int i = 0; i < wspc.package.GetCount(); i++) {
		String pkg_name = wspc.package.GetKey(i);
		auto& pkg = wspc.package[i];
		String dir = GetFileDirectory(pkg.path);
		MetaSrcPkg& mpkg = env.GetAddPkg(dir);
		for (auto& file : pkg.file) {
			if (file.separator) continue;
			String filename = file;
			String ext = GetFileExt(filename);
			String path = AppendFileName(dir, filename);
			if (EcsIndexer::AcceptExt(ext))
				EcsIndexer::RunPath(path);
		}
	}
}

END_UPP_NAMESPACE
