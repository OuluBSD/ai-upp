# Neso Event Routing Pattern - Analysis

**Date**: 2026-01-17
**Purpose**: Document the event routing pattern from Microsoft's Neso engine to guide our implementation

---

## Architecture Overview

### 1. Listener Interface Pattern

**File**: `Neso/Engine/SpatialInteractionSystem.h` lines 14-30

```cpp
class ISpatialInteractionListener abstract {
public:
    virtual void OnSourceDetected(const SpatialInteractionSourceEventArgs& args) {};
    virtual void OnSourceLost(const SpatialInteractionSourceEventArgs& args) {};
    virtual void OnSourcePressed(const SpatialInteractionSourceEventArgs& args) {};
    virtual void OnSourceUpdated(const SpatialInteractionSourceEventArgs& args) {};
    virtual void OnSourceReleased(const SpatialInteractionSourceEventArgs& args) {};
};
```

**Key Points**:
- Pure abstract interface with default empty implementations
- Systems inherit from this to receive events
- Event data passed via platform-specific args struct

---

### 2. Event System (Broadcaster)

**File**: `Neso/Engine/SpatialInteractionSystem.{h,cpp}`

```cpp
class SpatialInteractionSystem final : public System<SpatialInteractionSystem> {
public:
    void AddListener(std::shared_ptr<ISpatialInteractionListener> listener) {
        m_spatialInteractionListeners.Add(std::move(listener));
    }

    void RemoveListener(std::shared_ptr<ISpatialInteractionListener> listener) {
        m_spatialInteractionListeners.Remove(std::move(listener));
    }

protected:
    void Initialize() override;   // Bind to platform event source
    void Uninitialize() override; // Unbind from platform event source

private:
    ISpatialInteractionManager m_spatialInteractionManager{ nullptr };
    ListenerCollection<ISpatialInteractionListener> m_spatialInteractionListeners;

    void BindEventHandlers();     // Connect platform events to Handle* methods
    void ReleaseEventHandlers();  // Disconnect platform events

    void HandleSourcePressed(...);  // Receives platform event, broadcasts to listeners
    // ... more handlers
};
```

**Initialize/Uninitialize Pattern** (lines 11-21):
```cpp
void SpatialInteractionSystem::Initialize() {
    m_spatialInteractionManager = SpatialInteractionManager::GetForCurrentView();
    BindEventHandlers();  // Connect to Windows VR API
}

void SpatialInteractionSystem::Uninitialize() {
    ReleaseEventHandlers();  // Disconnect from Windows VR API
    m_spatialInteractionManager = nullptr;
}
```

**Event Binding Pattern** (lines 23-41):
```cpp
void SpatialInteractionSystem::BindEventHandlers() {
    // Use std::bind to connect platform events to member functions
    m_sourceTokens[Pressed] = m_spatialInteractionManager.SourcePressed(
        std::bind(&SpatialInteractionSystem::HandleSourcePressed, this, _1, _2));
    // ... more bindings
}
```

**Event Broadcasting Pattern** (lines 74-82):
```cpp
void SpatialInteractionSystem::HandleSourcePressed(
    const SpatialInteractionManager& /*sender*/,
    const SpatialInteractionSourceEventArgs& args)
{
    // Broadcast to all registered listeners
    for (const auto& listener : m_spatialInteractionListeners.PurgeAndGetListeners()) {
        listener->OnSourcePressed(args);
    }
}
```

---

### 3. Listener Registration Pattern

**File**: `DemoRoom/ToolboxSystem.{h,cpp}`

**Class Declaration** (lines 22-24):
```cpp
class ToolboxSystem :
    public Neso::System<ToolboxSystem>,           // CRTP base, provides shared_from_this()
    public Neso::ISpatialInteractionListener       // Listener interface
{
    // ... implementation
};
```

**Registration in Start()** (line 90):
```cpp
void ToolboxSystem::Start() {
    // ... create entities ...

    // REGISTER: Find SpatialInteractionSystem via engine, register self
    m_engine.Get<SpatialInteractionSystem>()->AddListener(shared_from_this());
}
```

**Unregistration in Stop()** (line 95):
```cpp
void ToolboxSystem::Stop() {
    // UNREGISTER: Find SpatialInteractionSystem via engine, unregister self
    m_engine.Get<SpatialInteractionSystem>()->RemoveListener(shared_from_this());
}
```

**Event Handler** (lines 165-232):
```cpp
void ToolboxSystem::OnSourcePressed(const SpatialInteractionSourceEventArgs& args) {
    if (args.State().Source().Kind() != SpatialInteractionSourceKind::Controller)
        return;

    // Handle tool switching logic based on button press
    if (args.PressKind() == SpatialInteractionPressKind::Menu) {
        m_showToolbox = !m_showToolbox;
        // ... toolbox positioning logic
    }
}
```

---

### 4. Engine System Lookup

