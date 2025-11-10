#include "NodeEditor.h"

class NodeEditorApp : public TopWindow {
private:
    GraphNodeCtrl nodeEditor;
    ToolBar toolBar;

public:
    NodeEditorApp();
    void AddSampleNodes();
    void AddSampleLinks();
    void AddSampleGroups();
    void CreateSampleScene();
    void AddSampleAnimations();

    typedef NodeEditorApp CLASSNAME;
    
    virtual void ToolMenu(Bar& bar);
    void OnSave();
    void OnLoad();
    void OnAddNode();
    void OnToggleAnimations();
    void OnFocusSelection();
};

NodeEditorApp::NodeEditorApp() {
    Title("Advanced Node Editor Showcase - GraphLib");
    Sizeable().Zoomable();
    Size(1024, 768); // Set a reasonable default size

    // Add toolbar for demonstrating features
    AddFrame(toolBar);
    toolBar.Set(THISBACK(ToolMenu));

    // Add the node editor control to the window
    Add(nodeEditor.SizePos());

    // Create a complex sample scene to demonstrate all the new features
    CreateSampleScene();
}

void NodeEditorApp::CreateSampleScene() {
    // Add sample nodes
    AddSampleNodes();
    
    // Add sample links
    AddSampleLinks();
    
    // Add sample groups
    AddSampleGroups();
    
    // Add sample animations
    AddSampleAnimations();
}

void NodeEditorApp::AddSampleNodes() {
    // Add a few sample nodes with pins
    Node& node1 = nodeEditor.AddNode("Source", Point(100, 100));
    node1.SetLabel("Data Source")
         .SetFill(LtBlue())
         .SetStroke(2, Black());

    // Add input and output pins to the first node
    node1.AddPin("out1", PinKind::Output, 0);  // Output data
    node1.AddPin("out2", PinKind::Output, 1);  // Output signal

    Node& node2 = nodeEditor.AddNode("Processor", Point(300, 80));
    node2.SetLabel("Processor")
         .SetFill(LtGreen())
         .SetStroke(2, Black());

    // Add input and output pins to the second node
    node2.AddPin("in1", PinKind::Input, 0);   // Input data
    node2.AddPin("out", PinKind::Output, 0);  // Output processed data

    Node& node3 = nodeEditor.AddNode("Output", Point(500, 120));
    node3.SetLabel("Output")
         .SetFill(LtRed())
         .SetStroke(2, Black());

    // Add input pin to the third node
    node3.AddPin("in", PinKind::Input, 0);    // Input processed data

    // Add more sample nodes to demonstrate features
    Node& mathNode = nodeEditor.AddNode("MathOp", Point(300, 200));
    mathNode.SetLabel("Math Operation")
            .SetFill(LtCyan())
            .SetStroke(2, Black());
    mathNode.AddPin("in1", PinKind::Input, 0);
    mathNode.AddPin("in2", PinKind::Input, 1);
    mathNode.AddPin("out", PinKind::Output, 0);

    Node& logicNode = nodeEditor.AddNode("Logic", Point(100, 250));
    logicNode.SetLabel("Logic Gate")
            .SetFill(LtYellow())
            .SetStroke(2, Black());
    logicNode.AddPin("cond", PinKind::Input, 0);
    logicNode.AddPin("action", PinKind::Output, 0);
}

void NodeEditorApp::AddSampleLinks() {
    // Connect the nodes via their pins
    nodeEditor.AddEdge("Source", "out1", "Processor", "in1", 1.0);
    nodeEditor.AddEdge("Processor", "out", "Output", "in", 1.0);
    nodeEditor.AddEdge("Source", "out2", "Logic", "cond", 1.0);
    nodeEditor.AddEdge("Logic", "action", "MathOp", "in1", 1.0);
}

