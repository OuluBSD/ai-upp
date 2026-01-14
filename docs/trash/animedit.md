# AnimEdit / AnimEditLib Roadmap (C++ / Ultimate++)

A structured multi-stage roadmap for building the new animation-driven editor
suite using Ultimate++ in C++.

---

## 🌱 Phase 0 — Foundation (Current Phase)

### ✅ 0.1 Core Model Library (AnimEditLib)
- Sprite, Frame, Animation model objects  
- Vec2, RectF math types  
- Lookup helpers  
- Versioning (major/minor)

### 🔜 0.2 Serialization Layer
- JSON or U++-style serialization  
- Project load/save  
- Backward-compatibility stubs

---

## 🏗 Phase 1 — Editor Skeletons

### 1.1 AnimEdit Application Integration
- Add AnimEditLib as a dependency  
- Provide empty stub windows for:
  - Animation Editor
  - Entity Editor
  - World Texture Editor

### 1.2 Unified Resource Registry
- Runtime container for:
  - All sprites  
  - Frames  
  - Animations  
  - Texture slots  
  - Entities  
- Global notification system (Signals/Slots or callbacks)

---

## 🎞 Phase 2 — Animation Editor Implementation

### 2.1 Parts Panel (Sprites)
- Display sprite list  
- Drag & drop onto canvas  
- Filtering by category  

### 2.2 Canvas System
- Grid rendering  
- Origin crosshair  
- SpriteInstance transform editing  
- Ghost-frame overlay toggle  

### 2.3 Timeline
- Frame cards (thumbnail, name, index, duration slider)  
- Add / remove / reorder frame references  
- Independent FrameList vs. Animation Timeline logic  

### 2.4 Right-Side Panels
- Frame list  
- SpriteInstances list  
- Collision rectangles list  
- Animation list  

---

## 🌍 Phase 3 — Texture & Entity Editors

### 3.1 New Texture Editor
- Texture slots = Animation references  
- Category-aware sprite/animation filtering  
- World-linked assignment

### 3.2 New Entity Editor
- Animation slot binding  
- Basic behavior parameters (AI, physics, properties)  
- No sprite composition here (delegated to Animation Editor)

---

## 🧪 Phase 4 — Runtime Integration

### 4.1 Playback Engine
- Real-time animation playback  
- Sprite batching (optional)  
- Interpolation support  

### 4.2 Live Sync System
- Shared memory or event bus  
- “Edit → Runtime” hot reload  
- “Runtime → Editor” feedback (state watching)

---

## 🚀 Phase 5 — Polishing & Tooling

### 5.1 Visual improvements
- Timeline zoom  
- Ghost-frame opacity control  
- High-DPI UI  

### 5.2 Migration Tools
- Convert old Entity Editor data → new format  
- Convert old textures → sprites/animations  

### 5.3 Project Packaging
- Export spritesheets  
- Export animation metadata  
- Export game-ready resources  

---

## 🏁 Final Vision

A fully modular, animation-centric tool suite where:

- **AnimEditLib** is the shared foundation  
- **AnimEdit** is the GUI shell  
- Sprites, frames, and animations are first-class citizens  
- World textures and entities reference animations  
- Entire editor–runtime pipeline stays live and reactive  

