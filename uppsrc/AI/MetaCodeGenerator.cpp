#include "AI.h"

NAMESPACE_UPP

MetaCodeGenerator::MetaCodeGenerator() {
	
}

String MetaCodeGenerator::GetResult(int pkg, int file) const {
	PkgFile key(pkg,file);
	int i = files.Find(key);
	if (i >= 0) return files[i].code;
	return "";
}

const MetaCodeGenerator::File* MetaCodeGenerator::GetResultFile(int pkg, int file) const {
	PkgFile key(pkg,file);
	int i = files.Find(key);
	if (i >= 0) return &files[i];
	return 0;
}

bool MetaCodeGenerator::Process(const MetaNodeSubset& np) {
	MetaNode& n = *np.n;
	ASSERT(!n.only_temporary);
	
	MetaEnvironment& env = MetaEnv();
	files.Clear();
	
	// Find all unique files
	Vector<Vector<int>> pkgfiles;
	FindFiles(np, pkgfiles);
	
	// Find most visible file
	VectorMap<PkgFile,int> pkgfile_counts;
	int pkg_i = 0;
	for (auto& pkg : pkgfiles) {
		int file_i = 0;
		for (auto& file_count : pkg) {
			if (file_count > 0)
				pkgfile_counts.Add(PkgFile(pkg_i, file_i), file_count);
			file_i++;
		}
		pkg_i++;
	}
	SortByValue(pkgfile_counts, StdGreater<int>());
	
	// Create file headers for files which were found
	for(int i = 0; i < pkgfile_counts.GetCount(); i++)
		files.Add(pkgfile_counts.GetKey(i));
	
	// Create file contents
	for(int i = 0; i < files.GetCount(); i++) {
		const PkgFile& key = files.GetKey(i);
		File& f = files[i];
		
		// Find all parent nodes of the file
		Vector<MetaNode*> nodes;
		FindNodes(np, key, nodes);
		
		// Sort nodes based on code position
		struct MetaNodePtrSorter {
			bool operator()(const MetaNode* a, const MetaNode* b) const {
				return IsFilePosLess(a->begin, b->begin);
			}
		};
		Sort(nodes, MetaNodePtrSorter());
		
		// Remove overlapping nodes
		for(int j = 0; j < nodes.GetCount(); j++) {
			const MetaNode& n0 = *nodes[j];
			for(int k = j+1; k < nodes.GetCount(); k++) {
				const MetaNode& n1 = *nodes[k];
				if (IsRangesOverlapping(n0.begin, n0.end, n1.begin, n1.end)) {
					if (IsSubset(n0.begin, n0.end, n1.begin, n1.end))
						nodes.Remove(k--);
					else {
						nodes.Remove(j--);
						break;
					}
				}
			}
		}
		
		// Load original file
		const auto& pkg = env.pkgs[key.pkg];
		String path = pkg.GetFullPath(key.file);
		String content = LoadFile(path);
		
		// Write code
		VectorMap<int,String> lines;
		Vector<MetaNode*> comments;
		Vector<MetaNode*> comment_to_node;
		for (MetaNode* n : nodes) {
			// TODO namespaces
			
			TextRange range;
			range.begin = n->begin;
			range.end = n->end;
			//range.begin = Point(0,lines.GetCount());
			Vector<String> area_lines = GetStringArea(content, n->begin, n->end);
			for(int i = 0; i < area_lines.GetCount(); i++)
				lines.Add(n->begin.y + i, area_lines[i]);
			//range.end = Point(0,lines.GetCount());
			
			// Add empty line
			if (lines.GetCount())
				lines.Add(lines.TopKey());
			
			n->FindAllDeep(METAKIND_COMMENT, comments);
			
			f.code_nodes.Add(range, const_cast<MetaNode*>(n));
		}
		
		// Add comments
		for (MetaNode* comment : comments) {
			int line0 = comment->begin.y;
			int best_pos = 0, best_line = 0;
			for(int i = 0; i < lines.GetCount(); i++) {
				int line1 = lines.GetKey(i);
				if (line1 > line0)
					break;
				best_pos = i;
				best_line = line1;
			}
			lines.Insert(best_pos, best_line, "// " + comment->id);
			
			if (best_pos >= comment_to_node.GetCount())
				comment_to_node.SetCount(best_pos+1,0);
			comment_to_node[best_pos] = comment;
		}
		
		// Write
		f.code = Join(lines.GetValues(), "\n");
		for (const MetaNode* n : nodes)
			f.range_nodes.Add(TextRange(n->begin,n->end), const_cast<MetaNode*>(n));
		
		f.editor_to_line <<= lines.GetKeys();
		f.comment_to_node <<= comment_to_node;
	}
	
	return true;
}

void MetaCodeGenerator::FindFiles(const MetaNodeSubset& np, Vector<Vector<int>>& pkgfiles) {
	const MetaNode& n = *np.n;
	if (n.pkg >= 0 && n.file >= 0) {
		if (n.pkg >= pkgfiles.GetCount())
			pkgfiles.SetCount(n.pkg+1);
		Vector<int>& file = pkgfiles[n.pkg];
		if (n.file >= file.GetCount())
			file.SetCount(n.file+1, 0);
		file[n.file]++;
	}
	for (const MetaNodeSubset& s : np.sub)
		FindFiles(s, pkgfiles);
}

void MetaCodeGenerator::FindNodes(const MetaNodeSubset& np, const PkgFile& key, Vector<MetaNode*>& nodes) {
	MetaNode& n = *np.n;
	if (MetaEnvironment::IsMergeable(n.kind)) {
		for (const MetaNodeSubset& s : np.sub) {
			FindNodes(s, key, nodes);
		}
	}
	else {
		if (n.pkg == key.pkg && n.file == key.file)
			nodes.Add(&n);
	}
}

END_UPP_NAMESPACE
