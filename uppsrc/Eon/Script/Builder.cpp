#include "Script.h"
#include <ByteVM/ByteVM.h>

NAMESPACE_UPP

namespace Eon {


AtomBuilder& AtomBuilder::Assign(String id, Value val) {
	assigns.Add(id, val);
	return *this;
}

AstNode& AtomBuilder::CreateNode(AstNode& root) {
	ASSERT(id.parts.GetCount());
	FileLocation loc; // empty
	AstNode* cont = &root;
	for (auto& p : id.parts) {
		AstNode& n = cont->val.GetAdd<AstNode>(p);
		cont = &n;
		n.src = Cursor_NamePart;
		n.val.id = p;
		n.val.id = p;
	}
	cont->src = Cursor_AtomStmt;
	if (!assigns.IsEmpty()) {
		AstNode* expr = &cont->GetAdd(loc, Cursor_CompoundStmt);
		for(int i = 0; i < assigns.GetCount(); i++) {
			AstNode* exprstmt = &expr->Add(loc);
			exprstmt->src = Cursor_ExprStmt;
			
			AstNode& name = exprstmt->val.GetAdd<AstNode>(assigns.GetKey(i));
			name.src = Cursor_Unresolved;
			name.str = assigns.GetKey(i);
			
			AstNode& val = name.val.GetAdd<AstNode>();
			
			Value v = assigns[i];
			if (v.GetType() == INT64_V) {
				val.src = Cursor_Literal_INT64;
				val.i64 = (int64)v;
			}
			else if (v.GetType() == INT_V) {
				val.src = Cursor_Literal_INT32;
				val.i64 = (int)v;
			}
			else if (v.GetType() == DOUBLE_V) {
				val.src = Cursor_Literal_DOUBLE;
				val.dbl = (double)v;
			}
			else if (v.GetType() == STRING_V) {
				val.src = Cursor_Literal_STRING;
				val.str = v.ToString();
			}
			else {
				throw Exc("Can't assign value: " + v.ToString());
			}
			
			AstNode& assign = exprstmt->val.GetAdd<AstNode>();
			assign.src = Cursor_Op_ASSIGN;
			assign.arg[0] = &name;
			assign.arg[1] = &val;
			exprstmt->rval = &assign;
		}
	}
	return *cont;
}

AtomBuilder& LoopBuilder::AddAtom(String id) {
	auto& atom = atoms.Add();
	atom.id.Parse(id);
	return atom;
}

AstNode& LoopBuilder::CreateNode(AstNode& root) {
	ASSERT(id.parts.GetCount());
	AstNode* cont = &root;
	for (auto& p : id.parts) {
		AstNode& n = cont->val.GetAdd<AstNode>(p);
		cont = &n;
		n.src = Cursor_NamePart;
		n.val.id = p;
	}
	cont->src = Cursor_LoopStmt;
	for(int i = 0; i < atoms.GetCount(); i++) {
		atoms[i].CreateNode(*cont);
	}
	return *cont;
}

AtomBuilder& DriverBuilder::AddAtom(String id) {
	auto& atom = atoms.Add();
	atom.id.Parse(id);
	return atom;
}

AstNode& DriverBuilder::CreateNode(AstNode& root) {
	ASSERT(id.parts.GetCount());
	AstNode* cont = &root;
	for (auto& p : id.parts) {
		AstNode& n = cont->val.GetAdd<AstNode>(p);
		cont = &n;
		n.src = Cursor_NamePart;
		n.val.id = p;
	}
	cont->src = Cursor_DriverStmt;
	AstNode* compound = &cont->GetAdd(FileLocation(), Cursor_CompoundStmt);
	for(int i = 0; i < atoms.GetCount(); i++) {
		atoms[i].CreateNode(*compound);
	}
	return *cont;
}

LoopBuilder& ChainBuilder::AddLoop(String id) {
	auto& loop = loops.Add();
	loop.id.Parse(id);
	return loop;
}

AstNode& ChainBuilder::CreateNode(AstNode& root) {
	ASSERT(id.parts.GetCount());
	AstNode* cont = &root;
	for (auto& p : id.parts) {
		AstNode& n = cont->val.GetAdd<AstNode>(p);
		cont = &n;
		n.src = Cursor_NamePart;
		n.val.id = p;
	}
	cont->src = Cursor_ChainStmt;
	AstNode* compound = &cont->GetAdd(FileLocation(), Cursor_CompoundStmt);
	for(int i = 0; i < loops.GetCount(); i++) {
		loops[i].CreateNode(*compound);
	}
	return *cont;
}

AtomBuilder& NetBuilder::AddAtom(String id) {
	auto& atom = atoms.Add();
	atom.id.Parse(id);
	return atom;
}

void NetBuilder::Connect(String from_atom, int from_port, String to_atom, int to_port) {
	auto& c = connections.Add();
	c.from_atom = from_atom;
	c.from_port = from_port;
	c.to_atom = to_atom;
	c.to_port = to_port;
}

AstNode& NetBuilder::CreateNode(AstNode& root) {
	ASSERT(id.parts.GetCount());
	AstNode* cont = &root;
	for (auto& p : id.parts) {
		AstNode& n = cont->val.GetAdd<AstNode>(p);
		cont = &n;
		n.src = Cursor_NamePart;
		n.val.id = p;
	}
	cont->src = Cursor_NetStmt;
	AstNode* compound = &cont->GetAdd(FileLocation(), Cursor_CompoundStmt);
	for(int i = 0; i < atoms.GetCount(); i++) {
		atoms[i].CreateNode(*compound);
	}
	for(int i = 0; i < connections.GetCount(); i++) {
		auto& c = connections[i];
		AstNode& n = compound->Add(FileLocation());
		n.src = Cursor_Op_LINK;
		
		AstNode& from = n.val.GetAdd<AstNode>();
		from.src = Cursor_Unresolved;
		from.str = c.from_atom + "." + IntStr(c.from_port);
		
		AstNode& to = n.val.GetAdd<AstNode>();
		to.src = Cursor_Unresolved;
		to.str = c.to_atom + "." + IntStr(c.to_port);
		
		n.arg[0] = &from;
		n.arg[1] = &to;
	}
	return *cont;
}

DriverBuilder& MachineBuilder::AddDriver(String id) {
	auto& driver = drivers.Add();
	driver.id.Parse(id);
	return driver;
}

ChainBuilder& MachineBuilder::AddChain(String id) {
	auto& chain = chains.Add();
	chain.id.Parse(id);
	return chain;
}

NetBuilder& MachineBuilder::AddNet(String id) {
	auto& net = nets.Add();
	net.id.Parse(id);
	return net;
}

AstNode& MachineBuilder::CreateNode(AstNode& root) {
	ASSERT(id.parts.GetCount());
	AstNode* cont = &root;
	for (auto& p : id.parts) {
		AstNode& n = cont->val.GetAdd<AstNode>(p);
		cont = &n;
		n.src = Cursor_NamePart;
		n.val.id = p;
	}
	cont->src = Cursor_MachineStmt;
	AstNode* compound = &cont->GetAdd(FileLocation(), Cursor_CompoundStmt);
	for(int i = 0; i < drivers.GetCount(); i++) {
		drivers[i].CreateNode(*compound);
	}
	for(int i = 0; i < chains.GetCount(); i++) {
		chains[i].CreateNode(*compound);
	}
	for(int i = 0; i < nets.GetCount(); i++) {
		nets[i].CreateNode(*compound);
	}
	return *cont;
}

LoopBuilder& Builder::AddLoop(String id) {
	auto& loop = loops.Add();
	loop.id.Parse(id);
	return loop;
}

MachineBuilder& Builder::AddMachine(String id) {
	auto& machine = machines.Add();
	machine.id.Parse(id);
	return machine;
}

NetBuilder& Builder::AddNet(String id) {
	auto& net = nets.Add();
	net.id.Parse(id);
	return net;
}

AstNode* Builder::CompileAst() {
	int i = val.Find("root");
	if (i >= 0)
		val.Remove(i);

	AstNode& root = val.GetAdd<AstNode>("root");
	for (auto& loop : loops) {
		AstNode& n = loop.CreateNode(root);

	}
	for (auto& machine : machines) {
		AstNode& n = machine.CreateNode(root);
	}
	for (auto& net : nets) {
		AstNode& n = net.CreateNode(root);
	}

	return &root;
}

void Builder::Visit(Vis& v) {

}


}

END_UPP_NAMESPACE
