#include <CtrlLib/CtrlLib.h>
#include "ShaderToy/ShaderToy.h"

using namespace Upp;

struct ShaderToyExampleApp : public TopWindow {
    typedef ShaderToyExampleApp CLASSNAME;
    PipelineEditor editor;
    
    ShaderToyExampleApp() {
        Add(editor.SizePos());
        Title("ShaderToy Editor Example");
        
        // Create some example nodes
        editor.CreateShaderNode("MainShader", Point(100, 100));
        editor.CreateTextureNode("MainTexture", Point(100, 200));
        editor.CreateRenderOutputNode("Output", Point(300, 150));
    }
    
    virtual void Close() { 
        Destroy(); 
    }
};

GUI_APP_MAIN
{
    ShaderToyExampleApp().Run();
}