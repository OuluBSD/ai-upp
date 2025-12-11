#!/usr/bin/env node

// Mock theide-cli for testing purposes
import process from 'process';

const args = process.argv.slice(2);

// Simulate different CLI responses based on the arguments
if (args.includes('workspace_stats')) {
  console.log(JSON.stringify({ 
    total_packages: 5, 
    total_files: 120, 
    total_lines: 54321,
    dependencies: 8 
  }));
} else if (args.includes('graph_stats')) {
  console.log(JSON.stringify({ 
    nodes: 25, 
    edges: 42, 
    complexity_score: 7.3 
  }));
} else if (args.includes('export_proposal')) {
  console.log(JSON.stringify({ 
    proposal_id: 'mock-proposal-1',
    suggested_changes: ['refactor Foo class', 'optimize Bar function'],
    estimated_effort: 3
  }));
} else if (args.includes('explore_futures')) {
  console.log(JSON.stringify({ 
    scenario_count: 5,
    outcomes: ['positive', 'neutral', 'positive', 'negative', 'positive']
  }));
} else if (args.includes('apply_scenario')) {
  console.log(JSON.stringify({ 
    success: true,
    files_changed: ['file1.cpp', 'file2.cpp'],
    patch_applied: true
  }));
} else if (args.includes('scenario_revert')) {
  console.log(JSON.stringify({ 
    success: true,
    files_reverted: ['file1.cpp']
  }));
} else if (args.includes('evolution_summary')) {
  console.log(JSON.stringify({ 
    timeline: ['initial', 'feature-added', 'refactor-complete'],
    major_events_count: 12
  }));
} else if (args.includes('lifecycle_phase')) {
  console.log(JSON.stringify({ 
    current_phase: 'maintenance',
    confidence: 0.85
  }));
} else if (args.includes('lifecycle_drift')) {
  console.log(JSON.stringify({ 
    drift_percentage: 12.5,
    drift_trend: 'increasing'
  }));
} else if (args.includes('lifecycle_stability')) {
  console.log(JSON.stringify({ 
    stability_score: 0.78,
    risk_level: 'medium'
  }));
} else {
  // Default response if command not recognized
  console.error('Mock: Command not recognized');
  process.exit(1);
}