# Directories and hierarchy

Look in the folder to see if there is a file with the same name as the folder and a ".upp" extension. There may be a "description" section that briefly explains the contents of the folder.

Files ending in ".upp" are project files or package configuration files for TheIDE development software. These files contain a file list and a list of required packages. The required packages list indicates where to download the package's ".upp" file (when the folders to search are known), so the package list is a recursive directory (unless there are circular dependencies).

**So by following the ".upp" files you can figure out the package hierarchy.**


- autotest/
	- Hundreds of test packages for each part of the library (e.g., Core, Xml, Http, Unicode, etc.).
- bazaar/
	- A large collection of separate add‑on packages and sample applications such as AESStream, ChromiumBrowser, CoWork, FSMon, TabBar, and more.
	- Previously, this was packages added by external users that did not provide any Core functionality. Now, you can think of this as a staging area for some of the features that are imported into Core.
- bazaar/plugin
	- external libraries that have been imported into this repository. These have a U++ TheIDE project file, so they are easy to include in projects.
- benchmarks/
	- Performance benchmarks: AllMaps, CoWork, LZ4, PainterBenchmark, VectorBig, etc.
- doc/
	- Contains rather irrelevant documentation files in Markdown format. The code documentation can be found in folders (not files) ending in ".tpp" which contain files ending in ".tpp" in this project's own QTF file format (a bit like RichText).
- examples/
	- Small sample projects (e.g., AddressBook, GoogleMaps, PainterExamples, SQLApp, SvgView).
- reference/
	- Example code and tests for nearly every feature (e.g., AESEncryption, ArrayCtrlDnD, CoWorkCancel, HttpServer, Xmlize).
- tutorial/
	- Step‑by‑step tutorial packages: Gui01–Gui24, Draw01–Draw06, Sql01–Sql06, Network01–02, etc.
- uppsrc/
	- The main source tree; contains modules like Core, Draw, CtrlCore, CtrlLib, Painter, PdfDraw, Sql, Terminal, and many others.
- upptst/
	- Test applications; parallel examples such as upptst/DrawArc, upptst/PdfText, upptst/GuiMtTest, upptst/Zip, etc.
- rainbow/
	- Various platform ports and graphical experiments (SDL20, WinFb, Gtk, PaintGl, …).
- uppbox/
	- Tools, scripts, and installation packages (MakeInstall*, WinInstaller, UppBuilder, Unicode, …).
- obsolete/
	- Older versions or removed features, organized into subdirectories (src, tutorial, uppsrc, upptst, etc.).

Additionally, the root directory contains files such as AGENTS.md, LICENSE, README.md, Makefile, configure, and other build/configuration files.


## "uppsrc" directory

### "ide" directory
The ide directory contains sub-packages that complement ide, but which are strongly related to other main packages, but which cannot be moved there because they require the ide package, and it cannot be made a general requirement.

### Repeated parts of the name: Core, Ctrl, Lib, Draw, AI, Meta, Eon, Gubo, Rich:

Core packages contain the most essential code. Core alone is the most essential, and CtrlCore contains all the most essential GUI code. Lib is less essential compared to Core (CtrlCore -> CtrlLib).

#### The "Core -> Draw -> Ctrl" chain (and the "Core2 -> Draw2 -> Ctrl2" chain)
The "Core -> Draw -> Ctrl" division is based on the fact that "Core" code can be run in a headless environment and in a terminal, and it does not contain graphics at all. "Draw" code can be run in a headless environment on a server, but it contains code that is strongly included outside the headless environment and in user applications. "Ctrl" code is for client programs and graphical user interfaces, and it is cross-platform and cross-platform, supporting several user interface libraries (Qt, Gtk, Win32, X11, Cocoa).

#### The "Core2 -> Vfs -> Meta -> AI -> Eon" chain
Core2 (v2) contains very fundamental classes and functions that are also strongly related to Vfs,Meta,AI code, and it would be too much to include in every regular program via the Core (v1) package.

Vfs is strongly associated with AST functions and the Clang parser of the code. However, it is also used for all dynamic data management within the user program and even for C++ metaprogramming.

The Meta folder contains code that is more general than the AI-centric code in the AI ​​folder. Meta contains a lot of code related to the IDE package, where custom GUI tools are made using the Vfs hierarchy, where the tool is related to the location of the Vfs tree, and probably also to the Vfs clang-based AST plugin, meta-programming and its visualization.

The AI ​​folder contains artificial intelligence functions built on the runtime VFS tree, which may have a GUI tool or visualization.

Eon originally stood for ECS Object Notation, but that name doesn't mean that anymore, it's just a nice name for anything ECS ​​related. It also includes code for a game/multimedia engine, where the engine parts are also in a kind of ECS-like world, but where the ECS world of the game is a completely different thing, with different classes. The Eon and ECS code is heavily based on the Vfs code, and also uses artificial intelligence functions, so it is the last in the chain.

