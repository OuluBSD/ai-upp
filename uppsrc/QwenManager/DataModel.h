#ifndef _QwenManager_DataModel_h_
#define _QwenManager_DataModel_h_


struct QwenServerConnectionConf : Pte<QwenServerConnectionConf> {
	String name; // user given name for connection
	String directory; // directory of the qwen server (usually repository's root)
	String host = "localhost"; // host for TCP connection
	int port = 8765; // port for TCP connection
	String connection_type = "tcp"; // connection type: tcp, stdin, pipe
	bool auto_start = false; // Whether to auto-start this connection

	// Connection state
	mutable bool is_connected = false;
	mutable bool is_healthy = false;

	String GetAddress() const; // Returns connection address
	String GetStatusString() const; // Returns connection status
	String GetLabel() const;


	void Jsonize(JsonIO& jio);
};



struct QwenProject : Pte<QwenProject> {
	int64 uniq = -1; // unique id
	String name; // non-unique name
	String preferred_connection_name; // equal to QwenServerConnectionConf::name
	String git_origin_addr; // e.g. git@github.com:OuluBSD/ai-upp.git

	// Link to server configuration
	Ptr<QwenServerConnectionConf> srv;

	// Track sessions
	Vector<String> session_ids; // List of session IDs for this project

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
