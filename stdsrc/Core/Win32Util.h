#pragma once
#ifndef _Core_Win32Util_h_
#define _Core_Win32Util_h_

#include <string>
#include "Core.h"

NAMESPACE_UPP

// Cross-platform utility functions

// Get the full path to the currently executing executable
std::string GetExeFilePath();

// Get the directory containing the currently executing executable
std::string GetExeFileDirectory();

// Get the user's home directory
std::string GetHomeDirectory();

// Get the system's temporary directory
std::string GetTempDirectory();

// Check if a file exists at the given path
bool FileExists(const std::string& path);

// Check if the path is a directory
bool IsDirectory(const std::string& path);

// Get the command line used to start this process
std::string GetCommandLine();

// Open a URL in the default browser
void OpenUrl(const std::string& url);

// Get the current process ID
int GetProcessId();

END_UPP_NAMESPACE

#endif