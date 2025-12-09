#include "CoreSemantic.h"

CoreSemantic::CoreSemantic() {
}

bool CoreSemantic::AnalyzeWorkspace(CoreWorkspace& ws,
                                   CoreAssist& assist,
                                   CoreGraph& graph,
                                   String& error) {
    entities.Clear();
    clusters.Clear();
    subsystems.Clear(); // NEW: Clear subsystems as well

    // Collect symbols from CoreAssist
    const auto& symbols = assist.GetSymbols();
    for (const auto& sym : symbols) {
        Entity e;
        e.name = sym.name;
        e.kind = sym.kind;
        e.file = sym.file;
        e.line = sym.line;
        entities.Add(std::move(e));
    }

    // Build relations between entities
    BuildRelations(assist, graph);

    // Compute attributes for each entity
    ComputeAttributes();

    // NEW: Compute inferred relations after building basic relations and attributes
    InferCoOccurrence();
    InferConceptualSimilarity();
    InferLayering(graph);
    InferRoles();

    // NEW: Compute behavioral inference (SemanticAssist v3)
    InferDataflow(assist);
    InferBehaviorSignatures();
    InferPipelineStages();

    // Build the behavior graph
    behavior_graph = BuildBehaviorGraph();

    // NEW: Detect patterns and anti-patterns (SemanticAssist v4)
    DetectPatterns();
    DetectAntiPatterns();
    ComputeArchitectureScores();
    diagnostic = BuildArchitectureDiagnostic();

    // Build clusters of related entities
    BuildClusters();

    // NEW: Build subsystems based on clusters and inferred relations
    BuildSubsystems();

    return true;
}

const Vector<CoreSemantic::Entity>& CoreSemantic::GetEntities() const {
    return entities;
}

const Vector<CoreSemantic::Cluster>& CoreSemantic::GetClusters() const {
    return clusters;
}

const Vector<CoreSemantic::Subsystem>& CoreSemantic::GetSubsystems() const {
    return subsystems;
}

Vector<CoreSemantic::Entity> CoreSemantic::FindEntitiesByName(const String& pattern) const {
    Vector<Entity> result;
    for (const auto& e : entities) {
        if (e.name.Find(pattern) >= 0) {
            result.Add(e);
        }
    }
    return result;
}

Vector<CoreSemantic::Cluster> CoreSemantic::FindClustersForEntity(const String& name) const {
    Vector<Cluster> result;
    for (const auto& c : clusters) {
        for (const auto& entity : c.entities) {
            if (entity == name) {
                result.Add(c);
                break;
            }
        }
    }
    return result;
}

void CoreSemantic::BuildRelations(const CoreAssist& assist, const CoreGraph& graph) {
    // Build "calls" relations by scanning for function calls in source files
    for (auto& entity : entities) {
        if (entity.kind == "function") {
            // Look for function calls in the same file
            Vector<String> calls;
            
            // Try to read the file to find function calls
            FileIn in(entity.file);
            if (in.IsOpen()) {
                String content = LoadFile(entity.file);
                // Basic function call detection (heuristic)
                Vector<String> lines = Split(content, '\n');
                for (int i = 0; i < lines.GetCount(); i++) {
                    String line = lines[i];
                    // Remove comments
                    int pos = line.Find("//");
                    if (pos >= 0) {
                        line = line.Mid(0, pos);
                    }
                    
                    // Look for function calls: identifier followed by parentheses
                    // This is a basic heuristic - real implementation would be more sophisticated
                    for (const auto& other_entity : entities) {
                        if (other_entity.name != entity.name) {
                            pos = 0;
                            while ((pos = line.Find(other_entity.name, pos)) >= 0) {
                                // Check if this is followed by a '(' to indicate a call
                                int next_pos = pos + other_entity.name.GetLength();
                                if (next_pos < line.GetLength() && line[next_pos] == '(') {
                                    calls.Add(other_entity.name);
                                    break;
                                }
                                pos++;
                            }
                        }
                    }
                }
            }
            
            // Store the calls relation
            if (calls.GetCount() > 0) {
                entity.relations.Set("calls", AsValue(calls));
            }
        }
        
        // Build "uses_type" relations by checking type references in function signatures
        // This is a basic heuristic approach - real implementation would parse signatures properly
        Vector<String> used_types;
        for (const auto& other_entity : entities) {
            if (other_entity.kind == "class" || other_entity.kind == "struct" || 
                other_entity.kind == "enum") {
                // In a real implementation, we would check the function signature
                // for references to other types
                used_types.Add(other_entity.name);
            }
        }
        if (used_types.GetCount() > 0) {
            entity.relations.Set("uses_type", AsValue(used_types));
        }
    }
    
    // Build "depends_on_package" relations from CoreGraph edges
    const auto& edges = graph.GetEdges();
    for (const auto& edge : edges) {
        // Find the entity that corresponds to the source of the dependency
        for (auto& entity : entities) {
            if (entity.file == edge.from) {
                // Add dependency relation
                if (!entity.relations.Contains("depends_on_package")) {
                    entity.relations.Set("depends_on_package", Array<String>());
                }
                
                Array<String> deps = entity.relations.Get("depends_on_package");
                deps.Add(edge.to);
                entity.relations.Set("depends_on_package", AsValue(deps));
            }
        }
    }
}

