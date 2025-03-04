#ifndef _AI_Core_Perspective_h_
#define _AI_Core_Perspective_h_


NAMESPACE_UPP


struct Perspective : Component
{
	COMPONENT_CONSTRUCTOR(Perspective)
	
	struct Attr : Moveable<Attr> {
		String		positive;
		String		negative;
		void Visit(NodeVisitor& v) {v.Ver(1)(1)("positive",positive)("negative",negative);}
	};
	String			description;
	String			reference;
	Vector<Attr>	attrs;
	Vector<String>	user;
	
	void Visit(NodeVisitor& v) override {
		v.Ver(1)
		(1)	("description", description)
			("reference", reference)
			("attrs", attrs, VISIT_VECTOR)
			("user", user);
	}
	static int GetKind() {return METAKIND_ECS_COMPONENT_PERSPECTIVE;}
};

INITIALIZE(Perspective);


END_UPP_NAMESPACE

#endif
