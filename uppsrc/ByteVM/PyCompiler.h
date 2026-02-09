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
	
	Vector<Vector<int>> break_targets;
	Vector<int> continue_targets;

	bool IsId() const;
	bool IsId(const char *id) const;
	bool IsInt() const;
	bool IsDouble() const;
	bool IsString() const;
	bool IsStmtEnd() const {
		if (IsEof()) return true;
		int type = tokens[pos].type;
		if (type == TK_END_STMT || type == TK_NEWLINE || type == TK_SEMICOLON) return true;
		if (type == TK_COMMENT || type == TK_BLOCK_COMMENT) return true;
		if (type == TK_PUNCT && tokens[pos].str_value == ";") return true;
		return false;
	}
	int  GetLine() const;

	void Expect(int token);
	void ExpectId(const char *id);

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
