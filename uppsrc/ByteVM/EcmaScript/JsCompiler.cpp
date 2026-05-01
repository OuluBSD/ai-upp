#include "JsCompiler.h"

NAMESPACE_UPP

void JsCompiler::JsEmit(int code)
{
	PyIR& p = ir.Add();
	p.line = GetLine();
	p.code = code;
}

void JsCompiler::JsEmit(int code, int iarg)
{
	PyIR& p = ir.Add();
	p.line = GetLine();
	p.code = code;
	p.iarg = iarg;
}

void JsCompiler::EmitConst(const PyValue& v)
{
	PyIR& p = ir.Add();
	p.line = GetLine();
	p.code = JS_LOAD_CONST;
	// We need a way to store JsValue in PyIR or map it.
	// For now, we assume PyIR::value can hold JsValue-like data (it is a PyValue).
	// This might need a JsIR or PyValue change if they diverge too much.
	// Since PyValue is a class with a union and type, we can probably use it.
	p.arg = v;
}

void JsCompiler::EmitName(int code, const String& name)
{
	// Map PY_ opcodes to JS_ opcodes
	int jscode = code;
	if (code == PY_LOAD_NAME) jscode = JS_LOAD_NAME;
	else if (code == PY_STORE_NAME) jscode = JS_STORE_NAME;
	else if (code == PY_LOAD_GLOBAL) jscode = JS_LOAD_GLOBAL;
	else if (code == PY_LOAD_FAST) jscode = JS_LOAD_FAST;
	else if (code == PY_STORE_FAST) jscode = JS_STORE_FAST;
	else if (code == PY_LOAD_ATTR) jscode = JS_LOAD_ATTR;
	else if (code == PY_STORE_ATTR) jscode = JS_STORE_ATTR;
	
	JsEmit(jscode, global_names.FindAdd(name));
}

void JsCompiler::ParseBlock()
{
	if(HaveToken(TK_BRACKET_BEGIN)) {
		while(!IsEof() && !HaveToken(TK_BRACKET_END))
			Statement();
	}
	else Statement();
}

void JsCompiler::Statement()
{
	if(HaveToken(TK_SEMICOLON)) return;
	
	if(IsId("let") || IsId("var") || IsId("const")) {
		Next();
		do {
			String name = Peek().str_value;
			Expect(TK_ID);
			if(HaveToken(TK_ASS)) {
				Expression();
				EmitName(PY_STORE_NAME, name);
			}
		} while(HaveToken(TK_COMMA));
		Expect(TK_SEMICOLON);
	}
	else if(IsId("if")) {
		Next();
		Expect(TK_PARENTHESIS_BEGIN);
		Expression();
		Expect(TK_PARENTHESIS_END);
		int if_false = Label();
		JsEmit(JS_POP_JUMP_IF_FALSE, 0);
		ParseBlock();
		if(IsId("else")) {
			Next();
			int end_if = Label();
			JsEmit(JS_JUMP_ABSOLUTE, 0);
			Patch(if_false, ir.GetCount());
			ParseBlock();
			Patch(end_if, ir.GetCount());
		}
		else {
			Patch(if_false, ir.GetCount());
		}
	}
	else if(IsId("while")) {
		Next();
		int start_loop = ir.GetCount();
		Expect(TK_PARENTHESIS_BEGIN);
		Expression();
		Expect(TK_PARENTHESIS_END);
		int end_loop = Label();
		JsEmit(JS_POP_JUMP_IF_FALSE, 0);
		ParseBlock();
		JsEmit(JS_JUMP_ABSOLUTE, start_loop);
		Patch(end_loop, ir.GetCount());
	}
	else if(IsId("return")) {
		Next();
		if(!IsStmtEnd()) Expression();
		else EmitConst(PyValue()); // return undefined
		JsEmit(JS_RETURN_VALUE);
		if(HaveToken(TK_SEMICOLON));
	}
	else {
		Expression();
		JsEmit(JS_POP_TOP);
		HaveToken(TK_SEMICOLON);
	}
}

void JsCompiler::Expression()
{
	PyCompiler::Expression();
}

void JsCompiler::Compile(Vector<PyIR>& out)
{
	try {
		while(!IsEof())
			Statement();
		JsEmit(JS_LOAD_CONST); // implicit return undefined
		JsEmit(JS_RETURN_VALUE);
		out = pick(ir);
	}
	catch(Exc& e) {
		throw Exc(file + " " + e);
	}
}

END_UPP_NAMESPACE
