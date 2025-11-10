#include "ShaderToy.h"
#include <GL/glew.h>  // OpenGL Extension Wrangler
#include <GL/gl.h>    // OpenGL library

#ifdef PLATFORM_WIN32
#include <windows.h>
#endif

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
    // Create a simple visual representation for Shader nodes
    // In a real implementation, this would show a preview of the shader output
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
    // Try to load and display the texture if available
    if (!imagePath.IsEmpty()) {
        // For this implementation, we'll simulate showing a texture preview
        // by drawing a placeholder that indicates the texture is loaded
        
        // This would need to be implemented within the actual GraphLib node drawing system
        // For now, we're just setting up the functionality to eventually support it
    }
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
    // Show a representation of the cube map faces
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
    // Show a representation of the 3D volume
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
    // Show a representation of keyboard states or key mappings
    // For now, we'll just visualize the key mappings that are set
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
    // Show the final rendered output
}

// EditorLastFrame implementation
EditorLastFrame::EditorLastFrame() 
    : frameBufferId(0), frameSize(Size(800, 600)), feedbackDelay(1) {
    AddOutputPin("LastFrame", PinType::FRAME); // Previous frame texture output
}

void EditorLastFrame::RenderContent() {
    // Render last frame preview
    // Show the previous frame texture
}

// EditorKeyboard visualization implementation
void EditorKeyboard::DrawKeyboardVisualization(Draw& draw, const Rect& rect, const VectorMap<String, bool>& activeKeys) {
    // Draw a visual representation of the keyboard with active keys highlighted
    int keyWidth = rect.GetWidth() / 15;  // Approximate number of keys per row
    int keyHeight = rect.GetHeight() / 5; // Number of rows in a simplified keyboard
    int margin = 2;
    
    // Define a simplified keyboard layout
    String keyLayout[5][15] = {
        {"ESC", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "PRT", "DEL"},
        {"~", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "BKSP", "HOME"},
        {"TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "\\", "PGUP"},
        {"CAPS", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "ENTER", "PGDN", "END"},
        {"SHIFT", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "UP", "SHIFT", "RGT", "LFT"}
    };
    
    // Draw the keyboard
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 15; col++) {
            if (keyLayout[row][col] != "") {
                Rect keyRect(
                    rect.left + col * keyWidth + margin,
                    rect.top + row * keyHeight + margin,
                    rect.left + (col + 1) * keyWidth - margin,
                    rect.top + (row + 1) * keyHeight - margin
                );
                
                // Check if this key is active
                Color keyColor = activeKeys.Get(keyLayout[row][col], false) ? Red() : White();
                draw.DrawRect(keyRect, keyColor);
                draw.DrawRect(keyRect, Black());
                
                // Draw key label
                draw.DrawText(keyRect.left + 2, keyRect.top + 2, 
                             keyLayout[row][col], StdFont(), Black());
            }
        }
    }
}

