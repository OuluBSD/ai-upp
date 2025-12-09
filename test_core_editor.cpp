#include "clicore/clicore.h"
#include "clicore/CoreEditor.h"

using namespace Upp;

int main() {
    // Test basic syntax and functionality
    CoreEditor editor;
    
    // Test basic operations
    String error;
    
    // Test creating an editor and basic operations
    const char* test_content = "Hello, World!\nThis is a test.\n";
    editor.Insert(0, test_content);
    
    // Test goto line
    int pos;
    if(editor.GotoLine(2, pos)) {
        // Success
    }
    
    // Test search
    int found_pos;
    if(editor.FindFirst("test", 0, true, found_pos)) {
        // Found
    }
    
    // Test replace all
    int count;
    editor.ReplaceAll("test", "example", true, count);
    
    // Test undo/redo
    editor.Undo();
    editor.Redo();
    
    return 0;
}