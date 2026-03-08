# C++ AMP Compatibility Layer (AMP)

This package provides a compatibility and abstraction layer for C++ AMP (Accelerated Massive Parallelism), allowing for GPGPU-style data-parallel programming with a seamless CPU fallback.

## Features

- **GPGPU Acceleration**: Utilizes C++ AMP on Windows with compatible hardware when compiled with Visual Studio and the `flagAMP` flag.
- **CPU Fallback**: Provides a complete functional fallback using standard CPU multi-threading (via U++ `Thread` and `CPU_Cores()`) for non-Windows platforms (Linux) or environments without AMP support.
- **Unified API**: Offers a consistent `concurrency` namespace, `array_view`, `index`, `extent`, and `parallel_for_each` regardless of the backend.
- **Performance Utilities**: Includes parallel-safe math functions (`AmpTanh`, `AmpSqrt`, `AmpFabs`) and atomic operations.
- **Device Discovery**: Provides tools to enumerate and query compatible AMP accelerator devices (`GetAmpDevices`, `GetAmpDeviceMemory`).

## Usage

Include `AMP.h` and use the `PARALLEL` macro for functions intended to run on the accelerator.

```cpp
#include <AMP/AMP.h>

void MyParallelTask(Vector<float>& data) {
    array_view<float, 1> view(data.GetCount(), data.Begin());
    
    parallel_for_each(view.extent, [=](index<1> idx) PARALLEL {
        view[idx] = AmpTanh(view[idx]);
    });
    
    view.synchronize();
}
```

## Mission Alignment

This package is part of the "Public Technical / Generic" mission area. It is intended to be moved to `../ai-upp/uppsrc/` as a generic utility for the U++ ecosystem.
