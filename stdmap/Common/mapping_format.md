# U++ to STL Mapping Documentation Format

## Overview

This document defines the standard format for documenting mappings between U++ types/functions and their STL equivalents. The purpose is to provide clear, consistent documentation that can guide both manual and automated conversion of U++ code to standard C++ with STL usage.

## Document Structure

Each mapping document should follow this structure:

### 1. Header Information
```
# U++ <Package> to STL Mapping

**U++ Package**: <Package Name>  
**STL Equivalent**: <Standard Library Components>  
**Status**: Draft | In Progress | Complete  
**Last Updated**: YYYY-MM-DD  
**Authors**: [List of contributors]
```

### 2. Class/Type Mapping Table

| U++ Type | STL Equivalent | Status | Notes |
|----------|----------------|--------|-------|
| Vector<T> | std::vector<T> | Complete | Direct replacement possible |
| Array<T> | std::vector<std::unique_ptr<T>> | Complete | Different ownership model |
| Index<T> | std::unordered_set<T> | In Progress | Requires custom hash |
| String | std::string | Complete | Nearly direct replacement |

### 3. Method Mapping Table

| U++ Method | STL Equivalent | Status | Notes |
|------------|----------------|--------|-------|
| Vector::Add() | std::vector::push_back() | Complete | Same semantics |
| Vector::GetCount() | std::vector::size() | Complete | Same return type |
| String::GetLength() | std::string::size() | Complete | Different method name |

### 4. Detailed Mapping Information

For each major type, provide detailed information:

#### U++ Class: Vector

**U++ Declaration:**
```cpp
template <class T>
class Vector {
    void Add(const T& x);
    int GetCount() const;
    T& operator[](int i);
    // ... other methods
};
```

**STL Equivalent:**
```cpp
template <class T>
class std::vector {
    void push_back(const T& x);
    size_t size() const;
    T& operator[](size_t i);
    // ... other methods
};
```

**Mapping Details:**
- `Vector::Add(x)` → `std::vector::push_back(x)`
- `Vector::GetCount()` → `std::vector::size()`
- `Vector::operator[](i)` → `std::vector::operator[](static_cast<size_t>(i))`
- Index type difference: `int` vs `size_t`

**Usage Conversion Examples:**
```cpp
// U++
Vector<int> v;
v.Add(42);
int count = v.GetCount();

// STL
std::vector<int> v;
v.push_back(42);
size_t count = v.size();
```

**Compatibility Notes:**
- Index type conversion may be needed
- Error handling differs (U++ with ASSERT, STL with exceptions)
- Memory allocation strategy may differ

### 5. Conversion Patterns

Document common conversion patterns:

#### Pattern: Container Initialization
- U++: `Vector<int> v; v.Add(1); v.Add(2);`
- STL: `std::vector<int> v = {1, 2};` or `std::vector<int> v{1, 2};`

#### Pattern: Iteration
- U++: `for(int i = 0; i < v.GetCount(); i++) { /* use v[i] */ }`
- STL: `for(const auto& item : v) { /* use item */ }`

### 6. Known Issues and Workarounds

List any known compatibility issues and suggested workarounds.

## Documentation Standards

### Status Values
- **Not Started**: Item identified but not yet documented
- **In Progress**: Documentation is being worked on
- **Complete**: Mapping is fully documented
- **Requires Custom**: No direct STL equivalent, custom solution needed
- **Not Applicable**: Feature not needed in STL context

### Naming Conventions
- File names: `{package}_{type}_mapping.md` (e.g., `core_vector_mapping.md`)
- Directory structure: `/stdmap/{Package}/{Type}_mapping.md`
- Table of contents in main documentation file

### Content Guidelines
1. Be precise about type differences
2. Include code examples for complex mappings
3. Note any behavioral differences
4. Highlight breaking changes that require attention
5. Provide migration strategies for complex cases
```

## File Organization

```
stdmap/
├── Core/
│   ├── Vector_mapping.md
│   ├── String_mapping.md
│   ├── Array_mapping.md
│   └── ...
├── Draw/
│   ├── Image_mapping.md
│   ├── Draw_mapping.md
│   └── ...
├── CtrlCore/
│   ├── Ctrl_mapping.md
│   ├── TopWindow_mapping.md
│   └── ...
├── CtrlLib/
│   ├── Button_mapping.md
│   ├── EditField_mapping.md
│   └── ...
├── Common/
│   ├── mapping_format.md (this file)
│   ├── api_extractor.cpp
│   └── ...
└── mapping_summary.md
```

## Example Mapping Document

Here's a complete example for String:

```markdown
# U++ Core String to STL Mapping

**U++ Package**: Core  
**STL Equivalent**: std::string  
**Status**: Complete  
**Last Updated**: 2024-01-16  
**Authors**: AI Assistant

## Mapping Table

| U++ Method | STL Equivalent | Status | Notes |
|------------|----------------|--------|-------|
| String::GetLength() | std::string::size() | ✓ Complete | Same behavior |
| String::operator[] | std::string::operator[] | ✓ Complete | Different return type |
| String::Left(int) | std::string::substr(0, n) | ✓ Complete | Different method name |
| String::Right(int) | std::string::substr(size()-n) | ✓ Complete | Different method name |

## Detailed Mapping

### U++ String Class

**Declaration:**
```cpp
class String : public std::string {
    int GetLength() const;
    int operator[](int i) const;
    String Left(int count) const;
    String Right(int count) const;
    // ... other methods
};
```

**STL Equivalent:**
```cpp
class std::string {
    size_t size() const;
    const char& operator[](size_t i) const;
    std::string substr(size_t pos, size_t len) const;
    // ... other methods
};
```

## Usage Conversion Examples

### Length/Size Access
```cpp
// U++
String s = "Hello";
int len = s.GetLength();

// STL
std::string s = "Hello";
size_t len = s.size();
```

### String Slicing
```cpp
// U++
String s = "Hello";
String left = s.Left(2);   // "He"

// STL
std::string s = "Hello";
std::string left = s.substr(0, 2);  // "He"
```

### Index Access
```cpp
// U++
String s = "Hello";
int char_code = s[0];      // Returns int (unsigned char value)

// STL
std::string s = "Hello";
char ch = s[0];            // Returns char
```

## Compatibility Notes

1. **Return Types**: U++ String::operator[] returns int (character code), STL returns char
2. **Method Names**: U++ has convenience methods like Left(), Right() that have no direct STL equivalent
3. **Null Handling**: U++ String can represent null state differently than STL
4. **Encoding**: Both support UTF-8 but U++ has more built-in encoding utilities
```

This format provides a standardized way to document mappings between U++ and STL functionality, making it easier to understand and implement conversions.