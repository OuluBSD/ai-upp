#include <ByteVM/ByteVM.h>
#include <Core/Core.h>

using namespace Upp;

int main(int argc, char** argv) {
    try {
        String src = LoadFile("/common/active/sblo/Dev/ai-upp/test_lambda_minimal.py");
        if (src.IsEmpty()) {
            Cout() << "Failed to load test file\n";
            return 1;
        }
        
        Cout() << "Loaded " << src.GetCount() << " bytes\n";
        
        PyVM vm;
        if (vm.LoadModule("test", src, "/common/active/sblo/Dev/ai-upp/test_lambda_minimal.py")) {
            Cout() << "SUCCESS: Compilation passed!\n";
            vm.Run();
            Cout() << "SUCCESS: Execution passed!\n";
        } else {
            Cout() << "FAILED: Compilation error\n";
            return 1;
        }
    } catch (Exc& e) {
        Cout() << "ERROR: " << e << "\n";
        return 1;
    }
    return 0;
}
