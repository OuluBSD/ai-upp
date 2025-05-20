#ifndef _AICore_Omni_h_
#define _AICore_Omni_h_




struct AiThread : Pte<AiThread> {
	virtual ~AiThread() {}
	virtual void Visit(Vis& v) = 0;
};

class CompletionThread : public virtual AiThread {
	
public:
	struct Session {
		struct Item : Moveable<Item> {
			String txt;
			
			void Visit(Vis& v) {
				v.Ver(1)
				(1)	("txt", txt);
			}
		};
		Vector<Item> items;
		
		void Visit(Vis& v) {
			v.Ver(1)
			(1)	("items", items, VISIT_VECTOR);
		}
	};
	
	Array<Session> sessions;
	
public:
	typedef CompletionThread CLASSNAME;
	
	void Visit(Vis& v) override {
		v.Ver(1)
		(1)	("sessions", sessions, VISIT_VECTOR);
	}
};

/*
ChatAgent
	- inter-agent-communication
ChatDirectorAgent
*/
class ChatThread : public virtual AiThread {
	
public:
	struct Attachment : Moveable<Attachment> {
		String mime;
		String filepath; // if any
		String data; // if any
		String source_info; // if any
		void Visit(Vis& v) {
			v.Ver(1)
			(1)	("mime", mime)
				("filepath", filepath)
				("data", data)
				("source_info", source_info);
		}
	};
	struct Session {
		struct Item : Moveable<Item> {
			AiMsgType type = MSG_NULL;
			String content;
			String username;
			Time created;
			Vector<Attachment> attachments;
			
			void Visit(Vis& v) {
				v.Ver(1)
				(1)	("type", (int&)type)
					("content", content)
					("username", username)
					("created", created)
					("attachments", attachments, VISIT_VECTOR)
					;
			}
		};
		String name;
		Time created;
		Time changed;
		Vector<Item> items;
		
		void Visit(Vis& v) {
			v.Ver(1)
			(1)	("name", name)
				("created", created)
				("changed", changed)
				("items", items, VISIT_VECTOR)
				;
		}
	};
	
	Array<Session> sessions;
	
public:
	typedef ChatThread CLASSNAME;
	
	void Visit(Vis& v) override {
		v.Ver(1)
		(1)	("sessions",sessions, VISIT_VECTOR)
			;
	}
};

struct StageThread : MetaNodeExt {
	
	/*
	struct Stage {
		
		void Visit(Vis& v) override {
			v.Ver(1)
			(1)	;
		}
	};
	struct Session {
		String id;
		Value version;
		Array<Stage> stages;
		
		void Visit(Vis& v) override {
			v.Ver(1)
			(1)	("id", id)
				("version", version)
				("stages", stages, VISIT_VECTOR)
				;
		}
	};
	
	Array<Session> sessions;
	*/
	Vector<String> stage_name_presets;
	
public:
	CLASSTYPE(StageThread)
	StageThread(MetaNode& n) : MetaNodeExt(n) {}
	
	void Visit(Vis& v) override {
		v.Ver(1)
		(1)	//("sessions", sessions, VISIT_VECTOR)
			("stage_name_presets", stage_name_presets)
		;
	}
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_AI_STAGE_SESSION;}
	
};

INITIALIZE(StageThread)

struct ChainThread : MetaNodeExt {
	CLASSTYPE(ChainThread)
	ChainThread(MetaNode& n) : MetaNodeExt(n) {}
	void Visit(Vis& v) override {
		v.Ver(0)
			;
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_AI_CHAIN;}
};

INITIALIZE(ChainThread)

class SpeechTranscriptionThread : public virtual AiThread {
	
public:
	typedef SpeechTranscriptionThread CLASSNAME;
	
	void Visit(Vis& v) override {v.Ver(0);}
};

class SpeechGenerationThread : public virtual AiThread {
	
public:
	typedef SpeechGenerationThread CLASSNAME;
	
	void Visit(Vis& v) override {v.Ver(0);}
};

class ImageGenerationThread : public virtual AiThread {
	
public:
	typedef ImageGenerationThread CLASSNAME;
	
	void Visit(Vis& v) override {v.Ver(0);}
};

class ImageVisionThread : public virtual AiThread {
	
public:
	typedef ChatThread CLASSNAME;
	
	void Visit(Vis& v) override {v.Ver(0);}
};

class MetaEnvThread : public virtual AiThread {
	
public:
	typedef MetaEnvThread CLASSNAME;
	
	void Visit(Vis& v) override {v.Ver(0);}
};

class OmniThread :
	public CompletionThread,
	public ChatThread,
	public SpeechTranscriptionThread,
	public SpeechGenerationThread,
	public ImageGenerationThread,
	public ImageVisionThread,
	public MetaEnvThread,
	public SoundDiscussionManager
{
	
public:
	typedef OmniThread CLASSNAME;
	OmniThread();
	
	void Visit(Vis& v) override;
	void OnPhraseEnd(SoundPhrase& p) override;
	void OnMessageEnd(SoundMessage&) override;
	void OnDiscussionEnd(SoundDiscussion&) override;
	
	static OmniThread& Single() {static OmniThread m; return m;}
};




#endif
