#ifndef _AI_TextCore_Dataset_h_
#define _AI_TextCore_Dataset_h_

NAMESPACE_UPP


#if 0
class TextDatabase {

public:
	// TODO check if this is the best place (compare to SrcTextData or something in DatasetPtrs)
	Array<Entity> entities;
	
	// TODO check if these can be removed & moved to user files
	
	static TextDatabase& Single() {static TextDatabase db; return db;}
};
#endif

END_UPP_NAMESPACE

#endif
