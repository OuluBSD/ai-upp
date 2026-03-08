# Task: Implement StatusBar

## Goal
Create status bar showing: Line, Col, Format, Line Ending, Edit Mode, Memory Usage.

## Implementation in PythonIDE

```cpp
// In PythonIDE.h
class PythonIDE : public TopWindow {
    // ...
    void UpdateStatusBar();

    struct StatusInfo {
        int line = 1;
        int column = 1;
        String format = "UTF-8";
        String line_ending = "LF";
        String edit_mode = "RW";
        int memory_percent = 0;
    } status_info;

    Timer status_timer;
};

// In PythonIDE.cpp
void PythonIDE::UpdateStatusBar()
{
    // Update from current editor
    if(CodeEditor* ed = GetCurrentEditor()) {
        Point pos = ed->GetCursor();
        status_info.line = pos.y + 1;
        status_info.column = pos.x + 1;
        status_info.edit_mode = ed->IsReadOnly() ? "RO" : "RW";
    }

    // Calculate memory usage %
    size_t mem_used = MemoryUsedKb();
    size_t mem_total = MemoryTotalKb();  // Platform-specific
    if(mem_total > 0)
        status_info.memory_percent = (int)((mem_used * 100) / mem_total);

    // Build status text
    String status = Format(
        "Line: %d  Col: %d     %s     %s     %s     Mem: %d%%",
        status_info.line,
        status_info.column,
        status_info.format,
        status_info.line_ending,
        status_info.edit_mode,
        status_info.memory_percent
    );

    statusbar.Set(status);
}

PythonIDE::PythonIDE()
{
    // ... existing code ...

    // Update status bar every 500ms
    status_timer.Set(-500, [=] { UpdateStatusBar(); });

    UpdateStatusBar();
}
```

## Memory Usage Calculation

```cpp
// Add to PythonIDE class
static size_t MemoryUsedKb()
{
#ifdef PLATFORM_WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        return pmc.WorkingSetSize / 1024;
#elif defined(PLATFORM_POSIX)
    // Read /proc/self/status
    String status = LoadFile("/proc/self/status");
    Vector<String> lines = Split(status, '\n');
    for(const String& line : lines) {
        if(line.StartsWith("VmRSS:")) {
            int kb = ScanInt(line.Mid(6));
            return kb;
        }
    }
#endif
    return 0;
}

static size_t MemoryTotalKb()
{
#ifdef PLATFORM_WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if(GlobalMemoryStatusEx(&statex))
        return statex.ullTotalPhys / 1024;
#elif defined(PLATFORM_POSIX)
    // Read /proc/meminfo
    String meminfo = LoadFile("/proc/meminfo");
    Vector<String> lines = Split(meminfo, '\n');
    for(const String& line : lines) {
        if(line.StartsWith("MemTotal:")) {
            return ScanInt(line.Mid(9));
        }
    }
#endif
    return 0;
}
```

## Files Modified
- `uppsrc/ScriptIDE/PythonIDE.h`
- `uppsrc/ScriptIDE/PythonIDE.cpp`

## Testing
1. Run ScriptIDE
2. Verify status bar shows:
   - Line: 1  Col: 1
   - UTF-8 (or current encoding)
   - LF (or CRLF)
   - RW (or RO)
   - Mem: X%
3. Move cursor, verify line/col update
4. Make file read-only, verify RW→RO

## Success Criteria
- Status bar updates in real-time
- Line/Col accurate
- Memory % shows current usage
- All fields visible and formatted correctly
