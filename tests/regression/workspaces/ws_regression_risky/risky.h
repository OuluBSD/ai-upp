#ifndef RISKY_H
#define RISKY_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>

// A more complex and potentially risky class definition
class ComplexRiskyClass {
private:
    char* buffer;           // Raw pointer - potential issue
    size_t buffer_size;
    std::mutex access_mutex;
    
public:
    ComplexRiskyClass(size_t size) : buffer_size(size) {
        buffer = new char[size];  // Potential resource management issue
    }
    
    // Unsafe copy constructor that can cause double deletion
    ComplexRiskyClass(const ComplexRiskyClass& other) {
        buffer_size = other.buffer_size;
        buffer = new char[buffer_size];
        memcpy(buffer, other.buffer, buffer_size);  // Potential security issue
    }
    
    // Unsafe assignment operator
    ComplexRiskyClass& operator=(const ComplexRiskyClass& other) {
        if (this != &other) {
            delete[] buffer;  // Memory leak possible if new fails
            buffer_size = other.buffer_size;
            buffer = new char[buffer_size];
            memcpy(buffer, other.buffer, buffer_size);  // Potential security issue
        }
        return *this;
    }
    
    ~ComplexRiskyClass() {
        delete[] buffer;  // Potential issue with double deletion if copy is used
    }
    
    // Function that might throw exceptions
    void risky_operation() {
        if (std::rand() % 2) {
            throw std::runtime_error("Intentionally risky operation failed");
        }
    }
    
    // Function that could be modified in dangerous ways
    void dangerous_modify() {
        // This code could be problematic when modified by AI agents
        for (size_t i = 0; i <= buffer_size; ++i) {  // Off-by-one error
            buffer[i] = 'A' + (i % 26);
        }
    }
};

#endif