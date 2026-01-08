#ifndef MAIN_H
#define MAIN_H

// This file has unnecessary includes
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

// Intentionally complex data structure to test refactoring
struct ComplexData {
    std::vector<std::string> data;
    std::map<int, std::string> lookup;
    std::function<void()> callback;
    
    // Unused member variable
    int unused_member;
    
    void process();
};

#endif