// PipelineEditor implementation
PipelineEditor::PipelineEditor() : glBackend(nullptr) {
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
    
    // Clean up OpenGL backend
    if (glBackend) {
        delete glBackend;
        glBackend = nullptr;
    }
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

// Helper method to find node ID at a specific position
String PipelineEditor::GetNodeIdAtPos(Point p) {
    // In a real implementation, this would check which node contains the point p
    // For now, we'll return an empty string to indicate no specific node was found
    // This would need to use the GraphLib's internal positioning system
    
    // Iterate through all nodes to check if point p is within any node's bounds
    for (int i = 0; i < nodeMap.GetCount(); i++) {
        String nodeId = nodeMap.GetKey(i);
        // Using GraphLib::GraphNodeCtrl's internal node representation
        // to check if point p is within the node's bounds
        if (IsPointInNode(p, nodeId)) { // This would need to be a real method
            return nodeId;
        }
    }
    return String();
}

bool PipelineEditor::IsPointInNode(Point p, const String& nodeId) {
    // This method would determine if a point is within a node's bounds
    // For now, returning false as the implementation would depend on GraphLib internals
    return false;
}

void PipelineEditor::RightDown(Point p, dword key) {
    // Determine which node was right-clicked, if any
    String clickedNodeId = GetNodeIdAtPos(p);
    if (!clickedNodeId.IsEmpty()) {
        // Get the node type and open appropriate context menu
        EditorNode* node = nodeMap.Get(clickedNodeId, nullptr);
        if (node) {
            String nodeType = node->GetNodeType();
            if (nodeType == "Shader") {
                OpenShaderNodeContextMenu(clickedNodeId, p);
            } else if (nodeType == "Texture") {
                OpenTextureNodeContextMenu(clickedNodeId, p);
            } else if (nodeType == "CubeMap") {
                OpenCubeMapNodeContextMenu(clickedNodeId, p);
            } else if (nodeType == "Volume") {
                OpenVolumeNodeContextMenu(clickedNodeId, p);
            } else if (nodeType == "Keyboard") {
                OpenKeyboardNodeContextMenu(clickedNodeId, p);
            } else if (nodeType == "RenderOutput") {
                OpenRenderOutputNodeContextMenu(clickedNodeId, p);
            } else if (nodeType == "LastFrame") {
                OpenLastFrameNodeContextMenu(clickedNodeId, p);
            }
        }
    } else {
        // If no node was clicked, show a simple message box instead of context menu
        // The proper PopupMenu implementation would require more specific U++ knowledge
        PromptOK("Right-clicked on background");
    }
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

    // Create or retrieve the OpenGL backend
    if (!glBackend) {
        glBackend = new OpenGLBackend();
    }

    // Compile all shader nodes
    for (int i = 0; i < nodeInstances.GetCount(); i++) {
        EditorNode* node = nodeInstances[i];
        node->Compile(glBackend);
    }
}

void PipelineEditor::ExecutePipeline() {
    // Execute the rendering pipeline
    // Process nodes in execution order following the topological sort
    
    if (!glBackend) {
        return; // No OpenGL backend available
    }

    // Determine execution order using topological sort
    Vector<String> executionOrder = TopologicalSort();
    
    // Execute nodes in the determined order
    VectorMap<String, Value> nodeOutputs; // Store outputs from each node
    
    for (const String& nodeId : executionOrder) {
        EditorNode* node = nodeMap.Get(nodeId, nullptr);
        if (!node) continue;
        
        // Get inputs for this node by checking connections
        VectorMap<String, Value> inputs = GetNodeInputs(nodeId, nodeOutputs);
        
        // Execute the node
        VectorMap<String, Value> outputs; // Placeholder for outputs
        bool success = node->Execute(glBackend, inputs, outputs);
        
        if (success) {
            // Store the outputs for use by downstream nodes
            StoreNodeOutputs(nodeId, outputs, nodeOutputs);
        }
    }
}

Vector<String> PipelineEditor::TopologicalSort() {
    Vector<String> result;
    Vector<String> allNodes;
    
    // Get all node IDs
    for (int i = 0; i < nodeMap.GetCount(); i++) {
        allNodes.Add(nodeMap.GetKey(i));
    }
    
    // Simple topological sort implementation
    Vector<bool> visited(allNodes.GetCount(), false);
    
    std::function<void(int)> visit = [&](int nodeIndex) {
        if (visited[nodeIndex]) return;
        visited[nodeIndex] = true;
        
        // Visit all nodes that this node connects to
        String currentNodeId = allNodes[nodeIndex];
        for (const PipelineLink& link : links) {
            if (link.fromNodeId == currentNodeId) {
                int nextNodeIndex = allNodes.GetIndex(link.toNodeId);
                if (nextNodeIndex >= 0 && nextNodeIndex < allNodes.GetCount()) {
                    visit(nextNodeIndex);
                }
            }
        }
        
        result.Add(currentNodeId);
    };
    
    for (int i = 0; i < allNodes.GetCount(); i++) {
        if (!visited[i]) {
            visit(i);
        }
    }
    
    // Reverse the result to get correct execution order
    Vector<String> sortedResult;
    for (int i = result.GetCount() - 1; i >= 0; i--) {
        sortedResult.Add(result[i]);
    }
    
    return sortedResult;
}

VectorMap<String, Value> PipelineEditor::GetNodeInputs(const String& nodeId, const VectorMap<String, Value>& nodeOutputs) {
    VectorMap<String, Value> inputs;
    
    // Find all links that connect TO this node
    for (const PipelineLink& link : links) {
        if (link.toNodeId == nodeId) {
            // Find the output value from the source node
            Value inputValue = nodeOutputs.Get(link.fromNodeId, Value());
            inputs.GetAdd(link.toPinId) = inputValue; // Connect to the specific input pin
        }
    }
    
    return inputs;
}

void PipelineEditor::StoreNodeOutputs(const String& nodeId, const VectorMap<String, Value>& outputs, 
                                      VectorMap<String, Value>& allOutputs) {
    // Store all outputs from this node
    for (int i = 0; i < outputs.GetCount(); i++) {
        allOutputs.GetAdd(nodeId) = outputs[i];
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

// ShaderProgram implementation
ShaderProgram::ShaderProgram() : programId(0), vertexShaderId(0), fragmentShaderId(0), compiled(false), linked(false) {
}

ShaderProgram::~ShaderProgram() {
    if (programId) {
        glDeleteProgram(programId);
    }
    if (vertexShaderId) {
        glDeleteShader(vertexShaderId);
    }
    if (fragmentShaderId) {
        glDeleteShader(fragmentShaderId);
    }
}

bool ShaderProgram::Compile(const String& vertexShader, const String& fragmentShader) {
    // Create shader objects
    vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile vertex shader
    const char* vsSource = vertexShader;
    glShaderSource(vertexShaderId, 1, &vsSource, NULL);
    glCompileShader(vertexShaderId);

    // Check for vertex shader compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        return false;
    }

    // Compile fragment shader
    const char* fsSource = fragmentShader;
    glShaderSource(fragmentShaderId, 1, &fsSource, NULL);
    glCompileShader(fragmentShaderId);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShaderId, 512, NULL, infoLog);
        return false;
    }

    compiled = true;
    return true;
}

