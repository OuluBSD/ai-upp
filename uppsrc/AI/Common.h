#ifndef _AI_Common_h_
#define _AI_Common_h_


struct CurrentFileClang;
struct CurrentFileContext;


NAMESPACE_UPP


typedef enum : int {
	AITASK_TRANSLATE,
	AITASK_TRANSLATE_SONG_DATA,
	AITASK_CREATE_IMAGE,
	AITASK_EDIT_IMAGE,
	AITASK_VARIATE_IMAGE,
	AITASK_VISION,
	AITASK_RAW_COMPLETION,
	AITASK_GENERIC_PROMPT,
	
	AITASK_COUNT
} AiTaskType;






struct VisionArgs {
	int fn = 0;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				;
	}
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
	
};

struct GenericPromptArgs {
	int fn = 0;
	VectorMap<String, Vector<String>> lists;
	String response_title;
	bool is_numbered_lines = false;
	
	void Jsonize(JsonIO& json) {
		json	("fn", fn)
				("lists", lists)
				("t", response_title)
				("nl", is_numbered_lines)
				;
	}
	
	String Get() const {return StoreAsJson(*this);}
	void Put(const String& s) {LoadFromJson(*this, s);}
};




struct AiAnnotationItem : Moveable<AiAnnotationItem>, AnnotationItem {
	struct Comment : Moveable<Comment> {
		int rel_line = -1;
		hash_t line_hash = 0;
		String txt;
		//void Serialize(Stream& s);
		void Jsonize(JsonIO& json);
	};
	Array<Comment> comments;
	
	void RemoveCommentLine(int rel_line);
	Comment* FindComment(int rel_line);
	//void Serialize(Stream& s);
	void Jsonize(JsonIO& json);
};

struct AiFileInfo : Moveable<AiFileInfo> {
	Array<AiAnnotationItem> ai_items;
	
	
	//void Serialize(Stream& s);
	void Jsonize(JsonIO& json);
	void UpdateLinks(FileAnnotation& ann);
};


END_UPP_NAMESPACE

#endif
