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

### Playbook Engine v1 – High-Level Workflows & Safety-Oriented Automation

The system now includes a Playbook Engine that enables high-level, safe automation workflows. Playbooks are declarative, data-driven sequences of operations with built-in safety constraints that allow AI agents to trigger complex procedures without manually orchestrating each low-level command.

Key features:
* **Declarative Workflows** - Playbooks are defined in JSON with ordered steps
* **Safety Constraints** - Built-in limits on risk, actions, and apply operations
* **Multi-Step Orchestration** - Sequences of related operations with shared state
* **Deterministic Execution** - Consistent, auditable results for AI agent trust

Available commands:
* `list_playbooks` - Discover available automation workflows and their safety characteristics
* `run_playbook` - Execute a named playbook with multi-step automation and safety checks

Example usage:
```bash
# List available playbooks
theide-cli --workspace-root . --json list_playbooks

# Run a conservative cleanup playbook
theide-cli --workspace-root . --json run_playbook --playbook_id safe_cleanup_cycle
```

This enables AI agents to perform complex automation tasks with confidence in safety constraints and predictable outcomes.

### MCP Integration (Node.js)

ai-upp-mcp exposes AI-UPP CLI as an MCP server.

* Tools: workspace_overview, optimization_proposal, explore_futures, apply_scenario, revert_patch, evolution_summary, lifecycle_status, list_playbooks, run_playbook.
* See mcp/ai-upp-mcp/README.md for details.

The system now includes MCP (Model Context Protocol) integration via a dedicated Node.js server that exposes AI-UPP functionality to LLM agents. This provides a standardized interface for AI agents to interact with the codebase through structured tool calls.

Key features:
* **Standardized Protocol** - Implements MCP 2024-02-02 specification over stdin/stdout
* **JSON-RPC Interface** - All communication uses structured JSON-RPC format
* **Deterministic Operations** - All tools provide consistent, auditable results
* **No GUI Dependencies** - All operations run in headless mode via the CLI

This integration enables AI agents like Qwen, Claude, and others to perform sophisticated codebase analysis, refactoring, and automation operations by calling these standardized tools.

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

## Supervisor v4 — Adaptive Priority Engine (APE)

The system has been enhanced with an Adaptive Priority Engine (APE) that enables learning from past proposals, applied scenarios, and successes/failures to dynamically adjust suggestion priorities and strategy profile weights. This system implements a persistent "project health model" that can predict which suggestions are likely to yield high-value improvements.

### Core Features:

* **Project Memory System** - Persistent experience log that stores metrics and outcomes of applied changes:
  * **Timestamp** - When the change was applied
  * **Proposal ID** - Unique identifier for the applied change
  * **Metrics Before & After** - Telemetry, semantic, and architectural metrics before and after the change
  * **Deltas** - Computed differences in metrics for impact assessment
  * **Benefit/Risk/Confidence Scores** - Original assessment scores for the change
  * **Applied/Reverted Status** - Whether the change was successfully applied and later reverted

* **Adaptive Priority Engine (APE)** - Heuristic-based learning system that adjusts priorities based on past experiences:
  * **Adaptive Weights** - Four key multipliers that evolve over time:
    * `benefit_multiplier`: How much benefit is weighted
    * `risk_penalty`: How heavily risk is penalized
    * `confidence_boost`: How much to boost confidence
    * `novelty_bias`: How much to favor new types of actions
  * **Dynamic Adjustment** - Weights evolve based on project history:
    * If past accepted changes produced good deltas → increase benefit multiplier
    * If many high-risk suggestions failed → increase risk penalty
    * If high-confidence predictions tend to succeed → increase confidence boost
    * If project stagnates → increase novelty bias (favor new types of actions)

* **Predictive Value Function** - Formula that estimates the likely value of a suggestion:
  ```
  value = s.benefit_score * adaptive.benefit_multiplier
        - s.risk_score     * adaptive.risk_penalty
        + s.confidence_score * adaptive.confidence_boost
        + semantic_novelty(s) * adaptive.novelty_bias;
  ```

* **APE Integration** - The prediction value influences ranking in:
  * `GenerateOptimizationPlan`
  * `BuildScenario`
  * `BuildProposal`

### Project Memory Storage:

* **Location**: Stored at `<workspace-root>/.aiupp/project_memory.json`
* **Format**: U++-style JSON file with complete change history
* **Persistence**: Automatically loaded on workspace initialization and saved after applying scenarios

### Adaptive Learning Logic:

The APE continuously adapts based on project history analytics:

* **AverageBenefit()** - Overall benefit score average across all changes
* **AverageRisk()** - Overall risk score average across all changes
* **AverageConfidence()** - Overall confidence score average
* **CountHighValueChanges()** - Number of changes with benefit_score > 0.7
* **CountFailedChanges()** - Number of changes that were applied but later reverted

### Available APE Commands:

* **supervisor_predict** - Predicts the value of suggestions for a symbol, file, or entity:
  * Inputs: `symbol` OR `file` OR `entity` (one required)
  * Output: Predicted suggestion value, explanation fields, and adaptive weight multipliers

* **memory_export** - Exports the complete project memory history to JSON format:
  * Input: Optional `path` for export location (default: current directory)
  * Output: Number of exported entries and output file path

* **memory_import** - Imports project memory from JSON format, with merge or replace modes:
  * Input: `file` path to import and `mode` (merge or replace)
  * Output: Number of imported entries and import mode used

### Command Examples:

1. After a few accepted proposals, predict suggestion value for a specific symbol:
   ```bash
   theide-cli --json supervisor_predict --symbol "ParseNode"
   ```

2. Export memory for backup or transfer:
   ```bash
   theide-cli --json memory_export > history.json
   ```

3. Import memory into another workspace:
   ```bash
   theide-cli --json memory_import --file history.json
   ```

### JSON Output Example for supervisor_predict:

```json
{
  "predicted_value": 0.65,
  "explanation": {
    "input_symbol": "ParseNode",
    "input_file": "",
    "input_entity": "",
    "benefit_score": 0.5,
    "risk_score": 0.3,
    "confidence_score": 0.7,
    "predicted_value": 0.65
  },
  "adaptive_weights": {
    "benefit_multiplier": 1.1,
    "risk_penalty": 1.0,
    "confidence_boost": 1.2,
    "novelty_bias": 0.1
  }
}
```

### Example Adaptive Evolution:

* **Initial State**: All adaptive weights start at 1.0 (except novelty_bias at 0.1)
* **After Good Changes**: `benefit_multiplier` increases, encouraging more beneficial changes
* **After Failed High-Risk Changes**: `risk_penalty` increases, making the system more cautious
* **After Successful High-Confidence Predictions**: `confidence_boost` increases, trusting confidence estimates more
* **During Project Stagnation**: `novelty_bias` increases, encouraging exploration of new improvement types

### Key Benefits:

