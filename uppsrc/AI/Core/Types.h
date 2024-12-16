#ifndef _AI_TextCore_Types_h_
#define _AI_TextCore_Types_h_

NAMESPACE_UPP

#define DATASET_ITEM(type, name, kind, group, desc) struct type;
DATASET_LIST
#undef DATASET_ITEM

typedef String NoPointerExc;


END_UPP_NAMESPACE

#endif
