# Scene3D Project Format (Draft)

Status: Draft v0

## Goals
- Store scene graph, object transforms, pointcloud references, and HMD pose streams.
- Support live capture + offline replay.
- Keep core format engine-agnostic (no SoftHMD dependency).

## File Extension
- `.scene3d`

## Encoding
- UTF-8 text, line-based key/value with blocks.
- Compatible with U++ `StoreAsJson` later if we move to JSON.

## High-Level Layout
- Header
- Project
- Scenes
- Objects
- Assets
- Pointcloud
- Pose Stream

## Header
```
scene3d_version=0
created=YYYY-MM-DDTHH:MM:SSZ
```

## Project Block
```
project {
    name=MyScene
    units=meters
    fps=60
    kps=5
}
```

## Scene Block
```
scene {
    id=scene0
    name=Default Scene
    length=120
}
```

## Object Block
```
object {
    id=obj0
    name=HMD
    type=camera|model|octree|pointcloud|empty
    parent=scene0
    pose_ref=pose0
    model_ref=model0
    octree_ref=pc0
}
```

## Pose Stream Block
```
pose_stream {
    id=pose0
    format=quat
    unit=meters
    samples=120
    data=[
        t, px, py, pz, qx, qy, qz, qw
        ...
    ]
}
```

## Pointcloud Block
```
pointcloud {
    id=pc0
    format=xyz
    source=octree
    data_ref=pc0.bin
}
```

## Assets Block
```
asset {
    id=model0
    type=model
    path=share/models/...
}
```

## Notes
- Binary blobs (pointcloud, mesh) should be stored alongside the `.scene3d` file.
- A future version can embed JSON or binary chunks if needed.
- Calibration can be referenced via asset path (e.g., `calibration_ref=share/calibration/...`).
