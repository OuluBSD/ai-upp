#include <iostream>

void crash_me() {
    int *p = nullptr;
    *p = 123;
}

int main() {
    std::cout << "Crashing soon..." << std::endl;
    crash_me();
    return 0;
}