void CoreSemantic::ComputeAttributes() {
    for (auto& entity : entities) {
        // Calculate LOC (Lines of Code)
        FileIn in(entity.file);
        if (in.IsOpen()) {
            String content = LoadFile(entity.file);
            Vector<String> lines = Split(content, '\n');
            entity.attributes.Set("LOC", lines.GetCount());
        } else {
            entity.attributes.Set("LOC", 0);
        }
        
        // Calculate fanout (number of outgoing relations)
        int fanout = 0;
        for (const auto& key : entity.relations.GetKeys()) {
            Value val = entity.relations.Get(key);
            if (val.Is<Array<String>>()) {
                Array<String> arr = val;
                fanout += arr.GetCount();
            } else {
                fanout += 1; // For single relations
            }
        }
        entity.attributes.Set("fanout", fanout);
        
        // Calculate fannin (number of incoming relations)
        int fannin = 0;
        for (const auto& other_entity : entities) {
            if (other_entity.name != entity.name) {
                for (const auto& key : other_entity.relations.GetKeys()) {
                    Value val = other_entity.relations.Get(key);
                    if (val.Is<Array<String>>()) {
                        Array<String> arr = val;
                        for (const auto& item : arr) {
                            if (item == entity.name) {
                                fannin++;
                            }
                        }
                    } else {
                        if (val.ToString() == entity.name) {
                            fannin++;
                        }
                    }
                }
            }
        }
        entity.attributes.Set("fannin", fannin);
        
        // Calculate complexity score (simple heuristic)
        // Using lines of code as a base for complexity
        int complexity = entity.attributes.Get("LOC", 0);
        entity.attributes.Set("complexity", complexity);
    }
}

void CoreSemantic::BuildClusters() {
    clusters.Clear();

    // Simple clustering algorithm: group entities that have mutual relations
    Vector<bool> clustered(entities.GetCount(), false);

    for (int i = 0; i < entities.GetCount(); i++) {
        if (!clustered[i]) {
            // Start a new cluster with the current entity
            Cluster cluster;
            cluster.name = entities[i].name + "_cluster";
            cluster.entities.Add(entities[i].name);
            clustered[i] = true;

            // Find related entities
            for (int j = 0; j < entities.GetCount(); j++) {
                if (i != j && !clustered[j]) {
                    bool related = false;

                    // Check if entities[i] has a relation to entities[j]
                    for (const auto& key : entities[i].relations.GetKeys()) {
                        Value val = entities[i].relations.Get(key);
                        if (val.Is<Array<String>>()) {
                            Array<String> arr = val;
                            for (const auto& item : arr) {
                                if (item == entities[j].name) {
                                    related = true;
                                    break;
                                }
                            }
                        } else {
                            if (val.ToString() == entities[j].name) {
                                related = true;
                                break;
                            }
                        }
                        if (related) break;
                    }

                    // Check if entities[j] has a relation to entities[i]
                    if (!related) {
                        for (const auto& key : entities[j].relations.GetKeys()) {
                            Value val = entities[j].relations.Get(key);
                            if (val.Is<Array<String>>()) {
                                Array<String> arr = val;
                                for (const auto& item : arr) {
                                    if (item == entities[i].name) {
                                        related = true;
                                        break;
                                    }
                                }
                            } else {
                                if (val.ToString() == entities[i].name) {
                                    related = true;
                                    break;
                                }
                            }
                            if (related) break;
                        }
                    }

                    if (related) {
                        cluster.entities.Add(entities[j].name);
                        clustered[j] = true;
                    }
                }
            }

            // Compute cluster metrics
            cluster.metrics.Set("size", cluster.entities.GetCount());

            // Calculate average complexity
            double total_complexity = 0;
            for (const auto& entity_name : cluster.entities) {
                for (const auto& entity : entities) {
                    if (entity.name == entity_name) {
                        total_complexity += entity.attributes.Get("complexity", 0);
                    }
                }
            }
            if (cluster.entities.GetCount() > 0) {
                cluster.metrics.Set("avg_complexity", total_complexity / cluster.entities.GetCount());
            } else {
                cluster.metrics.Set("avg_complexity", 0);
            }

            clusters.Add(std::move(cluster));
        }
    }
}

// NEW: Infer co-occurrence relationships between entities
void CoreSemantic::InferCoOccurrence() {
    // For every file, record which entities appear together
    Map<String, Vector<String>> file_entities;

    for (const auto& entity : entities) {
        file_entities.GetAdd(entity.file).Add(entity.name);
    }

    // For each entity, check which other entities appear in the same files
    for (auto& entity : entities) {
        for (const auto& other_entity : entities) {
            if (entity.name == other_entity.name) continue;  // Skip self

            // Check if they appear in the same file
            const auto& file_entities_list = file_entities.Get(entity.file, Vector<String>());
            for (const auto& file_entity : file_entities_list) {
                if (file_entity == other_entity.name) {
                    // Check if already in co_occurs_with to avoid duplicates
                    bool already_added = false;
                    for (const auto& co_entity : entity.co_occurs_with) {
                        if (co_entity == other_entity.name) {
                            already_added = true;
                            break;
                        }
                    }
                    if (!already_added) {
                        entity.co_occurs_with.Add(other_entity.name);
                    }
                    break;
                }
            }
        }
    }
}