bool ShaderProgram::Link() {
    if (!compiled) {
        return false;
    }

    // Create shader program
    programId = glCreateProgram();

    // Attach shaders
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    // Link the program
    glLinkProgram(programId);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programId, 512, NULL, infoLog);
        return false;
    }

    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);

    linked = true;
    return true;
}

void ShaderProgram::Use() {
    if (linked) {
        glUseProgram(programId);
    }
}

void ShaderProgram::SetUniform(const String& name, double value) {
    if (linked) {
        glUniform1f(glGetUniformLocation(programId, name), (float)value);
    }
}

void ShaderProgram::SetUniformVec3(const String& name, double x, double y, double z) {
    if (linked) {
        glUniform3f(glGetUniformLocation(programId, name), (float)x, (float)y, (float)z);
    }
}

void ShaderProgram::SetUniformVec4(const String& name, double x, double y, double z, double w) {
    if (linked) {
        glUniform4f(glGetUniformLocation(programId, name), (float)x, (float)y, (float)z, (float)w);
    }
}

// TextureResource implementation
TextureResource::TextureResource() : textureId(0), size(Size(0, 0)), loaded(false) {
}

TextureResource::~TextureResource() {
    if (textureId) {
        glDeleteTextures(1, &textureId);
    }
}

bool TextureResource::LoadFromFile(const String& path) {
    // For now, we'll just create a placeholder implementation
    // In a real implementation, we'd load image data from the file
    // and upload it to the GPU
    
    // Generate texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Placeholder texture data (white 1x1 texture)
    unsigned char data[] = { 255, 255, 255, 255 }; // White pixel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    size = Size(1, 1);
    loaded = true;
    
    return true;
}

