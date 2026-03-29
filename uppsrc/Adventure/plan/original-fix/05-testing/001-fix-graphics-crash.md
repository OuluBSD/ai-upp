# Phase 05: Testing and Debugging

## Status: IN PROGRESS

## Task 001: Fix Graphics Crash ✓

### Issue

Adventure crashed on startup with GDK/Cairo assertions and SIGTRAP (exit code 133).

### Root Cause

Window attempted to paint before initialization completed:
- `GetRect()` called in constructor (before window created)
- Paint events fired during init
- GDK/Cairo assertions from invalid state

### Fix Applied

**Files Modified**:
1. `App.cpp`:
   - `GetRect()` → `SetRect(0, 0, 512, 512)` in constructor
   - Added `Hide()` to prevent painting during init
   - Added `MarkInitialized()` method
   - Guard in `ProcessScript()` for init state

2. `Program.h`:
   - Added `MarkInitialized()` declaration

3. `ProgramDraw.cpp`:
   - Null check for program pointer in `Paint()`

4. `Adventure.cpp`:
   - Call `MarkInitialized()` after `Init()`
   - Explicit `Show()` before `Run()`

### Test Results

| Metric | Before | After |
|--------|--------|-------|
| Exit code | 133 (SIGTRAP) | 124 (timeout success) |
| GDK warnings | Yes | Yes (harmless) |
| Stderr | Crash | Clean |
| Status | ❌ Crash | ✅ Runs |

### Verification

```bash
# Build
script/build.py -mc 1 Adventure

# Run (5 second test)
timeout 5 bin/Adventure
echo $?  # Should be 124 (timeout success)

# Check logs (no crash)
cat ~/.local/state/u++/log/Adventure.log
```

## Remaining Tasks

- [ ] **002-test-python-loading** - Verify Game.py loads
- [ ] **003-test-bindings** - Test Python bindings work
- [ ] **004-compare-esc-python** - Verify behavior matches ESC
- [ ] **005-document-api** - Write Python API docs

## Next Steps

1. Test Python script loading (Task 002)
2. Verify all 52 bindings work
3. Compare Python vs ESC behavior
