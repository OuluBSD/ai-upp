# Graphics - Agent Notes

Purpose
- Rendering backend for U++ (OpenGL, SoftRend, DX11 WIP) used by Eon and other systems.

Key Files
- `TBufferStage.cpp`: draw loop, program selection, pipeline binding.
- `TState.cpp`/`TState.h`: ModelState/DataObject VAO/VBO/EBO setup and drawing.
- `TProgram.cpp`/`TPipeline.cpp`: shader compile/link and program pipeline management.
- `Ogl.cpp`: OpenGL backend bindings.

OpenGL Pitfalls
- Do not keep a program pipeline bound while using `glUseProgram`. In the ECS path (`BufferStageT::Process`), unbind the pipeline (`Gfx::UnbindProgramPipeline`) before `UseProgram`.
- Repeated `GL_INVALID_OPERATION in glDrawElements` often indicates an invalid pipeline/UseProgram mix or missing VAO/EBO state; inspect `DataObjectT::Refresh` and `Paint`.
- apitrace “shader compiler issue … Shader Stats” lines are driver info, not a link failure. Treat it as an error only if the log shows a failed link (e.g., “program is not linked successfully”).

Model Loading
- When a prefab is loaded via `ModelCache`, the `GfxModelState` still needs `LoadModel(*model)` to populate OpenGL buffers; otherwise only the skybox renders.
