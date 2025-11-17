# U++ to STL Mapping Documentation

This directory contains comprehensive mapping documentation between U++ and STL C++ features for code conversion purposes. It documents what U++ declarations map to their STL equivalents to enable automated or manual code conversion between the two systems.

## Directory Structure

- `Core/` - Mappings for U++ Core package (containers, strings, smart pointers, utilities, algorithms, I/O, threading, time/date)
- `Draw/` - Mappings for U++ Draw package (graphics, drawing operations)
- `CtrlCore/` - Mappings for U++ CtrlCore package (GUI base classes, window management, events)
- `CtrlLib/` - Mappings for U++ CtrlLib package (GUI controls, layouts, menus)
- `Common/` - Common utilities and base classes used across packages

## Core Package Mappings

### Container Mappings
- **Vector ↔ std::vector** - See [container_mapping.md](Core/container_mapping.md)
- **Array ↔ std::vector of unique_ptr** - See [container_mapping.md](Core/container_mapping.md)
- **Index ↔ std::unordered_set or std::set** - See [container_mapping.md](Core/container_mapping.md)
- **Map ↔ std::map or std::unordered_map** - See [container_mapping.md](Core/container_mapping.md)
- **BiVector ↔ std::deque** - See [container_mapping.md](Core/container_mapping.md)
- **InVector ↔ std::vector (indirect)** - See [container_mapping.md](Core/container_mapping.md)

### String Mappings
- **String ↔ std::string** - See [string_mapping.md](Core/string_mapping.md)
- **WString ↔ std::wstring** - See [string_mapping.md](Core/string_mapping.md)
- **StringBuffer ↔ std::string or std::ostringstream** - See [string_mapping.md](Core/string_mapping.md)
- **StringStream ↔ std::stringstream** - See [string_mapping.md](Core/string_mapping.md)

### Smart Pointer Mappings
- **One ↔ std::unique_ptr** - See [smart_pointer_mapping.md](Core/smart_pointer_mapping.md)
- **Ptr ↔ std::shared_ptr** - See [smart_pointer_mapping.md](Core/smart_pointer_mapping.md)
- **pick() ↔ std::move()** - See [smart_pointer_mapping.md](Core/smart_pointer_mapping.md)

### Utility Mappings
- **Tuple ↔ std::tuple** - See [utility_mapping.md](Core/utility_mapping.md)
- **Optional ↔ std::optional** - See [utility_mapping.md](Core/utility_mapping.md)
- **Value ↔ std::any or variant-based** - See [utility_mapping.md](Core/utility_mapping.md)
- **Function ↔ std::function** - See [utility_mapping.md](Core/utility_mapping.md)
- **Callback ↔ std::function<void(...)>** - See [utility_mapping.md](Core/utility_mapping.md)

### Algorithm Mappings
- **Sort ↔ std::sort and related** - See [algorithm_mapping.md](Core/algorithm_mapping.md)
- **Find ↔ std::find and related** - See [algorithm_mapping.md](Core/algorithm_mapping.md)
- **Other algorithms ↔ STL equivalents** - See [algorithm_mapping.md](Core/algorithm_mapping.md)

### I/O System Mappings
- **Stream ↔ std::iostream hierarchy** - See [io_mapping.md](Core/io_mapping.md)
- **StringStream ↔ std::stringstream** - See [io_mapping.md](Core/io_mapping.md)
- **FileStream ↔ std::fstream** - See [io_mapping.md](Core/io_mapping.md)
- **FileIn/FileOut ↔ std::ifstream/std::ofstream** - See [io_mapping.md](Core/io_mapping.md)
- **Serialization ↔ Manual stream operations** - See [io_mapping.md](Core/io_mapping.md)

### Threading Mappings
- **Thread ↔ std::thread** - See [threading_mapping.md](Core/threading_mapping.md)
- **Mutex ↔ std::mutex** - See [threading_mapping.md](Core/threading_mapping.md)
- **Semaphore ↔ std::counting_semaphore** - See [threading_mapping.md](Core/threading_mapping.md)
- **ConditionVariable ↔ std::condition_variable** - See [threading_mapping.md](Core/threading_mapping.md)
- **RWMutex ↔ std::shared_mutex** - See [threading_mapping.md](Core/threading_mapping.md)
- **Atomic ↔ std::atomic** - See [threading_mapping.md](Core/threading_mapping.md)

### Time/Date Mappings
- **Date ↔ std::chrono::year_month_day (C++20)** - See [time_date_mapping.md](Core/time_date_mapping.md)
- **Time ↔ std::chrono::time_point** - See [time_date_mapping.md](Core/time_date_mapping.md)
- **Time/Date utilities ↔ std::chrono utilities** - See [time_date_mapping.md](Core/time_date_mapping.md)

## Mapping Format

Each mapping document follows this structure:

### U++ Class/Function: ClassName

#### U++ Declaration
```cpp
class Vector {
    void Add(const T& x);
    int GetCount() const;
    // ...
};
```

#### STL Equivalent
```cpp
class std::vector {
    void push_back(const T& x);
    size_t size() const;
    // ...
};
```

#### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Vector | std::vector | ✓ Complete | |
| Vector::Add() | std::vector::push_back() | ✓ Complete | |
| Vector::GetCount() | std::vector::size() | ✓ Complete | |

#### Conversion Notes
- U++ Vector uses GetCount(), STL uses size()
- U++ uses Add(), STL uses push_back()
- Both support operator[] for direct access

## Goals

1. **Complete Declaration Mapping** - Document all U++ declarations with STL equivalents
2. **Integration with stdsrc** - Reference existing stdsrc implementations
3. **Bi-directional Conversion** - Provide both U++ → STL and STL → U++ mappings