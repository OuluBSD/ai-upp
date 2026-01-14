#ifndef _Eon_Script_Builder_h_
#define _Eon_Script_Builder_h_


namespace Eon {


struct AtomBuilder
{
	Id id;
	VectorMap<String,Value> assigns;

	AtomBuilder&	Assign(String id, Value val);
	AstNode&		CreateNode(AstNode& root);

};

struct LoopBuilder
{
	Id id;
	Array<AtomBuilder> atoms;

	AtomBuilder&	AddAtom(String id);
	AstNode&		CreateNode(AstNode& root);

};

struct DriverBuilder
{
	Id id;
	Array<AtomBuilder> atoms;

	AtomBuilder&	AddAtom(String id);
	AstNode&		CreateNode(AstNode& root);
};

struct ChainBuilder
{
	Id id;
	Array<LoopBuilder> loops;

	LoopBuilder&	AddLoop(String id);
	AstNode&		CreateNode(AstNode& root);
};

struct NetBuilder
{
	Id id;
	Array<AtomBuilder> atoms;
	Vector<NetConnectionDef> connections;

	AtomBuilder&	AddAtom(String id);
	void			Connect(String from_atom, int from_port, String to_atom, int to_port);
	AstNode&		CreateNode(AstNode& root);
};

struct MachineBuilder
{
	Id id;
	Array<DriverBuilder> drivers;
	Array<ChainBuilder> chains;
	Array<NetBuilder> nets;

	DriverBuilder&	AddDriver(String id);
	ChainBuilder&	AddChain(String id);
	NetBuilder&		AddNet(String id);
	AstNode&		CreateNode(AstNode& root);
};

struct Builder : VfsValueExt
{
	METANODE_EXT_CONSTRUCTOR(Builder)

	Array<LoopBuilder> loops;
	Array<MachineBuilder> machines;
	Array<NetBuilder> nets;

	LoopBuilder&		AddLoop(String id);
	MachineBuilder&		AddMachine(String id);
	NetBuilder&			AddNet(String id);
	AstNode*			CompileAst();
	void				Visit(Vis& v) override;
};



}


#endif
