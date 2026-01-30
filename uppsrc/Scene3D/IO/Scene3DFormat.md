# Scene3D Project Format

Status: v1 (current)

## Goals
- Store scene graph, object transforms, pointcloud references, and HMD pose streams.
- Support live capture + offline replay.
- Keep core format engine-agnostic (no SoftHMD dependency).

## File Extension
- `.scene3d`

## Encodings
- **JSON**: human-readable for tests, review, and hand edits.
- **Binary**: compact format for large scenes and pointclouds.

Both encodings use the same field layout and are serialized via U++ `Vis`.

## Top-Level Document
```
{
  "version": 1,
  "name": "ProjectName",
  "project": { ... },
  "active_scene": 0,
  "focus": [0, 0, 0],
  "program": "ModelerApp"
}
```

## Project
```
project {
  scenes: [ Scene, ... ]
}
```

## Scene
```
Scene {
  GeomDirectory: { ... }
}
```

## Directory
```
GeomDirectory {
  name: "",
  subdir: { "dir_name": GeomDirectory, ... },
  objs: [ GeomObject, ... ]
}
```

## Object
```
GeomObject {
  name: "camera",
  type_i: 2,          // enum index
  timeline: GeomTimeline,
  data: GeomObjectData
}
```

## Timeline
```
GeomTimeline {
  keypoints: [ GeomKeypoint, ... ]
}

GeomKeypoint {
  key: 0,
  value: GeomObjectData
}
```

## Object Data (current)
```
GeomObjectData {
  frame_id: 0,
  position: vec3,
  orientation: quat
}
```

## Notes
- JSON uses the same fields as binary; binary storage is recommended for large scenes.
- Versioning is mandatory; loaders should refuse unknown major versions.
- Additional fields should be introduced with backward-compatible defaults.
