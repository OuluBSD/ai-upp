#include <AI/AI.h>
#include <ide/ide.h>


NAMESPACE_UPP


void AionFile::Clear() {
	map.Clear();
}

void AionFile::Load(String path) {
	this->path = path;
	Clear();
	if (FileExists(path))
		LoadFromJsonFile(*this, path);
}

void AionFile::Save() {
	if (!path.IsEmpty())
		StoreAsJsonFile(*this, path);
}

void AionFile::Jsonize(JsonIO& json) {
	json	("map", map)
			;
}

void AionStatementRange::Jsonize(JsonIO& json) {
	
}

void AionNode::Jsonize(JsonIO& json) {
	json	("ranges", ranges)
			;
}

void AionSource::Update() {
	String content = LoadFile(path);
	
	ArrayMap<String, FileAnnotation>& x = CodeIndex();
	int i = x.Find(path);
	if (i >= 0) {
		FileAnnotation& f = x[i];
		
		String txt = LoadFile(path);
		Vector<String> lines = Split(txt, "\n", false);
		
		for(int i = 0; i < f.items.GetCount(); i++) {
			const auto& item = f.items[i];
			LOG(i << ": " << item.pos << ": " << item.id << ", type:" << item.type << ", nest: " << item.nest);
			
			String item_txt;
			for (int l = item.begin.y; l <= item.end.y; l++) {
				if (l < 0 || l >= lines.GetCount())
					continue;
				const String& line = lines[l];
				int begin = 0, end = line.GetCount();
				if (l == item.begin.y)
					begin = item.begin.x;
				else if (l == item.end.y)
					end = min(end, item.end.x);
				
				if (!item_txt.IsEmpty())
					item_txt.Cat('\n');
				item_txt << line.Mid(begin, end-begin);
			}
			LOG(item_txt);
		}
	}
	
	// Add the task inside CurrentFileThread() loop
	// TODO
	
	// Wait until the task is done (all add Proxy(WhenUpdated) cb to CurrentFileThread)
	
	WhenUpdated();
}

END_UPP_NAMESPACE

