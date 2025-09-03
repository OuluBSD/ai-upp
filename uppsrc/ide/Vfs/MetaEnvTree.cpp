#include "Vfs.h"


MetaEnvTree::MetaEnvTree() {
	AddFrame(menu);
	Add(split.SizePos());
	
	split.Horz() << lsplit << code;
	ltsplit.Horz() << pkgs << files;
	lsplit.Vert() << ltsplit << stmts << focus;
	lsplit.SetPos(2000,0);
	lsplit.SetPos(6000,1);
	
	code.Highlight("cpp");
	
	pkgs.AddColumn("#");
	pkgs.AddColumn("Package");
	pkgs.AddColumn("Directory");
	pkgs.AddIndex("IDX");
	pkgs.ColumnWidths("1 3 5");
	files.AddColumn("File");
	files.AddIndex("IDX");
	
	pkgs.WhenCursor << THISBACK(DataPkg);
	files.WhenCursor << THISBACK(DataFile);
	stmts.WhenCursor << THISBACK(DataTreeSelection);
	focus.WhenCursor << THISBACK(DataFocusSelection);
	
	menu.Set([this](Bar& b) {
		b.Add("Update", THISBACK(Data)).Key(K_Q);
	});
	PostCallback(THISBACK(Data));
}

void MetaEnvTree::Data() {
	IdeMetaEnvironment& env = IdeMetaEnv();
	
	int row = 0;
	pkgs.Set(row, 0, "<global>");
	pkgs.Set(row, "IDX", -1);
	row++;
	for(int i = 0; i < env.pkgs.GetCount(); i++) {
		VfsSrcPkg& pkg = env.pkgs[i];
		AttrText dir(pkg.dir);
		dir.NormalInk(GrayColor());
		pkgs.Set(row, 0, i);
		pkgs.Set(row, 1, pkg.GetTitle());
		pkgs.Set(row, 2, dir);
		pkgs.Set(row, "IDX", i);
		row++;
	}
	pkgs.SetCount(row);
	
	if (!pkgs.IsCursor() && pkgs.GetCount())
		pkgs.SetCursor(0);
	else
		DataPkg();
}

void MetaEnvTree::DataPkg() {
	IdeMetaEnvironment& env = IdeMetaEnv();
	
	if (!pkgs.IsCursor()) {
		files.Clear();
		stmts.Clear();
		focus.Clear();
		code.Clear();
		return;
	}
	int row = 0;
	files.Set(row,0,"<global>");
	files.Set(row,"IDX",-1);
	row++;
	int pkg_i = pkgs.Get("IDX");
	if (pkg_i >= 0) {
		VfsSrcPkg& pkg = env.pkgs[pkg_i];
		for(int i = 0; i < pkg.rel_files.GetCount(); i++) {
			files.Set(row,0,pkg.rel_files[i]);
			files.Set(row,"IDX",i);
			row++;
		}
	}
	files.SetCount(row);
	
	if (!files.IsCursor() && files.GetCount())
		files.SetCursor(0);
	else
		DataFile();
}

void MetaEnvTree::DataFile() {
	IdeMetaEnvironment& env = IdeMetaEnv();
	
	if (!pkgs.IsCursor() || !files.IsCursor()) {
		stmts.Clear();
		focus.Clear();
		code.Clear();
		return;
	}
	int pkg_i = pkgs.Get("IDX");
	int file_i = files.Get("IDX");
	
	int prev = stmts.IsCursor() ? stmts.GetCursor() : -1;
	Point scroll = stmts.GetScroll();
	
	int count = 0;
	stmts.Clear();
	stmt_ptrs.SetCount(0);
	if (pkg_i < 0) {
		AddStmtNodes(0, env.env.root, 0, count);
	}
	else if (pkg_i >= 0 && file_i < 0) {
		subset.Clear();
		stmt_ptrs.Clear();
		env.SplitValue(env.env.root, subset, pkg_i);
		AddStmtNodes(0, *subset.n, &subset, count);
	}
	else {
		subset.Clear();
		stmt_ptrs.Clear();
		env.SplitValue(env.env.root, subset, pkg_i, file_i);
		AddStmtNodes(0, *subset.n, &subset, count);
	}
	stmts.OpenDeep(0);
	
	if (prev >= 0 && prev < stmt_ptrs.GetCount()) {
		stmts.SetCursor(prev);
		stmts.ScrollTo(scroll);
	}
	else if (!stmts.IsCursor())
		stmts.SetCursor(0);
	else
		DataTreeSelection();
}