* **Learning from Experience**: The system improves over time by learning from past successes and failures
* **Deterministic Heuristics**: No ML libraries required, fully explainable to humans
* **Persistent Memory**: Experience is remembered across sessions in simple JSON format
* **Adaptive Priorities**: Suggestion ranking improves based on what has worked in the specific project
* **Transparent Evolution**: Adaptive weight values can be inspected and understood
* **NoGUI Compatible**: Full functionality available through command-line interface

This subsystem enables AI agents to exhibit **adaptive intelligence** by learning from the outcomes of their suggestions and continuously improving their decision-making process based on project-specific experience, making it more likely to propose high-value improvements over time.

## Cross-Workspace Intelligence (CWI) v1

The system now includes a Cross-Workspace Intelligence (CWI) v1 layer that extends the learning capabilities beyond individual project boundaries. This global memory layer enables AI agents to leverage experience from multiple, unrelated workspaces while maintaining deterministic, heuristic-based, and auditable behavior.

### Core Concepts:

* **Global Knowledge Base** - A persistent knowledge store at the user level (`~/.aiupp/global_knowledge.json`) that aggregates experience across all projects
* **ProjectMemory vs GlobalKnowledge** - Local project memory (`ProjectMemory`) remains for workspace-specific learning, while `GlobalKnowledge` provides cross-workspace intelligence
* **Deterministic Merging** - Statistics are aggregated using additive averages rather than complex model updates
* **NoGUI & Auditable** - All behavior is transparent and explainable through JSON APIs
* **Human-Explainable** - Meta-weights and prediction factors are accessible and interpretable

### Core Features:

* **Pattern Statistics** - Tracks historical success rates, benefits, risks, and confidence for common patterns across all projects
* **Refactor Statistics** - Records usage, success rate, and complexity impact of various refactoring operations
* **Topology Statistics** - Captures architectural metrics and trends across workspaces
* **Workspace Snapshots** - Records state information for cross-workspace analysis
* **Meta-Supervisor** - Adjusts local supervisor priorities based on global statistics

### Global Knowledge Data Structure:

```json
{
  "workspace_snapshots": [
    {
      "workspace_root": "/path/to/workspace",
      "timestamp": 1697582400,
      "scenario_name": "cleanup_includes_v1"
    }
  ],
  "pattern_stats": {
    "cleanup_includes_and_rebuild": {
      "occurrences": 127,
      "successes": 102,
      "failures": 25,
      "avg_benefit": 0.64,
      "avg_risk": 0.23,
      "avg_confidence": 0.78
    }
  },
  "refactor_stats": {
    "rename_symbol_safe": {
      "uses": 89,
      "successful": 82,
      "reverted": 7,
      "avg_delta_complexity": -0.15
    }
  },
  "topology_stats": {
    "avg_cycles": 2.3,
    "avg_depth": 4.7,
    "avg_coupling": 0.31,
    "count": 15
  }
}
```

### Available CWI Commands:

* **global_stats** - Returns aggregated statistics from the global knowledge base
  * Provides cross-workspace pattern, refactor, and topology statistics
  * Enables AI agents to understand general trends across all projects

* **global_predict** - Predicts success likelihood based on global knowledge
  * Accepts `pattern`, `refactor`, or `topology` parameters
  * Returns predicted success rate and contributing factors
  * Includes meta-weights used in prediction

* **export_global_knowledge** - Exports the global knowledge base to a JSON file
  * Enables backup and transfer of cross-workspace intelligence
  * Supports sharing of learned heuristics across different users/environments

* **import_global_knowledge** - Imports global knowledge from a JSON file
  * Merges or replaces the current global knowledge base
  * Enables restoration from backup or integration of external knowledge

### Meta-Supervisor Integration:

The Meta-Supervisor extension uses global statistics to adjust local decision-making:

* **MetaWeights Structure**:
  * `pattern_success_bias`: Adjustment based on historically successful patterns
  * `refactor_success_bias`: Adjustment based on historically risky refactors
  * `topology_risk_adjustment`: Adjustment based on problematic topology patterns

* **PredictValue Integration**: The supervisor's `PredictValue` method now incorporates global insights:
  * Value += meta.pattern_success_bias * pattern_match_score
  * Value -= meta.refactor_success_bias * risky_refactor_factor
  * Value -= meta.topology_risk_adjustment * architecture_risk_factor

### How CWI Influences Supervisor Decisions:

* **Global Pattern Success Rates**: Actions matching historically successful patterns receive higher priority
* **Refactor Risk Adjustment**: Operations similar to historically risky refactors receive additional penalties
* **Topology Trend Awareness**: Workspaces with problematic topologies trigger more conservative strategies

### Command Examples:

```bash
# Get global statistics
theide-cli --json global_stats

# Predict success likelihood for a pattern
theide-cli --json global_predict --pattern pipeline_stage

# Export global knowledge for backup
theide-cli --json export_global_knowledge

# Import global knowledge from another user
theide-cli --json import_global_knowledge --file colleague_knowledge.json
```

### JSON Output Examples:

**global_stats:**
```json
{
  "pattern_stats": {
    "cleanup_includes_and_rebuild": {
      "occurrences": 127,
      "successes": 102,
      "failures": 25,
      "avg_benefit": 0.64,
      "avg_risk": 0.23,
      "avg_confidence": 0.78
    }
  },
  "refactor_stats": {
    "rename_symbol_safe": {
      "uses": 89,
      "successful": 82,
      "reverted": 7,
      "avg_delta_complexity": -0.15
    }
  },
  "topology_stats": {
    "avg_cycles": 2.3,
    "avg_depth": 4.7,
    "avg_coupling": 0.31,
    "count": 15
  }
}
```

**global_predict:**
```json
{
  "prediction": 0.72,
  "factors": {
    "pattern_name": "cleanup_includes_and_rebuild",
    "historical_success_rate": 0.80,
    "pattern_bias": 0.15
  },
  "meta_weights": {
    "pattern_success_bias": 0.15,
    "refactor_success_bias": 0.05,
    "topology_risk_adjustment": 0.2
  }
}
```

### Key Benefits:

* **Cross-Workspace Learning**: Experience from one project improves decisions in others
* **Deterministic Intelligence**: No ML models required, fully explainable heuristics
* **Persistent Global Memory**: Knowledge accumulates across all projects and sessions
* **Meta-Level Adaptation**: Adjusts local behavior based on global patterns
* **Auditable Decisions**: All global influences on decisions are trackable and explainable
* **NoGUI Compatible**: Full functionality available through command-line interface

This subsystem enables AI agents to exhibit **cross-workspace intelligence** by learning from the collective experience across all projects and applying those insights to make better decisions in any individual workspace, creating a continuously improving ecosystem of knowledge and best practices.

## Lifecycle Supervisor v1

The system introduces a Lifecycle Supervisor v1 that uses temporal slopes, semantic complexity, architecture diagnostics, and pattern density to classify the current project phase and influence supervisor decisions.

