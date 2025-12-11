# Playbook Engine v1

The Playbook Engine provides a way to automate complex multi-step workflows with built-in safety constraints. It allows AI agents to execute predefined sequences of operations without manually orchestrating each low-level command.

## Core Concepts

### Playbook Structure
A playbook consists of:
- **ID**: Unique identifier for the playbook
- **Description**: Human-readable description of what the playbook does
- **Safety Level**: A value from 0.0 to 1.0 indicating how conservative the playbook is (higher = more conservative)
- **Constraints**: Safety parameters that limit what the playbook can do
- **Steps**: An ordered list of actions to perform

### Constraints
Playbooks support various safety constraints:
- `max_risk`: Maximum risk score allowed for operations (0.0 to 1.0)
- `max_actions`: Maximum number of individual changes allowed
- `allow_apply`: Whether the playbook is allowed to apply changes (boolean)

### Actions
Supported actions in playbooks:
- `scan_workspace`: Analyzes the workspace and stores results in shared state
- `generate_proposal`: Creates an optimization proposal for a package
- `simulate`: Placeholder for simulation operations
- `resolve_conflicts`: Resolves any conflicts that may have occurred
- `apply_if_safe`: Applies changes if they meet safety criteria
- `record_evolution_summary`: Records the evolution summary after changes
- `apply_scenario`: Directly applies a scenario if constraints allow

## Usage Examples

### CLI Usage
```bash
# List available playbooks
theide-cli --workspace-root . --json list_playbooks

# Run a specific playbook
theide-cli --workspace-root . --json run_playbook --playbook_id safe_cleanup_cycle
```

### MCP Usage (via AI agents)
```json
{
  "method": "mcp.tool.execute",
  "params": {
    "name": "list_playbooks",
    "arguments": {
      "workspace_root": "/path/to/workspace"
    }
  }
}
```

```json
{
  "method": "mcp.tool.execute",
  "params": {
    "name": "run_playbook",
    "arguments": {
      "workspace_root": "/path/to/workspace",
      "playbook_id": "safe_cleanup_cycle"
    }
  }
}
```

## Example Playbook Definition

```json
{
  "id": "safe_cleanup_cycle",
  "description": "Run a conservative include/complexity cleanup cycle.",
  "safety_level": 0.9,
  "constraints": {
    "max_actions": 10,
    "max_risk": 0.3,
    "allow_apply": true
  },
  "steps": [
    { "id": "step1_scan", "action": "scan_workspace", "params": {} },
    {
      "id": "step2_propose",
      "action": "generate_proposal",
      "params": {
        "package": "auto_main",
        "max_actions": 10,
        "with_futures": true
      }
    },
    {
      "id": "step3_resolve",
      "action": "resolve_conflicts",
      "params": {}
    },
    {
      "id": "step4_apply_if_safe",
      "action": "apply_if_safe",
      "params": {
        "max_risk": 0.3,
        "require_positive_benefit": true
      }
    },
    {
      "id": "step5_record",
      "action": "record_evolution_summary",
      "params": {}
    }
  ]
}
```

## Safety Features

The Playbook Engine enforces safety through:

1. **Constraint Verification**: Each step checks constraints before execution
2. **Risk Assessment**: Operations are blocked if risk exceeds threshold
3. **Action Limits**: Limits on number of changes that can be applied
4. **Apply Control**: Explicit control over whether changes can be applied
5. **Progressive Execution**: Steps are executed in order with state sharing

This design ensures that complex automation can be safely orchestrated while maintaining full control over the operations performed.