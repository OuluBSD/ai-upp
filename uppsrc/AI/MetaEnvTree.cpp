#include "AI.h"

NAMESPACE_UPP

MetaEnvTree::MetaEnvTree() {
	AddFrame(menu);
	Add(split.SizePos());
	
	split.Horz() << lsplit << code;
	ltsplit.Horz() << pkgs << files;
	lsplit.Vert() << ltsplit << stmts << focus;
	lsplit.SetPos(2000,0);
	lsplit.SetPos(6000,1);
	
	code.Highlight("cpp");
	
	pkgs.AddColumn("Package");
	pkgs.AddIndex("IDX");
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
	MetaEnvironment& env = MetaEnv();
	
	int row = 0;
	pkgs.Set(row, 0, "<global>");
	pkgs.Set(row, "IDX", -1);
	for(int i = 0; i < env.pkgs.GetCount(); i++) {
		MetaSrcPkg& pkg = env.pkgs[i];
		pkgs.Set(row, 0, pkg.GetTitle());
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
	MetaEnvironment& env = MetaEnv();
	
	if (!pkgs.IsCursor()) {
		files.Clear();
		stmts.Clear();
		focus.Clear();
		code.Clear();
		return;
	}
	int pkg_i = pkgs.Get("IDX");
	if (pkg_i < 0) {
		files.SetCount(1);
		files.Set(0,0,"<global>");
		files.Set(0,"IDX",-1);
	}
	else {
		MetaSrcPkg& pkg = env.pkgs[pkg_i];
		for(int i = 0; i < pkg.files.GetCount(); i++) {
			files.Set(i,0,pkg.files[i].GetTitle());
			files.Set(i,"IDX",i);
		}
		files.SetCount(pkg.files.GetCount());
	}
	
	if (!files.IsCursor() && files.GetCount())
		files.SetCursor(0);
	else
		DataFile();
}

void MetaEnvTree::DataFile() {
	MetaEnvironment& env = MetaEnv();
	
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
	
	stmts.Clear();
	stmt_ptrs.SetCount(0);
	if (pkg_i < 0) {
		AddStmtNodes(0, env.root, 0);
	}
	else if (pkg_i >= 0 && file_i < 0) {
		subset.Clear();
		stmt_ptrs.Clear();
		env.SplitNode(env.root, subset, pkg_i);
		AddStmtNodes(0, *subset.n, &subset);
	}
	else {
		subset.Clear();
		stmt_ptrs.Clear();
		env.SplitNode(env.root, subset, pkg_i, file_i);
		AddStmtNodes(0, *subset.n, &subset);
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
	MetaNode& n = *stmt_ptrs[sel];
	AddFocusNodes(0, n, 0);
	focus.OpenDeep(0);
	focus.SetCursor(0);
}

void MetaEnvTree::DataFocusSelection() {
	MetaEnvironment& env = MetaEnv();
	code.Clear();
	if (!focus.IsCursor())
		return;
	int sel = focus.GetCursor();
	MetaNode& n = *focus_ptrs[sel];
	if (n.pkg < 0 ||n.file < 0)
		return;
	MetaSrcPkg& pkg = env.pkgs[n.pkg];
	String full_path = pkg.GetFullPath(n.file);
	String content = LoadFile(full_path);
	String node_content = GetStringRange(content, n.begin, n.end);
	code.SetData(node_content);
}

bool MetaEnvTree::Key(dword key, int count) {
	if (key == K_Q) {
		Data();
		return true;
	}
	return false;
}

void MetaEnvTree::AddStmtNodes(int tree_idx, MetaNode& n, MetaNodeSubset* ns) {
	if (tree_idx <= stmt_ptrs.GetCount())
		stmt_ptrs.SetCount(tree_idx+1,0);
	stmt_ptrs[tree_idx] = &n;
	
	String kind_str = n.GetKindString();
	String s = kind_str + ": " + n.id;
	if (n.type.GetCount()) s += " (" + n.type + ")";
	stmts.Set(tree_idx, s);
	
	switch (n.kind) {
	case CXCursor_CXXMethod:
	case CXCursor_Constructor:
	case CXCursor_Destructor:
	case CXCursor_CompoundStmt:
	case CXCursor_FunctionDecl:
	case CXCursor_FieldDecl:
	case CXCursor_CXXBaseSpecifier:
		break;
	default:
		if (ns) {
			for (MetaNodeSubset& s : ns->sub) {
				int idx = stmts.Add(tree_idx);
				AddStmtNodes(idx, *s.n, &s);
			}
		}
		else {
			for (MetaNode& s : n.sub) {
				int idx = stmts.Add(tree_idx);
				AddStmtNodes(idx, s, 0);
			}
		}
	}
}

void MetaEnvTree::AddFocusNodes(int tree_idx, MetaNode& n, MetaNodeSubset* ns) {
	if (tree_idx <= focus_ptrs.GetCount())
		focus_ptrs.SetCount(tree_idx+1,0);
	focus_ptrs[tree_idx] = &n;
	
	String kind_str = n.GetKindString();
	String s = kind_str + ": " + n.id;
	if (n.type.GetCount()) s += " (" + n.type + ")";
	focus.Set(tree_idx, s);
	if (ns) {
		for (MetaNodeSubset& s : ns->sub) {
			int idx = focus.Add(tree_idx);
			AddFocusNodes(idx, *s.n, &s);
		}
	}
	else {
		for (MetaNode& s : n.sub) {
			int idx = focus.Add(tree_idx);
			AddFocusNodes(idx, s, 0);
		}
	}
}

END_UPP_NAMESPACE