### Core Concepts:

* **Lifecycle Phases** - Projects are classified into four phases: early_growth, mature, declining, and legacy
* **Phase Detection** - Uses temporal dynamics, architectural metrics, and semantic entropy to determine the current phase
* **Adaptive Decision Making** - Supervisor behavior adapts based on the current lifecycle phase
* **Predictive Analysis** - Projects evolution and supervisor behavior over future events

### Lifecycle Phases:

* **Early Growth** - High growth phase with rapid expansion and frequent refactoring
  * High refactor bias and risk tolerance
  * Aggressive optimization suggestion behavior

* **Mature** - Stable phase with conservative changes and low volatility
  * Low refactor bias and risk tolerance
  * Conservative, risk-averse decision making

* **Declining** - Phase with increasing technical debt and maintenance focus
  * Medium refactor bias with alert behavior
  * Focus on maintainability and stability

* **Legacy** - Low-change phase with minimal interventions due to high risk
  * Minimal refactor bias and very low risk tolerance
  * Maintenance-focused with minimal changes

### Available Lifecycle Commands:

* **lifecycle_phase** - Returns the current lifecycle phase of the project with detailed metrics
* **lifecycle_phases** - Lists all known lifecycle phases with their characteristics
* **lifecycle_predict** - Analyzes how the current lifecycle phase affects supervisor optimization decisions and projects evolution over future events

### Integration with Supervisor:

The lifecycle phase influences the CoreSupervisor's decision-making through modifications to the prediction value calculation:

* **Early Growth**: Increases benefit score by growth_refactor_bonus for aggressive refactoring
* **Mature**: Reduces value by mature_conservatism_bias for conservative behavior
* **Declining**: Increases value by decline_alertness_factor for maintainability focus
* **Legacy**: Reduces value by legacy_risk_multiplier to minimize change risk

### Command Examples:

```bash
# Get current lifecycle phase
theide-cli --workspace-root . --json lifecycle_phase

# List all known lifecycle phases
theide-cli --workspace-root . --json lifecycle_phases

# Predict lifecycle impact on supervisor decisions
theide-cli --workspace-root . --json lifecycle_predict --events 20
```

### JSON Output Examples:

**lifecycle_phase:**
```json
{
  "name": "mature",
  "description": "Stable phase with conservative changes and low volatility",
  "stability": 0.8,
  "volatility": 0.2,
  "refactor_bias": 0.3,
  "metrics_used": {
    "temporal_trend": "slopes_and_variance",
    "architectural_diagnostic": "complexity_and_coupling",
    "semantic_entropy": "computed_entropy"
  }
}
```

**lifecycle_predict:**
```json
{
  "current_phase": "mature",
  "supervisor_impact": {
    "refactor_bias": "conservative",
    "risk_tolerance": "low",
    "optimization_aggression": "low"
  },
  "projected_evolution": [
    {
      "event_number": 1,
      "expected_change": "low",
      "expected_volatility": "low",
      "expected_complexity_change": "stable"
    }
  ]
}
```

This subsystem enables AI agents to exhibit **lifecycle-aware intelligence** by adapting their optimization behavior based on the project's lifecycle phase, ensuring appropriate decision-making strategies for each stage of the project's evolution.

## Lifecycle Supervisor v2 – Phase Drift & Stability

The system has been upgraded to Lifecycle Supervisor v2 with enhanced capabilities for tracking phase history, detecting phase drift, and computing a Longitudinal Stability Index (LSI). This provides AI agents with deeper insights into project evolution patterns and enables more sophisticated drift-aware decision making.

### Core Features:

* **Phase History Tracking** - Records the sequence of lifecycle phases over time with timestamps
* **Phase Drift Detection** - Quantifies how often and how violently the phase changes using metrics:
  * `transitions`: Total number of phase changes
  * `back_and_forth`: Count of oscillations (A→B→A patterns)
  * `avg_phase_duration`: Average time between phase changes
* **Longitudinal Stability Index (LSI)** - A long-term "stability / chaos" score from 0.0 to 1.0:
  * 0.0 = extremely unstable, thrashy
  * 1.0 = very stable, controlled evolution
* **Drift-Aware Decision Making** - Supervisor priorities adapt based on drift metrics and stability index
* **Historical Persistence** - Phase history is saved to `.aiupp/lifecycle.json` in the workspace root

### Heuristic LSI Calculation:

The LSI is computed using a combination of factors:
* Inverse relationship with high transitions and oscillations
* Direct relationship with long phase durations
* Adjustment based on temporal dynamics trend steepness

### Supervisor Integration:

The supervisor now adjusts its decision-making based on drift and stability metrics:

* **Low Stability Index (< 0.3)** - Supervisor becomes highly conservative:
  * Reduces suggestion value based on risk score
  * Focuses on stability and risk mitigation

* **High Drift (many transitions)** - Supervisor applies conservatism:
  * Reduces suggestion value based on cost score
  * Avoids expensive changes in volatile periods

* **High Stability Index (> 0.8) in mature/legacy phases** - Supervisor allows "big moves":
  * Increases suggestion value based on benefit score
  * Enables controlled "macro-refactoring" when stability is high

### Available Lifecycle Supervisor v2 Commands:

* **lifecycle_drift** - Returns phase drift metrics and stability index
  * Phase transition history with timestamps
  * Drift metrics: transitions, back_and_forth, avg_phase_duration
  * Stability index value

* **lifecycle_stability** - Returns Longitudinal Stability Index and classification
  * Stability index (0.0-1.0) with textual classification
  * Explanation of how stability affects supervisor priorities

* **lifecycle_timeline** - Returns complete phase history timeline
  * Array of all recorded phase changes with timestamps

### Command Examples:

```bash
# Get lifecycle drift metrics
theide-cli --workspace-root . --json lifecycle_drift

# Get stability index and classification
theide-cli --workspace-root . --json lifecycle_stability

# Get complete phase history timeline
theide-cli --workspace-root . --json lifecycle_timeline
```

### JSON Output Examples:

**lifecycle_drift:**
```json
{
  "transitions": 5,
  "back_and_forth": 2,
  "avg_phase_duration": 86400.0,
  "stability_index": 0.65,
  "history": [
    {
      "timestamp": 1699123456,
      "phase": "early_growth"
    },
    {
      "timestamp": 1699209856,
      "phase": "mature"
    }
  ]
}
```

**lifecycle_stability:**
```json
{
  "stability_index": 0.65,
  "classification": "managed",
  "supervisor_impact": "Supervisor balances risk and benefit, with moderate caution"
}
```

This subsystem enables AI agents to exhibit **deep lifecycle-aware intelligence** by understanding not just the current lifecycle phase but also the historical patterns of phase evolution. This allows for more sophisticated, context-aware decision-making that adapts to the project's stability profile over time.

## Meta-Orchestrator v1 – Multi-Project Roadmaps

