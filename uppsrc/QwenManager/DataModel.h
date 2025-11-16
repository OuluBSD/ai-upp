#ifndef _QwenManager_DataModel_h_
#define _QwenManager_DataModel_h_


struct QwenServerConnectionConf : Pte<QwenServerConnectionConf> {
	String name; // user given name for connection
	
	/* TODO: add classes or functions to link to Qwen package here
	- directory of the qwen serever (usually repository's root)
	*/
	
	String GetAddress() const; // TODO: implement
	String GetStatusString() const; // TODO: implement
	String GetLabel() const;
	
	
	void Jsonize(JsonIO& jio);
};



struct QwenProject : Pte<QwenProject> {
	int64 uniq = -1; // unique id
	String name; // non-unique name
	String preferred_connection_name; // equal to QwenServerConnectionConf::name
	String git_origin_addr; // e.g. git@github.com:OuluBSD/ai-upp.git
	
	// Temporary
	Ptr<QwenServerConnectionConf> srv;
	
	
	void Jsonize(JsonIO& jio);
};



struct QwenManagerState : Pte<QwenManagerState> {
	Array<QwenProject> projects;
	Array<QwenServerConnectionConf> servers;
	
	void Jsonize(JsonIO& jio);
	bool Load(String path="");
	void Store(String path="");
	
	static QwenManagerState& Global();
};



#endif
