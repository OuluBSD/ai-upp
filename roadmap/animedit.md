# AnimEdit / AnimEditLib Full Development Roadmap
Versioned, Multi‑Phase, Long‑Term Plan
=====================================

This roadmap describes the complete progression from the current prototype to a fully working,
production‑ready animation, entity, and world‑integration editor suite.  
It includes **major versions**, **minor versions**, and **well over 100 detailed tasks**.

---

# 🌑 MAJOR VERSION 0 — Foundations & Tooling
Primitive internal prototypes. No user-facing stability guarantees.

## **v0.1 — Core Data & Serialization (DONE)**
1. Implement core models (Sprite, Frame, Animation, etc.). ✅
2. Add JSON serialization for all models via Jsonize. ✅
3. Implement JSON loader/saver helpers (SaveProjectJson / LoadProjectJson). ✅
4. Create AnimEditLib test console application for round‑trip JSON tests. ✅
5. Add validation utilities for projects, animations, frames, and sprites. ✅
6. Add ID generation utilities for sprites, frames, animations, and collisions. ✅
7. Package cleanup and internal documentation pass for AnimEditLib. ✅

## **v0.2 — GUI Shell (DONE)**
8. Create AnimEdit GUI skeleton app and hook into workspace. ✅
9. Implement main window (AnimEditMain) with basic TopWindow behavior. ✅
10. Add main menu bar with File and Editors menus. ✅
11. Create placeholder windows: Animation Editor, Entity Editor, Texture Editor. ✅
12. Wire menu items to open the placeholder editor windows. ✅
13. Set up build configurations and verify application launch on target platforms. ✅

## **v0.3 — Editor State Management (DONE)**
14. Introduce AnimEditorState struct to encapsulate AnimationProject + metadata. ✅
15. Attach AnimEditorState to AnimEditorWindow and initialize with empty project. ✅
16. Implement NewProject() logic (clear state, create fresh project, update title). ✅
17. Implement OpenProject() using file dialog and JSON loader. ✅
18. Implement SaveProject() and SaveProjectAs() using file dialog and JSON saver. ✅
19. Add “dirty” flag tracking and propagate to window title. ✅
20. Add “Close Project” logic with dirty state confirmation dialog. ✅
21. Implement simple recent‑file list in main menu. ✅
22. Implement autosave interval (e.g., every X minutes when dirty). ✅
23. Implement backup file naming and cleanup policy. ✅
24. Add graceful error dialogs for failed load/save operations. ✅

---

# 🌒 MAJOR VERSION 1 — Animation Editor Alpha (Core UX)

## **v1.0 — Basic Canvas Functionality (IN PROGRESS)**
25. Implement canvas background grid drawing with configurable spacing. ✅
26. Implement origin crosshair rendering (red axes, gray background grid). ✅
27. Map mouse coordinates to world/canvas coordinates. ✅
28. Implement basic panning using middle mouse drag. ✅
29. Implement scroll‑wheel zoom in/out with cursor‑centric behavior. ✅
30. Add zoom level display in the UI (e.g., 100 %, 200 %). ✅
31. Draw SpriteInstances as simple colored rectangles or placeholders. ✅
32. Draw selection highlight around the active SpriteInstance. ✅
33. Implement click selection for SpriteInstances on the canvas. ✅
34. Implement drag‑to‑move for a single selected SpriteInstance. ✅
35. Add grid snapping toggle (on/off). ✅
36. Add snap‑to‑origin option for pivot alignment. ✅
37. Implement rotation via keyboard shortcuts (e.g., Q/E). ✅
38. Implement uniform scaling via keyboard shortcuts (e.g., +/-). ✅
39. Introduce an undo/redo stack for SpriteInstance transforms. ✅
40. Add simple toolbar buttons for undo/redo and snapping options. ✅

