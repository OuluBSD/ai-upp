#ifndef _SoftAudioGraph_GraphCore_h_
#define _SoftAudioGraph_GraphCore_h_

#include "Node.h"
#include "Connection.h"

NAMESPACE_SAGRAPH_BEGIN

class Graph {
public:
    int AddNode(One<Node> node);
    void Connect(int from, int to, int from_port = 0, int to_port = 0, float gain = 1.0f);
    bool Compile(String& error);
    void Prepare(const ProcessContext& ctx);
    void ProcessBlock();
    bool SetParam(int node_index, const String& id, double value);

    // For reference apps: allow writing into a sink node that commits to file/device.
    void SetBlockSize(int bs) { ctx_.block_size = bs; }
    void SetSampleRate(int sr) { ctx_.sample_rate = sr; }

private:
    struct CompiledNode {
        Node* node = nullptr;
        Vector<int> inputs; // indices of upstream nodes
        Bus output;
    };

    Array<Node> nodes_;
    Array<Edge> edges_;
    Array<CompiledNode> order_; // topologically sorted nodes with resolved inputs
    ProcessContext ctx_;
};

NAMESPACE_SAGRAPH_END

#endif
