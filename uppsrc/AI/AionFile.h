#ifndef _AI_AionFile_h_
#define _AI_AionFile_h_


NAMESPACE_UPP


struct AionStatementRange
{
	
	void Jsonize(JsonIO& json);
};

struct AionNode
{
	Array<AionStatementRange> ranges;
	
	void Jsonize(JsonIO& json);
	
};

struct AionFile
{
	String path;
	ArrayMap<String, AionNode> map;
	
	void Load(String path);
	void Save();
	void Clear();
	void Jsonize(JsonIO& json);
	
};

END_UPP_NAMESPACE


#endif