## **v1.1 — Parts Panel (Sprites Library) (DONE)**
41. Connect Parts panel to AnimationProject.sprites. ✅
42. Display sprite list with textual names and IDs. ✅
43. Add small icon/thumbnail rendering for sprites based on texture data. ✅
44. Add category filter (character / environment / effect / other). ✅
45. Add text filter (search by name or ID). ✅
46. Implement drag & drop from Parts panel to canvas to create SpriteInstances. ✅
47. Add right‑click context menu for sprite operations (edit, duplicate, delete). ✅
48. Implement basic sprite creation wizard (ID, texture path, region, pivot). ✅
49. Integrate Image Layers / texture import with sprite creation pipeline. ✅
50. Add sprite metadata editor UI (category, tags, description). ✅
51. Implement sorting options (by name, by category, by recent use). ✅
52. Add error handling for dangling sprite IDs (removed sprite used by frames). ✅

## **v1.2 — Timeline (Animation Sequence Editing)**
53. Visualize animation frames as horizontal card list in Timeline panel. ✅
54. Show frame index, frame name, and a tiny thumbnail for each. ✅
55. Implement frame selection from Timeline (sync with active Frame in state). ✅
56. Implement drag‑to‑reorder frames within the selected Animation. ✅
57. Add “Insert Frame” button (create new frame and insert into timeline). ✅
58. Add “Add Existing Frame” button (choose from global frame list). ✅
59. Add frame duplication from timeline (copy references and/or instances). ✅
60. Implement frame duration slider inside each timeline card. ✅
61. Implement play/pause/stop controls for animation preview. ✅
62. Implement loop mode toggle for preview (loop vs play once). ✅
63. Implement preview playback based on durations (defaultDuration / overrides).
64. Display current playback position as highlight on timeline.
65. Implement onion‑skin (ghost frame) toggle and frame range (prev/next).
66. Render onion skins in the canvas with configurable opacity.
67. Add timeline zoom (more or fewer frames visible at once).
68. Support keyboard navigation between frames (← / → arrows). ✅

## **v1.3 — Right‑Side Lists (Frames, Sprites, Collisions, Animations)**
69. Bind Frames list (left of right‑pane) to project.frames.
70. Allow creating a new Frame from Frames list via “+” header button.
71. Implement frame renaming and deletion from Frames list.
72. Ensure frames used by animations cannot be deleted without confirmation.
73. Display SpriteInstances for the active Frame in Sprites list.
74. Implement SpriteInstance selection from list (sync with canvas).
75. Allow reordering SpriteInstances and editing z‑index.
76. Implement collision rectangle list for the active Frame.
77. Allow creating new CollisionRect from list (“+” button).
78. Implement collision rectangle editing (position, size, optional ID).
79. Draw collision rectangles on canvas with semi‑transparent overlays.
80. Bind Animations list to project.animations with “+” header button.
81. Implement animation renaming and deletion from Animations list.
82. Ensure validation for animations (no empty frame sequences).
83. Add simple animation metadata editor fields (category, loop hint).
84. Ensure all three lists stay in sync with underlying AnimationProject.

---

# 🌓 MAJOR VERSION 2 — Entity Editor Alpha

## **v2.0 — Entity Model & Serialization**
85. Define Entity struct in AnimEditLib (ID, name, type).
86. Add named animation slots (e.g., Idle, Run, Attack) in Entity.
87. Add simple behavior parameters (speed, health, aggression, etc.).
88. Add serialization for Entity and Entity collections.
89. Add validation functions for Entity definitions.
90. Integrate Entities into AnimationProject or a parallel project file.

## **v2.1 — Entity Editor GUI**
91. Implement Entity Editor window layout (entity list + properties).
92. Bind entity list to project entities.
93. Add entity creation, duplication, and deletion.
94. Add animation slot assignment UI with dropdown for available animations.
95. Add property editor for behavior parameters (numeric fields, enums).
96. Add entity search/filter by name or tags.
97. Implement entity preview: show assigned animations in small preview area.
98. Implement basic entity validation feedback (warnings, errors).
99. Add entity import/export to separate JSON files.
100. Wire Entity Editor menu entries in main app (open from Editors menu).

