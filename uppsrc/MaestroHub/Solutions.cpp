#include "MaestroHub.h"

NAMESPACE_UPP

void SolutionsManager::Load(const String& maestro_root) {
	root = maestro_root;
	String path = AppendFileName(root, "docs/maestro/solutions.json");
	if(FileExists(path)) {
		String json = LoadFile(path);
		Value v = ParseJSON(json);
		if(v.Is<ValueArray>()) {
			patterns.Clear();
			for(const Value& item : v) {
				SolutionPattern& p = patterns.Add();
				p.name = item["name"];
				p.regex = item["regex"];
				p.description = item["description"];
				p.prompt_template = item["prompt_template"];
			}
		}
	}
}

void SolutionsManager::Save() {
	if(root.IsEmpty()) return;
	String dir = AppendFileName(root, "docs/maestro");
	RealizeDirectory(dir);
	String path = AppendFileName(dir, "solutions.json");
	
	JsonArray ja;
	for(const auto& p : patterns) {
		ja << Json("name", p.name)("regex", p.regex)("description", p.description)("prompt_template", p.prompt_template);
	}
	SaveFile(path, ja.ToString());
}

int SolutionsManager::Match(const String& message) {
	for(int i=0; i<patterns.GetCount(); i++) {
		RegExp r(patterns[i].regex);
		if(r.Match(message)) return i;
	}
	return -1;
}

SolutionsHub::SolutionsHub() {
	Title("Solutions Pattern Registry");
	Sizeable().Zoomable();
	
	list.AddColumn("Name");
	list.AddColumn("Regex");
	list.WhenCursor = THISBACK(OnSelect);
	
	editor.Add(name.TopPos(5, 20).HSizePos(5, 5));
	editor.Add(regex.TopPos(30, 20).HSizePos(5, 5));
	editor.Add(description.TopPos(55, 20).HSizePos(5, 5));
	editor.Add(prompt_template.VSizePos(80, 40).HSizePos(5, 5));
	
	editor.Add(add.SetLabel("Add").BottomPos(5, 30).LeftPos(5, 100));
	editor.Add(remove.SetLabel("Remove").BottomPos(5, 30).LeftPos(110, 100));
	editor.Add(save.SetLabel("Save").BottomPos(5, 30).RightPos(5, 100));
	
	add << THISBACK(OnAdd);
	remove << THISBACK(OnRemove);
	save << THISBACK(OnSave);
	
	split.Horz(list, editor);
	split.SetPos(3000);
	Add(split.SizePos());
}

void SolutionsHub::Load(const String& maestro_root) {
	root = maestro_root;
	sm.Load(root);
	list.Clear();
	for(const auto& p : sm.patterns) {
		list.Add(p.name, p.regex);
	}
	if(list.GetCount() > 0) list.SetCursor(0);
}

void SolutionsHub::OnSelect() {
	if(!list.IsCursor()) return;
	int i = list.GetCursor();
	const auto& p = sm.patterns[i];
	name.SetData(p.name);
	regex.SetData(p.regex);
	description.SetData(p.description);
	prompt_template.SetData(p.prompt_template);
}

void SolutionsHub::OnAdd() {
	SolutionPattern& p = sm.patterns.Add();
	p.name = "New Pattern";
	p.regex = ".*";
	list.Add(p.name, p.regex);
	list.SetCursor(list.GetCount()-1);
}

void SolutionsHub::OnRemove() {
	if(!list.IsCursor()) return;
	int i = list.GetCursor();
	sm.patterns.Remove(i);
	list.Remove(i);
}

void SolutionsHub::OnSave() {
	if(!list.IsCursor()) return;
	int i = list.GetCursor();
	SolutionPattern& p = sm.patterns[i];
	p.name = name.GetData().ToString();
	p.regex = regex.GetData().ToString();
	p.description = description.GetData().ToString();
	p.prompt_template = prompt_template.GetData().ToString();
	
	list.Set(i, 0, p.name);
	list.Set(i, 1, p.regex);
	
	sm.Save();
}

void SolutionsHub::OnTest() {}

END_UPP_NAMESPACE
