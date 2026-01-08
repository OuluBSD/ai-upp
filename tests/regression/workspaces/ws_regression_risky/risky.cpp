#include <iostream>
#include <string>
#include <memory>

// This represents potentially dangerous code that needs careful handling
class RiskyComponent {
public:
    RiskyComponent() {
        // Potentially unsafe operation
        risky_ptr = new int[1000];  // Raw pointer allocation
        for (int i = 0; i < 1000; ++i) {
            risky_ptr[i] = i;
        }
    }
    
    // Missing virtual destructor - potential issue
    ~RiskyComponent() {
        delete[] risky_ptr;  // Potential memory leak if exception occurs
    }
    
    // Unsafe function that could cause buffer overflow
    void unsafe_copy(const char* source) {
        if (source) {
            for (int i = 0; i < 2000; ++i) {  // Writing beyond array bounds
                risky_ptr[i] = source[i];  // Potential buffer overflow
            }
        }
    }
    
    // Function with potential infinite loop
    void infinite_loop() {
        while (true) {
            // This could cause problems when refactoring
            std::cout << "Looping..." << std::endl;
        }
    }
    
private:
    int* risky_ptr;
};

int main() {
    RiskyComponent rc;
    return 0;
}