#include "ScriptIDE.h"

NAMESPACE_UPP

FormExplorer::FormExplorer()
{
	Title("Form Explorer");
	Icon(Icons::Outline());

	Add(toolbar.TopPos(0, 36).HSizePos());
	Add(tree.VSizePos(36, 0).HSizePos());

	toolbar.Set([=](Bar& bar) { LayoutToolbar(bar); });

	tree.AddColumn("Type", 18);
	tree.AddColumn("Rect", 28);
	tree.AddColumn("Details", 54);
	tree.NoRoot(false);
	tree.MultiSelect(false);
	tree.EvenRowColor();
	tree.SetLineCy(22);
}

void FormExplorer::LayoutToolbar(Bar& bar)
{
	bar.Add(Icons::RefreshVariables(), [=] {}).Enable(false).Tip("Renderer updates this pane live");
	bar.Add(Icons::RemoveAll(), [=] { Clear(); }).Tip("Clear form explorer");
}

String FormExplorer::FormatRect(const Rect& r) const
{
	return AsString(r.left) + "," + AsString(r.top) + " " +
	       AsString(r.GetWidth()) + "x" + AsString(r.GetHeight());
}

void FormExplorer::SetScene(const Size& canvas_size, const Vector<FormExplorerEntry>& entries)
{
	tree.Clear();
	tree.SetRoot(Icons::Folder(), "Form");
	tree.SetRowValue(0, 0, "Canvas");
	tree.SetRowValue(0, 1, Format("%d x %d", canvas_size.cx, canvas_size.cy));
	tree.SetRowValue(0, 2, "");

	VectorMap<String, int> nodes;
	nodes.Add("", 0);

	auto ensure_node = [&](const String& full_path, const String& type, const Rect& rect, const String& details) -> int {
		int q = nodes.Find(full_path);
		if(q >= 0)
			return nodes[q];

		Vector<String> parts = Split(full_path, '/');
		String prefix;
		int parent_id = 0;
		for(int i = 0; i < parts.GetCount(); i++) {
			if(!prefix.IsEmpty())
				prefix << "/";
			prefix << parts[i];

			int existing = nodes.Find(prefix);
			if(existing >= 0) {
				parent_id = nodes[existing];
				continue;
			}

			bool leaf = i + 1 == parts.GetCount();
			int id = tree.Add(parent_id, Null, parts[i], true);
			tree.SetRowValue(id, 0, leaf ? type : "Group");
			tree.SetRowValue(id, 1, leaf ? FormatRect(rect) : "");
			tree.SetRowValue(id, 2, leaf ? details : "");
			tree.Open(parent_id);
			nodes.Add(prefix, id);
			parent_id = id;
		}
		return parent_id;
	};

	for(const FormExplorerEntry& e : entries)
		ensure_node(e.path, e.type, e.rect, e.details);

	tree.OpenDeep(0);
}

void FormExplorer::Clear()
{
	tree.Clear();
}

END_UPP_NAMESPACE
