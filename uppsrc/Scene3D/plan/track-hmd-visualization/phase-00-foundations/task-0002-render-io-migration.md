Task 0002 - Render/IO migration [DONE]

Checklist
- [x] Move Edit3D Renderer and SoftRendCtrl into Scene3D/Render.
- [x] Move VideoImporter/StagedVideoImporter into Scene3D/IO.
- [x] Update Edit3D to use Scene3D Render/IO context.

Notes
- EditRenderer now consumes Scene3DRenderContext with callbacks for refresh.
