# Task: Evidence Hub Integration

# Status: DONE

# Description
Implement a dedicated workspace for Evidence Packs, allowing users to collect, browse, and export audit-ready evidence for AI-driven changes. This provides transparency and accountability for autonomous actions.

# Objectives
- **Evidence Explorer**: Browse historical `EvidencePack` files stored in `docs/maestro/evidence`.
- **Active Collection**: UI trigger to snapshot current workspace state (git diffs, plan status, AI logs).
- **Verification**: UI for running `SemanticIntegrity` and `AuditTrail` correlation over existing packs.
- **Exporting**: Generate printable or portable reports (PDF/HTML).

# Technical Tasks
- [x] Create `EvidencePane` in `MaestroHub.h/cpp`.
- [x] Basic listing of `.json` evidence files.
- [x] Implement "Verify" logic using `Maestro` integrity checkers.
- [x] Connect "Export PDF" button to `PdfDraw` or `RichText` export.
- [x] Link evidence collection to the AI Assistant's "Finish Task" workflow.

# UI Requirements (WinXP Aesthetic)
- High-density `ArrayCtrl` for evidence items.
- Detail view using `RichTextView` with QTF formatting for deep inspection.
- WinXP-style toolbar with "Collect", "Export", and "Verify" icons.
- Use standard "Report" icons for collection units.