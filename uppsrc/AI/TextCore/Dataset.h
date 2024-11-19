#ifndef _AI_TextCore_Dataset_h_
#define _AI_TextCore_Dataset_h_

NAMESPACE_UPP

struct DatasetPtrs {
	Ptr<SrcTextData>		src;
	Ptr<Entity>				entity;
	Ptr<Component>			component;
	
	// Specialized components
	Ptr<Script>				script; // TODO rename to lyrics_draft
	Ptr<Lyrics>				lyrics;
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p) {
		src = p.src;
		entity = p.entity;
		component = p.component;
		script = p.script;
		lyrics = p.lyrics;
	}
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
};

class TextDatabase {

public:
	// TODO check if this is the best place (compare to SrcTextData or something in DatasetPtrs)
	Array<Entity> entities;
	
	// TODO check if these can be removed & moved to user files
	
	static TextDatabase& Single() {static TextDatabase db; return db;}
};

END_UPP_NAMESPACE

#endif
