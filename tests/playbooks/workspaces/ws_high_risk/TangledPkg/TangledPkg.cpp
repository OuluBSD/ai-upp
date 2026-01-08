#include "TangledPkg.h"
#include "../RiskyPkg/RiskyPkg.h" // Creating intentional dependency

TangledClass::TangledClass() {
    // Initialize complex data structures
    complexData.resize(10);
    
    // Potentially dangerous initialization
    dangerousPtr = std::make_shared<void*>(nullptr);
}

TangledClass::~TangledClass() {
    // Potentially risky cleanup code
    if(dangerousPtr && *dangerousPtr) {
        delete static_cast<int*>(*dangerousPtr);
        *dangerousPtr = nullptr;
    }
}

void TangledClass::complexFunction() {
    // Very complex function that changes many things
    for(int i = 0; i < 100; i++) {
        mutex1.lock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        mutex2.lock();
        
        // Risky operations
        auto it = complexData.begin();
        if(it != complexData.end()) {
            (*it)["key"] = i * 2;
        }
        
        mutex2.unlock();
        mutex1.unlock();
    }
}

bool TangledClass::riskyOperation() {
    // Operation that could have side effects elsewhere
    riskyThread = std::thread([this] {
        for(int i = 0; i < 1000; i++) {
            // Intensive operation
            volatile int sum = 0;
            for(int j = 0; j < 100; j++) {
                sum += j;
            }
            
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });
    
    riskyThread.join();
    
    return true;
}

void TangledClass::tangledDependency(RiskyPkgClass* obj) {
    // Creates tangle - modifies another package's object
    if(obj) {
        // Risky modification that affects external state
        obj->performExternalChange();
    }
}