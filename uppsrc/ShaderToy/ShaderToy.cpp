#include "ShaderToy.h"

NAMESPACE_UPP

// EditorNode implementation
EditorNode::EditorNode() {
    // Initialize base node functionality
}

EditorNode::~EditorNode() {
    // Clean up resources
}

void EditorNode::RenderContent() {
    // Default implementation - to be overridden by derived classes
}

bool EditorNode::ValidateConnection(const EditorNode* other, int thisPin, int otherPin) const {
    // Validate based on pin types - implementing basic compatibility rules:
    // - Output pins from this node connect to input pins of the other node
    // - Pin types must be compatible
    
    if (thisPin >= pins.GetCount() || otherPin >= other->pins.GetCount()) {
        return false;
    }
    
    const ShaderPin& thisPinInfo = pins[thisPin];
    const ShaderPin& otherPinInfo = other->pins[otherPin];
    
    // Check if pins are compatible (same type)
    // Using the kind values we store: 0 for Input, 1 for Output based on GraphLib::PinKind values
    bool thisIsOutput = (thisPinInfo.kind == 1);  // Output
    bool otherIsInput = (otherPinInfo.kind == 0);  // Input
    
    return (thisIsOutput && otherIsInput && thisPinInfo.type == otherPinInfo.type);
}

void EditorNode::AddInputPin(const String& id, PinType type) {
    // Use GraphLib's pin system
    graphNode.AddPin(id, GraphLib::PinKind::Input, (int)type);
    pins.Add(ShaderPin(id, (int)type, (int)GraphLib::PinKind::Input));
}

void EditorNode::AddOutputPin(const String& id, PinType type) {
    // Use GraphLib's pin system
    graphNode.AddPin(id, GraphLib::PinKind::Output, (int)type);
    pins.Add(ShaderPin(id, (int)type, (int)GraphLib::PinKind::Output));
}

// EditorShader implementation
EditorShader::EditorShader() {
    AddInputPin("UV", PinType::UV_COORDS);      // UV coordinates input
    AddOutputPin("Color", PinType::COLOR);      // Color output
    AddInputPin("Time", PinType::TIME);         // Time input
    AddInputPin("Resolution", PinType::RESOLUTION); // Resolution input
}

void EditorShader::RenderContent() {
    // Render GLSL code editor and shader preview
    // The actual rendering would happen through OpenGL in a real implementation
}

void EditorShader::SetCode(const String& code) {
    shaderCode = code;
}

void EditorShader::SetUniform(const String& name, double value) {
    uniforms.GetAdd(name) = value;
}

void EditorShader::SetUniformVec3(const String& name, double x, double y, double z) {
    Vector<double>& vec = uniformVec3s.GetAdd(name);
    vec.SetCount(3);
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
}

void EditorShader::SetUniformVec4(const String& name, double x, double y, double z, double w) {
    Vector<double>& vec = uniformVec4s.GetAdd(name);
    vec.SetCount(4);
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    vec[3] = w;
}

// EditorTexture implementation
EditorTexture::EditorTexture() 
    : samplerType(0), textureSize(Size(256, 256)) {
    AddOutputPin("Texture", PinType::TEXTURE); // Texture output
}

void EditorTexture::RenderContent() {
    // Render texture preview
}

void EditorTexture::SetImagePath(const String& path) {
    imagePath = path;
}

// EditorCubeMap implementation
EditorCubeMap::EditorCubeMap() 
    : samplerType(0) {
    AddOutputPin("CubeMap", PinType::CUBEMAP); // Cube map texture output
    // Initialize all 6 faces to empty strings
    imagePaths.SetCount(6);
    for(int i = 0; i < 6; i++) {
        imagePaths[i] = String();
    }
}

void EditorCubeMap::RenderContent() {
    // Render cube map preview
}

void EditorCubeMap::SetImagePath(const String& posX, const String& negX, 
                                 const String& posY, const String& negY, 
                                 const String& posZ, const String& negZ) {
    imagePaths[0] = posX;  // +X
    imagePaths[1] = negX;  // -X
    imagePaths[2] = posY;  // +Y
    imagePaths[3] = negY;  // -Y
    imagePaths[4] = posZ;  // +Z
    imagePaths[5] = negZ;  // -Z
}

// EditorVolume implementation
EditorVolume::EditorVolume() 
    : volumeSize(Size(64, 64)), volumeDepth(64), samplerType(0) {
    AddOutputPin("Volume", PinType::VOLUME); // 3D volume output
}

void EditorVolume::RenderContent() {
    // Render 3D volume preview
}

