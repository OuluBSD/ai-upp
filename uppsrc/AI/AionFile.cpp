#include <AI/AI.h>


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
	
END_UPP_NAMESPACE

