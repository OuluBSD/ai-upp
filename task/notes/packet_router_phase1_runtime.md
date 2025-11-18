# PacketRouter Phase 1 – Runtime API Implementation

## Goal
Implement the runtime PacketRouter class that actually routes packets at runtime, replacing the loop-based Link/Customer architecture. RouterNetContext currently builds metadata but delegates execution to legacy ChainContext.

## Current State
- ✓ RouterNetContext builds router metadata and writes to VFS
- ✓ Serialization/IDE integration complete
- ✗ No runtime packet routing exists
- ✗ Atoms still hardcoded to Exchange/Link model
- ✗ CustomerBase still drives packet generation

## Phase 1 Tasks

### Task 1: Create PacketRouter Stub Class
**File:** `uppsrc/Eon/Core/PacketRouter.{h,cpp}`
**Dependencies:** `Eon/Core/Atom.h`, `Vfs/Ecs/Interface.h`

**API Surface:**
```cpp
class PacketRouter {
public:
    struct PortHandle {
        AtomBase* atom;
        int port_index;
        RouterPortDesc::Direction direction;
        // Internal routing table index
        int router_index;
    };

    // Port registration (called during Atom::Initialize)
    PortHandle RegisterPort(AtomBase* atom,
                           RouterPortDesc::Direction dir,
                           int port_index,
                           const ValDevTuple& vd,
                           const ValueMap& metadata = ValueMap());

    // Connection management
    void Connect(PortHandle src, PortHandle dst, const ValueMap& policy = ValueMap());
    void Disconnect(PortHandle src, PortHandle dst);

    // Packet routing (called by Atom::EmitPacket)
    bool RoutePacket(PortHandle src_port, ForwardPacket& packet);

    // Flow control (credit-based)
    int  RequestCredits(PortHandle port, int requested_count);
    void AckCredits(PortHandle port, int ack_count);
    int  AvailableCredits(PortHandle port) const;

    // Diagnostics
    String DumpTopology() const;
    String DumpPortStatus() const;

private:
    struct Port {
        PortHandle handle;
        ValDevTuple vd;
        ValueMap metadata;
        Vector<int> connections;  // Indices into connection_table
        int credits_available = 1;  // Default: legacy-loop policy
    };

    struct Connection {
        int src_port_idx;
        int dst_port_idx;
        ValueMap policy;
    };

    Vector<Port> ports;
    Vector<Connection> connection_table;
    Index<AtomBase*> atom_index;
};
```

**Implementation Strategy:**
- Start with stub methods that LOG but don't route yet
- Store port/connection metadata for diagnostics
- Implement DumpTopology() first for debugging

### Task 2: Create upptst/RouterCore Test Package
**Purpose:** Unit tests for PacketRouter API without full Eon stack

**Test Cases:**
1. Port registration (sink/source, vd validation)
2. Connection table building (valid connections, detect invalid)
3. Credit allocation (request/ack, exhaustion cases)
4. Topology dump (verify metadata storage)

**Build:** Console package, no GUI dependencies
```bash
script/build-console.sh RouterCore
```

### Task 3: Extend Atom.h with Router Virtuals
**File:** `uppsrc/Eon/Core/Atom.h`

**New Virtuals:**
```cpp
class AtomBase {
public:
    // Called during Initialize to declare ports
    virtual void RegisterPorts(PacketRouter& router) {}

    // Notification when port has credits available
    virtual void OnPortReady(int port_id) {}

    // Replace hardcoded Send() with port-aware version
    virtual bool EmitPacket(int port_id, ForwardPacket& packet) { return false; }

protected:
    // Helpers for derived classes
    PacketRouter::PortHandle RegisterSinkPort(PacketRouter& router, int index, const ValDevTuple& vd);
    PacketRouter::PortHandle RegisterSourcePort(PacketRouter& router, int index, const ValDevTuple& vd);

private:
    Vector<PacketRouter::PortHandle> sink_ports;
    Vector<PacketRouter::PortHandle> source_ports;
};
```

**Backward Compatibility:**
- Default implementations do nothing (atoms opt-in to router mode)
- Legacy Exchange-based atoms continue working via CustomerBase shim

### Task 4: Proof-of-Concept Backend Integration
**Target:** Simple audio generator atom (`api/Audio` or similar)

**Steps:**
1. Override `RegisterPorts()` to declare one source port
2. Override `OnPortReady()` to generate audio packet
3. Override `EmitPacket()` to route via PacketRouter instead of Exchange
4. Add router policy metadata (credits_per_port=1 for legacy loop emulation)
5. Create test in `upptst/RouterCore` that validates audio generation through router

**Success Criteria:**
- Audio atom routes packets via PacketRouter
- Credit flow control limits generation rate
- Topology dump shows registered ports
- No CustomerBase dependency for this atom

### Task 5: CustomerBase Shim Analysis
**File:** `uppsrc/Eon/Core/Base.h`

**Approach:**
- Don't remove CustomerBase yet (breaks existing workloads)
- Add `#ifdef flagROUTER_RUNTIME` toggle
- When enabled, CustomerBase::Initialize registers ports via router
- When disabled, legacy Exchange behavior preserved

**Deliverable:** Document migration path in `uppsrc/Eon/AGENTS.md`

## Exit Criteria
- [ ] `PacketRouter.{h,cpp}` stub exists with full API surface
- [ ] `upptst/RouterCore` passes unit tests (port registration, connections, credits)
- [ ] One backend atom routes packets via PacketRouter (proof-of-concept)
- [ ] Topology dump shows router metadata for test atom
- [ ] CustomerBase shim approach documented

## Next Phase Dependencies
**Phase 2 (DSL Rewrite) requires:**
- Working PacketRouter runtime (this phase)
- At least one backend atom converted to router API
- Flow-control credit policy validated

## Timeline Estimate
- Task 1 (PacketRouter stub): 1-2 days
- Task 2 (RouterCore tests): 1 day
- Task 3 (Atom.h virtuals): 1 day
- Task 4 (Backend PoC): 2-3 days
- Task 5 (CustomerBase shim): 1 day

**Total:** ~1 week for Phase 1 runtime foundation
