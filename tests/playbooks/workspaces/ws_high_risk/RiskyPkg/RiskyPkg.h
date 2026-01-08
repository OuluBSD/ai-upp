#ifndef RISKY_PKG_H
#define RISKY_PKG_H

#include <Core/Core.h>
#include <CtrlCore/CtrlCore.h>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace Upp;

// Forward declaration to create circular dependency concept
class TangledClass;

class RiskyPkgClass {
public:
    RiskyPkgClass();
    ~RiskyPkgClass();
    
    void performExternalChange();
    bool destructiveOperation();
    void modifySystemResources();
    
private:
    std::vector<std::string> riskyData;
    std::fstream fileHandle;
    char* rawMemory;
    
    // Potentially dangerous operations
    void allocateLargeMemory();
    void deleteFiles(const std::string& path);
    void modifyGlobalState();
};

#endif