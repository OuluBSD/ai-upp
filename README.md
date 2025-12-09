# Topside

Topside (later U++) is a fork of Ultimate++ and a real-time engine for 2D/3D games and multimedia.

![alt text](uppbox/uppweb/Resources/Images/Logo.png?raw=true "U++ Logo")

[![CircleCI](https://dl.circleci.com/status-badge/img/gh/ultimatepp/ultimatepp/tree/master.svg?style=shield)](https://dl.circleci.com/status-badge/redirect/gh/ultimatepp/ultimatepp/tree/master)
[![U++ Discord](https://img.shields.io/discord/1046445457951424612)](https://discord.gg/8XzqQzXZzb)
![GitHub release (with filter)](https://img.shields.io/github/v/release/ultimatepp/ultimatepp)
[![License](https://img.shields.io/badge/License-BSD_2--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

## Introduction

U++ is a C++ cross-platform rapid application development framework focused on programmers productivity. It includes a set of libraries (GUI, SQL, Network, etc.), with an integrated development environment.

Rapid development is achieved by the [smart and aggressive use of C++](https://www.ultimatepp.org/www$uppweb$overview$en-us.html) rather than through fancy code generators. In this respect, U++ competes with popular scripting languages while preserving C/C++ runtime characteristics.

The U++ integrated development environment, TheIDE, introduces modular concepts to C++ programming. It features BLITZ-build technology to speedup C++ rebuilds by up to 4 times, Visual designers for U++ libraries, [Topic++](https://www.ultimatepp.org/app$ide$Topic$en-us.html) for documenting code and creating rich text resources for the applications (code documentation and examples) and [Assist++](https://www.ultimatepp.org/app$ide$Assist$en-us.html) - a powerful C++ code analyzer that provides features like code completion, abbreviation, navigation and conversion.

TheIDE can work with GCC, Clang, MinGW and Visual C++ while containing a fully featured debugger. It can also be used for developing non-U++ applications.

U++ supports following platforms on the production level: **Windows**, **macOS**, **GNU/Linux** & **FreeBSD**.

## What you can get with U++?

* Out of the box C++ libraries for cross-platform GUI development
* Fully featured IDE designed to develop complex and high performance applications.

It can be used for any work of your needs.

## License
U++ uses the BSD-2 Clause license. The license applies to all source code in this repository except for the situation when the directory contains the "Copying" file. In this case, the license contained in this file is valid for source codes within the directory in which it is present. Moreover, the new license stored in the "Copying" file applies to source files in child directories.

## Download

Main downloads:
* [Latest stable release](https://sourceforge.net/projects/upp/files/latest/download) - starts download with automatic platform detection

Stores download:
* [Flathub](https://flathub.org/apps/org.ultimatepp.TheIDE) - download latest stable U++ version from Flathub (Linux only)

Other downloads:
* [Stable releases](https://sourceforge.net/projects/upp/) - all stable releases including historical ones
* [Nightly build](https://www.ultimatepp.org/www$uppweb$download$en-us.html) - latest unstable release of U++ (might be unstable)

## U++ Resources

More information about the framework, can be found on the official [site](https://www.ultimatepp.org). Don't forget to check our rich [documentation](https://www.ultimatepp.org/www$uppweb$documentation$en-us.html).

## Examples

### GUI

Below is the code of trivial GUI application that displays "Hello World" string inside window:

```c++
#include <CtrlLib/CtrlLib.h>

class MyApp: public Upp::TopWindow {
public:
    MyApp()
    {
        Title("My application").Zoomable().Sizeable().SetRect(0, 0, 320, 200);
    }

    void Paint(Upp::Draw& w) override
    {
        w.DrawRect(GetSize(), Upp::SWhite);
        w.DrawText(10, 10, "Hello, world!", Upp::Arial(50), Upp::Magenta);
    }
};

GUI_APP_MAIN
{
    MyApp().Run();
}
```

### TheIDE

Standard part of U++ framework is integrated development environment, TheIDE.
<p align="center">
    <picture>
        <img alt="TheIDE - U++ Integrated Developemnt Enviroment" src="/uppbox/uppweb/Resources/Images/TheIDE.png" width="80%" height="80%">
    </picture>
</p>

### Additional examples

See here: [examples](https://www.ultimatepp.org/www$uppweb$examples$en-us.html). Moreover, exactly the same examples can be found in the **examples** and **references** directories located in this repository.

If you would like to see more screenshots, click [here](https://www.ultimatepp.org/www$uppweb$ss$en-us.html).

## Learning materials

### Tutorials

We prepared several tutorials that will allow you to learn most of the aspects of our integrated development environment, TheIDE and the U++ framework.

TheIDE:
- [**Getting started with TheIDE**](https://www.ultimatepp.org/app$ide$GettingStarted$en-us.html) - the introduction to the concepts of TheIDE.
- [TheIDE beginner guide](https://www.ultimatepp.org/app$ide$Guide_en-us.html) - continuation provides information how to solve common problems.

U++ framework:
- [**Core Tutorial**](https://www.ultimatepp.org/srcdoc$Core$Tutorial$en-us.html) - the introduction to the foundations behind the framework.
- [**GUI Tutorial**](https://www.ultimatepp.org/srcdoc$CtrlLib$Tutorial$en-us.html) - learn how to build graphical user interfaces with the U++.
- [Draw Tutorial](https://www.ultimatepp.org/srcdoc$Draw$DrawTutorial$en-us.html) - get knowledge about drawing custom content inside window or control.
- [Image Tutorial](https://www.ultimatepp.org/srcdoc$Draw$ImgTutorial$en-us.html) - check out the mechanism behind the image manipulation.
- [Network Tutorial](https://www.ultimatepp.org/srcdoc$Core$NetworkTutorial_en-us.html) - learn how to use U++ core components for network application creation.
- [Sql Tutorial](https://www.ultimatepp.org/srcdoc$Sql$tutorial$en-us.html) - introduction to using databases within U++ framework

If the above list is not enough, please visit a dedicated [website](https://www.ultimatepp.org/www$uppweb$Tutorials$en-us.html) where we have collected links to most of the learning resources.

### Books

If you are looking for books about the U++ framework, here is a list of books we recommend:
- [**Getting started with the U++ Framework**](https://www.amazon.com/dp/B0CQHX84VZ/) - the book written by Frederik Dumarey and reviewed by the main authors of U++. It is an excellent starting point for anyone new to the framework. Also, if you are an experienced U++ developer, you will find much valuable content, including a deep dive into Skylark (web framework) and Turtle (HTML, JS client).

## Extensibility

The functionality of the U++ framework can be easily extended by third-party packages. We introduced the concept of a global registry of repositories that can be downloaded and directly used in the project. To learn more about this technology, please visit [UppHub](https://github.com/ultimatepp/UppHub) repository.

UppHub conceptually is very similar to the package manager concept known from other technologies, and in big generalization, it can be called like that.

## Repository

### Repository layout

The U++ repository is divided into several directories. Some of them are:
* **autotest** - unit and integration tests for our framework
* **examples** - exemplary codes of complex applications
* **references** - reference examples demonstrating individual features of U++
* **tutorial** - exemplary code from our tutorials
* **uppsrc** - main sources of the framework
* **benchmarks** - code for benchmarking purposes
* **rainbow** - area for new GUI backends development
* **upptst** - manual testing code for the framework

## Telemetry & Analytics v1

The system now supports a comprehensive telemetry and analytics layer that enables external AI agents to gain situational awareness of the workspace. This provides rich introspection capabilities through structured JSON APIs.

### Available Telemetry Commands:

* **workspace_stats** - Provides structural analytics about the entire workspace
  * Number of packages
  * Total source files and distribution by extension
  * Average file size and largest file information
  * Main package name (if set)

* **package_stats** - Delivers detailed metrics about a specific package
  * Number of files and uses-count
  * Include count and total lines of code
  * Histogram of file sizes

* **file_complexity** - Computes complexity heuristics for a specific source file
  * Lines of code and estimated function count
  * Number of includes and nesting depth
  * Comment ratio

* **graph_stats** - Analyzes the dependency graph structure
  * Node and edge counts
  * Average outdegree and cycle detection
  * Longest dependency chain length
  * Most depended-on package identification

* **edit_history** - Tracks changes during the session
  * Records per-file size deltas
  * Timestamped edit history

### JSON Output Examples:

**workspace_stats:**
```json
{
  "packages": 4,
  "files": 132,
  "extensions": { "cpp": 84, "h": 48 },
  "largest_file": { "path": "src/Engine.cpp", "bytes": 42103 },
  "average_size": 3370,
  "main_package": "MyApp"
}
```

**file_complexity:**
```json
{
  "lines": 230,
  "functions": 7,
  "includes": 5,
  "nesting": 12,
  "comment_ratio": 0.18
}
```

**graph_stats:**
```json
{
  "nodes": 4,
  "edges": 3,
  "avg_out": 0.75,
  "cycles": false,
  "max_chain": 3,
  "most_depended_on": "Core"
}
```

This subsystem enables external AI to make **strategic decisions** by providing visibility into workspace health, change impact, code complexity, and quality signals.

## Optimization Loop v1

The system includes an iterative optimization subsystem that enables automated structural cleanup and refactoring of code packages. This provides a foundation for AI-driven refactoring through repeated incremental improvements with measurable outcomes.

### Core Features:

* **Iterative Refactoring** - Runs a sequence of optimizations repeatedly on a package
* **Telemetry-Based Scoring** - Uses heuristic metrics to evaluate improvement between iterations
* **Convergence Detection** - Stops automatically when improvements plateau or diminish
* **Regression Prevention** - Can halt when metrics indicate degradation
* **Built-in Refactoring Actions** - Currently includes operations like dead include removal and include canonicalization

### Available Optimization Command:

* **optimize_package** - Performs iterative structural cleanup on a specified package
  * Configurable maximum iterations
  * Convergence threshold for automatic termination
  * Options to stop on metric degradation
  * Detailed iteration history in JSON output

### Command Example:

```bash
theide-cli --workspace-root . --json optimize_package \
  --name MyApp \
  --max_iterations 10 \
  --converge_threshold 0.05
```

### JSON Output Example:

```json
{
  "iterations": [
    {
      "index": 0,
      "score_delta": -2.5,
      "changes": { "removed_includes": 4 }
    },
    {
      "index": 1,
      "score_delta": -0.8,
      "changes": { "removed_includes": 2 }
    }
  ],
  "reason": "converged",
  "success": true
}
```

This subsystem enables external AI to perform **systematic structural improvements** by automatically iterating over code with measurable quality heuristics to guide the process.

## AI Supervisor Layer v1

The system includes an AI Supervisor Layer that generates strategic, multi-step improvement plans for code packages. This component consumes telemetry, graph statistics, and complexity metrics to produce actionable recommendations for workspace enhancement.

### Core Features:

* **Strategic Planning** - Generates a multi-step plan with prioritized recommendations
* **Telemetry Integration** - Consumes workspace stats, package metrics, and complexity data
* **Heuristic Decision Making** - Uses rule-based algorithms to identify optimization opportunities
* **Structured Output** - Returns JSON describing recommended actions, parameters, and reasoning
* **AI Agent Ready** - Designed specifically for AI agents to use for autonomous project improvement

### Available Supervisor Command:

* **get_optimization_plan** - Analyzes a package and generates an AI-guided improvement plan
  * Identifies unused includes for cleanup
  * Detects symbol renaming opportunities
  * Flags graph simplification possibilities
  * Recommends optimization loop execution
  * Provides risk scoring based on complexity metrics

### Command Example:

```bash
theide-cli --workspace-root . --json get_optimization_plan --name MyApp
```

### JSON Output Example:

```json
{
  "steps": [
    {
      "action": "run_playbook",
      "target": "cleanup_includes_and_rebuild",
      "params": { "package": "MyApp" },
      "reason": "Many unused #includes detected."
    },
    {
      "action": "optimize_package",
      "params": { "name": "MyApp", "max_iterations": 5 },
      "reason": "High complexity score & long dependency chains."
    }
  ],
  "summary": "Package 'MyApp' shows structural issues: Risk score 0.72"
}
```

This subsystem enables external AI to make **strategic decisions** by providing a planning layer that interprets workspace telemetry and recommends specific improvement actions.

## Dynamic Strategy Engine (Supervisor v2)

The system now includes a Dynamic Strategy Engine that allows AI agents to control the optimization planning behavior through data-driven strategy profiles. This provides flexibility to adjust planning heuristics without recompiling the system.

### Core Features:

* **Data-Driven Strategies** - Strategy profiles are defined in JSON format in `metadata/strategies_v1.json`
* **Runtime Strategy Selection** - AI agents can switch between different strategies at runtime
* **Strategy-Aware Planning** - Optimization plans are generated using weights and thresholds from the active strategy
* **Extensible Configuration** - Strategies can define custom weights and threshold rules for different optimization playbooks

### Strategy Profile Format:

```json
{
  "strategies": [
    {
      "name": "default",
      "description": "Balanced cleanup and refactoring strategy.",
      "weights": {
        "include_density": 1.0,
        "complexity": 1.0,
        "dependency_depth": 1.0,
        "cycles": 1.5,
        "edit_volatility": 0.5
      },
      "playbook_thresholds": {
        "cleanup_includes_and_rebuild": {
          "min_include_density": 0.3
        },
        "rename_symbol_safe": {
          "min_symbol_occurrences": 100
        },
        "optimize_package": {
          "min_risk_score": 0.5
        }
      }
    }
  ]
}
```

### Available Strategy Commands:

* **list_strategies** - Returns an array of available strategies with their names and descriptions
* **get_strategy** - Returns detailed information about a specific strategy or the currently active strategy
* **set_strategy** - Changes the active strategy used for generating optimization plans
* **get_optimization_plan** and **workspace_plan** - Now include strategy information in their output

### Command Examples:

```bash
theide-cli --json list_strategies

theide-cli --json get_strategy
theide-cli --json get_strategy --name aggressive_cleanup

theide-cli set_strategy --name aggressive_cleanup
theide-cli --json get_optimization_plan --name MyApp
```

### CLI Usage:

The strategy engine provides both human-readable and JSON modes for all commands, maintaining compatibility with existing automation while enabling flexible AI agent behavior.

* **Strategy Selection**: Use `set_strategy --name STRATEGY_NAME` to select the active strategy
* **Plan Generation**: `get_optimization_plan` and `workspace_plan` commands now explicitly report which strategy generated the plan
* **Verification**: Use `get_strategy` to verify the currently active strategy before running planning commands

This subsystem enables AI agents to **adapt their optimization behavior dynamically** by selecting appropriate strategies based on project requirements, risk tolerance, or phase of development.

## Multi-Objective Planner (Supervisor v3)

The system has been upgraded to a Multi-Objective Planner (Supervisor v3) that evaluates suggestions using multiple objective dimensions, computes Pareto fronts, and provides multi-objective scores for each proposed action. This allows AI agents to make more sophisticated optimization decisions by considering benefit, cost, risk, and confidence simultaneously.

### Core Features:

* **Multi-Objective Evaluation** - Each suggestion is evaluated across four key dimensions:
  * **benefit_score**: Expected improvement from the action
  * **cost_score**: Estimated implementation cost and effort
  * **risk_score**: Risk of breakage or refactor impact
  * **confidence_score**: Heuristic confidence level in the suggestion

* **Pareto Optimization** - Ability to compute the Pareto-optimal subset of suggestions where no single suggestion dominates another across all objectives

* **Weighted Ranking** - Suggestios can be ranked using a weighted linear combination of objective scores based on strategy profile settings

* **Extensible Metrics** - Each suggestion includes an extensible metrics map for additional computed measurements

### Multi-Objective Fields in Suggestions:

Each suggestion now includes the following multi-objective fields:

```json
{
  "action": "run_playbook",
  "target": "cleanup_includes_and_rebuild",
  "params": { "package": "MyApp" },
  "reason": "Many unused #includes detected.",
  "benefit_score": 0.65,
  "cost_score": 0.23,
  "risk_score": 0.41,
  "confidence_score": 0.85,
  "metrics": {
    "impact": 2.83,
    "surface_area": 12,
    "graph_delta": 0
  }
}
```

### Available Supervisor v3 Commands:

* **supervisor_front** - Returns the Pareto-optimal suggestions for a package
  * Computes the Pareto front from all generated suggestions
  * Returns only non-dominated suggestions
  * Allows AI agents to focus on the most promising actions

* **supervisor_rank** - Ranks suggestions by multi-objective weighted score
  * Uses a weighted linear combination: `score = (benefit * w_benefit) + (confidence * w_confidence) - (cost * w_cost) - (risk * w_risk)`
  * Orders suggestions from highest to lowest score
  * Supports optional `limit` parameter to restrict number of results

### Strategy-Driven Objective Weights:

Strategy profiles now support an `objective_weights` section that influences both the computation of suggestion metrics and the ranking:

```json
{
  "name": "default",
  "description": "Balanced cleanup and refactoring strategy.",
  "weights": {
    "include_density": 1.0,
    "complexity": 1.0,
    "dependency_depth": 1.0,
    "cycles": 1.5,
    "edit_volatility": 0.5
  },
  "objective_weights": {
    "benefit": 1.0,
    "cost": 0.7,
    "risk": 1.2,
    "confidence": 1.0
  },
  "playbook_thresholds": {
    "cleanup_includes_and_rebuild": {
      "min_include_density": 0.3
    }
  }
}
```

### Command Examples:

```bash
# Get Pareto-optimal suggestions for a package
theide-cli --workspace-root . --json supervisor_front --package MyApp

# Get top 5 ranked suggestions for a package
theide-cli --workspace-root . --json supervisor_rank --package MyApp --limit 5

# Compare with traditional optimization plan
theide-cli --json get_optimization_plan --name MyApp
```

### JSON Output Examples:

**supervisor_front:**
```json
{
  "payload": [
    {
      "action": "run_playbook",
      "target": "cleanup_includes_and_rebuild",
      "params": { "package": "MyApp" },
      "reason": "Many unused #includes detected.",
      "benefit_score": 0.65,
      "cost_score": 0.23,
      "risk_score": 0.41,
      "confidence_score": 0.85,
      "metrics": {
        "impact": 2.83,
        "surface_area": 12,
        "graph_delta": 0
      }
    },
    {
      "action": "optimize_package",
      "params": { "name": "MyApp", "max_iterations": 5 },
      "reason": "High complexity score & long dependency chains.",
      "benefit_score": 0.72,
      "cost_score": 0.54,
      "risk_score": 0.38,
      "confidence_score": 0.91,
      "metrics": {
        "impact": 1.33,
        "surface_area": 12,
        "graph_delta": 0
      }
    }
  ]
}
```

**supervisor_rank:**
```json
{
  "payload": [
    {
      "action": "run_playbook",
      "target": "cleanup_includes_and_rebuild",
      "params": { "package": "MyApp" },
      "reason": "Many unused #includes detected.",
      "benefit_score": 0.65,
      "cost_score": 0.23,
      "risk_score": 0.41,
      "confidence_score": 0.85,
      "combined_score": 1.54,
      "metrics": {
        "impact": 2.83,
        "surface_area": 12,
        "graph_delta": 0
      }
    }
  ]
}
```

This subsystem enables AI agents to make **sophisticated multi-objective optimization decisions** by considering benefit, cost, risk, and confidence simultaneously, and allows them to focus on the most strategically valuable actions through Pareto optimization or weighted ranking.

## DeepSemanticAssist v1

The system now includes a DeepSemanticAssist v1 layer that performs static semantic analysis of code to extract relationships between entities, identify conceptual clusters, and provide insights about responsibility boundaries. This enables AI agents to understand code architecture at a deeper level and make more informed refactoring decisions.

### Core Features:

* **Semantic Entity Extraction** - Discovers and catalogs code entities (classes, functions, enums, etc.) with their metadata
* **Relationship Analysis** - Identifies connections between entities including function calls, type usage, and package dependencies
* **Conceptual Clustering** - Groups related entities into semantic clusters representing conceptual modules
* **Attribute Computation** - Calculates metrics like Lines of Code (LOC), fan-out (outgoing relations), fan-in (incoming relations), and complexity scores
* **Deterministic Output** - Provides consistent results for the same codebase state

### Semantic Entity Structure:

Each extracted entity includes the following information:

```json
{
  "name": "DatabaseConnection",
  "kind": "class",
  "file": "src/database/Connection.h",
  "line": 15,
  "relations": {
    "calls": ["connect", "disconnect", "execute_query"],
    "uses_type": ["SqlConnection", "QueryResult"],
    "depends_on_package": ["Core", "Network"]
  },
  "attributes": {
    "LOC": 125,
    "fanout": 8,
    "fannin": 3,
    "complexity": 12
  }
}
```

### Semantic Cluster Structure:

Clusters group related entities and include metrics about cohesion and coupling:

```json
{
  "name": "FileOps_cluster",
  "entities": [
    "FileReader",
    "FileWriter",
    "FileManager",
    "FileCache"
  ],
  "metrics": {
    "size": 4,
    "avg_complexity": 8.5
  }
}
```

### Available Semantic Commands:

* **semantic_entities** - Returns all semantic entities discovered in the workspace
  * Provides entity names, kinds, locations, relations, and computed attributes
  * Enables AI agents to explore the semantic landscape of the codebase

* **semantic_clusters** - Returns semantic clusters identified in the workspace
  * Groups related entities into conceptual modules
  * Provides metrics about cluster size and average complexity
  * Supports module-based reasoning and refactoring

* **semantic_find** - Searches for semantic entities by name pattern
  * Finds entities matching a specified pattern
  * Returns the same detailed information as semantic_entities

* **semantic_analyze** - Forces rerun of semantic analysis
  * Updates entity and cluster information based on current codebase state
  * Returns status information and counts of discovered entities and clusters

### Command Examples:

```bash
# Get all semantic entities in the workspace
theide-cli --workspace-root . --json semantic_entities

# Get semantic clusters
theide-cli --workspace-root . --json semantic_clusters

# Find entities matching a pattern
theide-cli --workspace-root . --json semantic_find --pattern Connection

# Force semantic analysis
theide-cli --workspace-root . --json semantic_analyze
```

### Integration with Supervisor v3:

The semantic information is integrated into the multi-objective planning process:

* **Semantic Snapshot** - Plans now include a semantic_snapshot with key metrics:
  * Number of clusters and entities
  * Largest cluster size
  * Top N entities by centrality (fanout/fannin)
  * Potential "god clusters" with extremely high metrics

* **Score Adjustment** - Suggestion scoring now considers semantic metrics:
  * Benefit scores are adjusted for coupling reduction opportunities
  * Cost scores increase for modifications to highly coupled clusters
  * Risk scores account for large/complex clusters
  * Confidence scores boost when semantic analysis data is available

### JSON Output for Plan with Semantic Snapshot:

```json
{
  "steps": [...],
  "summary": "Package 'MyApp' shows structural issues...",
  "strategy_info": {...},
  "semantic_snapshot": {
    "cluster_count": 12,
    "entity_count": 87,
    "largest_cluster_size": 15,
    "top_entities_by_centrality": [
      {
        "name": "CoreManager",
        "fanout": 23,
        "fannin": 18,
        "centrality": 20.3
      }
    ],
    "potential_god_clusters": [
      {
        "name": "CoreSystem_cluster",
        "size": 22,
        "avg_complexity": 12.7
      }
    ]
  }
}
```

This subsystem enables AI agents to make **architecture-aware optimization decisions** by understanding the conceptual structure of the codebase, identifying highly connected elements, and making refactoring decisions that improve code cohesion and reduce coupling.

## SemanticAssist v2 – Inference Layer

The system has been upgraded to SemanticAssist v2 with a new inference layer that goes beyond direct code relationships to identify higher-level conceptual connections, architectural layers, semantic roles, and subsystem boundaries. This enables AI agents to understand not just what code entities are connected, but why and how they form larger architectural patterns.

### Core Features:

* **Inferred Relations** - Identifies relationships that are not directly observable in code but emerge from usage patterns:
  * `co_occurs_with`: Symbols that frequently appear in the same files
  * `conceptually_related`: Derived from shared patterns, prefixes, or similar call graphs
  * `layer_dependency`: Inferred architectural layer ordering (e.g., util → core → app)
  * `role`: Possible semantic roles (e.g., "controller", "manager", "utility", "parser")

* **Semantic Propagation** - Propagates semantic information across entities to understand how changes might affect related components through inferred conceptual relationships

* **Subsystem Detection** - Automatically identifies semantic communities that represent cohesive subsystems within the larger codebase

* **Tagging & Labeling** - Applies semantic domain labels to entities and subsystems to facilitate architectural reasoning

* **CLI Access to Inferred Metadata** - Provides command-line access to the new inference layer data

* **Supervisor Integration** - Incorporates subsystem and role information into multi-objective planning and scoring

### Inferred Entity Relations:

Entities now include additional semantic fields beyond direct code relationships:

```json
{
  "name": "UserManager",
  "kind": "class",
  "file": "src/user/manager.h",
  "line": 12,
  "relations": {
    "calls": ["UserRepository::save", "EmailService::send"],
    "uses_type": ["User", "UserQuery"],
    "depends_on_package": ["Core"],
    "co_occurs_with": ["UserAuthenticator", "UserValidator"],
    "conceptually_related": ["PermissionManager", "RoleManager"]
  },
  "attributes": {
    "LOC": 87,
    "fanout": 12,
    "fannin": 3,
    "complexity": 8
  },
  "layer_dependency": "core",
  "role": "controller"
}
```

### Subsystem Structure:

Subsystems represent cohesive semantic communities with computed metrics:

```json
{
  "name": "controller_subsystem",
  "entities": [
    "UserManager",
    "PermissionManager",
    "RoleManager",
    "SessionController"
  ],
  "metrics": {
    "size": 4,
    "cohesion_score": 0.75,
    "coupling_score": 0.12,
    "complexity_sum": 37,
    "role_distribution": {
      "controller": 3,
      "manager": 1
    }
  }
}
```

### New Semantic CLI Commands:

* **semantic_subsystems** - Returns subsystem array with name, entities, and metrics:
  * Lists all detected subsystems with their composition and quality metrics
  * Enables AI agents to reason at the subsystem level

* **semantic_entity** - Returns full semantic info for one entity:
  * Includes both direct and inferred relations for a specific entity
  * Provides complete semantic context for focused analysis

* **semantic_roles** - Returns: entity → role mapping:
  * Shows the semantic role of each entity in the codebase
  * Allows AI agents to identify architectural patterns and responsibilities

* **semantic_layers** - Returns: entity → inferred architectural layer:
  * Maps each entity to its inferred architectural layer (base, core, app)
  * Enables layer-aware refactoring and dependency management

### Command Examples:

```bash
# Get all semantic subsystems
theide-cli --workspace-root . --json semantic_subsystems

# Get detailed info for a specific entity
theide-cli --workspace-root . --json semantic_entity --name DatabaseConnection

# Get role assignments for all entities
theide-cli --workspace-root . --json semantic_roles

# Get architectural layer assignments
theide-cli --workspace-root . --json semantic_layers
```

### Supervisor Integration:

The new inference data enhances Supervisor v3 scoring and planning:

* **Benefit Scoring** - Increases for suggestions that:
  * Reduce coupling between subsystems
  * Simplify high-level layers
  * Refactor "god components"

* **Risk Scoring** - Increases if:
  * Suggestion touches multiple subsystems
  * Entity has fragile role (parser, controller)

* **Confidence Scoring** - Increases if:
  * Subsystem structure is cohesive and supports the suggestion
  * Entity role fits proposed change

* **Semantic Snapshot Enhancement** - Plans now include:
  * Subsystem count and metrics
  * Role and layer distribution statistics
  * Potential "god components" with detailed role information
  * Subsystem cohesion and coupling metrics

### JSON Output for Plan with Enhanced Semantic Snapshot:

```json
{
  "steps": [...],
  "summary": "Package 'MyApp' shows structural issues...",
  "strategy_info": {...},
  "semantic_snapshot": {
    "cluster_count": 12,
    "entity_count": 87,
    "subsystem_count": 5,
    "largest_cluster_size": 15,
    "largest_subsystem_size": 8,
    "avg_subsystem_cohesion": 0.63,
    "avg_subsystem_coupling": 0.21,
    "top_entities_by_centrality": [...],
    "role_distribution": {
      "controller": 12,
      "utility": 23,
      "service": 8,
      "manager": 15,
      "parser": 4
    },
    "layer_distribution": {
      "base": 16,
      "core": 42,
      "app": 29
    },
    "potential_god_components": [
      {
        "name": "CoreManager",
        "role": "controller",
        "fannin": 28,
        "fanout": 22,
        "layer_dependency": "core"
      }
    ],
    "potential_god_clusters": [...],
    "subsystems": [
      {
        "name": "controller_subsystem",
        "size": 8,
        "cohesion_score": 0.81,
        "coupling_score": 0.15,
        "role_distribution": {
          "controller": 6,
          "manager": 2
        }
      }
    ]
  }
}
```

This subsystem enables AI agents to make **architectural-scale optimization decisions** by understanding not just the direct relationships between code entities, but the higher-level conceptual structures and roles that guide the design. The inferred relationship layer allows for more sophisticated reasoning about impact, risk, and benefit of changes across the codebase.

## SemanticAssist v3 – Behavioral & Cross-File Reasoning

The system has been further enhanced with SemanticAssist v3, which adds behavioral analysis of code entities with cross-file execution flow inference. This enables AI agents to understand not just what code entities exist and how they're related, but also how they behave and how their behavior affects the overall system structure.

### Core Features:

* **Behavior Signatures** - Each entity now has a behavior signature that classifies its operational characteristics:
  * `pure`: No detected side-effects, only computation
  * `io_bound`: Interacts with files, console, sockets, or other I/O
  * `stateful`: Modifies global or persistent state
  * `transformer`: Takes input and returns processed output
  * `pipeline_stage`: Categorized as "source", "transform", or "sink"
  * `side_effects`: Array of strings describing specific effects like "modifies_global", "writes_file"
  * `calls_heavy`: Boolean indicating if the function calls complex/high-cost functions
  * `dataflow`: Array of entities that participate in the transformation chain

* **Dataflow Inference** - Identifies execution flow between entities based on:
  * If A calls B and A returns something relying on B's output → B is upstream of A
  * If A passes its own arguments into B → B participates in A's transformation chain

* **Pipeline Inference** - Conceptually categorizes entities into pipeline stages:
  * Sources: Entities with io_bound and no inputs
  * Transforms: Entities that process data flow
  * Sinks: Entities with io_bound that return void

* **Subsystem Behavior Graph** - Represents behavioral relationships between entities:
  * Nodes: Code entities
  * Edges: Based on dataflow, conceptual pipeline stages, and subsystem interactions
  * Relations: "dataflow", "transform", "conceptual", etc.

* **CLI Access to Behavioral Data** - Provides command-line access to behavior signatures and pipeline information

* **Supervisor Integration** - Incorporates behavioral metrics into multi-objective planning and scoring

### Example Behavior Signature:

Entities now include detailed behavior information:

```json
{
  "name": "DataProcessor",
  "kind": "function",
  "file": "src/pipeline/processor.cpp",
  "line": 25,
  "behavior": {
    "pure": false,
    "io_bound": false,
    "stateful": true,
    "transformer": true,
    "pipeline_stage": "transform",
    "side_effects": ["modifies_global", "writes_file"],
    "calls_heavy": true,
    "dataflow": ["DataReader", "DataValidator"]
  },
  "relations": {...},
  "attributes": {...}
}
```

### New Semantic CLI Commands:

* **semantic_behavior** - Returns all behavior signatures for all entities:
  * Provides comprehensive behavioral information for the entire workspace
  * Enables AI agents to analyze system-wide behavioral patterns

* **semantic_behavior_entity** - Returns only a specific entity's behavior signature:
  * Provides detailed behavior information for one entity
  * Allows focused analysis of specific behavior patterns

* **semantic_behavior_graph** - Returns nodes and edges for behavior graph:
  * Shows behavioral relationships and dependencies
  * Enables graph-based reasoning about system behavior

* **semantic_pipeline** - Returns ordered pipeline stages (sources → transforms → sinks):
  * Provides logical execution flow visualization
  * Enables pipeline-aware refactoring and optimization

### Command Examples:

```bash
# Get all behavior signatures for all entities
theide-cli --workspace-root . --json semantic_behavior

# Get behavior signature for a specific entity
theide-cli --workspace-root . --json semantic_behavior_entity --name DataProcessor

# Get the behavior graph representation
theide-cli --workspace-root . --json semantic_behavior_graph

# Get the pipeline stage organization
theide-cli --workspace-root . --json semantic_pipeline
```

### Supervisor Integration:

The behavioral data enhances Supervisor v3 scoring and planning:

* **Benefit Scoring** - Increases for suggestions that:
  * Reduce pipeline complexity
  * Break long transform chains
  * Isolate IO-bound nodes
  * Simplify stateful logic

* **Cost Scoring** - Increases for:
  * Modifying stateful or IO-bound entities
  * Restructuring multi-stage pipelines

* **Confidence Scoring** - Increases when:
  * Behavior signature strongly supports suggested transformation

* **Behavior Snapshot Enhancement** - Plans now include:
  * Number of sources, transforms, and sinks in the pipeline
  * Longest pipeline chain length
  * Count of stateful, I/O-bound, and pure functions
  * Behavior graph node and edge counts

### JSON Output for Plan with Behavioral Snapshot:

```json
{
  "steps": [...],
  "summary": "Package 'MyApp' shows structural issues...",
  "strategy_info": {...},
  "semantic_snapshot": {
    "cluster_count": 12,
    "entity_count": 87,
    "subsystem_count": 5,
    "behavior": {
      "pure_functions_count": 23,
      "io_bound_count": 12,
      "stateful_count": 8,
      "transformer_count": 31,
      "calls_heavy_count": 5,
      "num_sources": 6,
      "num_transforms": 28,
      "num_sinks": 4,
      "longest_pipeline": 7
    },
    "behavior_graph_nodes": 87,
    "behavior_graph_edges": 142,
    "largest_cluster_size": 15,
    "largest_subsystem_size": 8,
    "avg_subsystem_cohesion": 0.63,
    "avg_subsystem_coupling": 0.21,
    ...
  }
}
```

This subsystem enables AI agents to make **behavior-aware optimization decisions** by understanding not just the structural and architectural relationships between code entities, but also how they behave functionally. The behavioral analysis layer allows for more sophisticated reasoning about side effects, I/O patterns, state management, and pipeline structures, enabling better planning of refactoring and optimization actions that consider both architectural and behavioral impact.

## SemanticAssist v4 – Architectural Pattern Detection

The system has been further enhanced with SemanticAssist v4, which adds architectural pattern and anti-pattern detection with comprehensive architecture diagnostics. This enables AI agents to identify common architectural patterns, detect problematic anti-patterns, and evaluate overall architecture quality using quantitative metrics.

### Core Features:

* **Pattern Detection** - Identifies common architectural patterns in the codebase:
  * `facade`: Entity heavily called by many modules but itself calls few
  * `adapter`: Entity with many thin methods delegating 1:1 to other components
  * `factory`: Entity that creates many different classes/return types
  * `pipeline_stage`: Entities forming sequence source → transform → sink
  * `utility`: Many static functions, low statefulness, high reuse

* **Anti-Pattern Detection** - Identifies problematic architectural anti-patterns:
  * `god_object`: High fan-in + high fan-out + complex entities
  * `feature_envy`: Methods that mostly operate on another object's data
  * `cyclic_dependency`: Multi-node cycles detected in semantic or graph layers
  * `layer_violation`: Lower layers depending on higher ones (SemanticAssist v2 layering)
  * `overloaded_responsibility`: Entity with diverse unrelated responsibilities
  * `dead_component`: No incoming references (unused module)

* **Architecture Diagnostics** - Computes quantitative architecture quality metrics:
  * `cohesion_score`: 0-1 scale measuring how related components are grouped (higher is better)
  * `coupling_score`: 0-1 scale measuring how much components depend on each other (lower is better)
  * `layering_score`: 0-1 scale measuring how well architectural layers are respected (higher is better)
  * `complexity_index`: Scalar measure of overall system complexity
  * `structural_entropy`: Scalar measure of randomness/disorder in the architecture

* **Anomaly Detection** - Identifies architectural outliers whose metrics fall outside statistical norms

* **CLI Access to Architecture Data** - Provides command-line access to detected patterns, anti-patterns, and diagnostic scores

* **Supervisor Integration** - Incorporates architecture diagnostics into multi-objective planning and scoring

### Example Pattern and Anti-Pattern Detection:

Entities now include detected patterns and anti-patterns:

```json
{
  "name": "UserFacade",
  "kind": "class",
  "file": "src/api/user_facade.h",
  "line": 8,
  "patterns": {
    "facade": true,
    "facade_incoming_calls": 42,
    "facade_outgoing_calls": 2
  },
  "antipatterns": {},
  "relations": {...},
  "attributes": {...}
}
```

```json
{
  "name": "GodObject",
  "kind": "class",
  "file": "src/core/god_object.cpp",
  "line": 15,
  "patterns": {},
  "antipatterns": {
    "god_object": true,
    "god_object_fannin": 38,
    "god_object_fanout": 45,
    "god_object_complexity": 127
  },
  "relations": {...},
  "attributes": {...}
}
```

### Architecture Diagnostic Structure:

The global architecture diagnostic includes patterns, anti-patterns, and scores:

```json
{
  "patterns": [
    {
      "entity_name": "UserFacade",
      "patterns": {
        "facade": true
      }
    },
    {
      "entity_name": "PipelineTransformer",
      "patterns": {
        "pipeline_stage": "transform"
      }
    }
  ],
  "antipatterns": [
    {
      "entity_name": "GodObject",
      "antipatterns": {
        "god_object": true,
        "god_object_fannin": 38
      }
    }
  ],
  "scores": {
    "cohesion_score": 0.68,
    "coupling_score": 0.42,
    "layering_score": 0.75,
    "complexity_index": 12.3,
    "structural_entropy": 2.87
  }
}
```

### New Semantic CLI Commands:

* **semantic_patterns** - Returns all detected patterns per entity:
  * Provides comprehensive pattern information for the entire workspace
  * Enables AI agents to identify architectural patterns and leverage them

* **semantic_antipatterns** - Returns all detected anti-patterns per entity:
  * Provides detailed anti-pattern information for remediation planning
  * Enables AI agents to identify and address architectural problems

* **semantic_architecture** - Returns global architecture diagnostics object:
  * Provides overall architecture quality scores and metrics
  * Shows patterns and anti-patterns across the entire codebase

* **semantic_outliers** - Returns entities whose metrics fall outside statistical norms:
  * Identifies architectural outliers that may need special attention
  * Enables detection of anomalous components that don't follow patterns

### Command Examples:

```bash
# Get all detected patterns per entity
theide-cli --workspace-root . --json semantic_patterns

# Get all detected anti-patterns per entity
theide-cli --workspace-root . --json semantic_antipatterns

# Get global architecture diagnostics
theide-cli --workspace-root . --json semantic_architecture

# Get architectural outliers
theide-cli --workspace-root . --json semantic_outliers
```

### Supervisor Integration:

The architecture diagnostics enhance Supervisor v3+ scoring and planning:

* **Benefit Scoring** - Increases for suggestions that:
  * Reduce coupling between pattern clusters
  * Eliminate anti-patterns (god objects, cycles, layer violations)
  * Improve cohesion and layering scores
  * Address dead components identified as anti-patterns

* **Risk Scoring** - Increases when:
  * Touching entities identified as façade or adapter (important architectural components)
  * Modifying tightly coupled clusters with detected patterns
  * Making changes that could create new anti-patterns

* **Confidence Scoring** - Boosted when:
  * Patterns/anti-patterns clearly support the suggestion
  * Suggestion aligns with architectural diagnostic recommendations
  * Architecture diagnostic scores support the proposed change

* **Architecture Snapshot Enhancement** - Plans now include:
  * Global pattern and anti-pattern counts
  * Architecture quality scores (cohesion, coupling, layering, etc.)
  * Identified problematic entities that need attention
  * Architecture diagnostic information in the semantic_snapshot

### JSON Output for Plan with Architecture Diagnostic Snapshot:

```json
{
  "steps": [...],
  "summary": "Package 'MyApp' shows structural issues...",
  "strategy_info": {...},
  "semantic_snapshot": {
    "cluster_count": 12,
    "entity_count": 87,
    "subsystem_count": 5,
    "architecture": {
      "patterns": [
        {"entity_name": "UserFacade", "patterns": {"facade": true}},
        {"entity_name": "DataProcessor", "patterns": {"pipeline_stage": "transform"}}
      ],
      "antipatterns": [
        {"entity_name": "GodObject", "antipatterns": {"god_object": true, "god_object_fannin": 42}}
      ],
      "scores": {
        "cohesion_score": 0.68,
        "coupling_score": 0.42,
        "layering_score": 0.75,
        "complexity_index": 12.3,
        "structural_entropy": 2.87
      }
    },
    "behavior": {...},
    "behavior_graph_nodes": 87,
    "behavior_graph_edges": 142,
    "largest_cluster_size": 15,
    ...
  }
}
```

This subsystem enables AI agents to make **architecture-aware optimization decisions** by understanding not just the structural, conceptual, and behavioral aspects of code entities, but also the higher-level architectural patterns and anti-patterns present in the system. The pattern detection and diagnostic capabilities allow for sophisticated reasoning about architectural quality, enabling AI agents to suggest improvements that enhance cohesion, reduce coupling, fix anti-patterns, and improve overall system architecture.

## Change Scenario Simulator & Auto-Plan Executor

The system now includes a Change Scenario Simulator & Auto-Plan Executor that allows AI agents and developers to plan, simulate, and execute complex code transformation scenarios in a controlled, deterministic manner. This subsystem enables strategic planning of codebase changes with predictive impact assessment before applying any real modifications.

### Core Features:

* **Scenario Planning** - Define sequences of actions as structured plans:
  * `type`: "playbook", "command", or "refactor" for different action categories
  * `target`: Specific playbook name, command, or refactoring operation
  * `params`: Action-specific parameters and configuration

* **Impact Simulation** - Estimate the effects of planned changes without applying them:
  * Predicts metric changes using heuristics (complexity reduction, cohesion improvement, etc.)
  * Computes before/after snapshots of telemetry, semantic, and architectural metrics
  * Provides quantified deltas showing expected improvements or risks

* **Controlled Execution** - Apply scenarios with full visibility and deterministic behavior:
  * Executes the sequence of actions as specified in the plan
  * Provides real before/after snapshots showing actual impact
  * Supports both simulation and application modes with clear distinction

* **Multi-Objective Scoring** - Evaluates scenarios using the multi-objective framework:
  * Benefit, cost, risk, and confidence scoring for the entire scenario
  * Integration with Supervisor v3 multi-objective metrics
  * Quantified impact assessment across multiple dimensions

### Scenario Plan Structure:

Scenarios are defined as JSON objects with the following structure:

```json
{
  "name": "auto_plan_1",
  "actions": [
    {
      "type": "playbook",
      "target": "cleanup_includes_and_rebuild",
      "params": { "package": "MyApp" }
    },
    {
      "type": "command",
      "target": "optimize_package",
      "params": { "name": "MyApp", "max_iterations": 5 }
    }
  ]
}
```

### Scenario Result Structure:

Simulation and execution results include comprehensive before/after analysis:

```json
{
  "plan": {
    "name": "auto_plan_1",
    "actions": [...]
  },
  "before": {
    "telemetry": { "complexity_index": 0.81 },
    "architecture": { "scores": { "cohesion": 0.42 } },
    "score": 0.35
  },
  "after": {
    "telemetry": { "complexity_index": 0.68 },
    "architecture": { "scores": { "cohesion": 0.55 } },
    "score": 0.52
  },
  "deltas": {
    "complexity_index": -0.13,
    "cohesion": 0.13,
    "score": 0.17
  },
  "applied": false
}
```

### Available Scenario Commands:

* **build_scenario** - Generates a scenario plan from Supervisor suggestions:
  * Creates a structured plan by converting optimization suggestions to scenario actions
  * Allows specifying maximum number of actions to include
  * Uses Supervisor v3 multi-objective scoring for action selection and ordering

* **simulate_scenario** - Estimates the impact of a scenario without real changes:
  * Takes a scenario plan as input (JSON string or file path)
  * Computes predicted before/after metrics using heuristics
  * Returns impact assessment with no actual file modifications
  * `applied` field is always `false` for simulation

* **apply_scenario** - Executes a scenario by applying real changes:
  * Takes a scenario plan as input (JSON string or file path)
  * Applies the sequence of actions to the codebase
  * Computes actual before/after metrics after changes
  * `applied` field is always `true` for real application
  * Returns patch information including `unified_diff` and `file_changes` fields

### Patch & Rollback Layer (Scenario Diffs & Reverts)

The system now includes comprehensive patch and rollback capabilities that allow you to preview, export, and revert changes made by scenarios:

* **scenario_diff** - Generate a diff for a scenario without applying it:
  * Creates a unified diff representation of what a scenario would change
  * Returns the same format as apply_scenario but with no actual changes made
  * Shows exact lines that would be added, removed, or modified

* **scenario_revert** - Revert changes based on a patch:
  * Applies the inverse of a patch to undo changes made by a previous scenario
  * Takes either a patch string directly or a path to a patch file
  * Can undo changes made by apply_scenario or other patch-generating operations

### Command Examples:

```bash
# Generate a scenario plan for a package (max 3 actions)
theide-cli --workspace-root . --json build_scenario --package MyApp --max_actions 3 > plan.json

# Simulate the impact of the plan (no real changes)
theide-cli --workspace-root . --json simulate_scenario --plan-file plan.json

# Apply the scenario (real changes applied)
theide-cli --workspace-root . --json apply_scenario --plan-file plan.json

# Build + simulate + diff only (show what will change)
theide-cli --workspace-root . --json build_scenario --package MyApp > plan.json
theide-cli --workspace-root . --json scenario_diff --plan-file plan.json

# Apply scenario and capture patch:
theide-cli --workspace-root . --json apply_scenario --plan-file plan.json > result.json
# Extract unified_diff from result.json and save to patch.txt

# Revert later:
theide-cli --workspace-root . --json scenario_revert --patch-file patch.txt
```
```

### Supervisor Integration:

The scenario system integrates with the AI Supervisor for strategic planning:

* **Automatic Plan Generation** - `build_scenario` uses Supervisor's multi-objective optimization:
  * Selects top-graded suggestions based on benefit/cost/risk/confidence scores
  * Converts suggestions to appropriate scenario action types (playbook, command, refactor)
  * Orders actions based on dependency relationships and strategic priorities

* **Multi-Objective Scoring** - Scenarios benefit from the full suite of metrics:
  * Complexity reduction predictions
  * Cohesion and coupling improvements
  * Architectural pattern enhancement
  * Behavioral structure optimization

* **Semantic Awareness** - Leverages SemanticAssist for impact prediction:
  * Predicts effects on subsystem boundaries
  * Anticipates changes to entity roles and relationships
  * Estimates pipeline structure modifications
  * Forecasts pattern and anti-pattern changes

### Deterministic and Auditable:

* **Explicit JSON Plans** - All scenarios are represented as explicit JSON documents:
  * Versionable in source control
  * Auditable before execution
  * Reversible by storing before/after states

* **Predictable Simulation** - Simulation mode provides consistent, reproducible estimates:
  * No actual file changes during simulation
  * Deterministic heuristic-based impact prediction
  * Clear separation between simulation and application

* **Controlled Application** - Application mode executes through existing CLI core:
  * Uses established refactoring, playbook, and optimization mechanisms
  * Maintains full logging and error handling
  * Provides real metrics after changes are applied

This subsystem enables AI agents to make **strategic, multi-step optimization decisions** by planning complex sequences of changes, predicting their impact before application, and executing them in a controlled, deterministic way. The clear separation between simulation and application provides a safe environment for experimenting with optimization strategies while maintaining full control over actual codebase modifications.

## AI Change Proposal Export (CI/Review Integration v1)

The system now includes an AI Change Proposal Export mechanism that produces fully structured "AI Change Proposals" suitable for CI pipelines, code review bots, pull request creation layers, and human review workflows. This feature combines supervisor reasoning, scenario simulation, patch & delta analysis, semantic/architecture insights, risk analysis, and a reproducible execution recipe (playbook) - all while remaining deterministic and headless (NOGUI, CLI-core only).

### Core Features:

* **Structured Proposal Generation** - Combines multiple analysis layers into a comprehensive proposal:
  * `supervisor_plan`: Raw supervisor optimization plan with multi-objective metrics
  * `scenario_plan`: Scenario plan derived from supervisor recommendations
  * `simulation_before/after`: Telemetry, semantic, and architecture snapshots
  * `deltas`: Differences between before and after states
  * `semantic_snapshot`: Semantic analysis of the codebase
  * `architecture_snapshot`: Architecture diagnostic information
  * `risk_score`, `confidence_score`, `benefit_score`: Multi-objective metrics
  * `metadata`: Free-form tags including strategy, playbook, timestamp, etc.

* **Deterministic and Reproducible** - Each proposal includes:
  * Stable ID generated from serialized scenario plan (SHA1 hash)
  * Complete context needed to reproduce the analysis
  * No actual edits are applied during proposal generation

* **Multi-Format Export** - Supports both structured and human-readable formats:
  * JSON format for machine processing in CI systems
  * Markdown format for human review and pull request descriptions

### Available Export Command:

* **export_proposal** - Generates a complete AI change proposal without applying changes:
  * Combines supervisor reasoning with scenario simulation
  * Computes benefit, risk, and confidence scores
  * Captures semantic and architecture snapshots
  * Outputs deterministic, review-ready proposal bundle

### Command Examples:

```bash
# Generate JSON proposal for a package
theide-cli --workspace-root . --json export_proposal --package MyApp > proposal.json

# Generate markdown report for PR summary
theide-cli --workspace-root . --format md export_proposal --package MyApp > proposal.md

# Generate proposal with custom max actions
theide-cli --workspace-root . --json export_proposal --package MyApp --max_actions 10 > proposal.json
```

### JSON Output Structure:

```json
{
  "id": "a1b2c3d4e5f6...",
  "package": "MyApp",
  "supervisor_plan": {
    "summary": "Package 'MyApp' shows structural issues...",
    "steps": [...],
    "semantic_snapshot": {...}
  },
  "scenario_plan": {
    "name": "default_scenario",
    "actions": [...],
    "metadata": {...}
  },
  "simulation_before": {
    "telemetry": {...},
    "semantic": {...},
    "architecture": {...},
    "score": 0.72
  },
  "simulation_after": {
    "telemetry": {...},
    "semantic": {...},
    "architecture": {...},
    "score": 0.85
  },
  "deltas": {...},
  "patch": "",  // Empty in v1 since no changes are applied
  "file_changes": [],
  "semantic_snapshot": {...},
  "architecture_snapshot": {...},
  "risk_score": 0.28,
  "confidence_score": 0.85,
  "benefit_score": 0.13,
  "metadata": {
    "timestamp": "2023-11-01 10:30:45",
    "strategy": "default"
  }
}
```

### Markdown Output Structure:

The markdown output contains:
* High-level summary of the change proposal
* Deltas and impact analysis
* Reasoning highlights from the supervisor
* Selected actions with benefit/risk considerations
* Semantic and architecture risk indicators
* Patch section as fenced code block (empty in v1)

### Integration with CI/Review Workflows:

* **Machine Review**: CI pipelines can process JSON proposals to validate quality gates
* **Code Review Bots**: Automated systems can analyze risk scores and flag concerns
* **PR Generation**: Structured proposals feed directly into pull request creation tools
* **Human Review**: Markdown reports provide contextual information for developers

### Key Benefits:

* **Transparency**: Complete audit trail of AI decision-making process
* **Reproducibility**: Deterministic IDs enable consistent tracking across systems
* **Safety**: No changes applied during proposal generation phase
* **Comprehensive Analysis**: Combines multiple AI analysis layers for informed decisions
* **Workflow Integration**: Formats designed for modern development toolchains

This subsystem enables AI agents to produce **machine-reviewable, human-interpretable change proposals** that bridge the gap between automated analysis and human oversight, supporting both CI systems and code review processes with comprehensive, structured information about proposed changes and their expected impact.
