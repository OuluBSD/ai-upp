#include <ByteVM/ByteVM.h>
#include <Core/Core.h>

using namespace Upp;

int main(int argc, char** argv) {
    try {
        String src = LoadFile("/common/active/sblo/Dev/ai-upp/test_lambda.py");
        if (src.IsEmpty()) {
            Cout() << "Failed to load test_lambda.py\n";
            return 1;
        }
        
        PyVM vm;
        if (vm.LoadModule("test", src, "/common/active/sblo/Dev/ai-upp/test_lambda.py")) {
            Cout() << "SUCCESS: Lambda test compiled!\n";
            vm.Run();
        } else {
            Cout() << "FAILED: Lambda test compilation error\n";
            return 1;
        }
    } catch (Exc& e) {
        Cout() << "ERROR: " << e << "\n";
        return 1;
    }
    return 0;
}
