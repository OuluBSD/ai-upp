#ifndef _ByteVM_PyCompiler_h_
#define _ByteVM_PyCompiler_h_

#include "Python.h"


NAMESPACE_UPP

class PyCompiler {
protected:
	const Vector<Token>& tokens;
	String file;
	int pos;
	
	Index<String> global_names;

	const Token& Peek() const;
	void Next();
	bool IsEof() const;
	bool IsToken(int type) const;
	bool HaveToken(int type) { if(IsToken(type)) { Next(); return true; } return false; }
	
	Vector<Vector<int>> break_targets;
	Vector<int> continue_targets;

	bool IsId() const;
	bool IsId(const char *id) const;
	bool IsInt() const;
	bool IsDouble() const;
	bool IsString() const;
	bool IsStmtEnd() const;
	int  GetLine() const;

	void Expect(int token);
	void ExpectId(const char *id);
	
	virtual void ParseBlock();
	virtual void Statement();
	virtual void Expression();
	
	void OrExpr();
	void AndExpr();
	void NotExpr();
	void Comparison();
	void BitAndExpr();
	void AddExpr();
	void MulExpr();
	void UnaryExpr();
	void PowerExpr();
	void PrimaryExpr();
	void Atom();
	
	void Emit(int code);
	void Emit(int code, int iarg);
	virtual void EmitConst(const PyValue& v);
	virtual void EmitName(int code, const String& name);
	bool IsGlobalName(const String& name) const { return global_names.Find(name) >= 0; }
	
	int  Label();
	void Patch(int label_pc, int target_pc);

public:
	Vector<PyIR> ir;

	PyCompiler(const Vector<Token>& tokens, String file = String()) : tokens(tokens), file(file), pos(0) {}
	virtual ~PyCompiler() {}
	
	void Compile(Vector<PyIR>& out);
	void CompileBlock(Vector<PyIR>& out);
	void CompileLambdaBody(Vector<PyIR>& out);
};

END_UPP_NAMESPACE

#endif