The system includes a Meta-Orchestrator subsystem that coordinates multiple independent workspaces, computes cross-project metrics, and generates roadmap proposals. This enables AI agents to make strategic decisions across multiple projects simultaneously, optimizing for global objectives while managing risk distribution.

### Core Features:

* **Multi-Workspace Coordination** - Manages analysis and planning across multiple independent project workspaces
* **Cross-Project Metrics** - Computes stability, lifecycle phase, entropy, size, and package count across all registered workspaces
* **Risk Distribution** - Avoids refactoring multiple unstable projects simultaneously, distributing risk appropriately
* **Strategic Roadmap Generation** - Creates coordinated roadmap suggestions that optimize for global objectives
* **Strategy Integration** - Uses configurable strategy weights to influence roadmap generation decisions
* **Deterministic Operation** - Provides consistent results for the same workspace state and strategy configuration

### Available Orchestrator Commands:

* **orchestrator_add_workspace** - Registers a workspace root for multi-project analysis
  * Requires a path to the workspace root directory
  * Adds the workspace to the orchestrator's registry for subsequent analysis

* **orchestrator_summaries** - Gets project summaries with computed metrics for all registered workspaces
  * Returns stability, lifecycle phase, entropy, size (LOC), and package count for each project
  * Provides a high-level overview of all registered projects

* **orchestrator_roadmap** - Generates a global roadmap across registered workspaces using specified strategy
  * Supports strategies: `default`, `stability-first`, `risk-distribution`, `sequential`
  * Creates coordinated optimization suggestions across projects
  * Distributes risk and optimizes for global objectives

### Command Usage:

```bash
# Register multiple workspaces for coordinated analysis
theide-cli orchestrator_add_workspace --path /proj/A
theide-cli orchestrator_add_workspace --path /proj/B

# Get summaries of all registered workspaces
theide-cli orchestrator_summaries --json

# Generate a global roadmap with default strategy
theide-cli orchestrator_roadmap --json

# Generate a roadmap with specific strategy (stability-first)
theide-cli orchestrator_roadmap --strategy stability-first --json
```

### Project Summary Structure:

Each project summary includes:

```json
{
  "name": "ProjectName",
  "path": "/path/to/project",
  "stability": 0.65,
  "lifecycle_phase": "mature",
  "entropy": 0.23,
  "size_loc": 45000,
  "packages": 12
}
```

### Global Roadmap Structure:

The roadmap includes strategy information, proposals for each project, and global metrics:

```json
{
  "strategy_name": "stability-first",
  "global_metrics": {
    "project_count": 2,
    "average_stability": 0.55,
    "stability_variance": 0.02,
    "stability_weight": 0.5,
    "risk_weight": 0.3,
    "entropy_weight": 0.2
  },
  "proposals": [
    {
      "project_name": "ProjectA",
      "project_path": "/proj/A",
      "priority": 0,
      "stability": 0.45,
      "risk_score": 0.55,
      "recommendation": {
        "action": "focus on stabilization first"
      }
    },
    {
      "project_name": "ProjectB",
      "project_path": "/proj/B",
      "priority": 1,
      "stability": 0.65,
      "risk_score": 0.35,
      "recommendation": {
        "action": "focus on stabilization first"
      }
    }
  ]
}
```

### Strategy Integration:

The orchestrator integrates with the Dynamic Strategy Engine to allow configurable behavior:

* **stability-first** - Prioritizes the most unstable projects first
* **risk-distribution** - Distributes refactoring risk across projects
* **sequential** - Processes projects in the order they were added
* **default** - Standard optimization approach

This subsystem enables AI agents to make **strategic multi-project decisions** by providing visibility into cross-workspace metrics and coordinated roadmap suggestions that optimize for global objectives while managing risk appropriately.

## Temporal Strategy Engine v1 – Seasonality, Release Cadence & Predictive Stability Windows

The system introduces a Temporal Strategy Engine v1 that analyzes long-term patterns in project lifecycle history to detect seasonality, infer release cadence, and predict optimal stability windows for applying refactors or strategic changes. This enables AI agents to make time-aware optimization decisions that align with natural project rhythms and predictable stability periods.

### Core Features:

* **Seasonality Detection** - Analyzes project history to identify recurring patterns of activity:
  * Uses deterministic heuristics to detect periodic swings in lifecycle phase entropy
  * Identifies patterns such as "pre_release_crunch", "post_release_cleanup", "innovation_cycle"
  * Quantifies intensity (0-1) and confidence (0-1) of detected patterns
  * Records peak activity points in development cycles

* **Release Cadence Inference** - Determines the average interval between release cycles:
  * Identifies stability peaks (periods of low entropy) in project history
  * Calculates average intervals between consecutive stability periods
  * Provides confidence score (0-1) based on consistency of intervals
  * Enables prediction of upcoming release cycle patterns

* **Stability Window Prediction** - Predicts optimal time ranges for safe structural changes:
  * Identifies periods of high stability (low entropy, predictable behavior)
  * Aligns predictions with inferred release cadence for strategic timing
  * Provides safety scores (0-1) for each predicted window
  * Supports strategic planning of refactoring activities

* **Temporal-Aware Supervisor** - The CoreSupervisor now incorporates temporal reasoning into decision making:
  * Adjusts suggestion values based on current position in stability windows
  * Avoids high-cost operations during detected "crunch" periods
  * Aligns large changes with cleanup windows in release cycles
  * Uses temporal weights: `avoid_crunch_multiplier`, `prefer_stability_bonus`, `release_cycle_alignment`

### Available Temporal Strategy Commands:

* **temporal_seasonality** - Returns detected seasonality patterns in project lifecycle
  * Name of each pattern (e.g., "pre_release_crunch", "innovation_cycle")
  * Intensity score (0-1) indicating strength of the pattern
  * Confidence score (0-1) in the detection
  * Array of peak activity timestamps or iteration indices

* **temporal_cadence** - Returns inferred release cadence from project history
  * Average interval between release cycles in analysis windows
  * Confidence score (0-1) in the inferred cadence

* **temporal_windows** - Returns predicted stability windows for safe changes
  * Start and end timestamps or iteration indices for each window
  * Predicted safety score (0-1) for each window

### Integration with CoreSupervisor:

The temporal information influences the CoreSupervisor's decision-making through modifications to the prediction value calculation:

* **Within Stability Windows**: Increases value by `temporal.prefer_stability_bonus * suggestion.confidence_score`
* **During Crunch Periods**: Reduces value by `temporal.avoid_crunch_multiplier * suggestion.cost_score`
* **Aligned with Release Cycles**: Increases value by `temporal.release_cycle_alignment * suggestion.benefit_score` for appropriate changes

### Command Examples:

```bash
# Get detected seasonality patterns
theide-cli --workspace-root . --json temporal_seasonality

# Get inferred release cadence
theide-cli --workspace-root . --json temporal_cadence

# Get predicted stability windows
theide-cli --workspace-root . --json temporal_windows
```

