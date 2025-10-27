#include <Core/Core.h>
#include <Vfs/Core/Core.h>
#include <Vfs/Overlay/Overlay.h>

using namespace Upp;

// Simple test to verify basic overlay functionality
void TestBasicOverlayFunctionality() {
    // This is a minimal test to verify that the overlay headers compile correctly
    // and that basic types are accessible
    
    // Test that we can create a SourceRef
    SourceRef source;
    
    // Test that we can access overlay types
    VfsOverlay* overlay = nullptr;
    
    // Test that we can access the MetaEnvironment
    MetaEnvironment& env = MetaEnv();
    
    // Just verify that these compile without errors
    // In a real implementation, we would do actual tests
}

CONSOLE_APP_MAIN
{
    // Run the basic test
    TestBasicOverlayFunctionality();
    
    // If we get here, the basic overlay functionality compiles
    Cout() << "Basic overlay functionality compiles successfully\n";
}