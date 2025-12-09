#ifndef _clicore_CoreSemantic_h_
#define _clicore_CoreSemantic_h_

#include "clicore.h"

class CoreSemantic : public Moveable<CoreSemantic> {
public:
    struct Entity : public Moveable<Entity> {
        String name;        // symbol name
        String kind;        // "class", "function", "enum", ...
        String file;
        int line = -1;

        ValueMap relations; // arbitrary mapping: "calls", "uses_type", "implements", etc.
        ValueMap attributes; // metrics: LOC, complexity, fanout, fannin, etc.

        // NEW: Inferred relation categories (SemanticAssist v2)
        Vector<String> co_occurs_with;  // symbols that frequently appear in the same files
        Vector<String> conceptually_related; // derived from shared patterns, prefixes, or similar call graphs
        String layer_dependency;         // inferred architectural layer ordering (base, core, app)
        String role;                     // possible semantic roles (controller, manager, utility, parser)

        // NEW: Behavior signatures (SemanticAssist v3)
        ValueMap behavior; // behavior signature fields (pure, io_bound, etc.)

        // NEW: Pattern and anti-pattern detection (SemanticAssist v4)
        ValueMap patterns;      // recognized patterns
        ValueMap antipatterns;  // recognized issues
    };

    struct Cluster : public Moveable<Cluster> {
        String name;
        Vector<String> entities;
        ValueMap metrics;
    };

    // NEW: Subsystem detection - moved before it's used in public interface
    struct Subsystem : Moveable<Subsystem> {
        String name;
        Vector<String> entities;
        ValueMap metrics;
    };

    // NEW: Behavioral analysis - Graph for representing subsystem behavior (SemanticAssist v3)
    struct BehaviorGraph : Moveable<BehaviorGraph> {
        Vector<String> nodes;  // entity names
        Vector< Tuple<String, String, String> > edges; // from, to, relation type ("dataflow", "transform", etc.)
    };

    // NEW: Architecture diagnostic structure (SemanticAssist v4)
    struct ArchitectureDiagnostic : Moveable<ArchitectureDiagnostic> {
        Vector<ValueMap> patterns;
        Vector<ValueMap> antipatterns;
        ValueMap scores; // cohesion, coupling, layering, complexity_index, entropy
    };

    CoreSemantic();

    bool AnalyzeWorkspace(CoreWorkspace& ws,
                          CoreAssist& assist,
                          CoreGraph& graph,
                          String& error);

    const Vector<Entity>& GetEntities() const;
    const Vector<Cluster>& GetClusters() const;
    const Vector<Subsystem>& GetSubsystems() const; // NEW: Added getter for subsystems
    BehaviorGraph GetBehaviorGraph() const; // NEW: Added getter for behavior graph
    const ArchitectureDiagnostic& GetArchitectureDiagnostic() const; // NEW: Getter for architecture diagnostic

    Vector<Entity> FindEntitiesByName(const String& pattern) const;
    Vector<Cluster> FindClustersForEntity(const String& name) const;

private:
    Vector<Entity> entities;
    Vector<Cluster> clusters;
    Vector<Subsystem> subsystems; // NEW: Added for SemanticAssist v2
    BehaviorGraph behavior_graph; // NEW: Added for SemanticAssist v3
    ArchitectureDiagnostic diagnostic; // NEW: Added for SemanticAssist v4

    void BuildRelations(const CoreAssist& assist, const CoreGraph& graph);
    void BuildClusters();
    void ComputeAttributes();

    // NEW: SemanticAssist v2 - Inference methods
    void InferCoOccurrence();
    void InferConceptualSimilarity();
    void InferLayering(CoreGraph& graph);
    void InferRoles();

    void BuildSubsystems();

    // NEW: SemanticAssist v3 - Behavioral inference methods
    void InferDataflow(const CoreAssist& assist);
    void InferBehaviorSignatures();
    void InferPipelineStages();

    // NEW: SemanticAssist v3 - Behavioral graph methods
    BehaviorGraph BuildBehaviorGraph() const;

    // NEW: SemanticAssist v4 - Pattern/anti-pattern detection methods
    void DetectPatterns();
    void DetectAntiPatterns();
    void ComputeArchitectureScores();
    ArchitectureDiagnostic BuildArchitectureDiagnostic() const; // NEW: Added getter method
};

#endif