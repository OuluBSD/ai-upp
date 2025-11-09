#ifndef _ShaderToy_ShaderToy_h
#define _ShaderToy_ShaderToy_h

#include <Core/Core.h>
#include <GraphLib/GraphLib.h>
#include <GraphLib/GraphNodeCtrl.h>
#include <GLCtrl/GLCtrl.h>
#include <CtrlLib/CtrlLib.h>

NAMESPACE_UPP

// Base class for all editor node types
class EditorNode {
public:
    EditorNode();
    virtual ~EditorNode();

    // Virtual methods for node functionality
    virtual void RenderContent();
    virtual String GetNodeType() const = 0;
    virtual bool ValidateConnection(const EditorNode* other, int thisPin, int otherPin) const;

    // Pin management - using GraphLib's pin system
    void AddInputPin(const String& id, int type = 0);
    void AddOutputPin(const String& id, int type = 0);
    
    // Use GraphLib's pin system by composition
    GraphLib::Node& GetGraphNode() { return graphNode; }
    const GraphLib::Node& GetGraphNode() const { return graphNode; }

private:
    GraphLib::Node graphNode;
};

// Specialized node types
class EditorShader : public EditorNode {
public:
    EditorShader();
    virtual String GetNodeType() const override { return "Shader"; }
    virtual void RenderContent() override;

    // Shader-specific functionality
    void SetCode(const String& code);
    String GetCode() const { return shaderCode; }

private:
    String shaderCode;
    // GLSL code editor widget
};

class EditorTexture : public EditorNode {
public:
    EditorTexture();
    virtual String GetNodeType() const override { return "Texture"; }
    virtual void RenderContent() override;

    // Texture-specific functionality
    void SetImagePath(const String& path);
    String GetImagePath() const { return imagePath; }

private:
    String imagePath;
};

class EditorCubeMap : public EditorNode {
public:
    EditorCubeMap();
    virtual String GetNodeType() const override { return "CubeMap"; }
    virtual void RenderContent() override;

private:
    // Cube map specific properties
};

class EditorVolume : public EditorNode {
public:
    EditorVolume();
    virtual String GetNodeType() const override { return "Volume"; }
    virtual void RenderContent() override;

private:
    // 3D volume specific properties
};

class EditorKeyboard : public EditorNode {
public:
    EditorKeyboard();
    virtual String GetNodeType() const override { return "Keyboard"; }
    virtual void RenderContent() override;

private:
    // Keyboard input specific properties
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

private:
    // Frame feedback specific properties
};

// Link structure for connecting pins between nodes
struct PipelineLink {
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

    // STTF (ShaderToy Transfer Format) support
    bool LoadSTTF(const String& filePath);
    bool SaveSTTF(const String& filePath);

    // Rendering
    void SetRenderTarget(const Image& target);
    Image GetRenderTarget() const { return renderTarget; }

private:
    Image renderTarget;
    Vector<PipelineLink> links;

    // Methods inherited from GraphNodeCtrl
    virtual void LeftDown(Point p, dword key) override;
    virtual void LeftUp(Point p, dword key) override;
    virtual void MouseMove(Point p, dword key) override;
    virtual void RightDown(Point p, dword key) override;
    virtual bool Key(dword key, int count) override;

    void SetNodeRenderFunction();
    bool ValidateConnection(String fromNodeId, String fromPinId, String toNodeId, String toPinId);
};

END_UPP_NAMESPACE

#endif