### JSON Output Examples:

**temporal_seasonality:**
```json
{
  "payload": [
    {
      "name": "seasonality_period_5",
      "intensity": 0.72,
      "confidence": 0.58,
      "peaks": [2, 7, 12, 17, 22]
    },
    {
      "name": "pre_release_crunch",
      "intensity": 0.81,
      "confidence": 0.72,
      "peaks": [4, 9, 14, 19, 24]
    }
  ]
}
```

**temporal_cadence:**
```json
{
  "payload": {
    "average_interval": 12,
    "confidence": 0.65
  }
}
```

**temporal_windows:**
```json
{
  "payload": [
    {
      "start": 0,
      "end": 2,
      "predicted_safety": 0.85
    },
    {
      "start": 6,
      "end": 8,
      "predicted_safety": 0.91
    }
  ]
}
```

This subsystem enables AI agents to exhibit **temporal intelligence** by understanding the natural rhythms and predictable patterns in project development cycles. By aligning optimization activities with stability windows and avoiding busy periods, AI agents can make more strategic time-aware decisions that improve success rates and reduce disruption.

## Temporal Strategy Engine v2 – Forecasting & Shock Modeling

The system extends the Temporal Strategy Engine to include forecasting capabilities, shock simulation, and long-term risk modeling. This provides AI agents with predictive capabilities to anticipate future lifecycle phases, model potential disruption events, and incorporate long-term risk considerations into decision-making processes.

### Core Features:

* **Lifecycle Forecasting** - Predicts future lifecycle phases and entropy evolution:
  * Uses entropy trend extrapolation and drift analysis to forecast future states
  * Provides predicted phase names and entropy values for each timestep in the horizon
  * Includes confidence scores that decrease over time to reflect increasing uncertainty
  * Supports configurable forecast horizon (default: 12 timesteps)
  * Identifies trends like "complexity_growth", "stabilization", and continuation of current phases

* **Risk Profile Computation** - Computes comprehensive long-term risk metrics based on historical patterns:
  * **Volatility Risk**: Based on entropy fluctuations and standard deviation of historical entropy values
  * **Schedule Risk**: Quantified from release cadence irregularity and timing inconsistencies
  * **Architectural Risk**: Calculated from entropy trend rates (rapidly increasing entropy indicates higher risk)
  * **Long-term Risk**: Weighted combination of the above risk components with configurable weights
  * **Possible Shocks**: List of potential shock scenarios with computed severity and probability

* **Shock Simulation** - Models potential disruption events and their impacts:
  * Simulates "developer_churn", "api_break", "mass_refactor", "team_reorg", and "dependency_break" scenarios
  * Uses deterministic lookup tables based on historical entropy patterns
  * Computes severity (0-1) based on project's current state and volatility
  * Computes probability (0-1) based on historical patterns and current trends
  * Returns consistent results for the same input conditions

* **Risk-Aware Supervisor** - The CoreSupervisor now incorporates long-term risk into decision-making:
  * **High Long-term Risk**: Reduces suggestion value using `risk_weights.avoid_high_risk * s.risk_score`
  * **Low Volatility Risk**: Increases suggestion value with `risk_weights.reward_low_volatility * s.benefit_score`
  * **High Shock Probability**: Penalizes expensive refactors with `risk_weights.shock_sensitivity * s.cost_score`

### Available Temporal Strategy Engine Commands:

* **temporal_forecast** - Returns lifecycle phase and entropy forecasts for future timesteps
  * `horizon` parameter (default: 12) specifies number of timesteps to predict
  * Each forecast point includes predicted phase, entropy, confidence, and timestep index
  * Confidence decreases over the forecast horizon due to increasing uncertainty

* **temporal_risk** - Returns comprehensive long-term risk profile for the project
  * Returns long-term, volatility, schedule, and architectural risk scores (all 0-1)
  * Includes list of possible shock scenarios with types, severities, and probabilities
  * Computed from historical lifecycle patterns and release cadence irregularity

* **temporal_shock** - Simulates a specific shock event and returns its projected impact
  * `type` parameter specifies shock type: "developer_churn", "api_break", "mass_refactor", "team_reorg", or "dependency_break"
  * Returns severity and probability scores (0-1) for the specified shock type
  * Uses deterministic models based on historical project metrics

### Integration with CoreSupervisor:

The risk profile information influences the CoreSupervisor's decision-making through enhanced prediction value calculations:

* **Risk-Aware Prioritization**: The supervisor now uses `PredictValue(s, mem, risk_profile)` when risk data is available
* **Conservative Bias**: When `risk_profile.long_term_risk > 0.7`, suggestion values decrease to avoid risky changes
* **Opportunity Sensing**: When `risk_profile.volatility_risk < 0.3`, suggestion values increase for beneficial changes
* **Shock Preparation**: When high-probability shocks are detected, expensive refactors are penalized

### Influence on Supervisor Scoring Example:

```cpp
// New risk-aware PredictValue method in CoreSupervisor
double CoreSupervisor::PredictValue(const Suggestion& s,
                                   const ProjectMemory& mem,
                                   const RiskProfile& risk_profile) const {
    double value = /* base calculation */;

    // If long_term_risk is high, decrease suggestion value to be more conservative
    if (risk_profile.long_term_risk > 0.7) {
        value -= risk_weights.avoid_high_risk * s.risk_score;
    }

    // If volatility_risk is low, increase suggestion value (safer time to optimize)
    if (risk_profile.volatility_risk < 0.3) {
        value += risk_weights.reward_low_volatility * s.benefit_score;
    }

    // If any shock_scenario has high probability, penalize expensive refactors
    for (const auto& shock : risk_profile.possible_shocks) {
        if (shock.probability > 0.5) {
            value -= risk_weights.shock_sensitivity * s.cost_score;
            break;
        }
    }

    return value;
}
```

### Command Examples:

```
theide-cli temporal_forecast --horizon 12 --json
theide-cli temporal_risk
theide-cli temporal_shock --type api_break
```

### JSON Output Examples:

**temporal_forecast:**
```json
{
  "payload": [
    {
      "t": 1,
      "predicted_phase": "complexity_growth",
      "predicted_entropy": 0.65,
      "confidence": 0.9
    },
    {
      "t": 2,
      "predicted_phase": "complexity_growth",
      "predicted_entropy": 0.67,
      "confidence": 0.8
    }
  ]
}
```

**temporal_risk:**
```json
{
  "payload": {
    "long_term_risk": 0.68,
    "volatility_risk": 0.45,
    "schedule_risk": 0.72,
    "architectural_risk": 0.55,
    "possible_shocks": [
      {
        "type": "developer_churn",
        "severity": 0.58,
        "probability": 0.62
      },
      {
        "type": "api_break",
        "severity": 0.42,
        "probability": 0.35
      }
    ]
  }
}
```

