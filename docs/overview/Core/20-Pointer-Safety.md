# Pointer Safety

## What this covers
This file documents Core's non-owning pointer-safety layer: `PteBase`, `Pte<T>`, `PtrBase`, and `Ptr<T>`.

## Design intent
[`uppsrc/Core/Ptr.h`](../../../uppsrc/Core/Ptr.h) provides a tracked observer pointer, not shared ownership. The goal is simple: let references to selected objects become null automatically when the pointee dies.

This is narrower than `shared_ptr` / `weak_ptr` style ownership. The pointee must opt in.

## Main types
### `PteBase` and `Pte<T>`
Objects that want to be tracked must derive from `PteBase` or, more commonly, `Pte<T>`.

That requirement is real and visible in the API:

- `PtrBase` stores a `PteBase*`
- `Ptr<T>::operator=(T*)` only works when `T*` is convertible to `PteBase*`
- destruction of the pointee is what notifies observers

Without that inheritance, `Ptr<T>` cannot track the object.

### `Ptr<T>`
`Ptr<T>` is the typed observer handle. It supports:

- nullable pointer semantics
- copy and assignment
- automatic nulling when the pointee is destroyed
- pointer-like access through `operator->`, conversion, and `Get()`

The implementation uses a shared tracking record (`Prec`) with an atomic reference count.

## Lifetime semantics
[`uppsrc/Core/Ptr.cpp`](../../../uppsrc/Core/Ptr.cpp) shows the core behavior:

- `PteBase::~PteBase()` sets the tracking record's `ptr` to null
- existing `Ptr<T>` observers keep the tracking record alive long enough to observe that null state
- no pointee ownership is transferred

So `Ptr<T>` answers "is the object still alive?" for opt-in objects, but it does not manage destruction.

## Debug and panic behavior
`PtrBase::PanicRelease(bool)` and the `panic` flag in the tracking record allow a stricter failure mode where releasing a still-observed pointee can trigger `Panic`.

That is diagnostic behavior around misuse. It does not change the basic contract that normal pointee destruction nulls observers.

## Tradeoffs
- safer than a raw pointer when the pointee supports tracking
- much lighter and more direct than shared ownership
- limited to classes that inherit from `Pte<T>` / `PteBase`
- not a general replacement for ownership models or cycle management

## Current vs legacy
This layer is current code, but it is specialized. It exists for selected object graphs and callback-style patterns, not as the universal pointer model for Core.

## See also
- [03-Threading.md](03-Threading.md)
- [09-Containers.md](09-Containers.md)
- [11-Callbacks-and-Events.md](11-Callbacks-and-Events.md)
