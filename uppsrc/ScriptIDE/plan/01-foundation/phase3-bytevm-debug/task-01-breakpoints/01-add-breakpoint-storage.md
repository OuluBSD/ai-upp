# Task: Add Breakpoint Storage to PyVM

## Goal
Add data structures and methods to PyVM for managing breakpoints.

## Changes to `uppsrc/ByteVM/PyVM.h`

### Add to PyVM class:
```cpp
class PyVM {
    // ... existing members ...

public:
    // Breakpoint management
    struct Breakpoint : Moveable<Breakpoint> {
        String file;
        int line;
        bool enabled;
        int hit_count;

        Breakpoint() : line(0), enabled(true), hit_count(0) {}
        Breakpoint(const String& f, int l) : file(f), line(l), enabled(true), hit_count(0) {}
    };

    // Add breakpoint
    void AddBreakpoint(const String& file, int line);
    void RemoveBreakpoint(const String& file, int line);
    void ClearBreakpoints();
    void EnableBreakpoint(const String& file, int line, bool enable = true);

    // Query breakpoints
    bool HasBreakpoint(const String& file, int line) const;
    const Vector<Breakpoint>& GetBreakpoints() const { return breakpoints; }

    // Breakpoint hit callback
    Event<const String&, int> WhenBreakpointHit;

private:
    Vector<Breakpoint> breakpoints;

    bool CheckBreakpoint(const String& file, int line);
};
```

## Implementation in `uppsrc/ByteVM/PyVM.cpp`

```cpp
void PyVM::AddBreakpoint(const String& file, int line)
{
    for(auto& bp : breakpoints)
        if(bp.file == file && bp.line == line)
            return; // Already exists

    breakpoints.Add(Breakpoint(file, line));
}

void PyVM::RemoveBreakpoint(const String& file, int line)
{
    for(int i = 0; i < breakpoints.GetCount(); i++)
        if(breakpoints[i].file == file && breakpoints[i].line == line) {
            breakpoints.Remove(i);
            return;
        }
}

void PyVM::ClearBreakpoints()
{
    breakpoints.Clear();
}

void PyVM::EnableBreakpoint(const String& file, int line, bool enable)
{
    for(auto& bp : breakpoints)
        if(bp.file == file && bp.line == line) {
            bp.enabled = enable;
            return;
        }
}

bool PyVM::HasBreakpoint(const String& file, int line) const
{
    for(const auto& bp : breakpoints)
        if(bp.file == file && bp.line == line && bp.enabled)
            return true;
    return false;
}

bool PyVM::CheckBreakpoint(const String& file, int line)
{
    for(auto& bp : breakpoints)
        if(bp.file == file && bp.line == line && bp.enabled) {
            bp.hit_count++;
            WhenBreakpointHit(file, line);
            return true;
        }
    return false;
}
```

## Files Modified
- `uppsrc/ByteVM/PyVM.h`
- `uppsrc/ByteVM/PyVM.cpp`

## Testing
Create simple test:
```cpp
PyVM vm;
vm.AddBreakpoint("test.py", 10);
ASSERT(vm.HasBreakpoint("test.py", 10));
vm.RemoveBreakpoint("test.py", 10);
ASSERT(!vm.HasBreakpoint("test.py", 10));
```

## Success Criteria
- Breakpoints can be added/removed
- HasBreakpoint() works correctly
- Breakpoints can be enabled/disabled