bool TextureResource::CreateFromData(Size size, const void* data, int channels) {
    // Generate texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Determine format based on channels
    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, size.cx, size.cy, 0, format, GL_UNSIGNED_BYTE, data);
    
    this->size = size;
    loaded = true;
    
    return true;
}

void TextureResource::Bind(unsigned int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void TextureResource::Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

// OpenGLBackend implementation
OpenGLBackend::OpenGLBackend() {
}

OpenGLBackend::~OpenGLBackend() {
    // Clean up all shader programs
    for (ShaderProgram* sp : shaderPrograms) {
        delete sp;
    }
    shaderPrograms.Clear();
    
    // Clean up all textures
    for (TextureResource* tr : textures) {
        delete tr;
    }
    textures.Clear();
}

ShaderProgram* OpenGLBackend::CreateShaderProgram(const String& vertexCode, const String& fragmentCode) {
    ShaderProgram* sp = new ShaderProgram();
    if (!sp->Compile(vertexCode, fragmentCode) || !sp->Link()) {
        delete sp;
        return nullptr;
    }
    
    shaderPrograms.Add(sp);
    return sp;
}

TextureResource* OpenGLBackend::CreateTexture() {
    TextureResource* tr = new TextureResource();
    textures.Add(tr);
    return tr;
}

TextureResource* OpenGLBackend::LoadTexture(const String& path) {
    TextureResource* tr = new TextureResource();
    if (!tr->LoadFromFile(path)) {
        delete tr;
        return nullptr;
    }
    
    textures.Add(tr);
    return tr;
}

void OpenGLBackend::SetRenderTarget(TextureResource* texture) {
    if (texture) {
        // In a complete implementation, we would bind a framebuffer object
        // that has the texture attached as color buffer
        texture->Bind();
    }
}

void OpenGLBackend::SetDefaultRenderTarget() {
    // Bind default framebuffer (the screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLBackend::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLBackend::Present() {
    // In a complete implementation, we would swap buffers
    // This is typically done at the window level, not in the backend itself
}

// Update EditorShader implementation to handle Compile and Execute methods
bool EditorShader::Compile(OpenGLBackend* backend) {
    if (shaderCode.IsEmpty()) {
        return false; // No shader code to compile
    }

    // Define a simple vertex shader that outputs UV coordinates for fragment shader
    String vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    // Use the user's shader code as fragment shader
    String fullFragmentShader = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform float iTime;
        uniform vec3 iResolution;
    )" + shaderCode + R"(
        void main() {
            mainImage(FragColor, TexCoord * iResolution.xy);
        }
    )";

    shaderProgram = backend->CreateShaderProgram(vertexShader, fullFragmentShader);
    return shaderProgram != nullptr;
}

bool EditorShader::Execute(OpenGLBackend* backend, const VectorMap<String, Value>& inputs, 
                          VectorMap<String, Value>& outputs) {
    if (!shaderProgram) {
        return false;
    }

    shaderProgram->Use();

    // Set uniforms from the node's uniform maps
    for (int i = 0; i < uniforms.GetCount(); i++) {
        shaderProgram->SetUniform(uniforms.GetKey(i), uniforms[i]);
    }

    for (int i = 0; i < uniformVec3s.GetCount(); i++) {
        const Vector<double>& vec = uniformVec3s[i];
        if (vec.GetCount() >= 3) {
            shaderProgram->SetUniformVec3(uniformVec3s.GetKey(i), vec[0], vec[1], vec[2]);
        }
    }

    for (int i = 0; i < uniformVec4s.GetCount(); i++) {
        const Vector<double>& vec = uniformVec4s[i];
        if (vec.GetCount() >= 4) {
            shaderProgram->SetUniformVec4(uniformVec4s.GetKey(i), vec[0], vec[1], vec[2], vec[3]);
        }
    }

    // Get time and resolution from inputs if available
    int timeIndex = inputs.Find("Time");
    if (timeIndex >= 0) {
        Value timeValue = inputs[timeIndex];
        shaderProgram->SetUniform("iTime", timeValue);
    }
    
    int resolutionIndex = inputs.Find("Resolution");
    if (resolutionIndex >= 0) {
        // We would need to extract the resolution from the Value
        // For now assuming it's a Vec3 representation
        // shaderProgram->SetUniformVec3("iResolution", resX, resY, resZ);
    }

    // Here we would render a quad covering the screen/output texture
    // For now, just return true indicating successful execution
    return true;
}

