#include "Graph.h"

NAMESPACE_SAGRAPH_BEGIN

int Graph::AddNode(Node* node) {
    int idx = nodes_.GetCount();
    nodes_.Add(node);
    return idx;
}

void Graph::Connect(int from, int to, int from_port, int to_port, float gain) {
    Edge e; e.from = from; e.to = to; e.from_port = from_port; e.to_port = to_port; e.gain = gain;
    edges_.Add(e);
}

bool Graph::Compile(String& error) {
    // Kahn's algorithm for topological sorting.
    int n = nodes_.GetCount();
    Vector<Vector<int>> adj(n);
    Vector<int> indeg(n, 0);
    for(const auto& e : edges_) {
        adj[e.from].Add(e.to);
        indeg[e.to]++;
    }
    Vector<int> q;
    for(int i = 0; i < n; ++i) if(indeg[i] == 0) q.Add(i);
    order_.Clear();
    int visited = 0;
    while(!q.IsEmpty()) {
        int u = q.Top(); q.Drop();
        visited++;
        CompiledNode cn; cn.node = ~nodes_[u];
        // Gather input indices for u
        for(const auto& e : edges_) if(e.to == u) cn.inputs.Add(e.from);
        order_.Add(cn);
        for(int v : adj[u]) { if(--indeg[v] == 0) q.Add(v); }
    }
    if(visited != n) { error = "Graph has cycles (not supported yet)"; order_.Clear(); return false; }
    return true;
}

void Graph::Prepare(const ProcessContext& ctx) {
    ctx_ = ctx;
    for(auto& cn : order_) {
        cn.node->Prepare(ctx_);
        // set output buffer channels according to node spec if provided (0=dynamic, default to 2)
        int ch = cn.node->GetOutputSpec(0).channels;
        cn.output.SetSize(ctx_.block_size, ch ? ch : 2);
    }
}

void Graph::ProcessBlock() {
    // For each node in order, collect input bus pointers, process, and store into its output bus.
    for(int i = 0; i < order_.GetCount(); ++i) {
        CompiledNode& cn = order_[i];
        Vector<Bus*> inputs;
        for(int src_idx : cn.inputs) {
            // find compiled node index for src_idx
            // order_ is in topo order; src might appear before current i
            // We can map node* to compiled index; for simplicity, scan.
            for(int j = 0; j < order_.GetCount(); ++j) {
                if(order_[j].node == ~nodes_[src_idx]) { inputs.Add(&order_[j].output); break; }
            }
        }
        cn.output.SetSize(ctx_.block_size, cn.output.channels ? cn.output.channels : 2);
        cn.node->Process(ctx_, inputs, cn.output);
    }
    ctx_.frame_cursor += ctx_.block_size;
}

bool Graph::SetParam(int node_index, const String& id, double value) {
    if(node_index < 0 || node_index >= nodes_.GetCount()) return false;
    Node* n = ~nodes_[node_index];
    if(!n) return false;
    return n->SetParam(id, value);
}

NAMESPACE_SAGRAPH_END
