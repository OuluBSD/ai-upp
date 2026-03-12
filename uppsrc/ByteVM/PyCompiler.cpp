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
	ir.Add(PyIR(code, GetLine(), file));
}

void PyCompiler::Emit(int code, int iarg)
{
	ir.Add(PyIR(code, iarg, GetLine(), file));
}

void PyCompiler::EmitConst(const PyValue& v)
{
	ir.Add(PyIR::Const(v, GetLine(), file));
}

void PyCompiler::EmitName(int code, const String& name)
{
	PyIR r(code, 0, GetLine(), file);
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

// Parse a block body after ':'. Handles both indented blocks and single-line bodies.
void PyCompiler::ParseBlock()
{
	while(!IsEof() && (IsStmtEnd() || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT))) Next();
	if(IsToken(TK_INDENT)) {
		Next(); // consume INDENT
		while(!IsToken(TK_DEDENT) && !IsEof()) {
			while(!IsEof() && (IsStmtEnd() || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT))) Next();
			if(IsToken(TK_DEDENT) || IsEof()) break;
			Statement();
		}
		if(IsToken(TK_DEDENT)) Next(); // consume DEDENT
	} else {
		// Single-line body (e.g. "if x > 0: x = 0")
		Statement();
	}
}

void PyCompiler::Compile(Vector<PyIR>& out)
{
	try {
		while(!IsEof()) {
			if (IsStmtEnd() || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT)) {
				Next();
				continue;
			}
			Statement();
		}
		EmitConst(PyValue()); // Load None
		Emit(PY_RETURN_VALUE);
		out = pick(ir);
	} catch (Exc& e) {
		Cout() << "Compilation error [" << file << "]: " << e << "\n";
		throw;
	}
}

