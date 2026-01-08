#include "RiskyPkg.h"
#include "../TangledPkg/TangledPkg.h" // Completing the circular dependency

RiskyPkgClass::RiskyPkgClass() {
    riskyData.resize(10000);  // Large allocation
    rawMemory = new char[1024 * 1024];  // 1MB allocation
}

RiskyPkgClass::~RiskyPkgClass() {
    delete[] rawMemory;
    rawMemory = nullptr;
}

void RiskyPkgClass::performExternalChange() {
    // External change - modifies global or external state
    for(auto& item : riskyData) {
        item = "CHANGED_BY_RISKY_OPERATION";
    }
}

bool RiskyPkgClass::destructiveOperation() {
    // Potentially destructive operation
    riskyData.clear();
    riskyData.shrink_to_fit();
    
    // Simulate an operation that might affect other parts of the system
    allocateLargeMemory();
    
    return true;
}

void RiskyPkgClass::modifySystemResources() {
    // Simulates modification of system resources
    // Would normally open files, connect to networks, etc.
    // For test purposes, just simulate potential for risk
    std::cout << "Potentially modifying system resources..." << std::endl;
}

void RiskyPkgClass::allocateLargeMemory() {
    // Allocate large amounts of memory to simulate resource-intensive operation
    const size_t size = 50 * 1024 * 1024; // 50 MB
    char* bigBuffer = new char[size];
    
    // Use the buffer to make sure it's allocated
    for(size_t i = 0; i < size; i += 1024) {
        bigBuffer[i] = static_cast<char>(i % 256);
    }
    
    delete[] bigBuffer;
}

void RiskyPkgClass::deleteFiles(const std::string& path) {
    // Function that simulates file deletion operations (not actually deleting for safety)
    std::cout << "Simulating file deletion in: " << path << std::endl;
}

void RiskyPkgClass::modifyGlobalState() {
    // Function that simulates modification of global state
    std::cout << "Simulating global state modification" << std::endl;
}