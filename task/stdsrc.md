# stdsrc Thread

**Goal**: Implement wrapper library in stdsrc that implements U++ Core functions using STL std c++ libraries

## Status: IN PROGRESS

---

## Completed Work

### Core Library (COMPLETED)
- [x] Implement all currently required classes in stdsrc/Core
  - Atomic.h, BiCont.h, BinUndoRedo.h, CharSet.h, CoAlgo.h, CoSort.h, CoWork.h
  - CritBitIndex.h, Cpu.h, Daemon.h, Debug.h, Diag.h, Dli.h, FileMapping.h
  - FilterStream.h, FixedMap.h, Fn.h, Heap.h, Huge.h, Inet.h, Ini.h
  - InMap.hpp, InVector.h, LinkedList.h, LocalProcess.h, Mt.h, NetNode.h
  - Ops.h, Other.h, PackedData.h, Parser.h, Random.h, Range.h, Recycler.h
  - Shared.h, SIMD.h, Socket.h, Sorted.h, Speller.h, SplitMerge.h, St.h
  - Topic.h, Topt.h, UnicodeInfo.h, Utf.h, ValueCache.h, ValueUtil.h
  - Vcont.hpp, WebSocket.h, Win32Util.h, Xmlize.h, xxHsh.h, z.h
  - AString.hpp, Convert.hpp, Index.hpp, Map.hpp, Other.hpp, Tuple.h
  - Utf.hpp, Value.hpp, Xmlize.hpp, InVector.hpp, CharFilter.h

### Draw Library (PARTIAL)
- [x] Implement Iml.h in stdsrc/Draw

### CtrlLib Library (PARTIAL)
- [x] Implement Splitter.h, ScrollBar.h, SliderCtrl.h in stdsrc/CtrlLib

---

## Pending Work

### Draw Library
- [ ] Implement missing classes in stdsrc/Draw to match uppsrc/Draw functionality:
  - [ ] Cham.h, DDARasterizer.h, Display.h (different from CtrlCore Display.h)
  - [ ] DrawUtil.h, ImageOp.h, Palette.h, Raster.h, SDraw.h, SIMD.h, Uhd.h
  - [ ] Drawing.h, DrawRasterData.h, DrawText.h, DrawTextUtil.h
  - [ ] ImageAnyDraw.h, ImageBlit.h, ImageChOp.h, ImageScale.h
  - [ ] MakeCache.h, Mify.h, RasterEncoder.h, RasterFormat.h, RasterWrite.h
  - [ ] RescaleFilter.h, SColors.h, SDrawClip.h, SDrawPut.h, SDrawText.h
  - [ ] SImageDraw.h

### CtrlCore Library
- [ ] Implement missing classes in stdsrc/CtrlCore to match uppsrc/CtrlCore functionality:
  - [ ] CtrlAttr.h, CtrlChild.h, CtrlClip.h, CtrlDraw.h, CtrlFrame.h
  - [ ] CtrlKbd.h, CtrlMouse.h, CtrlMt.h, CtrlPos.h, CtrlTimer.h
  - [ ] DHCtrl.h, Frame.h, SystemDraw.h, Util.h, MKeys.h, stdids.h
  - [ ] LocalLoop.h, MetaFile.h, EncodeRTF.h, ParseRTF.h

### CtrlLib Library
- [ ] Implement missing classes in stdsrc/CtrlLib to match uppsrc/CtrlLib functionality:
  - [ ] Bar.h, Ch.h, ChatCtrl.h, ColorPopup.h, ColorPusher.h, ColumnList.h
  - [ ] DateTimeCtrl.h, DisplayPopup.h, DlgColor.h, DropChoice.h, DropList.h
  - [ ] DropTree.h, EditCtrl.h, FileList.h, FileSel.h, FrameSplitter.h
  - [ ] HeaderCtrl.h, LabelBase.h, LineEdit.h, MenuBar.h, MenuItem.h
  - [ ] MultiButton.h, PageCtrl.h, PopUpList.h, PopupTable.h, Progress.h
  - [ ] PushCtrl.h, RichText.h, RichTextView.h, StaticCtrl.h, StatusBar.h
  - [ ] SuggestCtrl.h, TabCtrl.h, ToolBar.h, ToolButton.h, TreeCtrl.h
  - [ ] TimelineCtrl.h

---

## Related Efforts

### Platform Implementations
The wrapper library should support multiple backend implementations:
- WXWidgets/Gtk/Qt for stdsrc/{Draw, CtrlCore, CtrlLib}
- Native platform APIs: Windows, macOS, and other platform-specific implementations
- Ensure comprehensive stdtst package coverage for all wrapper library features

### VR ECS Engine
- [x] Convert Eon/Win VR ECS engine to work with OpenVR and OpenHMD in addition to current WinRT implementation
- [x] Address WinRT limitations and ensure UWP (Universal Windows Platform) compatibility for Eon/Win
- [x] Create CMake files to enable Visual Studio compilation for Eon/Win project

---

## Dependencies
- Requires: Standard C++ library (STL)
- Blocks: Code conversion tools (needs mapping for U++ → STL conversion)
- Related: uppstd thread (U++ to STL mapping documentation)