void EditorVolume::SetVolumePath(const String& path) {
    volumePath = path;
}

// EditorKeyboard implementation
EditorKeyboard::EditorKeyboard() 
    : outputType(0) {
    AddOutputPin("Keys", PinType::KEYBOARD); // Keyboard state output
}

void EditorKeyboard::RenderContent() {
    // Render keyboard visualization
}

void EditorKeyboard::SetKeyMapping(const String& keyName, int keyCode) {
    keyMappings.GetAdd(keyName) = keyCode;
}

// EditorRenderOutput implementation
EditorRenderOutput::EditorRenderOutput() {
    AddInputPin("Color", PinType::COLOR); // Color input
}

void EditorRenderOutput::RenderContent() {
    // Render output display
}

// EditorLastFrame implementation
EditorLastFrame::EditorLastFrame() 
    : frameBufferId(0), frameSize(Size(800, 600)), feedbackDelay(1) {
    AddOutputPin("LastFrame", PinType::FRAME); // Previous frame texture output
}

void EditorLastFrame::RenderContent() {
    // Render last frame preview
}

// PipelineEditor implementation
PipelineEditor::PipelineEditor() {
    // Initialize the pipeline editor
    SetNodeRenderFunction();
}

PipelineEditor::~PipelineEditor() {
    // Clean up EditorNode instances
    for(int i = 0; i < nodeInstances.GetCount(); i++) {
        delete nodeInstances[i];
    }
    nodeInstances.Clear();
    nodeMap.Clear();
}

EditorShader* PipelineEditor::CreateShaderNode(const String& id, Point position) {
    GraphLib::Node& graphNode = AddNode(id, position);
    EditorShader* node = new EditorShader();
    
    // Store the EditorNode instance for later cleanup
    nodeInstances.Add(node);
    nodeMap.GetAdd(id) = node;
    
    return node;
}

EditorTexture* PipelineEditor::CreateTextureNode(const String& id, Point position) {
    GraphLib::Node& graphNode = AddNode(id, position);
    EditorTexture* node = new EditorTexture();
    
    // Store the EditorNode instance for later cleanup
    nodeInstances.Add(node);
    nodeMap.GetAdd(id) = node;
    
    return node;
}

EditorCubeMap* PipelineEditor::CreateCubeMapNode(const String& id, Point position) {
    GraphLib::Node& graphNode = AddNode(id, position);
    EditorCubeMap* node = new EditorCubeMap();
    
    // Store the EditorNode instance for later cleanup
    nodeInstances.Add(node);
    nodeMap.GetAdd(id) = node;
    
    return node;
}

EditorVolume* PipelineEditor::CreateVolumeNode(const String& id, Point position) {
    GraphLib::Node& graphNode = AddNode(id, position);
    EditorVolume* node = new EditorVolume();
    
    // Store the EditorNode instance for later cleanup
    nodeInstances.Add(node);
    nodeMap.GetAdd(id) = node;
    
    return node;
}

EditorKeyboard* PipelineEditor::CreateKeyboardNode(const String& id, Point position) {
    GraphLib::Node& graphNode = AddNode(id, position);
    EditorKeyboard* node = new EditorKeyboard();
    
    // Store the EditorNode instance for later cleanup
    nodeInstances.Add(node);
    nodeMap.GetAdd(id) = node;
    
    return node;
}

EditorRenderOutput* PipelineEditor::CreateRenderOutputNode(const String& id, Point position) {
    GraphLib::Node& graphNode = AddNode(id, position);
    EditorRenderOutput* node = new EditorRenderOutput();
    
    // Store the EditorNode instance for later cleanup
    nodeInstances.Add(node);
    nodeMap.GetAdd(id) = node;
    
    return node;
}

EditorLastFrame* PipelineEditor::CreateLastFrameNode(const String& id, Point position) {
    GraphLib::Node& graphNode = AddNode(id, position);
    EditorLastFrame* node = new EditorLastFrame();
    
    // Store the EditorNode instance for later cleanup
    nodeInstances.Add(node);
    nodeMap.GetAdd(id) = node;
    
    return node;
}

void PipelineEditor::LeftDown(Point p, dword key) {
    // Call the base class implementation
    GraphLib::GraphNodeCtrl::LeftDown(p, key);
}

void PipelineEditor::LeftUp(Point p, dword key) {
    // Call the base class implementation
    GraphLib::GraphNodeCtrl::LeftUp(p, key);
}

void PipelineEditor::MouseMove(Point p, dword key) {
    // Call the base class implementation
    GraphLib::GraphNodeCtrl::MouseMove(p, key);
}

