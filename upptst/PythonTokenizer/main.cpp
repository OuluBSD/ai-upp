#include <Core/Core.h>
#include <Core/TextParsing/TextParsing.h>

using namespace Upp;

void TestNumericLiterals()
{
	Tokenizer t;
	t.Process("1_000_000 0b1010 0o777 0xdead_beef 1.5j", "test");
	const Vector<Token>& tokens = t.GetTokens();
	
	ASSERT(tokens[0].type == TK_INTEGER && tokens[0].str_value == "1000000");
	ASSERT(tokens[1].type == TK_BIN && tokens[1].str_value == "0b1010");
	ASSERT(tokens[2].type == TK_OCT && tokens[2].str_value == "0o777");
	ASSERT(tokens[3].type == TK_HEX && tokens[3].str_value == "0xdeadbeef");
	ASSERT(tokens[4].type == TK_IMAGINARY && tokens[4].str_value == "1.5");
}

void TestStringLiterals()
{
	Tokenizer t;
	// Use a simpler string to avoid escaping issues in this environment
	String code;
	code << "r'raw string' ";
	code << "b'bytes' ";
	code << "f\"format\" ";
	code << "'''multi\nline''' ";
	code << "\"\"\"double\nmulti\"\"\"";
	
	t.Process(code, "test");
	const Vector<Token>& tokens = t.GetTokens();
	
	ASSERT(tokens[0].type == TK_STRING && tokens[0].str_value == "raw string");
	ASSERT(tokens[1].type == TK_STRING && tokens[1].str_value == "bytes");
	ASSERT(tokens[2].type == TK_STRING && tokens[2].str_value == "format");
	ASSERT(tokens[3].type == TK_STRING_MULTILINE && tokens[3].str_value == "multi\nline");
	ASSERT(tokens[4].type == TK_STRING_MULTILINE && tokens[4].str_value == "double\nmulti");
}

void TestOperators()
{
	Tokenizer t;
	t.SkipPythonComments(true);
	t.Process("// ** := -> ... @=", "test");
	t.CombineTokens();
	const Vector<Token>& tokens = t.GetTokens();
	
	ASSERT(tokens[0].type == TK_FLOORDIV);
	ASSERT(tokens[1].type == TK_EXP);
	ASSERT(tokens[2].type == TK_WALRUS);
	ASSERT(tokens[3].type == TK_RETURN_HINT);
	ASSERT(tokens[4].type == TK_ELLIPSIS);
	ASSERT(tokens[5].type == TK_ATASS);
}

void TestIndentation()
{
	Tokenizer t;
	String code = 
		"def foo():\n"
		"    if True:\n"
		"        pass\n"
		"    return 42\n";
	
	t.HaveIdents(true);
	t.Process(code, "test");
	const Vector<Token>& tokens = t.GetTokens();
	
	int i = 0;
	ASSERT(tokens[i++].type == TK_ID); // def
	ASSERT(tokens[i++].type == TK_ID); // foo
	ASSERT(tokens[i++].type == TK_PARENTHESIS_BEGIN);
	ASSERT(tokens[i++].type == TK_PARENTHESIS_END);
	ASSERT(tokens[i++].type == TK_COLON);
	ASSERT(tokens[i++].type == TK_NEWLINE);
	ASSERT(tokens[i++].type == TK_INDENT);
	ASSERT(tokens[i++].type == TK_ID); // if
	ASSERT(tokens[i++].type == TK_ID); // True
	ASSERT(tokens[i++].type == TK_COLON);
	ASSERT(tokens[i++].type == TK_NEWLINE);
	ASSERT(tokens[i++].type == TK_INDENT);
	ASSERT(tokens[i++].type == TK_ID); // pass
	ASSERT(tokens[i++].type == TK_NEWLINE);
	ASSERT(tokens[i++].type == TK_DEDENT);
	ASSERT(tokens[i++].type == TK_ID); // return
	ASSERT(tokens[i++].type == TK_INTEGER); // 42
	ASSERT(tokens[i++].type == TK_NEWLINE);
	ASSERT(tokens[i++].type == TK_DEDENT);
	ASSERT(tokens[i++].type == TK_EOF);
}

void TestMultilineStatements()
{
	Tokenizer t;
	String code = 
		"x = (\n"
		"    1 +\n"
		"    2\n"
		")\n";
	
	t.HaveIdents(true);
	t.Process(code, "test");
	t.NewlineToEndStatement();
	const Vector<Token>& tokens = t.GetTokens();
	
	int i = 0;
	ASSERT(tokens[i++].type == TK_ID); // x
	ASSERT(tokens[i++].type == TK_ASS);
	ASSERT(tokens[i++].type == TK_PARENTHESIS_BEGIN);
	ASSERT(tokens[i++].type == TK_INTEGER); // 1
	ASSERT(tokens[i++].type == TK_PLUS);
	ASSERT(tokens[i++].type == TK_INTEGER); // 2
	ASSERT(tokens[i++].type == TK_PARENTHESIS_END);
	ASSERT(tokens[i++].type == TK_END_STMT);
	ASSERT(tokens[i++].type == TK_EOF);
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);
	
	TestNumericLiterals();
	TestStringLiterals();
	TestOperators();
	TestIndentation();
	TestMultilineStatements();
	
	LOG("=========== ALL TESTS OK");
}
