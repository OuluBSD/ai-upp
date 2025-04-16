#ifndef _AI_Omni_h_
#define _AI_Omni_h_


NAMESPACE_UPP

struct AiThread : Pte<AiThread> {
	virtual ~AiThread() {}
	virtual void Visit(NodeVisitor& vis) = 0;
};

class CompletionThread : public virtual AiThread {
	
public:
	struct Session {
		struct Item : Moveable<Item> {
			String txt;
			
			void Visit(NodeVisitor& vis) {
				vis.Ver(1)
				(1)	("txt", txt);
			}
		};
		Vector<Item> items;
		
		void Visit(NodeVisitor& vis) {
			vis.Ver(1)
			(1)	("items", items, VISIT_VECTOR);
		}
	};
	
	Array<Session> sessions;
	
public:
	typedef CompletionThread CLASSNAME;
	
	void Visit(NodeVisitor& vis) override {
		vis.Ver(1)
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
		void Visit(NodeVisitor& vis) {
			vis.Ver(1)
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
			
			void Visit(NodeVisitor& vis) {
				vis.Ver(1)
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
		
		void Visit(NodeVisitor& vis) {
			vis.Ver(1)
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
	
	void Visit(NodeVisitor& vis) override {
		vis.Ver(1)
		(1)	("sessions",sessions, VISIT_VECTOR)
			;
	}
};

class StageThread : public MetaNodeExt {
	
public:
	struct Stage {
		
		void Visit(NodeVisitor& vis) {
			vis.Ver(1)
			(1)	;
		}
	};
	
	struct Session {
		String id;
		Value version;
		Array<Stage> stages;
		
		void Visit(NodeVisitor& vis) {
			vis.Ver(1)
			(1)	("id", id)
				("version", version)
				("stages", stages, VISIT_VECTOR)
				;
		}
	};
	
	Array<Session> sessions;
	Vector<String> stage_name_presets;
	
public:
	typedef StageThread CLASSNAME;
	StageThread(MetaNode& n) : MetaNodeExt(n) {}
	
	void Visit(NodeVisitor& vis) override {
		vis.Ver(1)
		(1)	("sessions", sessions, VISIT_VECTOR)
			("stage_name_presets", stage_name_presets)
		;
	}
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_AI_STAGE;}
	
};

INITIALIZE(StageThread)

class SpeechTranscriptionThread : public virtual AiThread {
	
public:
	typedef SpeechTranscriptionThread CLASSNAME;
	
	void Visit(NodeVisitor& vis) override {vis.Ver(0);}
};

class SpeechGenerationThread : public virtual AiThread {
	
public:
	typedef SpeechGenerationThread CLASSNAME;
	
	void Visit(NodeVisitor& vis) override {vis.Ver(0);}
};

class ImageGenerationThread : public virtual AiThread {
	
public:
	typedef ImageGenerationThread CLASSNAME;
	
	void Visit(NodeVisitor& vis) override {vis.Ver(0);}
};

class ImageVisionThread : public virtual AiThread {
	
public:
	typedef ChatThread CLASSNAME;
	
	void Visit(NodeVisitor& vis) override {vis.Ver(0);}
};

class MetaEnvThread : public virtual AiThread {
	
public:
	typedef MetaEnvThread CLASSNAME;
	
	void Visit(NodeVisitor& vis) override {vis.Ver(0);}
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
	
	void Visit(NodeVisitor& vis) override;
	void OnPhraseEnd(SoundPhrase& p) override;
	void OnMessageEnd(SoundMessage&) override;
	void OnDiscussionEnd(SoundDiscussion&) override;
	
	static OmniThread& Single() {static OmniThread m; return m;}
};

END_UPP_NAMESPACE


#endif
