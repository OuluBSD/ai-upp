#include "TestPkg.h"

TestClass::TestClass() {
    value = 0;
}

TestClass::~TestClass() {
    // Cleanup code
}

int TestClass::simpleFunction(int x) {
    // Simple function with trivial complexity
    if(x > 0) {
        return x * 2;
    } else {
        return x / 2; // This could be simplified
    }
}

double TestClass::anotherFunction(double a, double b) {
    // More complex function
    std::vector<int> temp;
    for(int i = 0; i < 10; i++) {
        temp.push_back(i);
    }
    
    double result = a * b;
    for(auto& element : temp) { // Unused variable
        result += element;
    }
    
    return result;
}

void TestClass::unusedFunction() {
    // This function is intentionally unused to create "messy" code
    int localVar = 42;
    std::cout << "This function is not used: " << localVar << std::endl;
}