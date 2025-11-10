#ifndef _ShaderToy_ShaderToy_h
#define _ShaderToy_ShaderToy_h

#include <Core/Core.h>
#include <GraphLib/GraphLib.h>
#include <GraphLib/GraphNodeCtrl.h>
#include <GLCtrl/GLCtrl.h>
#include <CtrlLib/CtrlLib.h>
#include <Painter/Painter.h>

// GLSL Editor Dialog
struct GLSLEditorDialog : public TopWindow {
    EditField          codeEditor;
    Button             compileBtn;
    StaticRect         errorOutput;
    StaticText         title;
    
    typedef GLSLEditorDialog CLASSNAME;
    
    GLSLEditorDialog() {
        Add(title.TopPos(0, 24).HSizePos());
        title <<= "Shader Editor";
        Add(codeEditor.HSizePos().VSizePos(40, 80));
        Add(compileBtn.TopPos(0, 30).LeftPos(0, 100));
        compileBtn.SetLabel("Compile");
        Add(errorOutput.HSizePos().BottomPos(0, 20));
        errorOutput.SetFrame(ThinInsetFrame());
        errorOutput <<= "Ready";
    }
};

NAMESPACE_UPP

// Forward declarations for OpenGL backend
class OpenGLBackend;
class ShaderProgram;
class TextureResource;

// Pin types for shader connections
enum class PinType {
    UV_COORDS = 0,      // UV coordinates
    COLOR = 1,          // Color values
    TIME = 2,           // Time values
    RESOLUTION = 3,     // Resolution values
    TEXTURE = 4,        // Texture data
    CUBEMAP = 5,        // Cube map data
    VOLUME = 6,         // Volume texture data
    KEYBOARD = 7,       // Keyboard input
    FRAME = 8           // Frame feedback
};

// Simple structure for shader pins using U++-compatible types
struct ShaderPin : public Moveable<ShaderPin> {
    String id;
    int type;  // Using int instead of enum for compatibility
    int kind;  // Using int instead of enum for compatibility
    
    ShaderPin() : type(0), kind(0) {}
    ShaderPin(String pinId, int pinType, int pinKind) 
        : id(pinId), type(pinType), kind(pinKind) {}
};

// Base class for all editor node types
class EditorNode {
public:
    EditorNode();
    virtual ~EditorNode();

    // Virtual methods for node functionality
    virtual void RenderContent();
    virtual String GetNodeType() const = 0;
    virtual bool ValidateConnection(const EditorNode* other, int thisPin, int otherPin) const;
    
    // OpenGL-specific functionality
    virtual bool Compile(OpenGLBackend* backend) { return true; }
    virtual bool Execute(OpenGLBackend* backend, const VectorMap<String, Value>& inputs, 
                        VectorMap<String, Value>& outputs) { return true; }

    // Pin management - using GraphLib's pin system
    void AddInputPin(const String& id, PinType type);
    void AddOutputPin(const String& id, PinType type);
    const Vector<ShaderPin>& GetPins() const { return pins; }
    
    // Use GraphLib's pin system by composition
    GraphLib::Node& GetGraphNode() { return graphNode; }
    const GraphLib::Node& GetGraphNode() const { return graphNode; }

private:
    GraphLib::Node graphNode;
    Vector<ShaderPin> pins;  // Track our custom pins
};

// Specialized node types
class EditorShader : public EditorNode {
public:
    EditorShader();
    virtual String GetNodeType() const override { return "Shader"; }
    virtual void RenderContent() override;

    // OpenGL-specific functionality (overriding base class methods)
    virtual bool Compile(OpenGLBackend* backend) override;
    virtual bool Execute(OpenGLBackend* backend, const VectorMap<String, Value>& inputs, 
                        VectorMap<String, Value>& outputs) override;

    // Shader-specific functionality
    void SetCode(const String& code);
    String GetCode() const { return shaderCode; }
    void SetUniform(const String& name, double value);
    void SetUniformVec3(const String& name, double x, double y, double z);
    void SetUniformVec4(const String& name, double x, double y, double z, double w);
    const VectorMap<String, double>& GetUniforms() const { return uniforms; }
    const VectorMap<String, Vector<double>>& GetUniformVec3s() const { return uniformVec3s; }
    const VectorMap<String, Vector<double>>& GetUniformVec4s() const { return uniformVec4s; }
    
    // For UI
    void ShowEditorDialog();
    bool CompileShaderCode(const String& code, String& errorOutput);

private:
    String shaderCode;
    VectorMap<String, double> uniforms;        // Float uniforms
    VectorMap<String, Vector<double>> uniformVec3s; // Vec3 uniforms  
    VectorMap<String, Vector<double>> uniformVec4s; // Vec4 uniforms
    // GLSL code editor functionality
    ShaderProgram* shaderProgram;  // Pointer to compiled shader program
};

class EditorTexture : public EditorNode {
public:
    EditorTexture();
    virtual String GetNodeType() const override { return "Texture"; }
    virtual void RenderContent() override;

