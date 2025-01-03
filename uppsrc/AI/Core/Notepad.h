#ifndef _AI_Core_Notepad_h_
#define _AI_Core_Notepad_h_


NAMESPACE_UPP


struct Notepad : Component
{
	struct Note : Moveable<Note> {
		String title;
		String outcome;
		String reference;
		String description;
		Time created;
		void Visit(NodeVisitor& v) {
			v.Ver(1)
			(1)	("title", title)
				("outcome", outcome)
				("reference", reference)
				("description", description)
				("created", created)
				;
		}
	};
	Vector<Note> notes;
	
	COMPONENT_CONSTRUCTOR(Notepad)
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1)	("notes", notes, VISIT_VECTOR)
			;
	}
	
	static int GetKind() {return METAKIND_ECS_COMPONENT_NOTEPAD;}
	
};

INITIALIZE(Notepad)


END_UPP_NAMESPACE

#endif
