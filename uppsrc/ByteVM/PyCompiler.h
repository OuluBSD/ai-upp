#ifndef _ByteVM_PyCompiler_h_
#define _ByteVM_PyCompiler_h_

namespace Upp {

class PyCompiler {
	const Vector<Token>& tokens;
	int pos = 0;
	Vector<PyIR> ir;
	
	struct Loop {
		int start_pc;
		Vector<int> break_pcs;
	};
	Vector<Loop> loops;

	const Token& Peek() const;
	void Next();
	bool IsEof() const;
	bool IsToken(int type) const;
	bool IsId() const;
	bool IsId(const char *id) const;
	bool IsInt() const;
	bool IsDouble() const;
	bool IsString() const;
	
	int  GetLine() const;

	void Emit(int code);
	void Emit(int code, int iarg);
	void EmitConst(const PyValue& v);
	void EmitName(int code, const String& name);
	
	int  Label();
	void Patch(int label_pc, int target_pc);

	void Statement();
	void Expression();
	void OrExpr();
	void AndExpr();
	void NotExpr();
	void Comparison();
	void BitOrExpr();
	void BitXorExpr();
	void BitAndExpr();
	void ShiftExpr();
	void AddExpr();
	void MulExpr();
	void UnaryExpr();
	void PowerExpr();
	void PrimaryExpr();
	void Atom();

	void Expect(int token);
	void ExpectId(const char *id);

public:
	PyCompiler(const Vector<Token>& tokens) : tokens(tokens) {}
	
	void Compile(Vector<PyIR>& out);
	void CompileBlock(Vector<PyIR>& out);
};

}

#endif