void EditorShader::ShowEditorDialog() {
    // Create a modal dialog for editing shader code
    TopWindow dlg;
    dlg.SetRect(0, 0, 800, 600);
    dlg.SetTitle("Shader Editor");
    
    // Use a simple LineEdit for the shader code editor
    LineEdit editor;
    editor.Set(shaderCode);  // Set current shader code
    editor.SetFont(Monospace(12));
    
    // Add a compile button
    Button compileBtn;
    compileBtn.SetLabel("Compile");
    
    // Position controls
    dlg.Add(editor.VSizePos(40).HSizePos());
    dlg.Add(compileBtn.TopPos(0, 30).LeftPos(0, 100));
    
    // Lambda to handle compilation
    auto compileLambda = [&]() {
        String newCode = editor.Get();
        SetCode(newCode);  // Update the shader code
        
        // In a real implementation, we'd use the OpenGL backend to compile
        // For now, just show a simple message
        PromptOK("Shader code updated!");
    };
    
    // Set the compile button's action
    compileBtn.WhenAction = compileLambda;
    
    // Run the dialog
    dlg.Run();
    
    // Update the shader code when dialog closes
    String newCode = editor.Get();
    SetCode(newCode);
}

// Implementation of custom drawing functions for node icons
void PipelineEditor::DrawShaderNodeIcon(Draw& draw, const Rect& rect) {
    // Draw a shader-related icon (e.g., a triangle or shader symbol)
    draw.DrawRect(rect, White());
    draw.DrawText(rect.left + 5, rect.top + 5, "S", StdFont(), Black());
}

void PipelineEditor::DrawTextureNodeIcon(Draw& draw, const Rect& rect) {
    // Draw a texture-related icon (e.g., checkerboard pattern)
    draw.DrawRect(rect, White());
    // Draw a simple checkerboard pattern
    Color c1 = SColorFace();
    Color c2 = GrayColor();
    int size = rect.GetWidth() / 4;
    
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            Color c = ((x + y) % 2 == 0) ? c1 : c2;
            Rect r(rect.left + x * size, rect.top + y * size, 
                   rect.left + (x + 1) * size, rect.top + (y + 1) * size);
            draw.DrawRect(r, c);
        }
    }
}

void PipelineEditor::DrawCubeMapNodeIcon(Draw& draw, const Rect& rect) {
    // Draw a cube map icon (a cube)
    draw.DrawRect(rect, White());
    // Draw a simple cube representation
    int margin = rect.GetWidth() / 8;
    Rect cubeRect = rect.Inflated(-margin);
    
    // Draw cube outline
    draw.DrawRect(cubeRect, Black());
    
    // Draw cube edges to give it a 3D appearance
    Point top1 = Point(cubeRect.left + margin, cubeRect.top + margin);
    Point top2 = Point(cubeRect.right - margin, cubeRect.top + margin);
    Point top3 = Point(cubeRect.left + margin/2, cubeRect.top + margin/2);
    
    draw.DrawLine(top1.x, top1.y, top3.x, top3.y, 1, Black());
    draw.DrawLine(top2.x, top2.y, top2.x - margin, top3.y, 1, Black());
    draw.DrawLine(top3.x, top3.y + cubeRect.GetHeight(), top3.x + margin, top3.y + cubeRect.GetHeight() - margin, 1, Black());
}