void PipelineEditor::RightDown(Point p, dword key) {
    // Call the base class implementation
    GraphLib::GraphNodeCtrl::RightDown(p, key);
}

bool PipelineEditor::Key(dword key, int count) {
    // Call the base class implementation
    return GraphLib::GraphNodeCtrl::Key(key, count);
}

void PipelineEditor::SetNodeRenderFunction() {
    // This function is for setting rendering behavior, but GraphLib handles this differently
    // The rendering is handled by the GraphLib system
}

bool PipelineEditor::ValidateConnection(String fromNodeId, String fromPinId, String toNodeId, String toPinId) {
    // Find the EditorNode objects corresponding to the IDs
    EditorNode* fromNode = nodeMap.Get(fromNodeId, nullptr);
    EditorNode* toNode = nodeMap.Get(toNodeId, nullptr);
    
    if (!fromNode || !toNode) {
        return false;  // Nodes not found
    }
    
    // Find the specific pins by ID
    int fromPinIndex = -1;
    int toPinIndex = -1;
    
    const Vector<ShaderPin>& fromPins = fromNode->GetPins();
    for(int i = 0; i < fromPins.GetCount(); i++) {
        if (fromPins[i].id == fromPinId) {
            fromPinIndex = i;
            break;
        }
    }
    
    const Vector<ShaderPin>& toPins = toNode->GetPins();
    for(int i = 0; i < toPins.GetCount(); i++) {
        if (toPins[i].id == toPinId) {
            toPinIndex = i;
            break;
        }
    }
    
    if (fromPinIndex == -1 || toPinIndex == -1) {
        return false;  // Pins not found
    }
    
    // Validate the connection between these specific pins
    return fromNode->ValidateConnection(toNode, fromPinIndex, toPinIndex);
}

bool PipelineEditor::ConnectNodes(String fromNodeId, String fromPinId, String toNodeId, String toPinId) {
    // Validate the connection first
    if (!ValidateConnection(fromNodeId, fromPinId, toNodeId, toPinId)) {
        return false;
    }
    
    // Add the link to our internal list
    links.Add(PipelineLink(fromNodeId, fromPinId, toNodeId, toPinId));
    
    // Also potentially connect at the GraphLib level
    return true;
}

void PipelineEditor::DisconnectNodes(String fromNodeId, String fromPinId, String toNodeId, String toPinId) {
    // Find and remove the corresponding link
    for(int i = 0; i < links.GetCount(); i++) {
        const PipelineLink& link = links[i];
        if (link.fromNodeId == fromNodeId && link.fromPinId == fromPinId && 
            link.toNodeId == toNodeId && link.toPinId == toPinId) {
            links.Remove(i);
            break;
        }
    }
}

void PipelineEditor::BuildPipeline() {
    // Build the rendering pipeline from the node graph
    // 1. Determine execution order using topological sort
    // 2. Validate all connections
    // 3. Compile shaders if needed
    
    // First validate all connections
    ValidatePipeline();
    
    // In a real implementation, we would:
    // - Topologically sort nodes to determine execution order
    // - Validate that all required inputs are connected
    // - Compile shaders in EditorShader nodes
    // - Prepare texture resources for EditorTexture/EditorCubeMap/EditorVolume nodes
    // For now, just a basic implementation
}

void PipelineEditor::ExecutePipeline() {
    // Execute the rendering pipeline
    // Process nodes in execution order
    
    // In a real implementation, we would:
    // 1. Process nodes in execution order (computed during BuildPipeline)
    // 2. For each node:
    //    - Gather inputs from connected output pins
    //    - Execute node-specific functionality (run shader, load texture, etc.)
    //    - Store outputs to output pins
    // 3. Render the final output using EditorRenderOutput node
    
    // For now, just a basic implementation
    for(int i = 0; i < nodeInstances.GetCount(); i++) {
        nodeInstances[i]->RenderContent();
    }
}

void PipelineEditor::ValidatePipeline() {
    // Validate the entire pipeline for errors
}

bool PipelineEditor::LoadSTTF(const String& filePath) {
    // Load a ShaderToy Transfer Format file
    // Parse the file and reconstruct the node graph
    return true;
}

bool PipelineEditor::SaveSTTF(const String& filePath) {
    // Save the current node graph to STTF format
    return true;
}

void PipelineEditor::SetRenderTarget(const Image& target) {
    renderTarget = target;
}

END_UPP_NAMESPACE

#ifdef flagMAIN
GUI_APP_MAIN {
	using namespace Upp;
	TopWindow tw;
	PipelineEditor ew;
	tw.Add(ew.SizePos());
	tw.Run();
}
#endif

