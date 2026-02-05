# Maestro Use Case Scenarios: MegaFileUtil

Project Context: **MegaFileUtil** is a legacy C++ CLI tool for high-performance file renaming, searching, and batch metadata editing. It is being modernized to use the U++ framework.

---

## 1. `init` (Project Initialization)
- **UC 1.1: The Fresh Repo**: A developer just cloned MegaFileUtil and runs `init` to establish the `.maestro` structure and local `docs/` hierarchy.
- **UC 1.2: The Inherited Project**: An architect initializes Maestro in a sub-folder of a large mono-repo to isolate MegaFileUtil's planning from the rest of the company's code.
- **UC 1.3: Re-Initialization**: After a major repo restructuring, the user runs `init --force` to refresh the Maestro metadata while preserving existing runbooks.

## 2. `repo` (Repository Intelligence)
- **UC 2.1: Dependency Discovery**: User runs `repo scan` to find that MegaFileUtil depends on an obscure legacy `regex_lib`.
- **UC 2.2: Assembly Mapping**: User uses `repo list` to visualize how the `core`, `ui`, and `test` packages are grouped into assemblies.
- **UC 2.3: Integrity Check**: User runs `repo validate` to ensure all U++ package files (.upp) correctly reference existing source files.

## 3. `plan` (Task Management)
- **UC 3.1: The Roadmap**: User creates a "Phase 1: U++ Core Port" track and adds tasks for each major class.
- **UC 3.2: Dependency Locking**: User marks "Port Renamer Engine" as dependent on "U++ String Migration".
- **UC 3.3: Progress Audit**: Manager uses `plan list --status todo` to see exactly which porting tasks are unblocked and ready for AI assignment.

## 4. `tu` (Translation Unit Intelligence)
- **UC 4.1: Symbol Search**: User queries `tu query "FileHeader"` to find every header and implementation where the legacy struct is defined.
- **UC 4.2: Build Graph**: User runs `tu build MegaFileUtil` to generate a cached index of includes, helping the AI understand header gravity.
- **UC 4.3: Dead Code Detection**: User uses `tu info` to identify symbols that are defined but never referenced across the package.

## 5. `log` (Log Analysis)
- **UC 5.1: Build Failure Triage**: After a failed `make`, the user runs `log scan` on the build output to extract 50 compilation errors into a structured list.
- **UC 5.2: Runtime Crash Analysis**: User feeds a SEGFAULT backtrace from `gdb` into `log scan` to identify the specific line in `Renamer.cpp` that failed.
- **UC 5.3: Linting Cleanup**: User scans a `clang-tidy` output log to find and group "Naming Convention" warnings.

## 6. `issues` (Issue Tracking)
- **UC 6.1: From Log to Tracker**: User selects a "Null Pointer" finding from a build log and promotes it to a Maestro `issue`.
- **UC 6.2: Bulk Triage**: User selects 10 related "Deprecated Header" issues and uses the `Triage Wizard` to set them all to "analyzed".
- **UC 6.3: Solution Design**: User adds three manual "Solution" notes to an issue before handing it off to the AI for implementation.

## 7. `work` (AI Work Sessions)
- **UC 7.1: The Targeted Fix**: User starts a work session: "Fix the regex crash using the context from Issue #42".
- **UC 7.2: Subwork Branching**: During a large porting session, the AI starts a `subwork` session to specifically refactor a complex macro before continuing.
- **UC 7.3: Session Resumption**: A developer pauses a porting session on Friday and uses `work resume` on Monday to pick up exactly where the AI left off.

## 8. `runbook` (Instruction Design)
- **UC 8.1: The Migration Blueprint**: User designs a "Standard C++ to U++ String" runbook with steps for finding `std::string`, replacing with `Upp::String`, and updating `.upp` files.
- **UC 8.2: AI-Generated Guide**: User provides a freeform prompt "How do I port a Win32 file API to U++?" and uses `runbook resolve` to generate a 5-step interactive guide.
- **UC 8.3: Step Validation**: User executes a "Test Build" step within a runbook to verify that previous manual steps were done correctly.

## 9. `workflow` (State Machines)
- **UC 9.1: The Porting Pipeline**: User creates a PUML workflow defining the states: `Legacy` -> `Analyzed` -> `Mapped` -> `Ported` -> `Verified`.
- **UC 9.2: Visual Debugging**: User visualizes the active porting workflow to see that 15 files are currently in the "Mapped" state.
- **UC 9.3: Automated Transition**: User configures a rule so that a file automatically moves to "Ported" when the AI finishes a `work` session targeting it.

## 10. `settings` (Configuration)
- **UC 10.1: Model Switching**: User switches the global AI model from "Gemini Flash" to "Claude 3.5 Sonnet" for a particularly complex refactoring task.
- **UC 10.2: Path Override**: User configures a custom `docs_root` because their repo doesn't follow the default folder structure.
- **UC 10.3: Simulation Mode**: User toggles `simulate: true` to see what the AI *would* do without actually modifying any files.

## 11. `ops` (Operations)
- **UC 11.1: The Doctor**: User runs `ops doctor` to find that their `umk` path is misconfigured and their AI API key has expired.
- **UC 11.2: Workspace Health**: User runs a "health check" across 5 projects to see which ones have outdated TUs or pending issues.
- **UC 11.3: Cache Cleanup**: User uses `ops cleanup-cache` to remove AI breadcrumbs for sessions older than 30 days.

## 12. `evidence` (Audit & Proof)
- **UC 12.1: Pre-Fix Snapshot**: User runs `evidence collect` before a massive AI refactor to save the exact state of the codebase.
- **UC 12.2: The "Proof of Fix"**: After the AI fixes a bug, the user collects evidence including build logs, test results, and the code diff to include in a PR.
- **UC 12.3: Comparative Audit**: User compares two evidence packs to verify that the AI didn't accidentally delete comments during a porting task.

## 13. `playbook` (Rule sets)
- **UC 13.1: Style Guide Enforcement**: User creates a "U++ Naming Playbook" that instructs the AI to always use `CamelCase` for classes and `snake_case` for variables.
- **UC 13.2: Security Sandbox**: User defines a playbook that forbids the AI from ever using `system()` or `exec()` calls during a refactor.
- **UC 13.3: Playbook Linking**: User links the "Regex Optimization" playbook to a specific `runbook` to ensure the AI uses high-performance patterns.

## 14. `convert` (Batch Orchestration)
- **UC 14.1: The Mass Migration**: User runs `convert run --runbook string_porting --all` to refactor the entire MegaFileUtil codebase in one autonomous pass.
- **UC 14.2: Dry-Run Planning**: User runs `convert plan` to see exactly which files will be modified and in what order before committing to the run.
- **UC 14.3: Regression Replay**: After a batch conversion, the user runs `convert replay` to execute the original MegaFileUtil test suite against the new code to ensure zero regressions.