**temporal_shock:**
```json
{
  "payload": {
    "type": "api_break",
    "severity": 0.42,
    "probability": 0.35
  }
}
```

This subsystem enables AI agents to exhibit **predictive temporal intelligence** by forecasting future lifecycle states, modeling potential disruptions, and incorporating long-term risk considerations into their decision-making processes. By understanding future risk profiles and potential shock events, AI agents can make more strategic and resilient optimization decisions that account for both immediate benefits and long-term stability.

## Strategic Navigator v1 – Multi-Agent Goal-Oriented Planning

The system introduces a Strategic Navigator layer that represents multiple optimization agents, each with its own goal profile, allowing for explicit optimization goals and generating per-agent plans AND a merged, conflict-aware global plan. This enables AI agents to coordinate complex multi-objective optimization strategies while detecting and resolving conflicts between different optimization approaches.

### Key Features:
* **Multi-Agent Architecture** - Supports multiple strategic agents, each with specific optimization goals and preferences
  * `complexity_agent`: Focuses on reducing code complexity and entropy
  * `graph_agent`: Focuses on flattening dependencies and reducing coupling
  * Custom agents can be defined for specific optimization goals
* **Goal-Based Planning** - Agents define explicit optimization goals with configurable weights:
  * `complexity`: Weight for complexity reduction metrics
  * `entropy`: Weight for entropy reduction metrics
  * `coupling`: Weight for coupling reduction metrics
  * `cycles`: Weight for cycle elimination metrics
* **Conflict Detection** - Identifies conflicts between agent plans that could cause incompatible changes
  * Detects file conflicts where multiple agents try to modify the same files
  * Reports type of conflicts and conflicting files
* **Priority-Based Merging** - Creates a global plan by merging individual agent plans prioritizing by goal importance

### Agent Configuration:
Agents are defined in the metadata file `./metadata/agents_v1.json`:
```json
{
  "agents": [
    {
      "name": "complexity_agent",
      "preferences": {
        "strategy": "aggressive_cleanup",
        "risk_tolerance": 0.4
      },
      "goals": [
        {
          "id": "reduce_complexity",
          "description": "Reduce complexity and entropy of core packages.",
          "weights": {
            "complexity": 1.0,
            "entropy": 0.8,
            "coupling": 0.4
          },
          "priority": 0.9
        }
      ]
    },
    {
      "name": "graph_agent",
      "preferences": {
        "strategy": "stability_first",
        "risk_tolerance": 0.2
      },
      "goals": [
        {
          "id": "flatten_dependencies",
          "weights": {
            "cycles": 1.0,
            "coupling": 0.8
          },
          "priority": 0.8
        }
      ]
    }
  ]
}
```

### Available Strategic Navigator Commands:
* **list_agents** - Returns a list of all registered strategic agents with their names, preferences, and goals:
  * Shows agent names, preferences, and goal configurations
  * Enables AI agents to understand the available specialization options

* **agent_plan** - Builds an optimization plan for the specified agent based on its goals and preferences:
  * `agent_name` parameter specifies which agent to generate a plan for
  * Returns agent-specific plan with metadata and full proposal

* **global_plan** - Builds a global optimization plan by merging plans from all registered agents, detecting and resolving conflicts:
  * Returns individual agent plans, detected conflicts, and merged global plan
  * Orders actions by priority considering conflicts between agents

### Command Examples:
```bash
# List all registered agents
theide-cli --json list_agents

# Generate a plan for a specific agent
theide-cli --json agent_plan --agent_name complexity_agent

# Generate a merged global plan from all agents
theide-cli --json global_plan
```

### JSON Output Example for list_agents:
```json
{
  "status": 0,
  "message": "List of agents retrieved successfully",
  "payload": [
    {
      "name": "complexity_agent",
      "preferences": {
        "strategy": "aggressive_cleanup",
        "risk_tolerance": 0.4
      },
      "goals": [
        {
          "id": "reduce_complexity",
          "description": "Reduce complexity and entropy of core packages.",
          "weights": {
            "complexity": 1.0,
            "entropy": 0.8,
            "coupling": 0.4
          },
          "priority": 0.9
        }
      ]
    }
  ]
}
```

### JSON Output Example for agent_plan:
```json
{
  "status": 0,
  "message": "Agent plan generated successfully",
  "payload": {
    "agent_name": "complexity_agent",
    "metadata": {
      "timestamp": "2023-10-15 10:30:45",
      "strategy": "aggressive_cleanup",
      "risk_tolerance": 0.4,
      "goal_ids": ["reduce_complexity"],
      "goals_snapshot": [
        {
          "id": "reduce_complexity",
          "weights": {"complexity": 1.0, "entropy": 0.8, "coupling": 0.4},
          "priority": 0.9
        }
      ]
    },
    "proposal": {
      // CoreProposal structure for the agent
    }
  }
}
```

### JSON Output Example for global_plan:
```json
{
  "status": 0,
  "message": "Global plan generated successfully",
  "payload": {
    "agent_plans": [
      // Array of individual agent plans
    ],
    "conflicts": {
      // Map of detected conflicts between agent plans
    },
    "merged": {
      "ordered_plans": [
        // Agent plans ordered by priority
      ],
      "conflicts_resolved": true,
      "total_agents": 2,
      "conflict_count": 1,
      "conflicts": {
        // Detailed conflict information
      }
    }
  }
}
```

This subsystem enables AI agents to exhibit **strategic coordination** by managing multiple optimization agents with different goals, detecting potential conflicts between their plans, and creating a unified execution sequence that respects priorities and constraints. By coordinating multiple specialized agents, the system can address complex multi-objective optimization problems while maintaining consistency and avoiding contradictory changes.

## Conflict Resolver v1 – Patch-Level Negotiation

The system introduces a Conflict Resolver layer that operates after the Strategic Navigator, focusing on resolving conflicts between AgentPlans at the patch and change-set level. This system compares multiple proposals, identifies conflicts, evaluates trade-offs, and produces a final negotiated result.

### Key Features:
* **Multi-Level Conflict Detection** - Identifies various types of conflicts between agent plans:
  * `edit_overlap`: Two agents modifying the same file and same/nearby line range
  * `semantic_disagreement`: Two agents giving mutually incompatible semantic changes
  * `refactor_collision`: Incompatible refactoring operations like rename vs delete, pipeline reorder vs flatten
* **Trade-Off Evaluation** - Heuristically evaluates trade-offs using benefit, cost, risk, confidence, priorities, lifecycle weights, drift, seasonality, and other metrics
* **Deterministic Negotiation** - Produces reproducible merged plans by resolving conflicts and determining which actions to keep vs discard
* **Priority-Based Resolution** - Honors agent priorities and preferences when resolving conflicts

### Available Conflict Resolver Commands:
* **resolve_conflicts** - Analyzes conflicts between multiple agent plans and produces a negotiated result:
  * Returns list of detected conflicts between agent plans
  * Returns evaluated trade-offs and decisions
  * Returns negotiated result with final actions and discarded actions

