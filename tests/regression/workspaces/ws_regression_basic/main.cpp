#include <iostream>

// This function has unnecessary includes and unused variables
void function_with_issues() {
    int unused_var = 42;  // Unused variable
    std::cout << "Hello from the basic workspace!" << std::endl;
    
    // Simulate some complexity with nested loops
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            if (i * j > 50) {
                break;
            }
        }
    }
}

int main() {
    function_with_issues();
    return 0;
}