**File**: `Neso/Engine/Engine.h` lines 60-79

```cpp
template<typename SystemT>
std::shared_ptr<SystemT> Get() {
    auto system = TryGet<SystemT>();
    fail_fast_if(system == nullptr);  // Assert if not found
    return system;
}

template<typename SystemT>
std::shared_ptr<SystemT> TryGet() {
    static_assert(detail::is_system<SystemT>::value, "T should derive from System");
    fail_fast_if(!m_initialized);

    auto it = FindSystem(typeid(SystemT));  // Find by type_id
    return it == m_systems.end()
        ? nullptr
        : std::static_pointer_cast<SystemT>(*it);
}
```

**Key Points**:
- Systems stored in `vector<shared_ptr<SystemBase>>`
- Lookup by `type_id` (RTTI)
- `Get<T>()` asserts if not found, `TryGet<T>()` returns nullptr
- Systems access engine via `m_engine` member (injected in constructor)

---

### 5. ListenerCollection (Weak Pointer Pattern)

**File**: `Neso/Engine/ListenerCollection.h`

```cpp
template<typename T>
class ListenerCollection {
public:
    void Add(std::shared_ptr<T> listener) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_listeners.push_back(std::move(listener));  // Store as weak_ptr
    }

    void Remove(std::shared_ptr<T> listener) {
        std::unique_lock<std::mutex> lock(m_mutex);
        erase_if(&m_listeners, [&listener](const std::weak_ptr<T>& weak) {
            std::shared_ptr<T> ptr = weak.lock();
            return ptr == listener;
        });
    }

    std::vector<std::shared_ptr<T>> PurgeAndGetListeners() {
        std::vector<std::shared_ptr<T>> result;
        std::unique_lock<std::mutex> lock(m_mutex);

        // Remove dead weak_ptrs, collect live ones
        erase_if(&m_listeners, [&result](const std::weak_ptr<T>& weak) {
            std::shared_ptr<T> ptr = weak.lock();
            if (ptr) {
                result.push_back(std::move(ptr));
                return false;  // Keep in collection
            } else {
                return true;   // Remove from collection
            }
        });
        return result;
    }

private:
    std::mutex m_mutex;
    std::vector<std::weak_ptr<T>> m_listeners;  // Weak pointers allow systems to be destroyed
};
```

**Key Points**:
- Stores `weak_ptr<T>` to allow systems to be destroyed independently
- Thread-safe with mutex (important for async VR events)
- `PurgeAndGetListeners()` removes dead pointers and returns live ones
- Destructor asserts collection is empty (ensures proper cleanup)

---

## Comparison with Our Current Implementation

### What We Have

**File**: `uppsrc/Eon/Interaction/InteractionSystem.h`

```cpp
struct InteractionListener {
    // Similar interface but uses GeomEvent instead of platform-specific args
    virtual void OnControllerDetected(const GeomEvent& e) {}
    virtual void OnControllerLost(const GeomEvent& e) {}
    virtual void OnControllerPressed(const GeomEvent& e) {}
    virtual void OnControllerUpdated(const GeomEvent& e) {}
    virtual void OnControllerReleased(const GeomEvent& e) {}
};

struct InteractionManager {
    // OLD PATTERN - Uses Callback instead of direct listener collection
    using Cb = Callback2<const InteractionManager&, const GeomEvent&>;
    Cb WhenSourcePressed;
    Cb WhenSourceUpdated;
    Cb WhenSourceReleased;
    // ...
};
```

### What We Need to Change

1. **InteractionSystem needs listener collection**:
   - Add `Vector<InteractionListener*> listeners;` or use U++ Event<>
   - Add `AddListener(InteractionListener*)` method
   - Add `RemoveListener(InteractionListener*)` method
   - Fire events by iterating listeners

2. **InteractionManager connection**:
   - In `InteractionSystem::Initialize()`: Wire `InteractionManager` callbacks to `InteractionSystem::Fire*()` methods
   - In `InteractionSystem::Uninitialize()`: Clear callbacks

3. **Listener registration**:
   - Systems inherit from `InteractionListener`
   - In `Start()`: Get `InteractionSystem` via engine, call `AddListener(this)`
   - In `Stop()`: Get `InteractionSystem` via engine, call `RemoveListener(this)`

4. **Engine system lookup**:
   - Already have `Engine::TryGet<T>()` in our implementation (need to verify)
   - Systems should access via `GetEngine().TryGet<InteractionSystem>()`

---

## Implementation Plan

### Phase 1: InteractionSystem Listener Collection

**File**: `uppsrc/Eon/Interaction/InteractionSystem.h`