### Command Examples:
```bash
# Resolve conflicts between agent plans
theide-cli --json resolve_conflicts
```

### JSON Output Example for resolve_conflicts:
```json
{
  "status": 0,
  "message": "Conflicts resolved successfully",
  "payload": {
    "conflicts": [
      {
        "file": "src/main.cpp",
        "line": 42,
        "type": "edit_overlap",
        "agents": {
          "complexity_agent": {
            "action": {
              "type": "remove_dead_includes",
              "target": "src/main.cpp",
              "params": {}
            }
          },
          "graph_agent": {
            "action": {
              "type": "rename_function",
              "target": "src/main.cpp",
              "params": {
                "old_name": "main",
                "new_name": "application_main"
              }
            }
          }
        },
        "metadata": {
          "severity": "medium",
          "rationale": "Both agents attempting to modify the same file"
        }
      }
    ],
    "tradeoffs": [
      {
        "id": "honor_priority_complexity_agent",
        "description": "Honor higher priority agent: complexity_agent vs graph_agent",
        "score": 0.9,
        "rationale": {
          "reason": "Higher priority agent preference",
          "agent_a_priority": 0.9,
          "agent_b_priority": 0.8
        }
      }
    ],
    "result": {
      "final_actions": [
        {
          "type": "remove_dead_includes",
          "target": "src/main.cpp",
          "params": {}
        }
      ],
      "discarded_actions": [
        {
          "type": "rename_function",
          "target": "src/main.cpp",
          "params": {
            "old_name": "main",
            "new_name": "application_main"
          }
        }
      ],
      "tradeoffs": [
        // Array of trade-off decisions
      ]
    }
  }
}
```

### Trade-off Policies:
* **Lower Risk vs Higher Benefit**: Evaluates which conflicting action presents less risk or higher benefit
* **Lifecycle-Aware Bias**: Considers current project lifecycle phase when evaluating trade-offs
* **Stability Window Alignment**: Prefers actions that align with predicted stability windows
* **Agent Priority Resolution**: Honors agent priorities when other factors are equal
* **Multi-Agent Compromise Model**: Attempts to preserve beneficial aspects of both agent plans where possible

This subsystem enables AI agents to exhibit **patch-level negotiation intelligence** by analyzing and resolving conflicts between different strategic approaches in a deterministic, heuristic-based manner. The system evaluates trade-offs using multiple criteria (risk, benefit, priority) to produce a coherent execution plan that maximizes the value of multiple agent strategies while avoiding contradictory changes.

## Scenario Simulator v2 – Multi-Branch Futures & Outcome Horizon

The system now includes a Scenario Simulator v2 that enables AI agents to explore multiple hypothetical futures based on the current negotiated scenario. This provides pre-scored alternatives instead of just one plan, allowing for more informed decision-making.

### Core Features:

* **Multi-Branch Futures** - Generates different subsets of actions from the same base scenario, creating alternative "branches" to explore
* **Projected Metrics** - Heuristically predicts risk, benefit, complexity/coupling deltas, and other metrics without applying changes
* **Branch Scoring** - Uses lifecycle phase, temporal trends, and stability windows to compute fitness scores for each branch
* **Outcome Horizon** - Returns structured data suitable for AI agents to compare branches and make decisions
* **Deterministic & Heuristic-Based** - All behavior remains predictable and based on heuristics rather than machine learning

### Branch Generation Strategy:

The simulator creates branches such as:
* **"all_actions"** → all negotiated actions
* **"low_risk"** → only actions below some risk threshold
* **"high_benefit"** → top N actions by benefit
* **"stability_aligned"** → actions that fall into safe stability windows
* **"minimal_change"** → smallest set that yields non-trivial improvement

Each branch reuses the same base scenario with a filtered actions vector.

### Core Data Structures:

**FutureBranch** contains:
* `id`: Identifier for the branch (e.g., "branch_low_risk")
* `starting_point`: Snapshot of telemetry + architecture + semantic summary
* `actions`: Scenario actions included in this branch
* `projected_metrics`: Predicted complexity, coupling, cycles, risk, benefit, etc.
* `terminal_state`: Predicted "end" state snapshot
* `score`: Overall fitness score for this branch

**OutcomeHorizon** contains:
* `branches`: List of FutureBranch objects with full details
* `best_branch`: Minimal summary of top-scoring branch
* `stats`: Horizon-level statistics (min/max/avg risk, benefit, etc.)

### Available Commands:

* **explore_futures** - Explore multiple hypothetical futures based on the current negotiated scenario:
  * Generates different branches of actions
  * Scores them based on risk, benefit, complexity, and other metrics
  * Returns an outcome horizon for AI agents to compare

* **export_proposal --with-futures** - Extended version of the export_proposal command that optionally includes outcome horizon:
  * When the `--with-futures` flag is set to true, also calls explore_futures
  * Attaches the outcome_horizon to the proposal result
  * Enables AI agents to choose a specific branch and materialize only that branch

### Command Examples:

```bash
# Explore multiple futures based on the current negotiated scenario
theide-cli --workspace-root . --json explore_futures

# Get a proposal with the outcome horizon included
theide-cli --workspace-root . --json export_proposal --package MyApp --with-futures true

# Compare with regular proposal
theide-cli --json export_proposal --package MyApp
```

### JSON Output Example for explore_futures:

```json
{
  "branches": [
    {
      "id": "branch_all_actions",
      "projected_metrics": {
        "risk": 0.4,
        "benefit": 0.8,
        "complexity": 0.3,
        "coupling": 0.2,
        "cycles": 0.1,
        "complexity_delta": -0.12,
        "coupling_delta": -0.08,
        "entropy_delta": -0.05
      },
      "score": 0.73
    },
    {
      "id": "branch_low_risk",
      "projected_metrics": {
        "risk": 0.22,
        "benefit": 0.65,
        "complexity": 0.25,
        "coupling": 0.18,
        "cycles": 0.08,
        "complexity_delta": -0.08,
        "coupling_delta": -0.05,
        "entropy_delta": -0.03
      },
      "score": 0.81
    },
    {
      "id": "branch_high_benefit",
      "projected_metrics": {
        "risk": 0.55,
        "benefit": 0.92,
        "complexity": 0.45,
        "coupling": 0.35,
        "cycles": 0.15,
        "complexity_delta": -0.18,
        "coupling_delta": -0.12,
        "entropy_delta": -0.08
      },
      "score": 0.68
    }
  ],
  "best_branch": {
    "id": "branch_low_risk",
    "score": 0.81,
    "projected_metrics": {
      "risk": 0.22,
      "benefit": 0.65
    }
  },
  "stats": {
    "avg_score": 0.65,
    "max_score": 0.81,
    "min_risk": 0.22,
    "max_benefit": 0.92,
    "branch_count": 5
  }
}
```

