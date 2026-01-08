#ifndef TANGLED_PKG_H
#define TANGLED_PKG_H

#include <Core/Core.h>
#include <CtrlCore/CtrlCore.h>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <complex>
#include <random>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>

using namespace Upp;

// Circular dependency simulation (conceptual)
class RiskyPkgClass;  // Forward declaration

class TangledClass {
public:
    TangledClass();
    ~TangledClass();
    
    void complexFunction();
    bool riskyOperation();
    void tangledDependency(RiskyPkgClass* obj); // Intentionally creates tangle
    
private:
    std::vector<std::map<std::string, int>> complexData;
    std::shared_ptr<void*> dangerousPtr;
    std::thread riskyThread;
    
    // Multiple mutexes that could create deadlocks
    std::mutex mutex1;
    std::mutex mutex2;
};

#endif