---

# 🌔 MAJOR VERSION 3 — Texture Editor Alpha (World Tiles System)

## **v3.0 — Texture Slot Models**
101. Define Tile/TextureSlot struct in AnimEditLib (ID, category, flags).
102. Add association from TextureSlot to Animation (AnimationID).
103. Add tile metadata (solidity, friction, special flags).
104. Add serialization and validation for texture slots.
105. Ensure tile definitions can live in project or in a separate tileset file.

## **v3.1 — Texture Editor GUI**
106. Implement Texture Editor layout (slot list + properties + preview).
107. Bind slot list to texture slots data.
108. Add creation/deletion/duplication of texture slots.
109. Add animation binding dropdown per slot (choose any Animation).
110. Render animated preview of tile using same rendering as animation editor.
111. Add tile category filters (ground, background, hazard, decorative).
112. Add “bulk assign animation” tool for multiple tiles.
113. Add tile flags editor (solid, platform, damaging, slippery, etc.).
114. Validate tile definitions and highlight conflicts (e.g., missing animation).
115. Integrate Tile Editor open option in main menu.

---

# 🌕 MAJOR VERSION 4 — Level Editor Integration

## **v4.0 — Replace Old Texture System with Animation‑Based Tiles**
116. Replace legacy texture references in level data with TextureSlot IDs.
117. Update Level Editor to use texture slots instead of raw textures.
118. Implement animated tile rendering in the level view.
119. Add caching of animations per tile for performance.
120. Implement in‑memory hot reload when texture slots or animations change.
121. Add visual indication when tile/animation sync is in progress.

## **v4.1 — Level Editor UX Enhancements**
122. Add brush size controls for tile painting.
123. Add tile eyedropper tool.
124. Add tile preview on hover (in a small tooltip or side panel).
125. Add flatten/fill tools for areas.
126. Add minimap overview panel for level.
127. Add undo/redo integration for all tile editing actions.
128. Implement tile layer visibility toggles (foreground, background, collision).
129. Add per‑layer opacity controls.
130. Add keyboard shortcuts for tools (brush, fill, eyedropper, erase).

---

# 🌖 MAJOR VERSION 5 — Runtime Engine Sync

## **v5.0 — Live Preview Integration**
131. Design and implement event bus or observer system for editor→runtime updates.
132. Implement live sprite asset update in runtime when modified in editor.
133. Implement live frame update in runtime (frame changes show immediately).
134. Implement live animation update in runtime.
135. Implement live tile update for world/level rendering.
136. Implement live entity definition update in runtime (stats, animations).
137. Add runtime log console in editor (view engine logs in a panel).
138. Add simple “Connect/Disconnect to runtime” control in UI.
139. Handle runtime not available / connection failed states gracefully.

## **v5.1 — Playback Engine Improvements**
140. Add interpolation options for smoother animation playback (runtime).
141. Implement animation blending (cross‑fade between two animations).
142. Add animation state machine support (idle → run → jump transitions).
143. Optimize sprite batching for fewer draw calls.
144. Add culling of off‑screen animations/tiles.
145. Add runtime performance overlay (FPS, draw calls, animation count).
146. Provide configurable time scaling for slow‑motion / fast‑forward preview.

---

# 🌗 MAJOR VERSION 6 — Advanced Editing Tools & UX Polish

## **v6.0 — Canvas Power Tools**
147. Implement multi‑selection of SpriteInstances.
148. Implement box‑selection (drag rectangle).
149. Implement copy/paste of SpriteInstances between frames.
150. Implement duplicate‑in‑place for SpriteInstances.
151. Implement instance locking and visibility flags.
152. Add configurable snapping options (grid size, angle snapping).
153. Add canvas rulers (top/left).
154. Add guidelines and draggable reference lines.
155. Add quick‑align tools (align to origin, center, etc.).
156. Implement “nudge” movement with arrow keys and configurable step size.