    // Texture-specific functionality
    void SetImagePath(const String& path);
    String GetImagePath() const { return imagePath; }
    void SetSamplerType(int type) { samplerType = type; }
    int GetSamplerType() const { return samplerType; }
    Size GetTextureSize() const { return textureSize; }
    void SetTextureSize(Size size) { textureSize = size; }

private:
    String imagePath;
    int samplerType;      // Sampler type (2D, 3D, etc.)
    Size textureSize;     // Texture dimensions
};

class EditorCubeMap : public EditorNode {
public:
    EditorCubeMap();
    virtual String GetNodeType() const override { return "CubeMap"; }
    virtual void RenderContent() override;

    // Cube map-specific functionality
    void SetImagePath(const String& posX, const String& negX, 
                      const String& posY, const String& negY, 
                      const String& posZ, const String& negZ);
    const Vector<String>& GetImagePaths() const { return imagePaths; }
    void SetSamplerType(int type) { samplerType = type; }
    int GetSamplerType() const { return samplerType; }

private:
    Vector<String> imagePaths;  // 6 faces of the cube map: [posX, negX, posY, negY, posZ, negZ]
    int samplerType;              // Sampler type for cube maps
};

class EditorVolume : public EditorNode {
public:
    EditorVolume();
    virtual String GetNodeType() const override { return "Volume"; }
    virtual void RenderContent() override;

    // Volume-specific functionality
    void SetVolumePath(const String& path);
    String GetVolumePath() const { return volumePath; }
    void SetVolumeSize(Size sz) { volumeSize = sz; }
    Size GetVolumeSize() const { return volumeSize; }
    void SetVolumeDepth(int depth) { volumeDepth = depth; }
    int GetVolumeDepth() const { return volumeDepth; }
    void SetSamplerType(int type) { samplerType = type; }
    int GetSamplerType() const { return samplerType; }

private:
    String volumePath;  // Path to volume data
    Size volumeSize;    // 2D volume dimensions (x, y)
    int volumeDepth;    // 3rd dimension of volume (z)
    int samplerType;    // Sampler type for volumes
};

class EditorKeyboard : public EditorNode {
public:
    EditorKeyboard();
    virtual String GetNodeType() const override { return "Keyboard"; }
    virtual void RenderContent() override;

    // Keyboard-specific functionality
    void SetKeyMapping(const String& keyName, int keyCode);
    const VectorMap<String, int>& GetKeyMappings() const { return keyMappings; }
    void SetOutputType(int type) { outputType = type; }  // 0=raw, 1=processed
    int GetOutputType() const { return outputType; }

private:
    VectorMap<String, int> keyMappings;  // Key name to code mapping
    int outputType;                      // Output format type
    
    // Keyboard visualization functionality
    void DrawKeyboardVisualization(Draw& draw, const Rect& rect, const VectorMap<String, bool>& activeKeys);
};

class EditorRenderOutput : public EditorNode {
public:
    EditorRenderOutput();
    virtual String GetNodeType() const override { return "RenderOutput"; }
    virtual void RenderContent() override;

private:
    // Render output specific properties
};

class EditorLastFrame : public EditorNode {
public:
    EditorLastFrame();
    virtual String GetNodeType() const override { return "LastFrame"; }
    virtual void RenderContent() override;

    // Frame feedback-specific functionality
    void SetFrameBufferId(int id) { frameBufferId = id; }
    int GetFrameBufferId() const { return frameBufferId; }
    Size GetFrameSize() const { return frameSize; }
    void SetFrameSize(Size sz) { frameSize = sz; }
    void SetFeedbackDelay(int delay) { feedbackDelay = delay; }  // How many frames to delay
    int GetFeedbackDelay() const { return feedbackDelay; }

private:
    int frameBufferId;      // ID of the frame buffer to use
    Size frameSize;         // Size of the frame buffer
    int feedbackDelay;      // Feedback delay in frames
};

// Link structure for connecting pins between nodes
struct PipelineLink : public Moveable<PipelineLink> {
    String fromNodeId;
    String fromPinId;
    String toNodeId;
    String toPinId;

    PipelineLink(String fromNode, String fromPin, String toNode, String toPin)
        : fromNodeId(fromNode), fromPinId(fromPin),
          toNodeId(toNode), toPinId(toPin) {}
    
    // Add copy constructor to fix compilation error
    PipelineLink(const PipelineLink& other)
        : fromNodeId(other.fromNodeId), fromPinId(other.fromPinId),
          toNodeId(other.toNodeId), toPinId(other.toPinId) {}
};

// Main pipeline editor class
class PipelineEditor : public GraphLib::GraphNodeCtrl {
public:
    PipelineEditor();
    virtual ~PipelineEditor();

    // Node creation methods
    EditorShader* CreateShaderNode(const String& id, Point position);
    EditorTexture* CreateTextureNode(const String& id, Point position);
    EditorCubeMap* CreateCubeMapNode(const String& id, Point position);
    EditorVolume* CreateVolumeNode(const String& id, Point position);
    EditorKeyboard* CreateKeyboardNode(const String& id, Point position);
    EditorRenderOutput* CreateRenderOutputNode(const String& id, Point position);
    EditorLastFrame* CreateLastFrameNode(const String& id, Point position);