void MetaEnvTree::DataTreeSelection() {
	focus.Clear();
	if (!stmts.IsCursor())
		return;
	focus_ptrs.SetCount(0);
	int sel = stmts.GetCursor();
	int count = 0;
	if (sel >= stmt_ptrs.GetCount()) return;
	VfsValue& n = *stmt_ptrs[sel];
	AddFocusNodes(0, n, 0, count);
	focus.OpenDeep(0);
	focus.SetCursor(0);
}

void MetaEnvTree::DataFocusSelection() {
	IdeMetaEnvironment& env = IdeMetaEnv();
	code.Clear();
	if (!focus.IsCursor())
		return;
	int sel = focus.GetCursor();
	const VfsValue& n = *focus_ptrs[sel];
	const AstValue* a = n;
	if (n.pkg < 0 || n.file < 0 || !a)
		return;
	VfsSrcPkg& pkg = env.pkgs[n.pkg];
	String full_path = pkg.GetFullPath(n.file);
	String content = LoadFile(full_path);
	String node_content = GetStringRange(content, a->begin, a->end);
	code.SetData(node_content);
}

bool MetaEnvTree::Key(dword key, int count) {
	if (key == K_Q) {
		Data();
		return true;
	}
	return false;
}

void MetaEnvTree::AddStmtNodes(int tree_idx, VfsValue& n, VfsValueSubset* ns, int& count) {
	if (count >= tree_limit) return;
	
	if (tree_idx <= stmt_ptrs.GetCount())
		stmt_ptrs.SetCount(tree_idx+1,0);
	stmt_ptrs[tree_idx] = &n;
	
	String s;
	bool skip_default = false;
	const AstValue* a = n;
	if (a) {
		String kind_str = VfsValue::AstGetKindString(a->kind);
		s = kind_str + ": " + n.id;
		if (a->type.GetCount()) s += " (" + a->type + ")";
		#if 1
		s += " " + IntStr(n.pkg) + ":" + IntStr(n.file);
		#endif
		switch (a->kind) {
		case CXCursor_CXXMethod:
		case CXCursor_Constructor:
		case CXCursor_Destructor:
		case CXCursor_CompoundStmt:
		case CXCursor_FunctionDecl:
		case CXCursor_FieldDecl:
		case CXCursor_CXXBaseSpecifier:
			skip_default = true;
			break;
		default:
			break;
		}
	}
	else {
		s = n.id;
		if (n.ext)
			s += " (" + n.ext->GetTypeCls().GetName() + ")";
	}
	
	stmts.Set(tree_idx, s);
	
	if (!skip_default) {
		if (ns) {
			for (VfsValueSubset& s : ns->sub) {
				int idx = stmts.Add(tree_idx);
				count++;
				AddStmtNodes(idx, *s.n, &s, count);
				if (count >= tree_limit) return;
			}
		}
		else {
			for (VfsValue& s : n.sub) {
				int idx = stmts.Add(tree_idx);
				count++;
				AddStmtNodes(idx, s, 0, count);
				if (count >= tree_limit) return;
			}
		}
	}
}

void MetaEnvTree::AddFocusNodes(int tree_idx, VfsValue& n, VfsValueSubset* ns, int& count) {
	if (count >= tree_limit) return;
	
	if (tree_idx <= focus_ptrs.GetCount())
		focus_ptrs.SetCount(tree_idx+1,0);
	focus_ptrs[tree_idx] = &n;
	
	String s;
	const AstValue* a = n;
	if (a) {
		String kind_str = n.AstGetKindString();
		s = kind_str + ": " + n.id;
		if (a->type.GetCount()) s += " (" + a->type + ")";
	}
	else {
		s = n.id;
		if (n.ext)
			s += " (" + n.ext->GetTypeCls().GetName() + ")";
	}
	focus.Set(tree_idx, s);
	
	if (ns) {
		for (VfsValueSubset& s : ns->sub) {
			int idx = focus.Add(tree_idx);
			count++;
			AddFocusNodes(idx, *s.n, &s, count);
			if (count >= tree_limit) break;
		}
	}
	else {
		for (VfsValue& s : n.sub) {
			int idx = focus.Add(tree_idx);
			count++;
			AddFocusNodes(idx, s, 0, count);
			if (count >= tree_limit) break;
		}
	}
}