#### The "Ctrl -> Gubo" chain
"Gubo" or "Graphical Cuboid" is a class that corresponds to the "Ctrl" libraries (almost one to one), but which is in the 3D world and not in the 2D world. All Ctrl classes can be tried to be converted into "Gubo" classes by adding a "dimension" or converting 2D->3D and taking into account the related issues.

GuboCore may include a wrapper for 3D engines, in the same way that CtrlCore uses gui libraries (Qt, Gnome, etc.). However, a custom engine is essential (which is implemented using Eon's graph structures), and this can be implemented using the VirtualWorld class (as is the VirtualGui package for 2D Ctrl functions). This is a bit uncertain and undefined yet.

#### The "CtrlCore -> CtrlLib" chain

CtrlCore contains all the general and essential structures, which allows the use of different GUI libraries, and which is very essential for GUI-related things.

CtrlLib contains all the specialized GUI-Widget classes and related classes. If a new general GUI widget is to be created, it is added here by default.

CtrlCore does not use widgets from other GUI libraries, only the main window, events and theme. All events are converted to CtrlCore internal events. The GUI library theme is loaded and CtrlCore "disguises" itself as a native GUI by loading the theme to improve the user experience.

#### The "RichText -> RichEdit" chain
Rich text is similar in concept to the RTF file format familiar from Microsoft WordPad, but in this library the information is in QTF files (rather than RTF) and contains slightly different functions.

The RichText package contains classes for viewing QTF text, and the RichEdit package classes can be used to implement a text editor similar to WordPad.

### Versions

Includes versions 1 and 2 of some packages, such as Core, Core2, CtrlLib, CtrlLib2, etc. This is done to freeze certain feature packages. Version 2 includes a more advanced VfsValue class and the ECS library on top of it.

### GridCtrl

Package for Excel -like features.

### ComputerVision

Contains functions similar to the OpenCV library, and can also wrap libraries similar to the OpenCV library. All computer vision and webcam related code is added here. Possibly also functions required by the image processing program.

### DesktopSuite

The VirtualGui function (and the VIRTUALGUI use flag) enables a desktop in e.g. an SDL2 window, and the classes in this package enable an operating system in an SDL2 (or similar) window. Here "Explorer" corresponds to the file manager. "TaskBar" corresponds to the Windows taskbar or OSX Docker. If the package threatens to become too large, sub-packages can be created in the folder, as in the ide package.

### Adventure

The Adventure package includes a SCUMM style game engine. This should use the Vfs,Meta,Eon classes, but it still contains old code.

### Esc, EscAnim

Esc is a scripting language, like JavaScript or ActionScript, but the syntax is different and this is implemented using the conventions and strengths of Ultimate++. This provides a smooth experience for C++ programmers in this repository, as the scripting language resembles familiar conventions as much as possible.

EscAnim corresponds to the functions of Adobe Flash (Adobe Animate) kernel in the way that Esc corresponds to ActionScript. This package contains a 2D graphics engine and classes for displaying graphics and multimedia. If you wanted to implement a program similar to Adobe Animate, then the core functions related to animation would be added here, which would give the user's programs functions similar to "Flash Player".

Functions similar to Adobe Animate itself would be in the EscAnimEdit package, except for the main program. The main program is integrated into the IDE.

## The "api" directory

This is a very important folder. The graph-based Eon engine uses atoms, which are located here. Atoms can wrap system libraries into audio, image, video, friends, etc. Atoms have an interface that defines the input and output ports and the format of the packets passing through them (video, audio, event), etc.

### "api/Audio"

These Atoms can wrap, for example, "portaudio", the native Windows audio libraries, "pipewire".

### "api/AudioHost"

These Atoms can wrap, for example, "VST", "LADSPA", "LV2", "CLAP".

### "api/Camera"

These Atoms can wrap, for example, "V4L", "Windows native camera api", "OpenCV", "Internet camera libraries".

### "api/Effect"

These Atoms can wrap, for example, specific "Reverb effect libraries", "FFT effect libraries", even specific "VST effects".

### "api/Graphics": OpenGL

This contains template functions for an OpenGL based framework. This can implement 3D pipelines that resemble OpenGL. This also includes a software renderer for debugging. Everything needs to work with the software renderer before it works with the native 3D library. Cinematic 3D renderers can also be added here if they are similar to the OpenGL API. DirectX is unlikely to support this, and probably needs its own "Graphics2" folder.

### "api/Graphics2": DirectX
DirectX template classes like in the "Graphics" package. Also includes a software renderer, which can be used to debug DirectX functions. Cinematic 3D renderers can also be added here if they are similar to the DirectX API. 

### "api/Hal": Hardware Abstraction Layer libraries

This can wrap comprehensive libraries such as SDL, SDL2, SDL3, Allegro, SFML. Even wrapping the Internet browser functionality of a WebAssembly target can happen here if the program is compiled with Emscripten.