void PyCompiler::CompileBlock(Vector<PyIR>& out)
{
	ir.Clear();
	try {
		while(!IsToken(TK_DEDENT) && !IsEof()) {
			while(!IsEof() && (IsStmtEnd() || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT))) Next();
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

void PyCompiler::CompileLambdaBody(Vector<PyIR>& out)
{
	ir.Clear();
	Expression();
	Emit(PY_RETURN_VALUE);
	out = pick(ir);
}

void PyCompiler::Statement()
{
	if(IsId("if")) {
		Next();
		Expression();
		this->Expect(TK_COLON);
		int jump_false = Label();
		Emit(PY_POP_JUMP_IF_FALSE, 0);
		ParseBlock();

		Vector<int> jump_ends;
		while(!IsEof() && (IsStmtEnd() || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT))) Next();

		while(IsId("elif")) {
			Next(); // skip elif
			int jump_next = Label();
			Emit(PY_JUMP_ABSOLUTE, 0); // Jump to end of if
			jump_ends.Add(jump_next);

			Patch(jump_false, Label()); // This elif's condition check starts here

			Expression();
			this->Expect(TK_COLON);
			jump_false = Label();
			Emit(PY_POP_JUMP_IF_FALSE, 0);
			ParseBlock();
			while(!IsEof() && (IsStmtEnd() || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT))) Next();
		}

		if(IsId("else")) {
			Next(); // skip else
			int jump_end = Label();
			Emit(PY_JUMP_ABSOLUTE, 0); // Jump to end of if
			jump_ends.Add(jump_end);

			Patch(jump_false, Label()); // Else block starts here

			this->Expect(TK_COLON);
			ParseBlock();
		}
		else {
			Patch(jump_false, Label()); // End of if (if no else)
		}

		for(int pc : jump_ends)
			Patch(pc, Label());
	}
	else if(IsId("while")) {
		Next();
		int start = Label();
		continue_targets.Add(start);
		break_targets.Add();

		Expression();
		this->Expect(TK_COLON);
		int jump_end = Label();
		Emit(PY_POP_JUMP_IF_FALSE, 0); // Jumps to Patch(jump_end, Label())
		ParseBlock();

		Emit(PY_JUMP_ABSOLUTE, start);
		Patch(jump_end, Label());

		for (int pc : break_targets.Top()) Patch(pc, Label());
		break_targets.Pop();
		continue_targets.Pop();
	}
	else if(IsId("for")) {
		Next();
		// Collect targets: single name or tuple (a, b, ...)
		Vector<String> targets;
		targets.Add(Peek().str_value);
		this->Expect(TK_ID);
		while(IsToken(TK_COMMA)) {
			Next(); // consume ','
			if(IsId() && !IsId("in")) {
				targets.Add(Peek().str_value);
				Next();
			}
		}
		this->ExpectId("in");
		Expression();
		Emit(PY_GET_ITER);

		int start = Label();
		continue_targets.Add(start);
		break_targets.Add();

		int jump_end = Label();
		Emit(PY_FOR_ITER, 0); // Jumps to Patch(jump_end, Label())

		if(targets.GetCount() == 1) {
			EmitName(PY_STORE_NAME, targets[0]);
		} else {
			// Tuple unpacking: store to temp, then subscript
			EmitName(PY_STORE_NAME, "__for_unpack__");
			for(int ti = 0; ti < targets.GetCount(); ti++) {
				EmitName(PY_LOAD_NAME, "__for_unpack__");
				EmitConst(PyValue((int64)ti));
				Emit(PY_BINARY_SUBSCR);
				EmitName(PY_STORE_NAME, targets[ti]);
			}
		}

		this->Expect(TK_COLON);
		ParseBlock();

		Emit(PY_JUMP_ABSOLUTE, start);
		Patch(jump_end, Label());

		for (int pc : break_targets.Top()) Patch(pc, Label());
		break_targets.Pop();
		continue_targets.Pop();
	}
	else if(IsId("break")) {
		Next();
		if (break_targets.IsEmpty()) throw Exc(Format("Line %d: 'break' outside loop", GetLine()));
		break_targets.Top().Add(Label());
		Emit(PY_JUMP_ABSOLUTE, 0);
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after 'break', found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	else if(IsId("continue")) {
		Next();
		if (continue_targets.IsEmpty()) throw Exc(Format("Line %d: 'continue' outside loop", GetLine()));
		Emit(PY_JUMP_ABSOLUTE, continue_targets.Top());
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after 'continue', found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	else if(IsId("def")) {
		Next();
		String name = Peek().str_value;
		this->Expect(TK_ID);
		this->Expect(TK_PARENTHESIS_BEGIN);
		Vector<String> args;
		if(!IsToken(TK_PARENTHESIS_END)) {
			do {
				args.Add(Peek().str_value);
				this->Expect(TK_ID);
			} while(IsToken(TK_COMMA) && (Next(), true));
		}
		this->Expect(TK_PARENTHESIS_END);
		this->Expect(TK_COLON);
		while(!IsEof() && (IsStmtEnd() || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT))) Next();
		this->Expect(TK_INDENT);
		
		PyCompiler sub(tokens, file);
		sub.pos = pos;
		Vector<PyIR> body;
		sub.CompileBlock(body);
		pos = sub.pos;
		this->Expect(TK_DEDENT);
		
			PyValue func = PyValue::Function(name);
			func.GetLambdaRW().ir = pick(body);
			func.GetLambdaRW().arg = pick(args);
			for (const String& a : func.GetLambda().arg)
				func.GetLambdaRW().arg_values.Add(PyValue(a));
			
			EmitConst(func);
			Emit(PY_MAKE_FUNCTION);
			EmitName(PY_STORE_NAME, name);
		}
	else if(IsId("class")) {
		Next();
		String class_name = Peek().str_value;
		this->Expect(TK_ID);
		// Optional base class list: (Base1, Base2, ...) — parsed but not yet used in VM
		if(IsToken(TK_PARENTHESIS_BEGIN)) {
			Next();
			int depth = 1;
			while(!IsEof() && depth > 0) {
				if(IsToken(TK_PARENTHESIS_BEGIN)) depth++;
				else if(IsToken(TK_PARENTHESIS_END)) depth--;
				Next();
			}
		}
		this->Expect(TK_COLON);
		while(!IsEof() && (IsStmtEnd() || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT))) Next();
		this->Expect(TK_INDENT);

		PyCompiler sub(tokens, file);
		sub.pos = pos;
		Vector<PyIR> body;
		sub.CompileBlock(body);
		pos = sub.pos;
		this->Expect(TK_DEDENT);

		// Emit class body as a no-arg function constant, then PY_BUILD_CLASS
		PyValue body_func = PyValue::Function(class_name + "_body");
		body_func.GetLambdaRW().ir = pick(body);

		EmitConst(body_func);
		Emit(PY_MAKE_FUNCTION);
		{
			PyIR build_ir(PY_BUILD_CLASS, 0, GetLine(), file);
			build_ir.arg = PyValue(class_name);
			ir.Add(build_ir);
		}
		EmitName(PY_STORE_NAME, class_name);
	}
	else if(IsId("pass")) {
		Next();
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after 'pass', found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	else if(IsId("return")) {
		Next();
		if(IsStmtEnd()) {
			EmitConst(PyValue());
		}
		else {
			Expression();
			if(IsToken(TK_COMMA)) {
				// Tuple return: return a, b, c
				int n = 1;
				while(IsToken(TK_COMMA)) {
					Next();
					if(IsStmtEnd()) break;
					Expression();
					n++;
				}
				Emit(PY_BUILD_TUPLE, n);
			}
		}
		Emit(PY_RETURN_VALUE);
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after 'return', found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	else if(IsId("import")) {
		Next();
		while(IsId()) {
			String module_name = Peek().str_value;
			Next();
			while(IsToken(TK_PUNCT)) {
				Next();
				if(IsId()) {
					module_name << "." << Peek().str_value;
					Next();
				}
				else break;
			}
			EmitName(PY_IMPORT_NAME, module_name);
			EmitName(PY_STORE_NAME, module_name);
			if(IsToken(TK_COMMA)) Next();
			else break;
		}
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after 'import', found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	else if(IsId("from")) {
		Next();
		if (!IsId()) throw Exc(Format("Line %d: Expected module name after 'from'", GetLine()));
		String module_name = Peek().str_value;
		Next();
		while(IsToken(TK_PUNCT)) {
			Next();
			if(IsId()) {
				module_name << "." << Peek().str_value;
				Next();
			}
			else break;
		}
		if (!IsId("import")) throw Exc(Format("Line %d: Expected 'import' after module name", GetLine()));
		Next();
		
		EmitName(PY_IMPORT_NAME, module_name);
		
		if (IsToken(TK_MUL)) { // from module import *
			Next();
			Emit(PY_IMPORT_STAR);
		}
		else {
			while (IsId()) {
				String name = Peek().str_value;
				EmitName(PY_IMPORT_FROM, name);
				EmitName(PY_STORE_NAME, name);
				Next();
				if (IsToken(TK_COMMA)) Next();
				else break;
			}
			Emit(PY_POP_TOP); // pop the module
		}
		
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after 'import', found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	// Augmented assignment: name op= expr  (e.g. x += 1)
	else if(IsId() && pos + 1 < tokens.GetCount() && (
	        tokens[pos+1].type == TK_ADDASS || tokens[pos+1].type == TK_SUBASS ||
	        tokens[pos+1].type == TK_MULASS || tokens[pos+1].type == TK_DIVASS ||
	        tokens[pos+1].type == TK_MODASS)) {
		String id = Peek().str_value;
		int op_type = tokens[pos+1].type;
		Next(); Next(); // id, op=
		EmitName(PY_LOAD_NAME, id);
		Expression();
		if(op_type == TK_ADDASS)      Emit(PY_BINARY_ADD);
		else if(op_type == TK_SUBASS) Emit(PY_BINARY_SUBTRACT);
		else if(op_type == TK_MULASS) Emit(PY_BINARY_MULTIPLY);
		else if(op_type == TK_DIVASS) Emit(PY_BINARY_TRUE_DIVIDE);
		else if(op_type == TK_MODASS) Emit(PY_BINARY_MODULO);
		EmitName(PY_STORE_NAME, id);
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after assignment, found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	// Augmented assignment: obj.attr op= expr  (e.g. self.x += 1)
	else if(IsId() && pos + 3 < tokens.GetCount()
	        && tokens[pos+1].type == TK_PUNCT
	        && tokens[pos+2].type == TK_ID && (
	        tokens[pos+3].type == TK_ADDASS || tokens[pos+3].type == TK_SUBASS ||
	        tokens[pos+3].type == TK_MULASS || tokens[pos+3].type == TK_DIVASS ||
	        tokens[pos+3].type == TK_MODASS)) {
		String obj_name = Peek().str_value;
		String attr = tokens[pos+2].str_value;
		int op_type = tokens[pos+3].type;
		Next(); Next(); Next(); Next(); // obj . attr op=
		EmitName(PY_LOAD_NAME, obj_name);
		Emit(PY_DUP_TOP);
		EmitName(PY_LOAD_ATTR, attr);
		Expression();
		if(op_type == TK_ADDASS)      Emit(PY_BINARY_ADD);
		else if(op_type == TK_SUBASS) Emit(PY_BINARY_SUBTRACT);
		else if(op_type == TK_MULASS) Emit(PY_BINARY_MULTIPLY);
		else if(op_type == TK_DIVASS) Emit(PY_BINARY_TRUE_DIVIDE);
		else if(op_type == TK_MODASS) Emit(PY_BINARY_MODULO);
		// Stack: [obj, new_val] — STORE_ATTR pops val then obj, correct order
		EmitName(PY_STORE_ATTR, attr);
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after assignment, found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	// Augmented assignment: obj.attr[idx] op= expr  (e.g. self.scores[i] += x)
	// Sequence: load obj.attr (DUP it), load idx (DUP it), BINARY_SUBSCR → current val, <rhs>, OP
	// then: ROT_THREE to get [new_val, list, idx_copy], STORE_SUBSCR
	else if(IsId() && pos + 4 < tokens.GetCount()
	        && tokens[pos+1].type == TK_PUNCT
	        && tokens[pos+2].type == TK_ID
	        && tokens[pos+3].type == TK_SQUARE_BEGIN) {
		String obj_name = Peek().str_value;
		String attr     = tokens[pos+2].str_value;
		int full_start_pos = pos; // save start in case we need to fall back completely
		Next(); Next(); Next();    // consume obj . attr
		// Save position BEFORE '[' so we can backtrack to it if needed
		int peek_pos_save = pos;
		int ir_save = ir.GetCount();
		this->Expect(TK_SQUARE_BEGIN);
		Expression();              // parse idx, emitting IR
		int idx_ir_count = ir.GetCount() - ir_save;
		this->Expect(TK_SQUARE_END);
		bool is_aug = IsToken(TK_ADDASS) || IsToken(TK_SUBASS) || IsToken(TK_MULASS) ||
		              IsToken(TK_DIVASS) || IsToken(TK_MODASS);
		bool is_plain_assign = IsToken(TK_ASS);
		if(is_aug) {
			// Save idx IR and rollback
			Vector<PyIR> idx_ir;
			for(int ki = ir_save; ki < ir_save + idx_ir_count; ki++)
				idx_ir.Add(ir[ki]);
			ir.SetCount(ir_save); // rollback idx IR
			int op_type = tokens[pos].type;
			Next(); // consume op=
			// Get current value: list[idx]
			EmitName(PY_LOAD_NAME, obj_name);
			EmitName(PY_LOAD_ATTR, attr);           // [list]
			for(auto& instr : idx_ir) ir.Add(instr); // [list, idx]
			Emit(PY_BINARY_SUBSCR);                  // [cur_val]
			Expression();                            // [cur_val, rhs]
			if(op_type == TK_ADDASS)      Emit(PY_BINARY_ADD);
			else if(op_type == TK_SUBASS) Emit(PY_BINARY_SUBTRACT);
			else if(op_type == TK_MULASS) Emit(PY_BINARY_MULTIPLY);
			else if(op_type == TK_DIVASS) Emit(PY_BINARY_TRUE_DIVIDE);
			else if(op_type == TK_MODASS) Emit(PY_BINARY_MODULO);
			// Stack: [new_val]
			// Need [list, idx, new_val] for STORE_SUBSCR (val=Pop, sub=Pop, obj=Pop)
			// Load list and idx, then use ROT_TWO twice to insert new_val at bottom of 3:
			// [new_val] → LOAD list → [new_val, list] → ROT_TWO → [list, new_val]
			// → emit idx → [list, new_val, idx] → ROT_TWO → [list, idx, new_val]
			EmitName(PY_LOAD_NAME, obj_name);
			EmitName(PY_LOAD_ATTR, attr);            // [new_val, list]
			Emit(PY_ROT_TWO);                        // [list, new_val]
			for(auto& instr : idx_ir) ir.Add(instr); // [list, new_val, idx]
			Emit(PY_ROT_TWO);                        // [list, idx, new_val]
			Emit(PY_STORE_SUBSCR);
		} else if(is_plain_assign) {
			// obj.attr[idx] = rhs
			ir.SetCount(ir_save);
			pos = peek_pos_save;
			EmitName(PY_LOAD_NAME, obj_name);
			EmitName(PY_LOAD_ATTR, attr);
			this->Expect(TK_SQUARE_BEGIN);
			Expression();
			this->Expect(TK_SQUARE_END);
			this->Expect(TK_ASS);
			Expression();
			Emit(PY_STORE_SUBSCR);
		} else {
			// Not an assignment at all (e.g. obj.attr[idx].method() or just reading)
			// Full backtrack to start of statement and use general Expression handler
			ir.SetCount(ir_save);
			pos = full_start_pos;
			Expression();
			Emit(PY_POP_TOP);
		}
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after expression, found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	else if(IsId() && pos + 3 < tokens.GetCount()
	        && tokens[pos+1].type == TK_PUNCT
	        && tokens[pos+2].type == TK_ID && tokens[pos+3].type == TK_ASS) {
		String obj = Peek().str_value;
		String attr = tokens[pos+2].str_value;
		Next(); // obj
		Next(); // .
		Next(); // attr
		Next(); // =
		EmitName(PY_LOAD_NAME, obj);
		Expression();
		EmitName(PY_STORE_ATTR, attr);
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after assignment, found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	// Tuple unpacking: a, b, c = expr
	else if(IsId() && pos + 1 < tokens.GetCount() && tokens[pos+1].type == TK_COMMA) {
		Vector<String> targets;
		targets.Add(Peek().str_value);
		Next(); // id
		while(IsToken(TK_COMMA)) {
			Next(); // ,
			if(IsId() && pos + 1 < tokens.GetCount() &&
			   (tokens[pos+1].type == TK_COMMA || tokens[pos+1].type == TK_ASS)) {
				targets.Add(Peek().str_value);
				Next();
			}
		}
		this->Expect(TK_ASS);
		Expression();
		EmitName(PY_STORE_NAME, "__unpack__");
		for(int ti = 0; ti < targets.GetCount(); ti++) {
			EmitName(PY_LOAD_NAME, "__unpack__");
			EmitConst(PyValue((int64)ti));
			Emit(PY_BINARY_SUBSCR);
			EmitName(PY_STORE_NAME, targets[ti]);
		}
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after assignment, found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	else if(IsId() && pos + 1 < tokens.GetCount() && tokens[pos+1].type == TK_ASS) {
		String id = Peek().str_value;
		Next(); // id
		Next(); // =
		Expression();
		EmitName(PY_STORE_NAME, id);
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after assignment, found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
	else if(IsId() && pos + 1 < tokens.GetCount() && tokens[pos+1].type == TK_SQUARE_BEGIN) {
		int start_pos = pos;
		int start_ir = ir.GetCount();
		String id = Peek().str_value;
		Next(); // id
		EmitName(PY_LOAD_NAME, id);
		this->Expect(TK_SQUARE_BEGIN);
		Expression();
		this->Expect(TK_SQUARE_END);
		if(IsToken(TK_ASS)) {
			Next(); // =
			Expression();
			Emit(PY_STORE_SUBSCR);
			if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after subscription assignment, found %s", GetLine(), Peek().GetTypeString()));
			if (IsToken(TK_NEWLINE) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
				Next();
		}
		else {
			// Not an assignment, backtrack and use normal expression parsing
			pos = start_pos;
			ir.SetCount(start_ir);
			Expression();
			Emit(PY_POP_TOP);
			if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after expression, found %s", GetLine(), Peek().GetTypeString()));
			if (IsToken(TK_NEWLINE) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
				Next();
		}
	}
	else {
		Expression();
		Emit(PY_POP_TOP);
		if (!IsStmtEnd()) throw Exc(Format("Line %d: Expected statement end after expression, found %s", GetLine(), Peek().GetTypeString()));
		while (IsToken(TK_NEWLINE) || IsToken(TK_COMMENT) || IsToken(TK_BLOCK_COMMENT) || (IsToken(TK_PUNCT) && Peek().str_value == ";"))
			Next();
	}
}

void PyCompiler::Expression()
{
	int start_of_a = Label();
	OrExpr();
	if(IsId("if")) {
		Next();
		
		Vector<PyIR> a_ir;
		for(int i = start_of_a; i < ir.GetCount(); i++)
			a_ir.Add(ir[i]);
		ir.SetCount(start_of_a);
		
		OrExpr(); // cond
		ExpectId("else");
		
		int jump_else = Label();
		Emit(PY_POP_JUMP_IF_FALSE, 0);
		
		int new_start_of_a = Label();
		int offset = new_start_of_a - start_of_a;
		for(auto& instr : a_ir) {
			if(instr.code == PY_JUMP_FORWARD || instr.code == PY_JUMP_IF_FALSE_OR_POP ||
			   instr.code == PY_JUMP_IF_TRUE_OR_POP || instr.code == PY_JUMP_ABSOLUTE ||
			   instr.code == PY_POP_JUMP_IF_FALSE || instr.code == PY_POP_JUMP_IF_TRUE ||
			   instr.code == PY_FOR_ITER)
			{
				instr.iarg += offset;
			}
			ir.Add(instr);
		}
		
		int jump_end = Label();
		Emit(PY_JUMP_ABSOLUTE, 0);
		
		Patch(jump_else, Label());
		Expression(); // b
		Patch(jump_end, Label());
	}
}

void PyCompiler::OrExpr()
{
	AndExpr();
	while(IsId("or")) {
		Next();
		int jump = Label();
		Emit(PY_JUMP_IF_TRUE_OR_POP, 0); // Jumps to Patch(jump, Label())
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
		Emit(PY_JUMP_IF_FALSE_OR_POP, 0); // Jumps to Patch(jump, Label())
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
	else if(IsId("in")) { Next(); AddExpr(); Emit(PY_COMPARE_OP, PY_CMP_IN); }
	else if(IsId("not")) {
		Next();
		if (IsId("in")) {
			Next();
			AddExpr();
			Emit(PY_COMPARE_OP, PY_CMP_NOT_IN);
		} else {
			// Backtrack
			pos--;
		}
	}
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
	while(IsToken(TK_PARENTHESIS_BEGIN) || IsToken(TK_SQUARE_BEGIN) || IsToken(TK_PUNCT)) {
		if(IsToken(TK_PARENTHESIS_BEGIN)) {
			Next();
			int nargs = 0;
			if(!IsToken(TK_PARENTHESIS_END)) {
				do {
					// Skip keyword argument name: id = expr
					if(IsId() && pos + 1 < tokens.GetCount()
					   && tokens[pos+1].type == TK_ASS) {
						Next(); Next(); // skip name and '='
					}
					// Handle generator expression: expr for var in iterable [if cond]
					// Compile as a list comprehension passed as argument
					int expr_start = ir.GetCount();
					Expression();
					if(IsId("for")) {
						// Generator expression: compile as list comprehension
						Vector<PyIR> item_ir;
						for(int ki = expr_start; ki < ir.GetCount(); ki++)
							item_ir.Add(ir[ki]);
						ir.SetCount(expr_start);
						Emit(PY_BUILD_LIST, 0);
						Next(); // consume 'for'
						// Support tuple unpacking in generator
						Vector<String> lvars;
						lvars.Add(Peek().str_value);
						this->Expect(TK_ID);
						while(IsToken(TK_COMMA)) {
							Next();
							if(IsId() && !IsId("in")) { lvars.Add(Peek().str_value); Next(); }
						}
						this->ExpectId("in");
						OrExpr(); // Use OrExpr to avoid consuming 'if' as ternary
						Emit(PY_GET_ITER);
						int loop_start = Label();
						int jump_end = Label();
						Emit(PY_FOR_ITER, 0);
						if(lvars.GetCount() == 1) {
							EmitName(PY_STORE_NAME, lvars[0]);
						} else {
							EmitName(PY_STORE_NAME, "__for_unpack__");
							for(int ti = 0; ti < lvars.GetCount(); ti++) {
								EmitName(PY_LOAD_NAME, "__for_unpack__");
								EmitConst(PyValue((int64)ti));
								Emit(PY_BINARY_SUBSCR);
								EmitName(PY_STORE_NAME, lvars[ti]);
							}
						}
						if(IsId("if")) {
							Next();
							Expression();
							int skip = Label();
							Emit(PY_POP_JUMP_IF_FALSE, 0);
							for(auto& ins : item_ir) ir.Add(ins);
							Emit(PY_LIST_APPEND, 2);
							Patch(skip, Label());
						} else {
							for(auto& ins : item_ir) ir.Add(ins);
							Emit(PY_LIST_APPEND, 2);
						}
						Emit(PY_JUMP_ABSOLUTE, loop_start);
						Patch(jump_end, Label());
					}
					nargs++;
				} while(IsToken(TK_COMMA) && (Next(), true));
			}
			this->Expect(TK_PARENTHESIS_END);
			Emit(PY_CALL_FUNCTION, nargs);
		}
		else if(IsToken(TK_SQUARE_BEGIN)) {
			Next();
			// Check for slice: [start:stop] or [:stop] or [start:]
			if(IsToken(TK_COLON)) {
				// [:stop]
				EmitConst(PyValue((int64)0));
				Next(); // consume ':'
				if(IsToken(TK_SQUARE_END)) EmitConst(PyValue((int64)-1)); // [:]
				else Expression(); // [:stop]
				this->Expect(TK_SQUARE_END);
				Emit(PY_BINARY_SLICE);
			} else {
				Expression();
				if(IsToken(TK_COLON)) {
					// [start:stop] or [start:]
					Next(); // consume ':'
					if(IsToken(TK_SQUARE_END)) EmitConst(PyValue((int64)-1)); // [start:]
					else Expression(); // [start:stop]
					this->Expect(TK_SQUARE_END);
					Emit(PY_BINARY_SLICE);
				} else {
					this->Expect(TK_SQUARE_END);
					Emit(PY_BINARY_SUBSCR);
				}
			}
		}
		else if(IsToken(TK_PUNCT)) {
			Next();
			if (IsId()) {
				String id = Peek().str_value;
				Next();
				EmitName(PY_LOAD_ATTR, id);
			} else {
				throw Exc(Format("Line %d: Expected member name after '.', found %s", GetLine(), Peek().GetTypeString()));
			}
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
		const Token& stok = Peek();
		String s = stok.str_value;
		if(stok.is_fstring) {
			// Parse f-string: split on {expr} segments and build concatenation
			// Each part emitted as a string; parts joined by BINARY_ADD
			int n_parts = 0;
			int i = 0;
			while(i <= s.GetCount()) {
				// Collect literal segment
				String lit;
				while(i < s.GetCount() && s[i] != '{') {
					if(s[i] == '}' && i+1 < s.GetCount() && s[i+1] == '}') { lit.Cat('}'); i += 2; }
					else lit.Cat(s[i++]);
				}
				if(lit.GetCount()) { EmitConst(PyValue(lit)); n_parts++; }
				if(i >= s.GetCount()) break;
				// Handle '{{' escape
				if(i+1 < s.GetCount() && s[i+1] == '{') { EmitConst(PyValue(String("{"))); n_parts++; i += 2; continue; }
				i++; // skip '{'
				// Collect expression until matching '}'
				String expr_src;
				int depth = 1;
				while(i < s.GetCount() && depth > 0) {
					if(s[i] == '{') depth++;
					else if(s[i] == '}') { depth--; if(depth == 0) break; }
					expr_src.Cat(s[i++]);
				}
				if(i < s.GetCount()) i++; // skip '}'
				// Compile the expression as str(expr)
				if(!expr_src.IsEmpty()) {
					Tokenizer etk;
					etk.SkipComments();
					etk.SkipPythonComments();
					etk.Process(expr_src, file);
					etk.NewlineToEndStatement();
					etk.CombineTokens();
					PyCompiler esub(etk.GetTokens(), file);
					// Emit: LOAD_GLOBAL str, expr..., CALL_FUNCTION 1
					EmitName(PY_LOAD_GLOBAL, "str");
					try { esub.Expression(); } catch(...) {}
					// Append esub's ir to our ir
					for(auto& ins : esub.ir) ir.Add(ins);
					Emit(PY_CALL_FUNCTION, 1);
					n_parts++;
				}
			}
			if(n_parts == 0) EmitConst(PyValue(s));
			else for(int pi = 1; pi < n_parts; pi++) Emit(PY_BINARY_ADD);
		} else {
			EmitConst(PyValue(s));
		}
		Next();
	}
	else if(IsId("lambda")) {
		Next(); // consume 'lambda'
		Vector<String> largs;
		while(IsId() && !IsId("in") && !IsId("for") && !IsId("if")) {
			largs.Add(Peek().str_value);
			Next();
			if(IsToken(TK_COMMA)) Next(); else break;
		}
		this->Expect(TK_COLON);
		PyCompiler sub(tokens, file);
		sub.pos = pos;
		Vector<PyIR> body;
		sub.CompileLambdaBody(body);
		pos = sub.pos;
		PyValue func = PyValue::Function("<lambda>");
		func.GetLambdaRW().ir = pick(body);
		func.GetLambdaRW().arg = pick(largs);
		for(const String& a : func.GetLambda().arg)
			func.GetLambdaRW().arg_values.Add(PyValue(a));
		EmitConst(func);
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
		if (IsToken(TK_PARENTHESIS_END)) {
			Next();
			Emit(PY_BUILD_TUPLE, 0);
		} else {
			int expr_start = ir.GetCount();
			Expression();
			if(IsId("for")) {
				// Generator expression: (expr for var in iterable [if cond])
				// Compile as list comprehension
				Vector<PyIR> item_ir;
				for(int ki = expr_start; ki < ir.GetCount(); ki++)
					item_ir.Add(ir[ki]);
				ir.SetCount(expr_start);
				Emit(PY_BUILD_LIST, 0);
				Next(); // consume 'for'
				Vector<String> lvars;
				lvars.Add(Peek().str_value);
				this->Expect(TK_ID);
				while(IsToken(TK_COMMA)) {
					Next();
					if(IsId() && !IsId("in")) { lvars.Add(Peek().str_value); Next(); }
				}
				this->ExpectId("in");
				OrExpr(); // use OrExpr to avoid consuming 'if' as ternary
				Emit(PY_GET_ITER);
				int loop_start = Label();
				int jump_end = Label();
				Emit(PY_FOR_ITER, 0);
				if(lvars.GetCount() == 1) {
					EmitName(PY_STORE_NAME, lvars[0]);
				} else {
					EmitName(PY_STORE_NAME, "__for_unpack__");
					for(int ti = 0; ti < lvars.GetCount(); ti++) {
						EmitName(PY_LOAD_NAME, "__for_unpack__");
						EmitConst(PyValue((int64)ti));
						Emit(PY_BINARY_SUBSCR);
						EmitName(PY_STORE_NAME, lvars[ti]);
					}
				}
				if(IsId("if")) {
					Next();
					Expression();
					int skip = Label();
					Emit(PY_POP_JUMP_IF_FALSE, 0);
					for(auto& ins : item_ir) ir.Add(ins);
					Emit(PY_LIST_APPEND, 2);
					Patch(skip, Label());
				} else {
					for(auto& ins : item_ir) ir.Add(ins);
					Emit(PY_LIST_APPEND, 2);
				}
				Emit(PY_JUMP_ABSOLUTE, loop_start);
				Patch(jump_end, Label());
				this->Expect(TK_PARENTHESIS_END);
			} else if (IsToken(TK_COMMA)) {
				int n = 1;
				while (IsToken(TK_COMMA)) {
					Next();
					if (IsToken(TK_PARENTHESIS_END)) break;
					Expression();
					n++;
				}
				this->Expect(TK_PARENTHESIS_END);
				Emit(PY_BUILD_TUPLE, n);
			} else {
				this->Expect(TK_PARENTHESIS_END);
			}
		}
	}
	else if(IsToken(TK_SQUARE_BEGIN)) {
		Next();
		if(IsToken(TK_SQUARE_END)) {
			Next();
			Emit(PY_BUILD_LIST, 0);
		} else {
			int expr_start = ir.GetCount();
			Expression();
			if(IsId("for")) {
				// List comprehension: [expr for var in iterable [if cond]]
				Vector<PyIR> item_ir;
				for(int ki = expr_start; ki < ir.GetCount(); ki++)
					item_ir.Add(ir[ki]);
				ir.SetCount(expr_start);
				Emit(PY_BUILD_LIST, 0);
				Next(); // consume 'for'
				// Support tuple unpacking: for a, b in iterable
				Vector<String> lvars;
				lvars.Add(Peek().str_value);
				this->Expect(TK_ID);
				while(IsToken(TK_COMMA)) {
					Next();
					if(IsId() && !IsId("in")) { lvars.Add(Peek().str_value); Next(); }
				}
				this->ExpectId("in");
				OrExpr(); // Use OrExpr (not Expression) to avoid consuming 'if' as ternary
				Emit(PY_GET_ITER);
				int loop_start = Label();
				int jump_end = Label();
				Emit(PY_FOR_ITER, 0);
				if(lvars.GetCount() == 1) {
					EmitName(PY_STORE_NAME, lvars[0]);
				} else {
					EmitName(PY_STORE_NAME, "__for_unpack__");
					for(int ti = 0; ti < lvars.GetCount(); ti++) {
						EmitName(PY_LOAD_NAME, "__for_unpack__");
						EmitConst(PyValue((int64)ti));
						Emit(PY_BINARY_SUBSCR);
						EmitName(PY_STORE_NAME, lvars[ti]);
					}
				}
				if(IsId("if")) {
					Next();
					Expression();
					int skip = Label();
					Emit(PY_POP_JUMP_IF_FALSE, 0);
					for(auto& ins : item_ir) ir.Add(ins);
					Emit(PY_LIST_APPEND, 2);
					Patch(skip, Label());
				} else {
					for(auto& ins : item_ir) ir.Add(ins);
					Emit(PY_LIST_APPEND, 2);
				}
				Emit(PY_JUMP_ABSOLUTE, loop_start);
				Patch(jump_end, Label());
				this->Expect(TK_SQUARE_END);
			} else {
				int n = 1;
				while(IsToken(TK_COMMA)) {
					Next();
					if(IsToken(TK_SQUARE_END)) break;
					Expression();
					n++;
				}
				this->Expect(TK_SQUARE_END);
				Emit(PY_BUILD_LIST, n);
			}
		}
	}
	else if(IsToken(TK_BRACKET_BEGIN)) {
		Next();
		int n = 0;
		if(!IsToken(TK_BRACKET_END)) {
			// Capture key IR
			int key_start = ir.GetCount();
			Expression(); // key
			this->Expect(TK_COLON);
			// Capture val IR
			int val_start = ir.GetCount();
			Expression(); // value
			int val_end = ir.GetCount();
			// Check for dict comprehension: {key: val for var in iterable}
			if(IsId("for")) {
				// Save key and val IR, reset
				Vector<PyIR> key_ir, val_ir;
				for(int ki = key_start; ki < val_start; ki++) key_ir.Add(ir[ki]);
				for(int ki = val_start; ki < val_end; ki++) val_ir.Add(ir[ki]);
				ir.SetCount(key_start);
				// Create empty dict and store in temp
				Emit(PY_BUILD_MAP, 0);
				EmitName(PY_STORE_NAME, "__dictcomp__");
				Next(); // consume 'for'
				// Support tuple unpacking: for a, b in iterable
				Vector<String> lvars;
				lvars.Add(Peek().str_value);
				this->Expect(TK_ID);
				while(IsToken(TK_COMMA)) {
					Next();
					if(IsId() && !IsId("in")) { lvars.Add(Peek().str_value); Next(); }
				}
				this->ExpectId("in");
				OrExpr();
				Emit(PY_GET_ITER);
				int loop_start = Label();
				int jump_end = Label();
				Emit(PY_FOR_ITER, 0);
				if(lvars.GetCount() == 1) {
					EmitName(PY_STORE_NAME, lvars[0]);
				} else {
					EmitName(PY_STORE_NAME, "__for_unpack__");
					for(int ti = 0; ti < lvars.GetCount(); ti++) {
						EmitName(PY_LOAD_NAME, "__for_unpack__");
						EmitConst(PyValue((int64)ti));
						Emit(PY_BINARY_SUBSCR);
						EmitName(PY_STORE_NAME, lvars[ti]);
					}
				}
				// Body: __dictcomp__[key] = val
				// STORE_SUBSCR pops: val(TOS), key(TOS1), dict(TOS2)
				auto emit_body = [&]() {
					EmitName(PY_LOAD_NAME, "__dictcomp__");
					for(auto& ins : key_ir) ir.Add(ins);
					for(auto& ins : val_ir) ir.Add(ins);
					Emit(PY_STORE_SUBSCR);
				};
				if(IsId("if")) {
					Next();
					Expression();
					int skip = Label();
					Emit(PY_POP_JUMP_IF_FALSE, 0);
					emit_body();
					Patch(skip, Label());
				} else {
					emit_body();
				}
				Emit(PY_JUMP_ABSOLUTE, loop_start);
				Patch(jump_end, Label());
				this->Expect(TK_BRACKET_END);
				// Leave dict on stack
				EmitName(PY_LOAD_NAME, "__dictcomp__");
			} else {
				n = 1;
				while(IsToken(TK_COMMA)) {
					Next();
					if(IsToken(TK_BRACKET_END)) break;
					Expression(); // key
					this->Expect(TK_COLON);
					Expression(); // value
					n++;
				}
				this->Expect(TK_BRACKET_END);
				Emit(PY_BUILD_MAP, n);
			}
		} else {
			this->Expect(TK_BRACKET_END);
			Emit(PY_BUILD_MAP, n);
		}
	}
	else {
		throw Exc(Format("Line %d: Expected atom, found %s", GetLine(), Peek().GetTypeString()));
	}
}

}
