# Scenarios 41-60: Playbooks & Evidence Vault
# Status: TODO

## Objective
Verify visual logic design, variant management, and secure evidence collection.

## Playbook Scenarios (41-50)
41. **New Playbook**: Click 'New' in 'PlaybookPane', name it "Accounting-Auth", and save.
42. **Visual Editor - Puml**: Edit the PlantUML source and verify the 'GraphView' updates its nodes/links.
43. **Graph Interaction**: Click a node in the graph and verify it highlights the corresponding PUML line.
44. **Playbook Validation**: Trigger 'Validate' on an incomplete playbook and verify error markers in the status bar.
45. **Variant Selection**: Create two variants of a step (e.g., "OAuth" vs "Simple") and toggle between them.
46. **Step Editor Wizard**: Open 'StepWizard', use AI Assist to generate a step description, and save.
47. **Toolbar Integration**: Use the Playbook toolbar to 'Zoom' and 'Fit' the graph view.
48. **Playbook Metadata**: Edit the playbook description and verify it persists in the JSON.
49. **Visual Logic Splitter**: Resize the splitter between code and graph and verify persistence.
50. **Playbook Export**: Export a playbook to a standalone file and verify its contents.

## Evidence Vault Scenarios (51-60)
51. **Evidence Collection**: Click 'Collect' and select a build log as evidence.
52. **Evidence Metadata**: Add tags (e.g., "Release", "Build-123") to an evidence item.
53. **Integrity Verification**: Click 'Verify' and ensure the cryptographic hash matches the original file.
54. **Detail Preview**: Select an evidence item (PDF or Image) and verify the preview pane renders it correctly.
55. **Audit Correlation**: Open 'AuditTrailCorrelator' and verify evidence items are linked to specific session events.
56. **PDF Export**: Generate a consolidated PDF report of all evidence for a specific task.
57. **Evidence List Sorting**: Sort by date/size/tag and verify order.
58. **Search - Evidence**: Filter evidence list by filename or tag.
59. **Audit Trail Detail**: Select an event and verify the 'RichText' displays the raw JSON event data.
60. **Event Filtering**: Filter the audit trail by event type (e.g., "AI_CALL", "FILE_SAVE").

## Verification Method
- Graph node counting.
- Hash comparison for evidence.
- PDF file existence check.