void NodeEditorApp::AddSampleGroups() {
    // Create a group to demonstrate grouping functionality
    GroupNode& group = nodeEditor.AddGroup("ProcessingGroup", "Processing Unit", Point(250, 50), Size(300, 150));
    group.label = "Processing Unit";
    group.header_clr = Color(80, 80, 80);
    group.body_clr = Color(50, 50, 50);
    group.border_clr = Color(120, 120, 120);
    group.border_width = 2;
    
    // Add nodes to the group
    nodeEditor.AddNodeToGroup("Processor", "ProcessingGroup");
    nodeEditor.AddNodeToGroup("MathOp", "ProcessingGroup");
    
    // Create another group
    GroupNode& group2 = nodeEditor.AddGroup("InputOutputGroup", "I/O Unit", Point(50, 50), Size(200, 200));
    group2.header_clr = Color(100, 80, 60);
    group2.body_clr = Color(70, 50, 30);
    group2.border_clr = Color(140, 120, 100);
    
    // Add nodes to the second group
    nodeEditor.AddNodeToGroup("Source", "InputOutputGroup");
    nodeEditor.AddNodeToGroup("Output", "InputOutputGroup");
    nodeEditor.AddNodeToGroup("Logic", "InputOutputGroup");
}

void NodeEditorApp::AddSampleAnimations() {
    // Enable flow animations for demonstration purposes
    nodeEditor.StartEdgeFlowAnimation();
}

void NodeEditorApp::ToolMenu(Bar& bar) {
    bar.Add("Add Node", CtrlImg::new_doc(), THISBACK(OnAddNode));
    bar.Separator();
    bar.Add("Save Layout", CtrlImg::save(), THISBACK(OnSave));
    bar.Add("Load Layout", CtrlImg::open(), THISBACK(OnLoad));
    bar.Separator();
    bar.Add("Toggle Animations", CtrlImg::plus(), THISBACK(OnToggleAnimations));
    bar.Add("Focus", THISBACK(OnFocusSelection));
    bar.Separator();
    bar.Add("Help", CtrlImg::question(), [this]() {
        PromptOK("Node Editor Features:\n"
                 "- Drag nodes around\n"
                 "- Right-click for context menu\n"
                 "- Click and drag pins to connect nodes\n"
                 "- Ctrl+A to select all\n"
                 "- Ctrl+C/V to copy/paste\n"
                 "- Ctrl+Z/X to cut\n"
                 "- Ctrl+F to focus on selection\n"
                 "- Ctrl+Mouse Wheel to zoom\n"
                 "- Alt+Drag to pan\n"
                 "- Shift+Click and drag to box select");
    });
}

void NodeEditorApp::OnSave() {
    String fileName = SelectFileSaveAs("Save Node Layout (*.xml)|*.xml");
    if (!fileName.IsEmpty()) {
        nodeEditor.SaveGraph(fileName);
    }
}

void NodeEditorApp::OnLoad() {
    String fileName = SelectFileOpen("Load Node Layout (*.xml)|*.xml");
    if (!fileName.IsEmpty()) {
        nodeEditor.LoadGraph(fileName);
    }
}

void NodeEditorApp::OnAddNode() {
    // Add a new node at a random position near the center
    Point pos = Point(300 + nodeEditor.GetGraph().GetNodeCount() * 20, 
                      200 + nodeEditor.GetGraph().GetNodeCount() * 10);
    Node& newNode = nodeEditor.AddNode("Node_" + IntStr(nodeEditor.GetGraph().GetNodeCount()), pos);
    newNode.SetLabel("New Node")
           .SetFill(RandomColor());
    newNode.AddPin("input", PinKind::Input, 0);
    newNode.AddPin("output", PinKind::Output, 0);
}

void NodeEditorApp::OnToggleAnimations() {
    // Toggle edge flow animations
    nodeEditor.StartEdgeFlowAnimation();
}

void NodeEditorApp::OnFocusSelection() {
    nodeEditor.FocusOnSelection();
}

GUI_APP_MAIN {
    NodeEditorApp().Run();
}