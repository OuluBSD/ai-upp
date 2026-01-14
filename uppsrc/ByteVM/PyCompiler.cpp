#include "ByteVM.h"

namespace Upp {

const Token& PyCompiler::Peek() const
{
	static Token eof_token;
	eof_token.type = TK_EOF;
	return pos < tokens.GetCount() ? tokens[pos] : eof_token;
}

void PyCompiler::Next()
{
	if(pos < tokens.GetCount()) pos++;
}

bool PyCompiler::IsEof() const
{
	return pos >= tokens.GetCount() || tokens[pos].type == TK_EOF;
}

bool PyCompiler::IsToken(int type) const
{
	return !IsEof() && tokens[pos].type == type;
}

bool PyCompiler::IsId() const
{
	return !IsEof() && tokens[pos].type == TK_ID;
}

bool PyCompiler::IsId(const char *id) const
{
	return IsId() && tokens[pos].str_value == id;
}

bool PyCompiler::IsInt() const
{
	return !IsEof() && (tokens[pos].type == TK_INTEGER || tokens[pos].type == TK_HEX || tokens[pos].type == TK_OCT || tokens[pos].type == TK_BIN);
}

bool PyCompiler::IsDouble() const
{
	return !IsEof() && (tokens[pos].type == TK_DOUBLE || tokens[pos].type == TK_FLOAT);
}

bool PyCompiler::IsString() const
{
	return !IsEof() && (tokens[pos].type == TK_STRING || tokens[pos].type == TK_STRING_MULTILINE);
}

int PyCompiler::GetLine() const
{
	return Peek().loc.line;
}

void PyCompiler::Emit(int code)
{
	ir.Add(PyIR(code, GetLine()));
}

void PyCompiler::Emit(int code, int iarg)
{
	ir.Add(PyIR(code, iarg, GetLine()));
}

void PyCompiler::EmitConst(const PyValue& v)
{
	ir.Add(PyIR::Const(v, GetLine()));
}

void PyCompiler::EmitName(int code, const String& name)
{
	PyIR r(code, GetLine());
	r.arg = PyValue(name);
	ir.Add(r);
}

int PyCompiler::Label()
{
	return ir.GetCount();
}

void PyCompiler::Patch(int label_pc, int target_pc)
{
	ir[label_pc].iarg = target_pc;
}

void PyCompiler::Expect(int token)
{
	if(!IsToken(token))
		throw Exc(Format("Line %d: Expected token %s, found %s", GetLine(), Token::GetTypeStringStatic(token), Peek().GetTypeString()));
	Next();
}

void PyCompiler::ExpectId(const char *id)
{
	if(!IsId(id))
		throw Exc(Format("Line %d: Expected '%s', found '%s'", GetLine(), id, Peek().str_value));
	Next();
}

void PyCompiler::Compile(Vector<PyIR>& out)
{
	try {
		while(!IsEof()) {
			while(IsToken(TK_END_STMT))
				Next();
			if(IsEof()) break;
			Statement();
		}
		EmitConst(PyValue()); // Load None
		Emit(PY_RETURN_VALUE);
		out = pick(ir);
	} catch (Exc& e) {
		Cout() << "Compilation error: " << e << "\n";
		throw;
	}
}

void PyCompiler::CompileBlock(Vector<PyIR>& out)
{
	ir.Clear();
	try {
		while(!IsToken(TK_DEDENT) && !IsEof()) {
			while(IsToken(TK_END_STMT)) Next();
			if(IsToken(TK_DEDENT) || IsEof()) break;
			Statement();
		}
		if(ir.IsEmpty() || ir.Top().code != PY_RETURN_VALUE) {
			EmitConst(PyValue());
			Emit(PY_RETURN_VALUE);
		}
		out = pick(ir);
	} catch (Exc& e) {
		Cout() << "Compilation error in block: " << e << "\n";
		throw;
	}
}

void PyCompiler::Statement()
{
	if(IsId("if")) {
		Next();
		Expression();
		Expect(TK_COLON);
		int jump_false = Label();
		Emit(PY_POP_JUMP_IF_FALSE, 0);
		
		while(IsToken(TK_END_STMT)) Next();
		Expect(TK_INDENT);
		while(!IsToken(TK_DEDENT) && !IsEof()) {
			while(IsToken(TK_END_STMT)) Next();
			if(IsToken(TK_DEDENT) || IsEof()) break;
			Statement();
		}
		Expect(TK_DEDENT);
		
		while(IsToken(TK_END_STMT)) Next();
		
		if(IsId("else")) {
			Next();
			int jump_end = Label();
			Emit(PY_JUMP_ABSOLUTE, 0);
			Patch(jump_false, Label());
			Expect(TK_COLON);
			while(IsToken(TK_END_STMT)) Next();
			Expect(TK_INDENT);
			while(!IsToken(TK_DEDENT) && !IsEof()) {
				while(IsToken(TK_END_STMT)) Next();
				if(IsToken(TK_DEDENT) || IsEof()) break;
				Statement();
			}
			Expect(TK_DEDENT);
			Patch(jump_end, Label());
		}
		else {
			Patch(jump_false, Label());
		}
	}
	else if(IsId("while")) {
		Next();
		int start = Label();
		Expression();
		Expect(TK_COLON);
		int jump_end = Label();
		Emit(PY_POP_JUMP_IF_FALSE, 0);
		
		while(IsToken(TK_END_STMT)) Next();
		Expect(TK_INDENT);
		while(!IsToken(TK_DEDENT) && !IsEof()) {
			while(IsToken(TK_END_STMT)) Next();
			if(IsToken(TK_DEDENT) || IsEof()) break;
			Statement();
		}
		Expect(TK_DEDENT);
		
		Emit(PY_JUMP_ABSOLUTE, start);
		Patch(jump_end, Label());
	}
	else if(IsId("for")) {
		Next();
		String target = Peek().str_value;
		Expect(TK_ID);
		ExpectId("in");
		Expression();
		Emit(PY_GET_ITER);
		
		int start = Label();
		int jump_end = Label();
		Emit(PY_FOR_ITER, 0);
		
		EmitName(PY_STORE_NAME, target);
		
		Expect(TK_COLON);
		while(IsToken(TK_END_STMT)) Next();
		Expect(TK_INDENT);
		while(!IsToken(TK_DEDENT) && !IsEof()) {
			while(IsToken(TK_END_STMT)) Next();
			if(IsToken(TK_DEDENT) || IsEof()) break;
			Statement();
		}
		Expect(TK_DEDENT);
		
		Emit(PY_JUMP_ABSOLUTE, start);
		Patch(jump_end, Label());
	}
	else if(IsId("for")) {
		Next();
		String target = Peek().str_value;
		Expect(TK_ID);
		ExpectId("in");
		Expression();
		Emit(PY_GET_ITER);
		
		int start = Label();
		int jump_end = Label();
		Emit(PY_FOR_ITER, 0);
		
		EmitName(PY_STORE_NAME, target);
		
		Expect(TK_COLON);
		while(IsToken(TK_END_STMT)) Next();
		Expect(TK_INDENT);
		while(!IsToken(TK_DEDENT) && !IsEof()) {
			while(IsToken(TK_END_STMT)) Next();
			if(IsToken(TK_DEDENT) || IsEof()) break;
			Statement();
		}
		Expect(TK_DEDENT);
		
		Emit(PY_JUMP_ABSOLUTE, start);
		Patch(jump_end, Label());
	}
	else if(IsId("def")) {
		Next();
		String name = Peek().str_value;
		Expect(TK_ID);
		Expect(TK_PARENTHESIS_BEGIN);
		Vector<String> args;
		if(!IsToken(TK_PARENTHESIS_END)) {
			do {
				args.Add(Peek().str_value);
				Expect(TK_ID);
			} while(IsToken(TK_COMMA) && (Next(), true));
		}
		Expect(TK_PARENTHESIS_END);
		Expect(TK_COLON);
		while(IsToken(TK_END_STMT)) Next();
		Expect(TK_INDENT);
		
		PyCompiler sub(tokens);
		sub.pos = pos;
		Vector<PyIR> body;
		sub.CompileBlock(body);
		pos = sub.pos;
		Expect(TK_DEDENT);
		
		PyValue func = PyValue::Function(name);
		func.GetLambdaRW().ir = pick(body);
		func.GetLambdaRW().arg = pick(args);
		
		EmitConst(func);
		EmitName(PY_STORE_NAME, name);
	}
	else if(IsId("return")) {
		Next();
		if(IsToken(TK_END_STMT)) {
			EmitConst(PyValue());
		}
		else {
			Expression();
		}
		Emit(PY_RETURN_VALUE);
		Expect(TK_END_STMT);
	}
	else if(IsId("print")) {
		Next();
		Expression();
		Emit(PY_PRINT_EXPR);
		Expect(TK_END_STMT);
	}
	else if(IsId() && pos + 1 < tokens.GetCount() && tokens[pos+1].type == TK_ASS) {
		String id = Peek().str_value;
		Next(); // id
		Next(); // =
		Expression();
		EmitName(PY_STORE_NAME, id);
		Expect(TK_END_STMT);
	}
	else {
		Expression();
		Emit(PY_POP_TOP);
		Expect(TK_END_STMT);
	}
}

void PyCompiler::Expression() { OrExpr(); }

void PyCompiler::OrExpr()
{
	AndExpr();
	while(IsId("or")) {
		Next();
		int jump = Label();
		Emit(PY_JUMP_IF_TRUE_OR_POP, 0);
		AndExpr();
		Patch(jump, Label());
	}
}

void PyCompiler::AndExpr()
{
	NotExpr();
	while(IsId("and")) {
		Next();
		int jump = Label();
		Emit(PY_JUMP_IF_FALSE_OR_POP, 0);
		NotExpr();
		Patch(jump, Label());
	}
}

void PyCompiler::NotExpr()
{
	if(IsId("not")) {
		Next();
		NotExpr();
		Emit(PY_UNARY_NOT);
	}
	else {
		Comparison();
	}
}

void PyCompiler::Comparison()
{
	AddExpr();
	if(IsToken(TK_EQ)) { Next(); AddExpr(); Emit(PY_COMPARE_OP, PY_CMP_EQ); }
	else if(IsToken(TK_INEQ)) { Next(); AddExpr(); Emit(PY_COMPARE_OP, PY_CMP_NE); }
	else if(IsToken(TK_LESS)) { Next(); AddExpr(); Emit(PY_COMPARE_OP, PY_CMP_LT); }
	else if(IsToken(TK_LSEQ)) { Next(); AddExpr(); Emit(PY_COMPARE_OP, PY_CMP_LE); }
	else if(IsToken(TK_GREATER)) { Next(); AddExpr(); Emit(PY_COMPARE_OP, PY_CMP_GT); }
	else if(IsToken(TK_GREQ)) { Next(); AddExpr(); Emit(PY_COMPARE_OP, PY_CMP_GE); }
}

void PyCompiler::AddExpr()
{
	MulExpr();
	while(IsToken(TK_PLUS) || IsToken(TK_MINUS)) {
		int op = Peek().type;
		Next();
		MulExpr();
		Emit(op == TK_PLUS ? PY_BINARY_ADD : PY_BINARY_SUBTRACT);
	}
}

void PyCompiler::MulExpr()
{
	UnaryExpr();
	while(IsToken(TK_MUL) || IsToken(TK_DIV) || IsToken(TK_FLOORDIV) || IsToken(TK_PERCENT)) {
		int op = Peek().type;
		Next();
		UnaryExpr();
		if(op == TK_MUL) Emit(PY_BINARY_MULTIPLY);
		else if(op == TK_DIV) Emit(PY_BINARY_TRUE_DIVIDE);
		else if(op == TK_FLOORDIV) Emit(PY_BINARY_FLOOR_DIVIDE);
		else Emit(PY_BINARY_MODULO);
	}
}

void PyCompiler::UnaryExpr()
{
	if(IsToken(TK_PLUS)) { Next(); UnaryExpr(); Emit(PY_UNARY_POSITIVE); }
	else if(IsToken(TK_MINUS)) { Next(); UnaryExpr(); Emit(PY_UNARY_NEGATIVE); }
	else if(IsToken(TK_TILDE)) { Next(); UnaryExpr(); Emit(PY_UNARY_INVERT); }
	else { PowerExpr(); }
}

void PyCompiler::PowerExpr()
{
	PrimaryExpr();
	if(IsToken(TK_EXP)) {
		Next();
		UnaryExpr();
		Emit(PY_BINARY_POWER);
	}
}

void PyCompiler::PrimaryExpr()
{
	Atom();
	while(IsToken(TK_PARENTHESIS_BEGIN) || IsToken(TK_SQUARE_BEGIN)) {
		if(IsToken(TK_PARENTHESIS_BEGIN)) {
			Next();
			int nargs = 0;
			if(!IsToken(TK_PARENTHESIS_END)) {
				do {
					Expression();
					nargs++;
				} while(IsToken(TK_COMMA) && (Next(), true));
			}
			Expect(TK_PARENTHESIS_END);
			Emit(PY_CALL_FUNCTION, nargs);
		}
		else { // TK_SQUARE_BEGIN
			Next();
			Expression();
			Expect(TK_SQUARE_END);
			Emit(PY_BINARY_SUBSCR);
		}
	}
}

void PyCompiler::Atom()
{
	if(IsInt()) {
		// Use ScanInt64 for different bases
		int64 v = 0;
		if (Peek().type == TK_HEX) v = ScanInt64(Peek().str_value, nullptr, 16);
		else if (Peek().type == TK_OCT) v = ScanInt64(Peek().str_value, nullptr, 8);
		else if (Peek().type == TK_BIN) v = ScanInt64(Peek().str_value, nullptr, 2);
		else v = StrInt64(Peek().str_value);
		EmitConst(PyValue(v));
		Next();
	}
	else if(IsDouble()) {
		EmitConst(PyValue(ScanDouble(Peek().str_value)));
		Next();
	}
	else if(IsString()) {
		EmitConst(PyValue(Peek().str_value));
		Next();
	}
	else if(IsId()) {
		String id = Peek().str_value;
		if(id == "True") EmitConst(PyValue(true));
		else if(id == "False") EmitConst(PyValue(false));
		else if(id == "None") EmitConst(PyValue());
		else {
			EmitName(PY_LOAD_NAME, id);
		}
		Next();
	}
	else if(IsToken(TK_PARENTHESIS_BEGIN)) {
		Next();
		Expression();
		Expect(TK_PARENTHESIS_END);
	}
	else if(IsToken(TK_SQUARE_BEGIN)) {
		Next();
		int n = 0;
		if(!IsToken(TK_SQUARE_END)) {
			do {
				Expression();
				n++;
			} while(IsToken(TK_COMMA) && (Next(), true));
		}
		Expect(TK_SQUARE_END);
		Emit(PY_BUILD_LIST, n);
	}
	else if(IsToken(TK_BRACKET_BEGIN)) {
		Next();
		int n = 0;
		if(!IsToken(TK_BRACKET_END)) {
			do {
				Expression(); // key
				Expect(TK_COLON);
				Expression(); // value
				n++;
			} while(IsToken(TK_COMMA) && (Next(), true));
		}
		Expect(TK_BRACKET_END);
		Emit(PY_BUILD_MAP, n);
	}
	else {
		throw Exc(Format("Line %d: Expected atom, found %s", GetLine(), Peek().GetTypeString()));
	}
}

}
