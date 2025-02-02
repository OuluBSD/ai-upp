#ifndef _ide_LLDB_LLDB_h_
#define _ide_LLDB_LLDB_h_

#include <map>
#include <lldb/API/LLDB.h>
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <algorithm>
#include <cstring>

#include <Core/Core.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "lldb/API/LLDB.h"

// clang-format off
#ifdef flagLINUX
	#include <imgui/imgui.h>
	#include <imgui/imgui_internal.h>
	#include <imgui/imgui_impl_glfw.h>
	#include <imgui/imgui_impl_opengl2.h>
#else
	#include "imgui.h"
	#include "imgui_internal.h"
	#include "imgui_impl_glfw.h"
	#include "imgui_impl_opengl2.h"
#endif
// clang-format on

using namespace Upp;

//#include <cxxopts.hpp>
//#include <fmt/format.h>

#include "LLDBCommandLine.h"
#include "StreamBuffer.h"
#include "FileSystem.h"

#include "Defer.h"
#include "FileViewer.h"
#include "FPSTimer.h"
#include "Log.h"
#include "Timer.h"

#include "Application.h"




#endif
