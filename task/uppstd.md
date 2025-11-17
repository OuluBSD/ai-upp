# uppstd Thread

**Goal**: Create comprehensive mapping between U++ and STL C++ features for code conversion

## Status: IN_PROGRESS

---

## Overview

This thread is about mapping U++ and STL c++ features in a way that we know what to replace with what. It's more like documentation for AI than actual code, though we can implement code if that's the best solution.

This will be used for code conversion between U++ and STL c++. It must show all declarations in U++ and in STL, and indicate whether we found the match or not yet.

---

## Goals

1. **Complete Declaration Mapping**
   - Document all U++ declarations
   - Find equivalent STL declarations
   - Mark which mappings are complete vs incomplete

2. **Integration with stdsrc**
   - This information is implemented in stdsrc code
   - We can learn from stdsrc for mapping
   - May need to extract info from stdsrc implementation

3. **Bi-directional Conversion**
   - U++ → STL conversion rules
   - STL → U++ conversion rules
   - Handle cases where no direct equivalent exists

---

## Structure

### Suggested Format

```markdown
## U++ Class/Function: ClassName

### U++ Declaration
```cpp
class Vector {
    void Add(const T& x);
    int GetCount() const;
    // ...
};
```

### STL Equivalent
```cpp
class std::vector {
    void push_back(const T& x);
    size_t size() const;
    // ...
};
```

### Mapping Table
| U++ | STL | Status | Notes |
|-----|-----|--------|-------|
| Vector | std::vector | ✓ Complete | |
| Vector::Add() | std::vector::push_back() | ✓ Complete | |
| Vector::GetCount() | std::vector::size() | ✓ Complete | |
| Vector::At() | std::vector::at() | ✓ Complete | Bounds checking |
| Vector::operator[] | std::vector::operator[] | ✓ Complete | No bounds check |

### Conversion Notes
- U++ Vector uses GetCount(), STL uses size()
- U++ uses Add(), STL uses push_back()
- Both support operator[] for direct access
```

---

## Categories to Map

### Core Containers
- Vector ↔ std::vector
- Array ↔ std::array
- Index ↔ std::unordered_set or custom
- Map ↔ std::map or std::unordered_map
- BiVector ↔ std::deque
- InVector ↔ ? (investigate)
- InMap ↔ ? (investigate)

### Strings
- String ↔ std::string
- WString ↔ std::wstring
- StringBuffer ↔ ?
- StringStream ↔ std::stringstream

### Smart Pointers
- One ↔ std::unique_ptr
- Pick ↔ ? (investigate move semantics)
- Ptr ↔ std::shared_ptr or raw pointer?

### Utilities
- Tuple ↔ std::tuple
- Optional ↔ std::optional (C++17)
- Value ↔ std::variant or custom
- Function ↔ std::function
- Callback ↔ std::function

### Algorithms
- Sort ↔ std::sort
- Find ↔ std::find
- GetIndex ↔ std::distance + std::find
- Filter ↔ std::copy_if
- Map functions ↔ std::transform

### I/O
- Stream ↔ std::iostream hierarchy
- FileIn/FileOut ↔ std::ifstream/std::ofstream
- StringStream ↔ std::stringstream

### Threading
- Thread ↔ std::thread
- Mutex ↔ std::mutex
- CoWork ↔ ? (thread pool - no direct STL equivalent)

### Time/Date
- Time ↔ std::chrono
- Date ↔ std::chrono or custom

---

## Work Items

### Phase 1: Core Package Analysis
- [x] Visit all Core package files to document U++ declarations
- [x] Compare Core package declarations with stdsrc implementations
- [x] Create Core container mappings (Vector, Array, Index, etc.)
- [x] Create Core string mappings (String, WString, etc.)
- [x] Create Core smart pointer mappings (One, Ptr, etc.)
- [x] Create Core utility mappings (Value, Function, Tuple, etc.)
- [x] Create Core algorithm mappings (Sort, Find, etc.)
- [x] Create Core I/O mappings (Stream, etc.)
- [x] Create Core threading mappings (Thread, Mutex, etc.)
- [x] Create Core time/date mappings (Time, Date, etc.)

### Phase 2: Draw Package Analysis
- [x] Visit all Draw package files to document U++ declarations
- [x] Compare Draw package declarations with stdsrc implementations
- [x] Create Draw type mappings (Image, RGBA, etc.)
- [x] Create Draw operation mappings (DrawRect, DrawLine, etc.)
- [x] Create Draw painting mappings (Painting, etc.)

### Phase 3: CtrlCore Package Analysis
- [x] Visit all CtrlCore package files to document U++ declarations
- [x] Compare CtrlCore package declarations with stdsrc implementations
- [x] Create CtrlCore base class mappings (Ctrl, Draw, etc.)
- [x] Create CtrlCore window mappings (TopWindow, etc.)
- [x] Create CtrlCore event handling mappings

### Phase 4: CtrlLib Package Analysis
- [ ] Visit all CtrlLib package files to document U++ declarations
- [ ] Compare CtrlLib package declarations with stdsrc implementations
- [ ] Create CtrlLib control mappings (Button, EditField, etc.)
- [ ] Create CtrlLib layout mappings
- [ ] Create CtrlLib menu/toolbar mappings

### Phase 5: Integration and Documentation
- [ ] Create comprehensive U++ → STL mapping document
- [ ] Analyze stdsrc implementation to extract existing mappings
- [ ] Document conversion patterns (not just class names, but usage patterns)
- [ ] Create reverse mapping (STL → U++)
- [ ] Identify cases with no direct equivalent
- [ ] Document workarounds for non-equivalent features
- [ ] Create examples of common conversion scenarios
- [ ] Build automated tools to assist with conversion (optional)

---

## Dependencies
- Requires: Understanding of both U++ and STL APIs
- Blocks: Automated code conversion tools
- Related: stdsrc thread (implementation of U++ API using STL), Code Translation tasks