### Integration with AI Drivers:

AI drivers (like Qwen via MCP) can now:
1. Call `export_proposal --with-futures` to get a proposal with the outcome horizon attached
2. Inspect the different branches and their metrics
3. Choose a specific branch ID
4. Subsequently ask CLI/AI-UPP to materialize only that branch via a filtered scenario

For v1, the system reports the horizon and AI agents can choose which branch to pursue. Actual branch-specific application can be added in future versions.

This subsystem enables AI agents to make **strategic multi-branch decisions** by providing multiple pre-scored alternatives based on the same base scenario, allowing for more nuanced and risk-aware planning.

## Meta-Evolution Engine v1 – Evolution Timeline & Change Genome

The Meta-Evolution Engine provides a comprehensive system for recording, analyzing, and learning from the historical evolution of codebases. It maintains a persistent timeline of all significant changes applied to the codebase, along with their outcomes, enabling AI agents to understand what strategies have been most effective in a particular repository.

### Core Concepts

* **EvolutionEvent**: A structured record of a significant change application, containing:
  * `timestamp`: When the event occurred
  * `id`: Unique identifier for the event
  * `package`: The package the event applied to
  * `agent_name`: Which agent / strategy profile triggered the change
  * `scenario_id`: Link to ScenarioPlan / Proposal id
  * `lifecycle_phase`: Lifecycle phase at time of change
  * `strategy`: Active strategy profile name
  * `change_kinds`: Categorical characterization ("rename", "include_cleanup", "graph_simplification", etc.)
  * `metrics_before` and `metrics_after`: Selected metrics snapshots
  * `deltas`: Normalized deltas (complexity, cycles, coupling, etc.)
  * `succeeded`: True if applied & kept
  * `reverted`: True if later undone
  * `context`: Snapshot of drift, stability, seasonality at the time

* **Change "Genome"**: The categorical characterization of change types, stored in `change_kinds`, allows for high-level analysis of what types of changes were applied and their effectiveness.

* **EvolutionSummary**: An aggregated view that provides:
  * Total event counts and success rates
  * Aggregated stats by change kind (what types of changes work best)
  * Success/failure and avg benefit/risk by strategy profile
  * Preferred change kinds by lifecycle phase

### Available Evolution Commands

* **evolution_timeline** - Retrieves the full timeline of evolution events in the repository
* **evolution_summary** - Provides aggregated summary of evolution events and their outcomes

### CLI Usage

```bash
theide-cli --workspace-root . --json evolution_timeline
theide-cli --workspace-root . --json evolution_summary
```

### Data Storage

* **Location**: Evolution data is stored at `<workspace-root>/.aiupp/evolution.json`
* **Persistent**: Events are automatically saved after each significant change application
* **Queryable**: Full timeline and aggregated statistics accessible through JSON APIs

### AI Integration

The Meta-Evolution Engine enables AI agents to ask strategic questions:

* "What usually works in this repo?" - Through `evolution_summary` by change kind and strategy
* "What strategy worked best historically?" - By examining success rates in `by_strategy` section
* "How has the codebase evolved structurally over time?" - Through the full timeline in `evolution_timeline`
* "Are we frequently reverting certain kinds of changes?" - By checking `reverted` flag and `reverted_count`

AI drivers (Qwen via MCP later) can call `evolution_summary` to:
1. Pick strategies that align with past successes
2. Avoid repeating historically bad patterns
3. Adapt their behavior per-repo based on evolution history

### Supervisor Integration

The AI Supervisor consumes evolution data to adjust its behavior:
* Strategies with low success rates (<70%) have their priority reduced over time
* High-success change kinds (like include cleanup with >90% success) get increased priority
* The system learns from historical outcomes to improve future decision-making

### Deterministic & Auditable

* **Heuristic-Based**: All evolution analysis uses deterministic heuristics, no ML required
* **Auditable**: Every recorded change is visible in the timeline with full context
* **NoGUI Compatible**: Full functionality available through command-line interface
* **Reproducible**: Evolution history can be analyzed and acted upon in a deterministic way

This subsystem enables AI agents to exhibit **meta-evolution intelligence** by learning from the historical outcomes of their changes, continuously improving their decision-making based on project-specific experience. By understanding what types of changes have been successful in a particular codebase, AI agents can adapt their strategies to maximize the likelihood of successful outcomes and avoid repeating patterns that historically led to failures or reverts.

## Playbook Test Harness

The system now includes a Playbook Test Harness that provides deterministic, scriptable testing for AI Playbooks, focusing on end-to-end execution and validation of safety constraints.

### Core Features:

* **End-to-End Execution** - Tests complete playbook execution cycles including "safe_cleanup_cycle"
* **Safety Constraint Validation** - Verifies max_risk, max_actions, and allow_apply constraints
* **Deterministic Operation** - All tests run without GUI dependencies, relying only on the AI-UPP CLI with --json flag
* **Telemetry Tracking** - Monitors structural effects, evolution, and diffs in controlled test workspaces

### Available Commands:

* **playbook_self_test** - Runs the playbook test harness for the current workspace (if configured)

### Test Structure:

The harness includes:
* **Test Workspaces** - Dedicated workspaces under `tests/playbooks/workspaces/` for different test scenarios
* **Test Configuration** - Test cases defined in `tests/playbooks/tests_v1.json`
* **Executable Harness** - Bash script `tests/playbooks/run_playbook_tests.sh` that runs all tests and validates expectations

### Test Harness Components:

* **ws_basic_cleanup** - Basic workspace to validate that "safe_cleanup_cycle" performs small, safe cleanups
* **ws_high_risk** - High-risk workspace to test that playbook safety constraints prevent applying changes when max_risk is low
* **Validation Logic** - Uses jq for JSON handling to validate expectations like:
  * Status validation (applied_or_simulated, blocked_by_constraint)
  * Apply validation (no_apply checks)
  * Event count validation (min/max bounds)

### Command Usage:

```
# Run the playbook test harness
tests/playbooks/run_playbook_tests.sh
```

### Example Test Configuration:

```json
{
  "tests": [
    {
      "id": "basic_safe_cleanup_applies",
      "workspace": "tests/playbooks/workspaces/ws_basic_cleanup",
      "playbook_id": "safe_cleanup_cycle",
      "expect": {
        "status": "applied_or_simulated",
        "max_events_increase": 5,
        "min_events_increase": 1,
        "allow_apply": true
      }
    }
  ]
}
```

### Deterministic & Auditable:

* **CLI-Only**: All operations run through command-line interface without GUI dependencies
* **Versionable**: Test configurations and workspaces are stored in the repository
* **Reproducible**: Tests return consistent results for the same workspace state
* **NoGUI Compatible**: Full functionality available through command-line interface

This subsystem enables AI agents to validate their behavior through **comprehensive deterministic testing**, ensuring that safety constraints are properly enforced and that playbook operations behave correctly under various conditions.

