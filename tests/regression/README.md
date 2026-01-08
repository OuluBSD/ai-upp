# Regression Lab v1

"Kun agentit alkavat tanssia, me rakennamme lattian."

The Regression Lab is a modular test and comparison framework that allows different AI agents (e.g., Qwen, Claude, Gemini, Codex) to perform the same tasks and compare:

- performance
- safety
- determinism
- impact on the codebase
- conflict proneness
- adherence to project governance rules

In business jargon: we're building an "inter-agent competitive analysis infrastructure" to see who dances to the beat and who steps on toes.

## Architecture

The Regression Lab consists of several components:

1. **RegressionSpec**: Represents a single regression test run with agent identity, workspace, scenario definition, and risk parameters.
2. **RegressionSuite**: A collection of regression specs that can be loaded from JSON files.
3. **RegressionRunner**: Orchestrates the actual execution of regression tests in temporary workspaces.
4. **CoreIde Integration**: Provides access to the regression capabilities from the IDE layer.
5. **CLI Commands**: Command-line interface for triggering regression tests.

## Components

### RegressionSpec
Represents one regression test run with the following properties:
- `agent_name`: Identity of the AI agent to test
- `workspace_root`: Path to the workspace to test on
- `playbook_or_scenario`: Name of the playbook or scenario to execute
- `allowed_risk`: Maximum risk threshold allowed (0.0 to 1.0)
- `max_actions`: Maximum number of actions the agent can take
- `comparison_baselines`: Baseline metrics for comparison

### RegressionSuite
Manages a collection of regression specs with methods for:
- Loading specs from JSON files
- Listing available specs
- Retrieving individual specs by name

### RegressionRunner
Executes regression tests in isolated temporary workspaces with:
- Workspace initialization and cleanup
- Agent invocation (stub in v1, MCP-based in v2)
- Result evaluation and reporting
- Metrics computation and delta analysis

## CLI Commands

### list_regression_specs
Lists all available regression test specifications.

```bash
ai-upp list_regression_specs
```

### run_regression
Executes a specific regression test.

```bash
ai-upp --workspace-root ws/ run_regression --name cleanup_regression --json
```

### compare_regressions
Compares the results of two regression test executions.

```bash
ai-upp compare_regressions --first_report report1.json --second_report report2.json
```

## How to Integrate MCP Agents

In v1, the agent invocation is stubbed. In v2, the `InvokeMCPAgent` method will connect to the MCP server to execute agent commands.

## How Companies Could Use This for Real Code Governance

Organizations can use the Regression Lab to:

1. **Compare AI Agents**: Evaluate different AI coding assistants on identical tasks
2. **Risk Assessment**: Measure how AI changes affect code quality and security
3. **Consistency Testing**: Ensure AI agents adhere to coding standards
4. **Regression Prevention**: Catch AI-induced regressions before they impact production
5. **Governance Compliance**: Verify that AI agents follow organizational policies

## Example Reports

The Regression Lab produces structured JSON reports with metrics on:

- deltas in complexity, cycles, entropy
- whether the agent stayed within allowed risk parameters
- whether conflicts arose during execution
- patch size and structure information
- supervisor score changes

Additionally, Markdown summaries are generated for easy human review.

## Future Compatibility

The Regression Lab is designed with future features in mind:
- Agent personality profiles
- Historical comparisons over time
- AI tournament mode
- Multi-agent negotiation regression