## **v6.1 — Project‑Wide Tools & Maintenance**
157. Add global search UI (find sprite, frame, animation by ID/name).
158. Implement reference graph viewer (what uses this sprite/frame/animation).
159. Add unused asset detection (sprites/frames/animations not referenced anywhere).
160. Implement safe cleanup for unused assets.
161. Add bulk renamer for sprites.
162. Add bulk renamer for animations.
163. Implement project statistics view (counts, memory estimates).
164. Add “validate all” command that runs all validations and shows a report.

## **v6.2 — UX & Visual Polish**
165. Implement dark mode and light mode themes.
166. Add theme selector and persistent UI preferences.
167. Implement customizable keyboard shortcuts.
168. Add toolbar icons for all major tools and editors.
169. Add docking layout saving/restoring (remember panel positions/sizes).
170. Add startup project selection / last opened project reopen behavior.
171. Add inline help/tooltips for all major controls.
172. Add simple onboarding pop‑up for first‑time users.

---

# 🌘 MAJOR VERSION 7 — Export, Packaging, and Pipelines

## **v7.0 — Export Tools**
173. Implement export pipeline for game‑ready animation data (binary/JSON).
174. Implement spritesheet export with packed textures.
175. Add support for multiple export profiles (different formats/paths).
176. Add batch export for multiple projects/levels.
177. Add command‑line export mode (headless).
178. Add export report with warnings and size summaries.

## **v7.1 — Import Tools & Compatibility**
179. Implement import from previous AnimEdit versions (legacy formats).
180. Implement limited import from external tools (e.g., generic JSON).
181. Add upgrade assistant for older formats to new data model.
182. Add sanity checks for imported data (ID collisions, missing textures).
183. Add template projects for common game types (platformer, adventure).

---

# 🌑 MAJOR VERSION 8 — Stability, QA, and Documentation

## **v8.0 — Stability & QA**
184. Create comprehensive unit test suite for AnimEditLib core.
185. Add regression tests for serialization and validation.
186. Add automated tests for ID generation logic.
187. Add smoke tests for GUI startup and major workflows.
188. Implement profiling scenarios for performance hotspots.
189. Run memory analysis / leak detection and fix issues.
190. Add fuzz testing for JSON loading to catch malformed inputs.

## **v8.1 — Documentation & Examples**
191. Write full user guide for Animation Editor.
192. Write full user guide for Entity Editor.
193. Write full user guide for Texture/Tile Editor.
194. Create detailed developer guide for AnimEditLib (API docs).
195. Generate Doxygen or similar reference documentation.
196. Create multiple sample projects (characters, tilesets, levels).
197. Create tutorial series (step‑by‑step animation pipeline).
198. Record or script demo videos/walkthroughs (optional).
199. Add in‑tool “Help → Documentation” links to local/online docs.

---

# 🌕 MAJOR VERSION 9 — Final Stable Release

## **v9.0 — AnimEdit 1.0**
200. Finalize project file format and guarantee backward compatibility.
201. Lock down major APIs of AnimEditLib for 1.0.
202. Perform final UI/UX pass to streamline workflows.
203. Fix all critical and major bugs from issue tracker.
204. Tag and build AnimEdit 1.0 release binaries/packages.
205. Publish release notes and changelog.
206. Create simple project website or landing page.
207. Provide downloadable installer or archive builds.

---

# 🌟 Beyond 1.0 — Long‑Term Ideas (Post‑Launch, Non‑Committed)

These are speculative, nice‑to‑have future directions:

208. AI‑assisted frame generation or inbetweening.
209. Node‑based animation graphs editor (blend trees, state machines).
210. Procedural animation tools (shake, bob, physics‑driven overlays).
211. Networked collaborative editing (multi‑user projects).
212. Plugin system for custom exporters and importers.
213. Scripting support (Lua / embedded C++ modules).
214. Cloud‑backed project syncing and versioning.
215. Integration with external asset stores or repositories.
