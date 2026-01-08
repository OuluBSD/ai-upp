#ifndef TESTPKG_H
#define TESTPKG_H

#include <Core/Core.h>
#include <CtrlCore/CtrlCore.h>
#include <iostream>
#include <vector>
#include <map>

using namespace Upp;

class TestClass {
public:
    TestClass();
    ~TestClass();
    
    int simpleFunction(int x);
    double anotherFunction(double a, double b);
    void unusedFunction(); // This function is intentionally unused
    
private:
    int value;
};

#endif