#include "VfsShell.h"

// Bridge TheIDE's headless command-line handlers into the VfsShell binary so
// console shells can reuse the same logic without linking against the full
// IDE application.
#include <ide/CommandLineHandler.cpp>
