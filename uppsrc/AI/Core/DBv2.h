#ifndef _AI_Core_DBv2_h_
#define _AI_Core_DBv2_h_

NAMESPACE_UPP


struct TextDbFraction {
	VectorMap<String, Token>	tokens;
	VectorMap<hash_t, TokenText> token_texts;
	Index<String> element_keys;
	Index<String> word_classes;
	VectorMap<String, ExportWord> words;
	VectorMap<hash_t, WordPairType> ambiguous_word_pairs;
	VectorMap<hash_t, VirtualPhrase> virtual_phrases;
	VectorMap<hash_t, VirtualPhrasePart> virtual_phrase_parts;
	VectorMap<hash_t, VirtualPhraseStruct> virtual_phrase_structs;
	VectorMap<hash_t, PhrasePart> phrase_parts;
	Index<String> struct_part_types;
	Index<String> struct_types;
	VectorMap<AttrHeader, ExportAttr> attrs;
	VectorMap<ActionHeader, ExportAction> actions;
	VectorMap<int, VectorMap<int, ExportParallel>> parallel; // TODO make these work again?
	VectorMap<int, VectorMap<int, ExportTransition>> trans;
	VectorMap<String, ExportDepActionPhrase> action_phrases;
	VectorMap<hash_t, ExportWordnet> wordnets;
	VectorMap<String, String> diagnostics;
	VectorMap<String, ExportSimpleAttr> simple_attrs;
	Vector<EntityDataset> entities; // TODO rename, remove src_ (source-data for analysis)
	Index<String> typeclasses;
	Vector<ContentType> contents;
	Vector<String> content_parts;
	dword lang = LNG_enUS;
	VectorMap<String,Vector<String>> typeclass_entities[TCENT_COUNT];
};


struct TextDbEntryFraction {
	String						src_text;
	ScriptStruct				tokenized_text;
	TextDbFraction				db;
};


END_UPP_NAMESPACE

#endif