    // Pipeline management
    void BuildPipeline();
    void ExecutePipeline();
    void ValidatePipeline();
    
    // Pipeline execution helpers
    Vector<String> TopologicalSort();
    VectorMap<String, Value> GetNodeInputs(const String& nodeId, const VectorMap<String, Value>& nodeOutputs);
    void StoreNodeOutputs(const String& nodeId, const VectorMap<String, Value>& outputs, 
                         VectorMap<String, Value>& allOutputs);

    // STTF (ShaderToy Transfer Format) support
    bool LoadSTTF(const String& filePath);
    bool SaveSTTF(const String& filePath);

    // Rendering
    void SetRenderTarget(const Image& target);
    Image GetRenderTarget() const { return renderTarget; }

private:
    VectorMap<String, EditorNode*> nodeMap;  // Map node IDs to EditorNode instances
    Vector<EditorNode*> nodeInstances;       // Track EditorNode instances for cleanup

private:
    Image renderTarget;
    Vector<PipelineLink> links;
    OpenGLBackend* glBackend;  // OpenGL backend for rendering

    // Methods inherited from GraphNodeCtrl
    virtual void LeftDown(Point p, dword key) override;
    virtual void LeftUp(Point p, dword key) override;
    virtual void MouseMove(Point p, dword key) override;
    virtual void RightDown(Point p, dword key) override;
    virtual bool Key(dword key, int count) override;

    void SetNodeRenderFunction();
    bool ValidateConnection(String fromNodeId, String fromPinId, String toNodeId, String toPinId);
    
    // Link management
    bool ConnectNodes(String fromNodeId, String fromPinId, String toNodeId, String toPinId);
    void DisconnectNodes(String fromNodeId, String fromPinId, String toNodeId, String toPinId);
    const Vector<PipelineLink>& GetLinks() const { return links; }
    
    // Custom drawing functions for node icons
    static void DrawShaderNodeIcon(Draw& draw, const Rect& rect);
    static void DrawTextureNodeIcon(Draw& draw, const Rect& rect);
    static void DrawCubeMapNodeIcon(Draw& draw, const Rect& rect);
    static void DrawVolumeNodeIcon(Draw& draw, const Rect& rect);
    static void DrawKeyboardNodeIcon(Draw& draw, const Rect& rect);
    static void DrawRenderOutputNodeIcon(Draw& draw, const Rect& rect);
    static void DrawLastFrameNodeIcon(Draw& draw, const Rect& rect);
    
    // Context menu functions for different node types
    void OpenShaderNodeContextMenu(const String& nodeId, Point pos);
    void OpenTextureNodeContextMenu(const String& nodeId, Point pos);
    void OpenCubeMapNodeContextMenu(const String& nodeId, Point pos);
    void OpenVolumeNodeContextMenu(const String& nodeId, Point pos);
    void OpenKeyboardNodeContextMenu(const String& nodeId, Point pos);
    void OpenRenderOutputNodeContextMenu(const String& nodeId, Point pos);
    void OpenLastFrameNodeContextMenu(const String& nodeId, Point pos);
    
    // Helper methods for context menu
    String GetNodeIdAtPos(Point p);
    bool IsPointInNode(Point p, const String& nodeId);
};

// OpenGL backend classes for shader compilation and rendering

// Shader compilation and management
class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();
    
    bool Compile(const String& vertexShader, const String& fragmentShader);
    bool Link();
    void Use();
    void SetUniform(const String& name, double value);
    void SetUniformVec3(const String& name, double x, double y, double z);
    void SetUniformVec4(const String& name, double x, double y, double z, double w);
    
    unsigned int GetId() const { return programId; }
    
private:
    unsigned int programId;
    unsigned int vertexShaderId;
    unsigned int fragmentShaderId;
    bool compiled;
    bool linked;
};

// Texture resource management
class TextureResource {
public:
    TextureResource();
    ~TextureResource();
    
    bool LoadFromFile(const String& path);
    bool CreateFromData(Size size, const void* data, int channels = 4);
    void Bind(unsigned int unit = 0);
    void Unbind();
    
    Size GetSize() const { return size; }
    unsigned int GetId() const { return textureId; }
    
private:
    unsigned int textureId;
    Size size;
    bool loaded;
};

// Main OpenGL backend class
class OpenGLBackend {
public:
    OpenGLBackend();
    ~OpenGLBackend();
    
    // Shader management
    ShaderProgram* CreateShaderProgram(const String& vertexCode, const String& fragmentCode);
    
    // Texture management
    TextureResource* CreateTexture();
    TextureResource* LoadTexture(const String& path);
    
    // Framebuffer operations
    void SetRenderTarget(TextureResource* texture);
    void SetDefaultRenderTarget();
    
    // Rendering operations
    void Clear();
    void Present();
    
    // Error handling
    String GetLastError() const { return lastError; }
    bool HasError() const { return !lastError.IsEmpty(); }
    
private:
    String lastError;
    Vector<ShaderProgram*> shaderPrograms;
    Vector<TextureResource*> textures;
};

END_UPP_NAMESPACE

#endif