// NEW: Infer conceptual similarity between entities
void CoreSemantic::InferConceptualSimilarity() {
    for (auto& entity : entities) {
        for (const auto& other_entity : entities) {
            if (entity.name == other_entity.name) continue;  // Skip self

            bool conceptually_similar = false;

            // Check if entities have similar prefixes
            String entity_prefix = entity.name.Left(5); // First 5 characters as prefix
            String other_prefix = other_entity.name.Left(5);

            if (entity_prefix == other_prefix) {
                conceptually_similar = true;
            }

            // Check call graph similarity (for functions)
            if (entity.kind == "function" && other_entity.kind == "function") {
                // Check if they call similar functions
                Value entity_calls = entity.relations.Get("calls", Value());
                Value other_calls = other_entity.relations.Get("calls", Value());

                if (entity_calls.Is<Array<String>>() && other_calls.Is<Array<String>>()) {
                    Array<String> entity_call_list = entity_calls;
                    Array<String> other_call_list = other_calls;

                    // Count common calls
                    int common_calls = 0;
                    for (const auto& call : entity_call_list) {
                        for (const auto& other_call : other_call_list) {
                            if (call == other_call) {
                                common_calls++;
                                break;
                            }
                        }
                    }

                    if (common_calls > 0) {
                        conceptually_similar = true;
                    }
                }
            }

            // Check type dependency similarity
            Value entity_types = entity.relations.Get("uses_type", Value());
            Value other_types = other_entity.relations.Get("uses_type", Value());

            if (entity_types.Is<Array<String>>() && other_types.Is<Array<String>>()) {
                Array<String> entity_type_list = entity_types;
                Array<String> other_type_list = other_types;

                // Count common types
                int common_types = 0;
                for (const auto& type : entity_type_list) {
                    for (const auto& other_type : other_type_list) {
                        if (type == other_type) {
                            common_types++;
                            break;
                        }
                    }
                }

                if (common_types > 0) {
                    conceptually_similar = true;
                }
            }

            if (conceptually_similar) {
                // Check if already in conceptually_related to avoid duplicates
                bool already_added = false;
                for (const auto& rel_entity : entity.conceptually_related) {
                    if (rel_entity == other_entity.name) {
                        already_added = true;
                        break;
                    }
                }
                if (!already_added) {
                    entity.conceptually_related.Add(other_entity.name);
                }
            }
        }
    }
}

// NEW: Infer architectural layering based on package dependency depth
void CoreSemantic::InferLayering(CoreGraph& graph) {
    // Map package names to dependency depths
    Map<String, int> package_depths;

    // Get dependency depths from the CoreGraph (simplified heuristic)
    // This would typically analyze the graph structure to determine depth
    const auto& edges = graph.GetEdges();

    // Build dependency graph structure
    Map<String, Vector<String>> package_deps;
    for (const auto& edge : edges) {
        package_deps.GetAdd(edge.from).Add(edge.to);
    }

    // Calculate depths using BFS from root packages (packages with no incoming dependencies)
    Vector<String> root_packages;
    for (const auto& entity : entities) {
        // Use directory as a proxy for package name
        String package = GetFileDirectory(entity.file);
        bool has_incoming_deps = false;

        // Check if any other package depends on this package
        for (const auto& deps : package_deps) {
            for (const auto& dep : deps) {
                if (dep == package) {
                    has_incoming_deps = true;
                    break;
                }
            }
            if (has_incoming_deps) break;
        }

        if (!has_incoming_deps) {
            root_packages.Add(package);
        }
    }

    // Set depths based on dependency structure
    for (const auto& entity : entities) {
        String package = GetFileDirectory(entity.file);

        // Use directory depth to determine architectural layer
        // For simplicity, count directory separators
        int depth = 0;
        for (int i = 0; i < package.GetLength(); i++) {
            if (package[i] == '/' || package[i] == '\\') {
                depth++;
            }
        }

        // Map directory depth to architectural layer
        if (depth <= 1) {
            entity.layer_dependency = "base";
        } else if (depth <= 3) {
            entity.layer_dependency = "core";
        } else {
            entity.layer_dependency = "app";
        }
    }
}

// NEW: Infer semantic roles based on naming patterns and graph properties
void CoreSemantic::InferRoles() {
    for (auto& entity : entities) {
        // Role detection based on name patterns
        if (entity.name.EndsWith("Manager") ||
            entity.name.EndsWith("Ctrl") ||
            entity.name.EndsWith("Controller")) {
            entity.role = "controller";
        } else if (entity.name.EndsWith("Parser") ||
                   entity.name.EndsWith("Reader") ||
                   entity.name.EndsWith("Converter")) {
            entity.role = "parser";
        } else {
            // Role detection based on graph properties
            int fannin = entity.attributes.Get("fannin", 0);
            int fanout = entity.attributes.Get("fanout", 0);

            // If many inbound edges (fannin), it's likely a service
            if (fannin > 5 && fanout <= 3) {
                entity.role = "service";
            // If many outbound edges (fanout), it's likely a utility
            } else if (fanout > 5 && fannin <= 3) {
                entity.role = "utility";
            // If it has many of both, it might be a "god" component (a problematic role)
            } else if (fannin > 3 && fanout > 3) {
                entity.role = "god_component";
            } else {
                entity.role = "utility"; // Default role
            }
        }
    }
}

