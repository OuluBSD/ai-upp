#include "NodeEditor.h"

class NodeEditorApp : public TopWindow {
private:
    GraphNodeCtrl nodeEditor;
    
public:
    NodeEditorApp();
    void AddSampleNodes();
    void AddSampleLinks();
    
    typedef NodeEditorApp CLASSNAME;
};

NodeEditorApp::NodeEditorApp() {
    Title("Node Editor Reference Implementation");
    Sizeable().Zoomable();
    
    // Add the node editor control to the window
    Add(nodeEditor.SizePos());
    
    // Add some sample nodes and connections
    AddSampleNodes();
    AddSampleLinks();
}

void NodeEditorApp::AddSampleNodes() {
    // Add a few sample nodes with pins
    Node& node1 = nodeEditor.AddNode("Source", Point(100, 100));
    node1.SetLabel("Data Source");
    node1.SetFill(LtBlue());
    
    // Add input and output pins to the first node
    node1.AddPin("out1", PinKind::Output, 0);  // Output data
    node1.AddPin("out2", PinKind::Output, 1);  // Output signal
    
    Node& node2 = nodeEditor.AddNode("Processor", Point(300, 80));
    node2.SetLabel("Processor");
    node2.SetFill(LtGreen());
    
    // Add input and output pins to the second node
    node2.AddPin("in1", PinKind::Input, 0);   // Input data
    node2.AddPin("out", PinKind::Output, 0);  // Output processed data
    
    Node& node3 = nodeEditor.AddNode("Output", Point(500, 120));
    node3.SetLabel("Output");
    node3.SetFill(LtRed());
    
    // Add input pin to the third node
    node3.AddPin("in", PinKind::Input, 0);    // Input processed data
}

void NodeEditorApp::AddSampleLinks() {
    // Connect the nodes via their pins
    nodeEditor.AddEdge("Source", "out1", "Processor", "in1", 1.0);
    nodeEditor.AddEdge("Processor", "out", "Output", "in", 1.0);
}

GUI_APP_MAIN {
    NodeEditorApp().Run();
}