#include "Vfs.h"

NAMESPACE_UPP
#include <AICore2/Types.h>

VfsValue* (*FindNodeEnvPtr)(Entity& n);
VfsValue* (*IdeVfsFillDatasetPtrsPtr)(DatasetPtrs&, hash_t type_hash);

void DatasetPtrs::operator=(const DatasetPtrs& p) {
	#define DATASET_ITEM(type, name, desc) name = p.name;
	DATASET_LIST
	#undef DATASET_ITEM
	
	editable_biography = p.editable_biography;
}

void DatasetPtrs::Clear() {
	#define DATASET_ITEM(type, name, desc) name = 0;
	DATASET_LIST
	#undef DATASET_ITEM
	editable_biography = 0;
}

#define DATASET_ITEM(type, name, desc) \
template <> void DatasetPtrs::Set<type>(type& o) {name = &o;}
BASE_EXT_LIST
COMPONENT_LIST
VIRTUALNODE_DATASET_LIST
#undef DATASET_ITEM

END_UPP_NAMESPACE
