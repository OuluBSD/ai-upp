#include <CtrlLib/CtrlLib.h>
#include <AI/Logic/TheoremProver.h>

using namespace Upp;
using namespace TheoremProver;

struct TheoremProverCtrl : TopWindow {
    Splitter hsplit;
    DocEdit  input;
    RichTextView output;
    
    void RunProof() {
        String code = input.GetData();
        // TODO: Parse and prove
        output.SetQTF(code); // Placeholder
    }
    
    TheoremProverCtrl() {
        Title("Theorem Prover GUI");
        Sizeable().Zoomable();
        
        hsplit.Horz(input, output);
        hsplit.SetPos(3000);
        
        Add(hsplit.SizePos());
        
        input <<= "P or not P";
    }
};

GUI_APP_MAIN {
    TheoremProverCtrl().Run();
}