void PipelineEditor::DrawVolumeNodeIcon(Draw& draw, const Rect& rect) {
    // Draw a volume texture icon (a 3D volume representation)
    draw.DrawRect(rect, White());
    // Draw a cube with inner slices to represent volume
    int margin = rect.GetWidth() / 8;
    Rect volumeRect = rect.Inflated(-margin);
    
    // Draw outer cube
    draw.DrawRect(volumeRect, Black());
    
    // Draw inner slices
    int slices = 3;
    int sliceHeight = volumeRect.GetHeight() / slices;
    for (int i = 1; i < slices; i++) {
        int y = volumeRect.top + i * sliceHeight;
        draw.DrawLine(volumeRect.left, y, volumeRect.right, y, 1, Gray());
    }
}

void PipelineEditor::DrawKeyboardNodeIcon(Draw& draw, const Rect& rect) {
    // Draw a keyboard icon
    draw.DrawRect(rect, White());
    // Draw a simplified keyboard with few keys
    int keyWidth = rect.GetWidth() / 8;
    int keyHeight = rect.GetHeight() / 3;
    
    // Draw a few keyboard keys
    for (int i = 0; i < 7; i++) {
        Rect keyRect(rect.left + i * keyWidth + 2, rect.top + 2, 
                     rect.left + (i + 1) * keyWidth - 2, rect.top + keyHeight - 2);
        draw.DrawRect(keyRect, Gray());
        draw.DrawRect(keyRect, Black());
    }
}

void PipelineEditor::DrawRenderOutputNodeIcon(Draw& draw, const Rect& rect) {
    // Draw a render output icon (e.g., a screen or monitor shape)
    draw.DrawRect(rect, White());
    // Draw a screen shape
    Rect screenRect = rect.Inflated(-4);
    draw.DrawRect(screenRect, Black());
    
    // Add a base for the monitor
    Rect base = Rect(screenRect.left + screenRect.GetWidth()/3, screenRect.bottom,
                     screenRect.right - screenRect.GetWidth()/3, screenRect.bottom + rect.GetHeight()/5);
    draw.DrawRect(base, Black());
}

void PipelineEditor::DrawLastFrameNodeIcon(Draw& draw, const Rect& rect) {
    // Draw a last frame icon (e.g., a rewind symbol or clock)
    draw.DrawRect(rect, White());
    // Draw a clock-like symbol
    draw.DrawEllipse(rect, Black());
    
    // Draw clock hands
    Point center(rect.left + rect.GetWidth()/2, rect.top + rect.GetHeight()/2);
    Point hourHand(center.x, center.y - rect.GetHeight()/4);
    Point minuteHand(center.x + rect.GetWidth()/5, center.y);
    
    draw.DrawLine(center.x, center.y, hourHand.x, hourHand.y, 1, Black());
    draw.DrawLine(center.x, center.y, minuteHand.x, minuteHand.y, 1, Black());
}

// Implementation of context menu functions for different node types
void PipelineEditor::OpenShaderNodeContextMenu(const String& nodeId, Point pos) {
    // For now, just call the editor directly rather than showing a menu
    EditorShader* shaderNode = dynamic_cast<EditorShader*>(nodeMap.Get(nodeId, nullptr));
    if (shaderNode) {
        shaderNode->ShowEditorDialog();
    }
}

void PipelineEditor::OpenTextureNodeContextMenu(const String& nodeId, Point pos) {
    PromptOK("Texture Node Context Menu");
}

void PipelineEditor::OpenCubeMapNodeContextMenu(const String& nodeId, Point pos) {
    PromptOK("CubeMap Node Context Menu");
}

void PipelineEditor::OpenVolumeNodeContextMenu(const String& nodeId, Point pos) {
    PromptOK("Volume Node Context Menu");
}

void PipelineEditor::OpenKeyboardNodeContextMenu(const String& nodeId, Point pos) {
    PromptOK("Keyboard Node Context Menu");
}

void PipelineEditor::OpenRenderOutputNodeContextMenu(const String& nodeId, Point pos) {
    PromptOK("RenderOutput Node Context Menu");
}

void PipelineEditor::OpenLastFrameNodeContextMenu(const String& nodeId, Point pos) {
    PromptOK("LastFrame Node Context Menu");
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

