#include <Core/Core.h>
#include <ByteVM/ByteVM.h>
#include <Geometry/Geometry.h>

using namespace Upp;

extern void RegisterGeometry(PyVM& vm);

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	PyVM vm;
	RegisterGeometry(vm);
	
	String code =
		"import geometry\n"
		"v = geometry.vec3(10, 20, 30)\n"
		"print('vec3:', v.__str__())\n"
		"print('length:', v.length())\n"
		"v.normalize()\n"
		"print('normalized:', v.__str__())\n"
		"print('new length:', v.length())\n"
		"\n"
		"m = geometry.mat4()\n"
		"print('identity mat4:')\n"
		"print(m.__str__())\n"
	;
	
try {
		Tokenizer tokenizer;
		tokenizer.SkipPythonComments();
		tokenizer.SkipNewLines(false);
		tokenizer.HaveIdents();
		if (tokenizer.Process(code, "test.py")) {
			tokenizer.CombineTokens();
			tokenizer.NewlineToEndStatement();
			PyCompiler compiler(tokenizer.GetTokens());
			Vector<PyIR> ir;
			compiler.Compile(ir);
			vm.SetIR(ir);
			vm.Run();
		}
	}
	catch (Exc& e) {
		Cout() << "Error: " << e << "\n";
	}
}