Add to `InteractionSystem`:
```cpp
class InteractionSystem : public System {
    // ... existing code ...

    void AddListener(InteractionListener* listener);
    void RemoveListener(InteractionListener* listener);

    void FireDetected(const GeomEvent& e);
    void FireLost(const GeomEvent& e);
    void FirePressed(const GeomEvent& e);
    void FireUpdated(const GeomEvent& e);
    void FireReleased(const GeomEvent& e);

private:
    Vector<InteractionListener*> listeners;  // Or use Event<const GeomEvent&>
};
```

**File**: `uppsrc/Eon/Interaction/InteractionSystem.cpp`

Implement:
```cpp
void InteractionSystem::AddListener(InteractionListener* listener) {
    VectorFindAdd(listeners, listener);
}

void InteractionSystem::RemoveListener(InteractionListener* listener) {
    VectorRemoveKey(listeners, listener);
}

void InteractionSystem::FirePressed(const GeomEvent& e) {
    for (InteractionListener* listener : listeners) {
        listener->OnControllerPressed(e);
    }
}
// ... same for other events
```

### Phase 2: Wire InteractionManager to InteractionSystem

**In `InteractionSystem::Initialize()`**:
```cpp
bool InteractionSystem::Initialize(const WorldState& ws) {
    // Get or create FakeSpatialInteractionManager
    if (!mgr) {
        mgr = new FakeSpatialInteractionManager();
        // ... setup
    }

    // Wire callbacks to Fire* methods
    mgr->WhenSourcePressed = [this](const InteractionManager& m, const GeomEvent& e) {
        this->FirePressed(e);
    };
    mgr->WhenSourceUpdated = [this](const InteractionManager& m, const GeomEvent& e) {
        this->FireUpdated(e);
    };
    // ... more callbacks

    return true;
}
```

**In `InteractionSystem::Uninitialize()`**:
```cpp
void InteractionSystem::Uninitialize() {
    if (mgr) {
        mgr->WhenSourcePressed.Clear();
        mgr->WhenSourceUpdated.Clear();
        // ... clear all callbacks
    }
}
```

### Phase 3: Update Listener Systems

**Example for `PlayerBodySystem`**:

Already inherits from `InteractionListener` ✓

**Add to `Start()`**:
```cpp
bool PlayerBodySystem::Start() {
    InteractionSystemPtr isys = GetEngine().TryGet<InteractionSystem>();
    if (isys) {
        isys->AddListener(this);
    }
    return true;
}
```

**Add to `Stop()`**:
```cpp
void PlayerBodySystem::Stop() {
    InteractionSystemPtr isys = GetEngine().TryGet<InteractionSystem>();
    if (isys) {
        isys->RemoveListener(this);
    }
}
```

**Same pattern for**:
- `ToolboxSystemBase`
- `PaintingInteractionSystemBase`
- `ShootingInteractionSystemBase`
- `ThrowingInteractionSystemBase`

### Phase 4: Call InteractionManager::Update()

**In `InteractionSystem::Update()`**:
```cpp
void InteractionSystem::Update(double dt) {
    if (mgr) {
        mgr->Update(dt);  // Process keyboard/mouse input, fire events
    }
}
```

---

## Key Differences: Neso vs Our Implementation

| Aspect | Neso | Our Implementation |
|--------|------|-------------------|
| Listener Storage | `ListenerCollection<T>` (weak_ptr) | Vector or Event<> (raw pointers) |
| Event Data | `SpatialInteractionSourceEventArgs` (Windows) | `GeomEvent` (platform-agnostic) |
| Registration | `shared_from_this()` | Raw `this` pointer |
| Thread Safety | Mutex in ListenerCollection | Not needed (single-threaded) |
| Callback Pattern | std::bind with platform events | U++ Callback or Event<> |
| System Lookup | `m_engine.Get<T>()` | `GetEngine().TryGet<T>()` |

---

## Questions Resolved

1. **How do events flow?**
   Platform → InteractionManager → InteractionSystem::Handle*() → Fire*() → Listeners

2. **Where is VfsValue-tree used?**
   NOT for event flow. Used to find systems via `Engine::Get<T>()` during Start()/Stop()

3. **When to register listeners?**
   In `Start()` (after all systems initialized), unregister in `Stop()`

4. **How to find InteractionSystem?**
   Via `m_engine.Get<InteractionSystem>()` or `GetEngine().TryGet<InteractionSystem>()`

5. **What about Event<> vs Callback?**
   User prefers Event<> for lambdas. Can use `Event<const GeomEvent&>` in InteractionSystem.

---

## Next Steps

1. ✅ Document Neso pattern (this file)
2. ☐ Check if our `Engine::TryGet<>()` exists and works
3. ☐ Implement listener collection in `InteractionSystem`
4. ☐ Wire `InteractionManager` callbacks in `Initialize()/Uninitialize()`
5. ☐ Add `Start()/Stop()` to listener systems with registration calls
6. ☐ Ensure `FakeSpatialInteractionManager::Update()` is called
7. ☐ Test event flow with debug logging
8. ☐ Verify tool switching works with keyboard/mouse input