// NEW: Build subsystems based on clusters and inferred relations
void CoreSemantic::BuildSubsystems() {
    // Clear any existing subsystems
    subsystems.Clear();

    // Combine clusters with inferred relations to form subsystems
    // Entities with strong conceptual_related + co_occurs_with coherence form subsystems

    // Mark which entities are already assigned to a subsystem
    Vector<bool> assigned(entities.GetCount(), false);

    for (int i = 0; i < entities.GetCount(); i++) {
        if (assigned[i]) continue;  // Skip if already assigned to a subsystem

        // Start a new subsystem with the current entity
        Subsystem subsystem;
        String entity_name = entities[i].name;
        subsystem.name = entity_name + "_subsystem";
        subsystem.entities.Add(entity_name);
        assigned[i] = true;

        // Find related entities based on conceptual relationships and co-occurrence
        for (int j = 0; j < entities.GetCount(); j++) {
            if (i == j || assigned[j]) continue;  // Skip self and already assigned entities

            bool should_add = false;

            // Check if this entity is conceptually related to any entity already in the subsystem
            for (const auto& sub_entity_name : subsystem.entities) {
                // Find the entity in our entities vector
                for (const auto& entity : entities) {
                    if (entity.name == sub_entity_name) {
                        // Check if current entity (j) is conceptually related to this entity
                        for (const auto& conceptually_related : entity.conceptually_related) {
                            if (conceptually_related == entities[j].name) {
                                should_add = true;
                                break;
                            }
                        }
                        if (should_add) break;

                        // Check if current entity (j) co-occurs with this entity
                        for (const auto& co_occurs : entity.co_occurs_with) {
                            if (co_occurs == entities[j].name) {
                                should_add = true;
                                break;
                            }
                        }
                        if (should_add) break;
                    }
                }
                if (should_add) break;
            }

            if (should_add) {
                subsystem.entities.Add(entities[j].name);
                assigned[j] = true;
            }
        }

        // Compute subsystem metrics
        subsystem.metrics.Set("size", subsystem.entities.GetCount());

        // Calculate cohesion score based on how many entities in the subsystem have
        // conceptual relationships with other entities in the subsystem
        int cohesion_connections = 0;
        for (const auto& entity_name : subsystem.entities) {
            for (const auto& other_entity_name : subsystem.entities) {
                if (entity_name != other_entity_name) {
                    // Find the entity in our entities vector
                    for (const auto& entity : entities) {
                        if (entity.name == entity_name) {
                            for (const auto& conceptually_related : entity.conceptually_related) {
                                if (conceptually_related == other_entity_name) {
                                    cohesion_connections++;
                                    break;
                                }
                            }
                            for (const auto& co_occurs_with : entity.co_occurs_with) {
                                if (co_occurs_with == other_entity_name) {
                                    cohesion_connections++;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        // Normalize cohesion score
        int max_possible_connections = subsystem.entities.GetCount() * (subsystem.entities.GetCount() - 1) * 2; // *2 for both conceptual and co-occurrence
        double cohesion_score = (max_possible_connections > 0) ?
                               (double)cohesion_connections / max_possible_connections : 0.0;
        subsystem.metrics.Set("cohesion_score", cohesion_score);

        // Calculate coupling score by checking relationships with entities outside this subsystem
        int coupling_connections = 0;
        Vector<String> all_subsystem_entities;
        for (const auto& sub_entity_name : subsystem.entities) {
            all_subsystem_entities.Add(sub_entity_name);
        }

        for (const auto& entity_name : all_subsystem_entities) {
            for (const auto& other_entity : entities) {
                bool is_in_subsystem = false;
                for (const auto& sub_entity_name : all_subsystem_entities) {
                    if (sub_entity_name == other_entity.name) {
                        is_in_subsystem = true;
                        break;
                    }
                }

                if (!is_in_subsystem) {
                    // Check relationships between subsystem entity and entity outside subsystem
                    for (const auto& entity : entities) {
                        if (entity.name == entity_name) {
                            for (const auto& conceptually_related : entity.conceptually_related) {
                                if (conceptually_related == other_entity.name) {
                                    coupling_connections++;
                                    break;
                                }
                            }
                            for (const auto& co_occurs_with : entity.co_occurs_with) {
                                if (co_occurs_with == other_entity.name) {
                                    coupling_connections++;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        // Normalize coupling score
        int total_external_entities = entities.GetCount() - subsystem.entities.GetCount();
        int max_possible_coupling = (total_external_entities > 0) ?
                                   subsystem.entities.GetCount() * total_external_entities * 2 : 1; // *2 for both conceptual and co-occurrence
        double coupling_score = (double)coupling_connections / max_possible_coupling;
        subsystem.metrics.Set("coupling_score", coupling_score);

        // Calculate complexity sum
        int complexity_sum = 0;
        for (const auto& entity_name : subsystem.entities) {
            for (const auto& entity : entities) {
                if (entity.name == entity_name) {
                    complexity_sum += entity.attributes.Get("complexity", 0);
                    break;
                }
            }
        }
        subsystem.metrics.Set("complexity_sum", complexity_sum);

        // Determine role distribution
        ValueMap role_distribution;
        for (const auto& entity_name : subsystem.entities) {
            for (const auto& entity : entities) {
                if (entity.name == entity_name) {
                    String role = entity.role;
                    if (role.IsEmpty()) role = "unknown";  // Default role if none assigned
                    int current_count = role_distribution.Get(role, 0);
                    role_distribution.Set(role, current_count + 1);
                    break;
                }
            }
        }
        subsystem.metrics.Set("role_distribution", role_distribution);

        // Name the subsystem based on dominant entity prefix or most common role
        if (subsystem.entities.GetCount() > 0) {
            // Try to determine a name based on common prefix or common role
            String most_common_role = "";
            int max_role_count = 0;
            for (const auto& key : role_distribution.GetKeys()) {
                int count = role_distribution.Get(key);
                if (count > max_role_count) {
                    max_role_count = count;
                    most_common_role = key;
                }
            }

            if (!most_common_role.IsEmpty() && most_common_role != "unknown") {
                subsystem.name = most_common_role + "_subsystem";
            } else {
                // Use dominant entity prefix as name
                String first_entity_name = subsystem.entities[0];
                String prefix = first_entity_name.Left(min(10, first_entity_name.GetLength()));
                subsystem.name = prefix + "_subsystem";
            }
        }

        subsystems.Add(std::move(subsystem));
    }
}

// NEW: SemanticAssist v3 - Behavioral inference methods
void CoreSemantic::InferDataflow(const CoreAssist& assist) {
    for (auto& entity : entities) {
        // Initialize behavior map if empty
        if (!entity.behavior.Contains("dataflow")) {
            entity.behavior.Set("dataflow", Array<String>());
        }

        // Infer dataflow based on relations (especially calls)
        if (entity.relations.Contains("calls")) {
            Value calls_value = entity.relations.Get("calls");
            if (calls_value.Is<Array<String>>()) {
                Array<String> calls = calls_value;

                // Add called entities to dataflow
                Array<String> dataflow_entities = entity.behavior.Get("dataflow");
                for (const auto& call : calls) {
                    // Check if this call is already in dataflow to avoid duplicates
                    bool already_added = false;
                    for (const auto& df_entity : dataflow_entities) {
                        if (df_entity == call) {
                            already_added = true;
                            break;
                        }
                    }
                    if (!already_added) {
                        dataflow_entities.Add(call);
                    }
                }
                entity.behavior.Set("dataflow", dataflow_entities);
            }
        }

        // Infer pure function behavior
        bool is_pure = true;
        bool has_io_calls = false;
        bool modifies_state = false;

        // Check for I/O related calls (heuristic-based)
        if (entity.relations.Contains("calls")) {
            Value calls_value = entity.relations.Get("calls");
            if (calls_value.Is<Array<String>>()) {
                Array<String> calls = calls_value;
                for (const auto& call : calls) {
                    // Check if the called function has I/O behavior
                    for (const auto& other_entity : entities) {
                        if (other_entity.name == call) {
                            if (other_entity.behavior.Contains("io_bound") &&
                                other_entity.behavior.Get("io_bound").Get<bool>(false)) {
                                has_io_calls = true;
                                is_pure = false;  // Not pure if calls I/O functions
                                break;
                            }
                            if (other_entity.behavior.Contains("stateful") &&
                                other_entity.behavior.Get("stateful").Get<bool>(false)) {
                                modifies_state = true;
                                is_pure = false;  // Not pure if calls stateful functions
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Check for files that might indicate I/O
        String file_lower = ToLower(entity.file);
        if (file_lower.Find("io") >= 0 || file_lower.Find("file") >= 0 ||
            file_lower.Find("socket") >= 0 || file_lower.Find("network") >= 0) {
            has_io_calls = true;
            is_pure = false;
        }

        // Set behavior flags
        entity.behavior.Set("pure", is_pure);
        entity.behavior.Set("io_bound", has_io_calls);
        entity.behavior.Set("stateful", modifies_state);
    }
}

void CoreSemantic::InferBehaviorSignatures() {
    for (auto& entity : entities) {
        // Infer transformer behavior (has inputs and returns output)
        bool is_transformer = false;
        int loc = entity.attributes.Get("LOC", 0);

        // Heuristic: if it has non-zero LOC and doesn't look like a pure I/O function,
        // it might be a transformer
        if (loc > 0) {
            bool is_pure_io = entity.behavior.Get("io_bound", false) &&
                             !entity.behavior.Get("pure", true);
            if (!is_pure_io) {
                is_transformer = true;
            }
        }

        entity.behavior.Set("transformer", is_transformer);

        // Infer if function calls "heavy" functions
        bool calls_heavy = false;
        if (entity.relations.Contains("calls")) {
            Value calls_value = entity.relations.Get("calls");
            if (calls_value.Is<Array<String>>()) {
                Array<String> calls = calls_value;
                for (const auto& call : calls) {
                    for (const auto& other_entity : entities) {
                        if (other_entity.name == call) {
                            if (other_entity.behavior.Contains("complexity") &&
                                other_entity.behavior.Get("complexity", 0) > 50) { // heuristic
                                calls_heavy = true;
                                break;
                            }
                            if (other_entity.behavior.Contains("calls_heavy") &&
                                other_entity.behavior.Get("calls_heavy", false)) {
                                calls_heavy = true;
                                break;
                            }
                        }
                    }
                    if (calls_heavy) break;
                }
            }
        }
        entity.behavior.Set("calls_heavy", calls_heavy);

        // Set complexity-based behavior
        entity.behavior.Set("complexity", entity.attributes.Get("complexity", 0));
    }
}

void CoreSemantic::InferPipelineStages() {
    for (auto& entity : entities) {
        String pipeline_stage = "unknown";

        // Determine pipeline stage based on behavior signatures
        bool is_io_bound = entity.behavior.Get("io_bound", false);
        bool is_pure = entity.behavior.Get("pure", true);
        bool has_inputs = entity.attributes.Get("fannin", 0) > 0;  // Has incoming dependencies
        bool has_outputs = entity.attributes.Get("fanout", 0) > 0; // Calls other functions

        if (is_io_bound && !has_inputs) {
            // I/O bound with no incoming dependencies -> source
            pipeline_stage = "source";
        } else if (!is_io_bound && has_inputs && has_outputs) {
            // Not I/O bound, has both inputs and outputs -> transform
            pipeline_stage = "transform";
        } else if (is_io_bound && !has_outputs) {
            // I/O bound with no outgoing calls -> sink
            pipeline_stage = "sink";
        }

        entity.behavior.Set("pipeline_stage", pipeline_stage);

        // Also store side effects
        Vector<String> side_effects;
        if (!entity.behavior.Get("pure", true)) {
            side_effects.Add("modifies_state");
        }
        if (entity.behavior.Get("io_bound", false)) {
            side_effects.Add("io_operation");
        }
        if (entity.behavior.Get("stateful", false)) {
            side_effects.Add("modifies_global");
        }

        Array<String> side_effects_array;
        for (const auto& effect : side_effects) {
            side_effects_array.Add(effect);
        }
        entity.behavior.Set("side_effects", side_effects_array);
    }
}

// NEW: SemanticAssist v3 - Build behavior graph from entity behaviors
CoreSemantic::BehaviorGraph CoreSemantic::BuildBehaviorGraph() const {
    BehaviorGraph graph;

    // Add all entities as nodes
    for (const auto& entity : entities) {
        graph.nodes.Add(entity.name);
    }

    // Build edges based on various relationships
    for (const auto& entity : entities) {
        // Add edges based on function calls (dataflow relation)
        if (entity.relations.Contains("calls")) {
            Value calls_value = entity.relations.Get("calls");
            if (calls_value.Is<Array<String>>()) {
                Array<String> calls = calls_value;
                for (const auto& called_entity : calls) {
                    // Find the called entity to make sure it exists
                    bool found = false;
                    for (const auto& e : entities) {
                        if (e.name == called_entity) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        graph.edges.Add(Tuple<String, String, String>(entity.name, called_entity, "dataflow"));
                    }
                }
            }
        }

        // Add edges based on pipeline stages
        String this_stage = entity.behavior.Get("pipeline_stage", String("unknown"));
        if (this_stage != "unknown") {
            // Add edges to entities that are in the next stage (e.g., from source to transform)
            for (const auto& other_entity : entities) {
                String other_stage = other_entity.behavior.Get("pipeline_stage", String("unknown"));

                // Create transformation edges: source -> transform -> sink
                if ((this_stage == "source" && other_stage == "transform") ||
                    (this_stage == "transform" && other_stage == "sink")) {
                    graph.edges.Add(Tuple<String, String, String>(entity.name, other_entity.name, "transform"));
                }
            }
        }

        // Add edges based on conceptual relationships from v2
        for (const auto& related_entity : entity.conceptually_related) {
            // Check if this related entity exists in our entity list
            bool found = false;
            for (const auto& e : entities) {
                if (e.name == related_entity) {
                    found = true;
                    break;
                }
            }
            if (found) {
                graph.edges.Add(Tuple<String, String, String>(entity.name, related_entity, "conceptual"));
            }
        }
    }

    return graph;
}

CoreSemantic::BehaviorGraph CoreSemantic::GetBehaviorGraph() const {
    return behavior_graph;
}

const CoreSemantic::ArchitectureDiagnostic& CoreSemantic::GetArchitectureDiagnostic() const {
    return diagnostic;
}

// NEW: SemanticAssist v4 - Pattern Detection Implementation
void CoreSemantic::DetectPatterns() {
    // Pattern 1: Facade - heavily called by many modules but calls few
    for (auto& entity : entities) {
        int fannin = entity.attributes.Get("fannin", 0);
        int fanout = entity.attributes.Get("fanout", 0);

        // Heuristics for facade pattern: many incoming calls, few outgoing calls
        if (fannin > 5 && fanout < 3) {
            entity.patterns.Set("facade", true);
            entity.patterns.Set("facade_incoming_calls", fannin);
            entity.patterns.Set("facade_outgoing_calls", fanout);
        }
    }

    // Pattern 2: Adapter - many thin methods delegating 1:1 to other components
    for (auto& entity : entities) {
        if (entity.kind == "class" || entity.kind == "struct") {
            // Check if the entity has many simple methods that delegate to others
            int loc = entity.attributes.Get("LOC", 0);
            int complexity = entity.attributes.Get("complexity", 0);

            // Heuristic: low complexity but not too small LOC suggests thin wrappers
            if (complexity < 5 && loc > 5 && loc < 50) {
                // Check if it has calls to other entities (suggesting delegation)
                if (entity.relations.Contains("calls")) {
                    Value calls_value = entity.relations.Get("calls");
                    if (calls_value.Is<Array<String>>()) {
                        Array<String> calls = calls_value;
                        if (calls.GetCount() > 2) {
                            entity.patterns.Set("adapter", true);
                            entity.patterns.Set("adapter_delegates_to_count", calls.GetCount());
                        }
                    }
                }
            }
        }
    }

    // Pattern 3: Factory - creates many different classes/return types
    for (auto& entity : entities) {
        // Look for entities that have 'create', 'make', or 'factory' in their name
        String lower_name = ToLower(entity.name);
        if (lower_name.Find("create") >= 0 || lower_name.Find("make") >= 0 ||
            lower_name.Find("factory") >= 0 || lower_name.Find("new") >= 0) {
            entity.patterns.Set("factory", true);
            // Could enhance by checking for actual creation patterns in code
        }

        // Also check for entities with many 'returns' or creation behavior
        if (entity.behavior.Contains("transformer") && entity.attributes.Get("fannin", 0) > 2) {
            // Check if it creates or returns many different types
            entity.patterns.Set("factory_potential", true);
        }
    }

    // Pattern 4: Pipeline Stage Set - entities forming sequence source → transform → sink
    // This is handled in the existing InferPipelineStages method, but we can enhance it
    for (auto& entity : entities) {
        String pipeline_stage = entity.behavior.Get("pipeline_stage", String("unknown"));
        if (pipeline_stage != "unknown") {
            entity.patterns.Set("pipeline_stage", pipeline_stage);
        }
    }

    // Pattern 5: Utility Module - many static functions, low statefulness, high reuse
    for (auto& entity : entities) {
        int fannin = entity.attributes.Get("fannin", 0);
        bool is_pure = entity.behavior.Get("pure", true);
        bool is_stateful = entity.behavior.Get("stateful", false);

        // Heuristic: highly called, pure, not stateful
        if (fannin > 5 && is_pure && !is_stateful) {
            entity.patterns.Set("utility", true);
            entity.patterns.Set("utility_reuse_factor", fannin);
        }
    }
}

void CoreSemantic::DetectAntiPatterns() {
    // Anti-pattern 1: God Object - High fan-in + high fan-out + complex
    for (auto& entity : entities) {
        int fannin = entity.attributes.Get("fannin", 0);
        int fanout = entity.attributes.Get("fanout", 0);
        int complexity = entity.attributes.Get("complexity", 0);

        // Heuristic: high incoming and outgoing connections, with high complexity
        if (fannin > 5 && fanout > 5 && complexity > 50) {
            entity.antipatterns.Set("god_object", true);
            entity.antipatterns.Set("god_object_fannin", fannin);
            entity.antipatterns.Set("god_object_fanout", fanout);
            entity.antipatterns.Set("god_object_complexity", complexity);
        }
    }

    // Anti-pattern 2: Feature Envy - Methods that mostly operate on another object's data
    // For this, we'd need to analyze method signatures and usage more deeply
    for (auto& entity : entities) {
        // Check if it heavily uses types from other entities
        ValueMap uses_types_map;
        if (entity.relations.Contains("uses_type")) {
            Value uses_type_value = entity.relations.Get("uses_type");
            if (uses_type_value.Is<Array<String>>()) {
                Array<String> uses_types = uses_type_value;
                if (uses_types.GetCount() > 5) {  // Heuristic: uses many different types
                    entity.antipatterns.Set("feature_envy", true);
                    entity.antipatterns.Set("feature_envy_type_count", uses_types.GetCount());
                }
            }
        }
    }

    // Anti-pattern 3: Cyclic Dependency Cluster - Multi-node cycle detected
    // This would require cycle detection in our graph - we can use existing cycle detection
    // For now, we'll add a placeholder for when more detailed cycle detection is implemented
    // We could also check for mutual calling relationships in the relations

    // Check for direct bidirectional calls
    for (int i = 0; i < entities.GetCount(); i++) {
        for (int j = i + 1; j < entities.GetCount(); j++) {
            // Check if entity i calls entity j
            bool i_calls_j = false;
            if (entities[i].relations.Contains("calls")) {
                Value calls_value = entities[i].relations.Get("calls");
                if (calls_value.Is<Array<String>>()) {
                    Array<String> calls = calls_value;
                    for (const auto& call : calls) {
                        if (call == entities[j].name) {
                            i_calls_j = true;
                            break;
                        }
                    }
                }
            }

            // Check if entity j calls entity i
            bool j_calls_i = false;
            if (entities[j].relations.Contains("calls")) {
                Value calls_value = entities[j].relations.Get("calls");
                if (calls_value.Is<Array<String>>()) {
                    Array<String> calls = calls_value;
                    for (const auto& call : calls) {
                        if (call == entities[i].name) {
                            j_calls_i = true;
                            break;
                        }
                    }
                }
            }

            // If both call each other, it's a simple cycle
            if (i_calls_j && j_calls_i) {
                entities[i].antipatterns.Set("cyclic_dependency", true);
                entities[i].antipatterns.Set("cyclic_with", entities[j].name);
                entities[j].antipatterns.Set("cyclic_dependency", true);
                entities[j].antipatterns.Set("cyclic_with", entities[i].name);
            }
        }
    }

    // Anti-pattern 4: Layer Violation - Lower layers depending on higher ones
    for (auto& entity : entities) {
        String layer = entity.layer_dependency;
        if (!layer.IsEmpty()) {
            // Check if this entity calls entities from higher layers
            if (entity.relations.Contains("calls")) {
                Value calls_value = entity.relations.Get("calls");
                if (calls_value.Is<Array<String>>()) {
                    Array<String> calls = calls_value;
                    for (const auto& call : calls) {
                        // Find the called entity and check its layer
                        for (const auto& other_entity : entities) {
                            if (other_entity.name == call) {
                                // Determine if this is a layer violation
                                // "app" layer shouldn't call "core" or "base" (lower layers)
                                // "core" layer shouldn't call "base" (lower layer)
                                bool is_violation = false;
                                String violation_desc = "";

                                if (layer == "app" && (other_entity.layer_dependency == "base" || other_entity.layer_dependency == "core")) {
                                    is_violation = true;
                                    violation_desc = "app_layer_calling_lower_layer";
                                } else if (layer == "core" && other_entity.layer_dependency == "base") {
                                    is_violation = true;
                                    violation_desc = "core_layer_calling_lower_layer";
                                }

                                if (is_violation) {
                                    entity.antipatterns.Set("layer_violation", true);
                                    entity.antipatterns.Set("layer_violation_type", violation_desc);
                                    entity.antipatterns.Set("layer_violation_target", call);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // Anti-pattern 5: Overloaded Responsibility - Entity with diverse unrelated responsibilities
    // This can be detected by checking the variety of roles or relations
    for (auto& entity : entities) {
        int responsibility_count = 0;

        // Count different types of relations
        for (const auto& key : entity.relations.GetKeys()) {
            responsibility_count++;
        }

        // Count conceptual relationships
        responsibility_count += entity.conceptually_related.GetCount();

        // Count co-occurrences
        responsibility_count += entity.co_occurs_with.GetCount();

        // If there are too many diverse relations, it might indicate overloaded responsibility
        if (responsibility_count > 10) {  // Heuristic threshold
            entity.antipatterns.Set("overloaded_responsibility", true);
            entity.antipatterns.Set("responsibility_diversity_score", responsibility_count);
        }
    }

    // Anti-pattern 6: Dead Component - No incoming references (unused module)
    for (auto& entity : entities) {
        int fannin = entity.attributes.Get("fannin", 0);

        if (fannin == 0) {
            // Check if it's not the main entry point or test file
            String file_lower = ToLower(entity.file);
            bool is_test = (file_lower.Find("_test") >= 0 || file_lower.Find("test_") >= 0 ||
                           file_lower.Find("test") >= 0);

            if (!is_test) {
                entity.antipatterns.Set("dead_component", true);
            }
        }
    }
}

void CoreSemantic::ComputeArchitectureScores() {
    // Calculate cohesion score based on related entities within the same conceptual group
    for (auto& entity : entities) {
        int cohesion_connections = 0;

        // Count connections to conceptually related entities
        for (const auto& related : entity.conceptually_related) {
            for (const auto& other_entity : entities) {
                if (other_entity.name == related) {
                    cohesion_connections++;
                    break;
                }
            }
        }

        // Count connections to co-occurring entities
        for (const auto& co_occurs : entity.co_occurs_with) {
            for (const auto& other_entity : entities) {
                if (other_entity.name == co_occurs) {
                    cohesion_connections++;
                    break;
                }
            }
        }

        // Normalize cohesion score (0-1)
        int max_possible_connections = entity.conceptually_related.GetCount() + entity.co_occurs_with.GetCount();
        double cohesion_score = (max_possible_connections > 0) ?
                               (double)cohesion_connections / max_possible_connections : 0.0;

        entity.attributes.Set("cohesion_score", cohesion_score);
    }

    // Calculate coupling score (how much the entity depends on others)
    for (auto& entity : entities) {
        int fanout = entity.attributes.Get("fanout", 0);
        int total_entities = entities.GetCount();

        // Normalize coupling score (0-1), where 0 is no coupling and 1 is maximum coupling
        double coupling_score = (total_entities > 1) ?
                               (double)fanout / (total_entities - 1) : 0.0;

        entity.attributes.Set("coupling_score", coupling_score);
    }
}

CoreSemantic::ArchitectureDiagnostic CoreSemantic::BuildArchitectureDiagnostic() const {
    ArchitectureDiagnostic diag;

    // Collect patterns across all entities
    for (const auto& entity : entities) {
        if (!entity.patterns.IsEmpty()) {
            ValueMap pattern_entry;
            pattern_entry.Set("entity_name", entity.name);
            pattern_entry.Set("patterns", entity.patterns);
            diag.patterns.Add(pattern_entry);
        }

        if (!entity.antipatterns.IsEmpty()) {
            ValueMap antipattern_entry;
            antipattern_entry.Set("entity_name", entity.name);
            antipattern_entry.Set("antipatterns", entity.antipatterns);
            diag.antipatterns.Add(antipattern_entry);
        }
    }

    // Compute overall architecture scores
    if (!entities.IsEmpty()) {
        double total_cohesion = 0.0;
        double total_coupling = 0.0;
        int entity_count = 0;

        for (const auto& entity : entities) {
            total_cohesion += entity.attributes.Get("cohesion_score", 0.0);
            total_coupling += entity.attributes.Get("coupling_score", 0.0);
            entity_count++;
        }

        double avg_cohesion = (entity_count > 0) ? total_cohesion / entity_count : 0.0;
        double avg_coupling = (entity_count > 0) ? total_coupling / entity_count : 0.0;

        // Layering score: check how well the architectural layers are respected
        int layer_violations = 0;
        for (const auto& entity : entities) {
            if (entity.antipatterns.Contains("layer_violation")) {
                layer_violations++;
            }
        }
        double layering_score = (entities.GetCount() > 0) ?
                               1.0 - ((double)layer_violations / entities.GetCount()) : 1.0;

        // Complexity index: average complexity across all entities
        double total_complexity = 0.0;
        for (const auto& entity : entities) {
            total_complexity += entity.attributes.Get("complexity", 0.0);
        }
        double complexity_index = (entity_count > 0) ? total_complexity / entity_count : 0.0;

        // Structural entropy: measure of randomness/disorder in the architecture
        double structural_entropy = 0.0;
        if (entity_count > 0) {
            double avg_fanin = 0.0;
            double avg_fanout = 0.0;

            for (const auto& entity : entities) {
                avg_fanin += entity.attributes.Get("fannin", 0);
                avg_fanout += entity.attributes.Get("fanout", 0);
            }
            avg_fanin /= entity_count;
            avg_fanout /= entity_count;

            // A simple entropy measure based on the distribution of fan-in and fan-out
            // Higher entropy indicates more disorder
            structural_entropy = (avg_fanin > 0 && avg_fanout > 0) ?
                                log(avg_fanin + avg_fanout) : 0.0;
        }

        diag.scores.Set("cohesion_score", avg_cohesion);
        diag.scores.Set("coupling_score", avg_coupling);
        diag.scores.Set("layering_score", layering_score);
        diag.scores.Set("complexity_index", complexity_index);
        diag.scores.Set("structural_entropy", structural_entropy);
    }

    return diag;
}