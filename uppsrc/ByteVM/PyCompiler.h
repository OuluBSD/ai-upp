#ifndef _ByteVM_PyCompiler_h_
#define _ByteVM_PyCompiler_h_

NAMESPACE_UPP

class PyCompiler {
	const Vector<Token>& tokens;
	int pos;
	
	Vector<PyIR> ir;
	
	const Token& Peek() const;
	void Next();
	bool IsEof() const;
	bool IsToken(int type) const;
	bool HaveToken(int type) { if(IsToken(type)) { Next(); return true; } return false; }
	
	bool IsId() const;
	bool IsId(const char *id) const;
	bool IsInt() const;
	bool IsDouble() const;
	bool IsString() const;
	
	void Expect(int token);
	void ExpectId(const char *id);
	
	int GetLine() const;
	
	void Statement();
	void Expression();
	void OrExpr();
	void AndExpr();
	void NotExpr();
	void Comparison();
	void AddExpr();
	void MulExpr();
	void UnaryExpr();
	void PowerExpr();
	void PrimaryExpr();
	void Atom();
	
	void Emit(int code);
	void Emit(int code, int iarg);
	void EmitConst(const PyValue& v);
	void EmitName(int code, const String& name);
	
	int  Label();
	void Patch(int label_pc, int target_pc);

public:
	PyCompiler(const Vector<Token>& tokens) : tokens(tokens), pos(0) {}
	
	void Compile(Vector<PyIR>& out);
	void CompileBlock(Vector<PyIR>& out);
};

END_UPP_NAMESPACE

#endif
