# PacketRouter Phase 2 - DSL Integration

## Goal
Integrate PacketRouter with the ChainContext/LoopContext system to automatically register atom ports and generate router connections from the DSL loop definitions.

## Architecture Overview

### Current Flow (Legacy)
1. `ChainContext::AddLoop()` creates atoms
2. `LoopContext::AddAtom()` initializes each atom
3. `LoopContext::MakePrimaryLinks()` creates Exchange connections
4. Atoms use Exchange/Link for packet flow

### Target Flow (Router Mode)
1. `ChainContext::AddLoop()` creates atoms
2. `LoopContext::AddAtom()` initializes each atom
3. `LoopContext::RegisterRouterPorts()` calls `atom->RegisterPorts(router)`
4. `LoopContext::MakeRouterConnections()` generates router connections
5. Atoms use PacketRouter for packet flow

## Phase 2 Tasks

### Task 1: Add PacketRouter to LoopContext
**File:** `uppsrc/Eon/Core/Context.h`

Add router support to LoopContext:
```cpp
class LoopContext {
public:
    // ...existing members...

    #ifdef flagROUTER_RUNTIME
    One<PacketRouter> router;
    #endif

    // New methods
    bool RegisterRouterPorts();
    bool MakeRouterConnections();
};
```

### Task 2: Implement Port Registration
**File:** `uppsrc/Eon/Core/Context.cpp`

Call RegisterPorts on each atom after initialization:
```cpp
bool LoopContext::RegisterRouterPorts() {
    #ifdef flagROUTER_RUNTIME
    if (!router)
        router.Create();

    for (auto& info : added) {
        if (info.a)
            info.a->RegisterPorts(*router);
    }
    return true;
    #else
    return true;
    #endif
}
```

### Task 3: Generate Router Connections
**File:** `uppsrc/Eon/Core/Context.cpp`

Map primary links to router connections:
```cpp
bool LoopContext::MakeRouterConnections() {
    #ifdef flagROUTER_RUNTIME
    if (!router || added.GetCount() < 2)
        return true;

    // Connect source port 0 of atom[i] to sink port 0 of atom[i+1]
    for (int i = 0; i < added.GetCount(); i++) {
        AddedAtom& src = added[i];
        AddedAtom& dst = added[(i + 1) % added.GetCount()];

        // Find source's output port and sink's input port
        // Use router_source_ports[0] and router_sink_ports[0]
        // Call router->Connect(src_handle, dst_handle)
    }
    return true;
    #else
    return true;
    #endif
}
```

### Task 4: Hook into AddLoop Flow
**File:** `uppsrc/Eon/Core/Context.cpp`

Modify AddLoop to call router registration:
```cpp
LoopContext& ChainContext::AddLoop(...) {
    // ...existing atom creation...

    if (make_primary_links)
        lc.MakePrimaryLinks();

    #ifdef flagROUTER_RUNTIME
    lc.RegisterRouterPorts();
    lc.MakeRouterConnections();
    #endif

    return lc;
}
```

### Task 5: Add PortHandle Storage to AtomBase
Currently AtomBase stores `Vector<int> router_sink_ports` and `router_source_ports`.
Need to also store full PortHandle for each to enable connection generation.

### Task 6: Create Test with Real Atoms
**File:** `upptst/RouterCore/` or new test package

Create a test that:
1. Uses ChainContext to build a simple loop
2. Verifies RegisterPorts was called on each atom
3. Verifies router connections match primary links
4. Uses flagROUTER_RUNTIME to toggle modes

## Integration Points

### Where Port Registration Happens
- After `LoopContext::AddAtom()` but before `MakePrimaryLinks()`
- In `ChainContext::AddLoop()` after all atoms are created

### Where Connection Generation Happens
- After `MakePrimaryLinks()` (both can coexist during transition)
- Uses atom interface info to map channels to ports

### Side Links
- Side link connections also need router mapping
- `LoopContext::ConnectSides()` should generate router connections
- Side channels start at index 1 (not 0)

## Flag Strategy

Use `flagROUTER_RUNTIME` to control:
- Router creation in LoopContext
- RegisterPorts calls on atoms
- Router connection generation
- Eventually: disable legacy Exchange path

## Testing Strategy

1. **Unit Test**: RouterCore with mock ChainContext
2. **Integration Test**: Simple Eon script with router mode
3. **Validation**: Compare topology dump with expected connections

## Files to Modify

- `uppsrc/Eon/Core/Context.h` - Add router member and methods
- `uppsrc/Eon/Core/Context.cpp` - Implement port registration and connections
- `uppsrc/Vfs/Ecs/Atom.h` - May need PortHandle storage
- `upptst/RouterCore/Main.cpp` - Add ChainContext integration tests

## Success Criteria

- [ ] LoopContext creates PacketRouter when flagROUTER_RUNTIME
- [ ] RegisterPorts called on all atoms in loop
- [ ] Router connections mirror primary link topology
- [ ] Topology dump shows correct port/connection structure
- [ ] Tests pass with both legacy and router modes

## Dependencies

- Phase 1 complete (PacketRouter API, Atom virtuals)
- flagROUTER_RUNTIME defined for router mode

## Timeline

- Task 1-2: Add router and port registration (1 day)
- Task 3-4: Connection generation (1 day)
- Task 5-6: PortHandle storage and testing (1 day)

**Total:** ~3 days for Phase 2 DSL integration
