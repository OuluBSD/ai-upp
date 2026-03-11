#include "ScriptCommon.h"

namespace Upp {

Vector<Linter::Message> Linter::Analyze(const String& code, const String& filename)
{
	Vector<Message> res;
	try {
		Tokenizer tk;
		tk.SkipComments();
		tk.SkipPythonComments();
		if(!tk.Process(code, filename)) {
			// Tokenizer error? Tokenizer doesn't throw, it just returns false?
			// Let's check how Tokenizer works.
		}
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens(), filename);
		Vector<PyIR> ir;
		compiler.Compile(ir);
	}
	catch (Exc& e) {
		Message& m = res.Add();
		m.is_error = true;
		m.text = e;
		
		// Parse line/column from exception if possible
		// Exc often contains " (file:line:col)"
		int q = e.ReverseFind(':');
		if(q >= 0) {
			String col_s = e.Mid(q + 1);
			m.column = ScanInt(col_s);
			int p = e.Mid(0, q).ReverseFind(':');
			if(p >= 0) {
				String line_s = e.Mid(p + 1, q - p - 1);
				m.line = ScanInt(line_s);
			}
		}
	}
	return res;
}

}
