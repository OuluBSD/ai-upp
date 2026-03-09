# Preferences Layout File

## Full GUI layout inventory for the Preferences window

This document is a **layout-oriented specification** for the Preferences
window. It is written as if it were the source-of-truth UI contract for
implementation in U++.

Goals:

-   enumerate **all captured GUI elements**
-   preserve visible labels and group titles
-   describe page and tab structure
-   provide implementation-oriented control mapping
-   optionally link each control group to backend ownership

This is intentionally more concrete than `PREFERENCES_SPEC.md`.\
Think of it as the missing bridge between screenshots and `.lay` /
constructor code.

------------------------------------------------------------------------

# 1. Window Skeleton

## Window title

``` text
Preferences
```

## Top-level structure

``` text
PreferencesWindow
├─ LeftNavPanel
│  ├─ NavigationList
│  └─ ResetToDefaultsButton
├─ RightContentPanel
│  └─ ActivePageHost
└─ BottomButtons
   ├─ OK
   ├─ Cancel
   └─ Apply
```

## Recommended U++ container layout

``` cpp
class PreferencesWindow : public TopWindow {
public:
    typedef PreferencesWindow CLASSNAME;

    ParentCtrl left_panel;
    ParentCtrl right_panel;

    ArrayCtrl nav;
    ParentCtrl page_host;

    Button reset_defaults;
    Button ok;
    Button cancel;
    Button apply;
};
```

------------------------------------------------------------------------

# 2. Left Navigation Layout

## Navigation entries (captured)

``` text
Appearance
Application
Python interpreter
Keyboard shortcuts
Code Analysis
Completion and linting
Debugger
Editor
Files
Help
History
IPython console
Profiler
Run
Status bar
Variable explorer
Working directory
Plugins
```

## Reset button text

``` text
Reset to defaults
```

------------------------------------------------------------------------

# 3. Bottom Button Row

Buttons captured:

``` text
OK
Cancel
Apply
```

Dialog-level left button:

``` text
Reset to defaults
```

------------------------------------------------------------------------

# 4. Appearance Page

## Layout

``` text
AppearancePage
├─ Group: Main interface
│  └─ Interface theme [DropList]
├─ Group: Syntax highlighting theme
│  ├─ Theme [DropList]
│  ├─ Edit selected theme [Button]
│  ├─ Reset to defaults [Button]
│  ├─ Create new theme [Button]
│  └─ Delete theme [Button]
├─ Group: Fonts
│  ├─ Monospace [font face DropList]
│  ├─ Monospace size [EditIntSpin]
│  ├─ Interface [font face DropList]
│  ├─ Interface size [EditIntSpin]
│  └─ Use the system default interface font [Option]
└─ Group: Previews
   ├─ Editor preview [CodeEditor / preview ctrl]
   └─ Interface font preview [Label / preview box]
```

## Visible texts

``` text
Main interface
Interface theme
Syntax highlighting theme
Edit selected theme
Reset to defaults
Create new theme
Delete theme
Fonts
Monospace
Interface
Use the system default interface font
Previews
Editor
Interface font
Sample text
```

## Backend links

-   ThemeManager
-   EditorThemeManager
-   FontPolicy
-   IDEWindow::ApplyTheme()
-   EditorManager::ApplyEditorTheme()

------------------------------------------------------------------------

# 5. Application Page

## Tabs

``` text
Interface
Advanced settings
```

## 5.1 Interface tab

``` text
ApplicationInterfaceTab
├─ Group: Screen resolution
│  ├─ Normal [radio]
│  ├─ Enable auto high DPI scaling [radio]
│  ├─ Set a custom high DPI scaling [radio]
│  ├─ Custom scaling value [EditString / numeric]
│  └─ help icons
└─ Group: Panes
   ├─ Show friendly message when some panes are empty [Option]
   ├─ Vertical tabs in panes [Option]
   ├─ Custom margin for panes [Option]
   ├─ pane margin value [EditIntSpin]
   ├─ Cursor blinking [Option]
   └─ cursor blink value ms [EditIntSpin]
```

Visible texts:

``` text
Screen resolution
Configuration for high DPI screens
Normal
Enable auto high DPI scaling
Set a custom high DPI scaling
Panes
Show friendly message when some panes are empty
Vertical tabs in panes
Custom margin for panes:
pixels
Cursor blinking:
ms
```

Backend links:

-   DisplayPolicy
-   DockingTabPolicy
-   PaneEmptyStatePolicy
-   CursorBlinkPolicy

## 5.2 Advanced settings tab

``` text
ApplicationAdvancedTab
├─ Language [DropList]
├─ Rendering engine [DropList]
├─ Use a single instance [Option]
├─ Prompt when exiting [Option]
├─ Show internal Spyder errors to report them to GitHub [Option]
├─ Check for updates on startup [Option]
├─ Check for stable releases only [Option]
└─ Disable zoom with Ctrl/Cmd + mouse wheel [Option]
```

Visible texts:

``` text
Language:
Rendering engine:
Use a single instance
Prompt when exiting
Show internal Spyder errors to report them to GitHub
Check for updates on startup
Check for stable releases only
Disable zoom with Ctrl/Cmd + mouse wheel
```

Backend links:

-   ApplicationLifecycle
-   SingleInstanceGuard
-   ErrorReportingPolicy
-   UpdateCheckPolicy
-   EditorZoomPolicy

------------------------------------------------------------------------

# 6. Python interpreter Page

``` text
PythonInterpreterPage
├─ Group: Python interpreter
│  ├─ Internal (same used by Spyder) [radio]
│  ├─ Selected interpreter: [radio]
│  ├─ interpreter path [EditString]
│  └─ browse button [Button]
├─ Group: Conda executable
│  ├─ Use a custom Conda/Mamba/Micromamba executable [Option]
│  ├─ conda executable path [EditString]
│  └─ browse button [Button]
└─ Group: User Module Reloader (UMR)
   ├─ Enable UMR [Option]
   ├─ Print list of reloaded modules [Option]
   └─ Select modules to exclude from being reloaded [Button]
```

Visible texts:

``` text
Python interpreter
Select the default Python interpreter for new IPython consoles and Editor code completion
Internal (same used by Spyder)
Selected interpreter:
Conda executable
Use a custom Conda/Mamba/Micromamba executable
User Module Reloader (UMR)
Enable UMR
Print list of reloaded modules
Select modules to exclude from being reloaded
```

Backend links:

-   ByteVMInterface
-   InterpreterLocator
-   CondaLocator
-   UMRPolicy
-   CompletionEnvironmentPolicy

------------------------------------------------------------------------

# 7. Keyboard shortcuts Page

``` text
KeyboardShortcutsPage
├─ instruction label
├─ ShortcutsTable [ArrayCtrl]
│  ├─ Context
│  ├─ Name
│  └─ Shortcut
└─ SearchRow
   ├─ Type to search [EditString]
   └─ Search button [Button]
```

Visible texts:

``` text
Customize a shortcut by double-clicking on its entry below.
Context
Name
Shortcut
Type to search
```

Backend links:

-   ActionRegistry
-   ShortcutRegistry
-   MenuBuilder
-   ToolBarBuilder

------------------------------------------------------------------------

# 8. Code Analysis Page

``` text
CodeAnalysisPage
├─ Group: Settings
│  └─ Save file before analyzing it [Option]
├─ Group: History
│  ├─ History: [EditIntSpin]
│  └─ results [Label]
└─ Group: Results
   ├─ Results are stored here: [Label]
   └─ path label [Label]
```

Visible texts:

``` text
Settings
Save file before analyzing it
History
The following option will be applied at next startup.
History:
results
Results
Results are stored here:
```

Backend links:

-   CodeAnalysisManager
-   AnalysisHistoryStore

------------------------------------------------------------------------

# 9. Completion and linting Page

## Tabs

``` text
General
Linting
Introspection
Code formatting
Advanced
Other languages
Snippets
```

## 9.1 General tab

``` text
CompletionGeneralTab
├─ Group: Completions
│  ├─ Show completion details [Option]
│  ├─ Enable code snippets [Option]
│  ├─ Show completions on the fly [Option]
│  ├─ Show automatic completions after characters entered: [EditIntSpin]
│  ├─ Show completion details after keyboard idle (ms): [EditIntSpin]
│  └─ Time to wait for all providers to return (ms): [EditIntSpin]
└─ Group: Providers
   ├─ Enable Fallback provider [Option]
   ├─ Enable Language Server Protocol (LSP) provider [Option]
   └─ Enable Text snippets provider [Option]
```

Visible texts:

``` text
Completions
Show completion details
Enable code snippets
Show completions on the fly
Show automatic completions after characters entered:
Show completion details after keyboard idle (ms):
Time to wait for all providers to return (ms):
Providers
Enable Fallback provider
Enable Language Server Protocol (LSP) provider
Enable Text snippets provider
```

Backend links:

-   CompletionManager
-   LSPClientManager
-   SnippetManager

## 9.2 Linting tab

``` text
CompletionLintingTab
├─ intro text label
├─ Group: Provider
│  ├─ Pyflakes (Basic) [radio]
│  ├─ Flake8 (Intermediate) [radio]
│  ├─ Ruff (Advanced) [radio]
│  └─ Disable linting [radio]
├─ Group: Provider options
│  └─ provider-specific area [ParentCtrl placeholder]
└─ Group: Additional options
   └─ Underline errors and warnings [Option]
```

Visible texts:

``` text
Spyder can highlight syntax errors and possible problems with your code in the editor by using one of the providers below
Provider
Pyflakes (Basic)
Flake8 (Intermediate)
Ruff (Advanced)
Disable linting
Provider options
Additional options
Underline errors and warnings
```

Backend links:

-   LintManager
-   DiagnosticRenderer

## 9.3 Introspection tab

``` text
CompletionIntrospectionTab
├─ Group: Basic features
│  ├─ Enable Go to definition [Option]
│  ├─ Follow imports when going to a definition [Option]
│  ├─ Show calltips [Option]
│  └─ Enable hover hints [Option]
└─ Group: Advanced
   ├─ explanatory label
   └─ preload modules text area [DocEdit]
```

Visible texts:

``` text
Basic features
Enable Go to definition
Follow imports when going to a definition
Show calltips
Enable hover hints
Advanced
Preload the following modules to make completion faster and more accurate:
```

Backend links:

-   SymbolResolver
-   HoverProvider
-   CalltipProvider
-   CompletionWarmupPolicy

## 9.4 Code formatting tab

``` text
CompletionCodeFormattingTab
├─ Group: Code formatting
│  ├─ Choose the code formatting provider: [DropList]
│  └─ Autoformat files on save [Option]
└─ Group: Line length
   ├─ Maximum allowed line length: [EditIntSpin]
   └─ Show vertical line at that length [Option]
```

Visible texts:

``` text
Code formatting
Spyder can use Autopep8 or Black to format your code for conformance to the PEP 8 convention.
Choose the code formatting provider:
Autoformat files on save
Line length
Maximum allowed line length:
Show vertical line at that length
```

Backend links:

-   FormatterManager
-   EditorSavePipeline

## 9.5 Advanced tab

``` text
CompletionAdvancedTab
├─ title / warning labels
├─ Enable advanced settings [Option]
├─ Module for the Python language server: [EditString]
├─ IP Address and port to bind the server to:
│  ├─ address [EditString]
│  └─ port [EditIntSpin]
├─ This is an external server [Option]
└─ Use stdio pipes to communicate with server [Option]
```

Visible texts:

``` text
Python Language Server configuration
Warning: Only modify these values if you know what you're doing!
Enable advanced settings
Module for the Python language server:
IP Address and port to bind the server to:
This is an external server
Use stdio pipes to communicate with server
```

Backend links:

-   LSPClientManager
-   LanguageServerProcessPolicy

## 9.6 Other languages tab

``` text
CompletionOtherLanguagesTab
├─ intro label
├─ Available servers table [ArrayCtrl]
│  ├─ Language
│  ├─ Address
│  └─ Command to execute
└─ Button row
   ├─ Add
   ├─ Delete
   └─ Reset / Refresh
```

Visible texts:

``` text
Spyder uses the Language Server Protocol to provide code completion and linting for its Editor.
Available servers:
Language
Address
Command to execute
```

Backend links:

-   ExternalLSPRegistry

## 9.7 Snippets tab

``` text
CompletionSnippetsTab
├─ intro label
├─ Language: [DropList]
├─ Available snippets table [ArrayCtrl]
│  ├─ Trigger text
│  └─ Description
└─ Button row
   ├─ Add
   ├─ Delete
   ├─ Reset / Refresh
   ├─ Import
   └─ Export
```

Visible texts:

``` text
Spyder allows to define custom completion snippets to use in addition to the ones offered by the Language Server Protocol (LSP).
Language:
Available snippets:
Trigger text
Description
```

Backend links:

-   SnippetManager
-   CompletionManager

------------------------------------------------------------------------

# 10. Debugger Page

``` text
DebuggerPage
├─ Group: Interaction
│  ├─ Prevent editor from closing files while debugging [Option]
│  ├─ Stop debugging on first line of files without breakpoints [Option]
│  ├─ Ignore Python libraries while debugging [Option]
│  ├─ Process execute events while debugging [Option]
│  └─ Use exclamation mark prefix for Pdb commands [Option]
├─ Group: Run code while debugging
│  ├─ explanatory label
│  └─ Lines: [EditString]
└─ Group: Execution Inspector
   └─ Exclude internal frames when inspecting execution [Option]
```

Visible texts:

``` text
Interaction
Prevent editor from closing files while debugging
Stop debugging on first line of files without breakpoints
Ignore Python libraries while debugging
Process execute events while debugging
Use exclamation mark prefix for Pdb commands
Run code while debugging
Lines:
Execution Inspector
Exclude internal frames when inspecting execution
```

Backend links:

-   DebugManager
-   FrameFilterPolicy
-   PdbCommandPolicy

------------------------------------------------------------------------

# 11. Editor Page

## Tabs

``` text
Display
Source code
Advanced
```

## 11.1 Display tab

``` text
EditorDisplayTab
├─ Group: Interface
│  ├─ Show tab bar [Option]
│  ├─ Show full file path above editor [Option]
│  ├─ Show class/function selector [Option]
│  └─ Allow scrolling past file end [Option]
├─ Group: Helpers
│  ├─ Show indent guides [Option]
│  ├─ Show code folding [Option]
│  ├─ Show line numbers [Option]
│  ├─ Show debugger breakpoints [Option]
│  └─ Show code annotations [Option]
└─ Group: Highlight
   ├─ Highlight current line [Option]
   ├─ Highlight current cell [Option]
   ├─ Highlight occurrences of selected text after [Option]
   └─ delay ms [EditIntSpin]
```

Visible texts:

``` text
Interface
Show tab bar
Show full file path above editor
Show class/function selector
Allow scrolling past file end
Helpers
Show indent guides
Show code folding
Show line numbers
Show debugger breakpoints
Show code annotations
Highlight
Highlight current line
Highlight current cell
Highlight occurrences of selected text after
milliseconds
```

Backend links:

-   CodeEditor
-   EditorTabBar
-   BreakpointGutter
-   AnnotationRenderer

## 11.2 Source code tab

``` text
EditorSourceCodeTab
├─ Group: Automatic changes
│  ├─ Automatically insert closing parentheses, brackets and braces [Option]
│  ├─ Automatically insert closing quotes [Option]
│  ├─ Automatically insert colons after 'for', 'if', 'def', etc [Option]
│  └─ Automatically un-indent 'else', 'elif', etc [Option]
├─ Group: Trailing whitespace
│  ├─ Strip all trailing spaces on save [Option]
│  ├─ Strip trailing spaces on changed lines [Option]
│  ├─ Automatically add missing end-of-file newline on save [Option]
│  └─ Strip blank lines at end of file on save [Option]
├─ Group: Indentation
│  ├─ Tab width: [EditIntSpin]
│  ├─ Indent with spaces instead of tabs [Option]
│  ├─ Intelligent backspace [Option]
│  └─ Tab always indents [Option]
└─ Group: End-of-line characters
   ├─ Fix mixed end-of-lines automatically and show warning dialog [Option]
   ├─ Convert end-of-line characters to the following on save: [Option]
   └─ EOL mode [DropList]
```

Visible texts:

``` text
Automatic changes
Automatically insert closing parentheses, brackets and braces
Automatically insert closing quotes
Automatically insert colons after 'for', 'if', 'def', etc
Automatically un-indent 'else', 'elif', etc
Trailing whitespace
Strip all trailing spaces on save
Strip trailing spaces on changed lines
Automatically add missing end-of-file newline on save
Strip blank lines at end of file on save
Indentation
Tab width:
spaces
Indent with spaces instead of tabs
Intelligent backspace
Tab always indents
End-of-line characters
Fix mixed end-of-lines automatically and show warning dialog
Convert end-of-line characters to the following on save:
```

Backend links:

-   EditorSavePipeline
-   IndentationPolicy
-   EOLNormalizer

## 11.3 Advanced tab

``` text
EditorAdvancedTab
├─ Group: Template
│  └─ Edit new file template [Button]
├─ Group: Autosave
│  ├─ Automatically save a backup copy of unsaved files [Option]
│  ├─ Autosave interval: [EditIntSpin]
│  └─ seconds [Label]
├─ Group: Docstring style
│  └─ Style: [DropList]
├─ Group: Multi-cursor
│  └─ Enable multi-cursor support [Option]
├─ Group: Multi-cursor paste behavior
│  ├─ Always paste the entire clipboard for each cursor [radio]
│  ├─ Paste one line per cursor if the number of lines and cursors match [radio]
│  └─ Always paste one line per cursor if there is more than one line in the clipboard [radio]
└─ Group: Mouse shortcuts
   └─ Edit mouse shortcut modifiers [Button]
```

Visible texts:

``` text
Template
Edit new file template
Autosave
Automatically save a backup copy of unsaved files
Autosave interval:
seconds
Docstring style
Style:
Multi-cursor
Enable multi-cursor support
Multi-cursor paste behavior
Always paste the entire clipboard for each cursor
Paste one line per cursor if the number of lines and cursors match
Always paste one line per cursor if there is more than one line in the clipboard
Mouse shortcuts
Edit mouse shortcut modifiers
```

Backend links:

-   NewFileTemplateManager
-   AutosaveManager
-   DocstringGenerator
-   MultiCursorController
-   MouseModifierPolicy

------------------------------------------------------------------------

# 12. Files Page

## Tabs

``` text
General
File associations
```

## 12.1 General tab

``` text
FilesGeneralTab
├─ Group: General options
│  ├─ Show hidden files [Option]
│  └─ Single click to open files [Option]
└─ Group: Filter settings
   ├─ explanatory label
   ├─ glob patterns editor [DocEdit]
   ├─ help icon
   └─ Reset to default values [Button]
```

Visible texts:

``` text
General options
Show hidden files
Single click to open files
Filter settings
Filter files by name, extension, or more using glob patterns. Please enter the glob patterns of the files you want to show, separated by commas.
Reset to default values
```

Backend links:

-   FilesPane
-   FileFilterPolicy

## 12.2 File associations tab

``` text
FilesAssociationsTab
├─ intro label
├─ File types group
│  ├─ File types list [ArrayCtrl / ColumnList]
│  ├─ Add [Button]
│  ├─ Remove [Button]
│  └─ Edit [Button]
└─ Associated applications group
   ├─ applications list [ArrayCtrl / ColumnList]
   ├─ Add [Button]
   ├─ Remove [Button]
   └─ Set default [Button]
```

Visible texts:

``` text
Here you can associate different external applications to open specific file extensions (e.g. .txt files with Notepad++ or .csv files with Excel).
File types:
Associated applications:
Add
Remove
Edit
Set default
```

Backend links:

-   FileOpenRouter
-   ExternalAppAssociationStore

------------------------------------------------------------------------

# 13. Help Page

``` text
HelpPage
├─ Group: Automatic connections
│  ├─ Editor [Option]
│  └─ IPython Console [Option]
├─ Group: Additional features
│  └─ Render mathematical equations [Option]
└─ Group: Source code
   └─ Wrap lines [Option]
```

Visible texts:

``` text
Automatic connections
This pane can automatically show an object's help information after a left parenthesis is written next to it.
Editor
IPython Console
Additional features
Render mathematical equations
Source code
Wrap lines
```

Backend links:

-   HelpPane
-   ObjectInspectorHooks
-   RichHelpRenderer

------------------------------------------------------------------------

# 14. History Page

``` text
HistoryPage
└─ Group: Display
   ├─ Wrap lines [Option]
   ├─ Show line numbers [Option]
   └─ Scroll automatically to last entry [Option]
```

Visible texts:

``` text
Display
Wrap lines
Show line numbers
Scroll automatically to last entry
```

Backend links:

-   HistoryPane
-   ConsoleHistoryStore

------------------------------------------------------------------------

# 15. IPython console Page

## Tabs

``` text
Interface
Plotting
Startup
Advanced
```

## 15.1 Interface tab

``` text
ConsoleInterfaceTab
├─ Group: Display
│  ├─ Show welcome message [Option]
│  ├─ Show calltips [Option]
│  └─ Show elapsed time [Option]
├─ Group: Confirmation
│  ├─ Ask for confirmation before closing [Option]
│  ├─ Ask for confirmation before restarting [Option]
│  └─ Ask for confirmation before removing all variables [Option]
├─ Group: Completion
│  ├─ Display: [DropList]
│  ├─ Use Jedi completion [Option]
│  └─ Use greedy completion [Option]
└─ Group: Output
   ├─ Buffer: [EditIntSpin]
   ├─ lines [Label]
   └─ Render SymPy symbolic math [Option]
```

Visible texts:

``` text
Display
Show welcome message
Show calltips
Show elapsed time
Confirmation
Ask for confirmation before closing
Ask for confirmation before restarting
Ask for confirmation before removing all variables
Completion
Display:
Use Jedi completion
Use greedy completion
Output
Buffer:
lines
Render SymPy symbolic math
```

Backend links:

-   ConsoleManager
-   ConsoleWidget
-   KernelLifecyclePolicy

## 15.2 Plotting tab

``` text
ConsolePlottingTab
├─ Group: Matplotlib support
│  ├─ Activate support [Option]
│  └─ Automatically import NumPy and Matplotlib modules [Option]
├─ Group: Graphics backend
│  └─ Backend: [DropList]
└─ Group: Inline backend
   ├─ Format: [DropList]
   ├─ Resolution: [EditString / numeric]
   ├─ Width: [EditIntSpin]
   ├─ Height: [EditIntSpin]
   ├─ Font size: [EditString / numeric]
   ├─ Bottom edge: [EditString / numeric]
   └─ Use a tight layout for inline plots [Option]
```

Visible texts:

``` text
Matplotlib support
Activate support
Automatically import NumPy and Matplotlib modules
Graphics backend
Choose how figures are displayed
Backend:
Inline backend
Settings for figures in the Plots pane
Format:
Resolution:
DPI
Width:
inches
Height:
inches
Font size:
points
Bottom edge:
of figure height
Use a tight layout for inline plots
```

Backend links:

-   PlotBackendPolicy
-   PlotsPane
-   MatplotlibBridge

## 15.3 Startup tab

``` text
ConsoleStartupTab
├─ Group: Run code
│  ├─ explanatory label
│  └─ Lines: [EditString / DocEdit]
└─ Group: Run a file
   ├─ Execute the following file: [Option]
   ├─ file path [EditString]
   └─ browse button [Button]
```

Visible texts:

``` text
Run code
Enter a code snippet to run when a new console is started.
Lines:
Run a file
Specify a Python file to execute at startup, similar to PYTHONSTARTUP
Execute the following file:
```

Backend links:

-   ConsoleStartupPolicy

## 15.4 Advanced tab

``` text
ConsoleAdvancedTab
├─ Group: Autocall
│  └─ Autocall: [DropList]
├─ Group: Autoreload
│  └─ Use autoreload [Option]
├─ Group: Prompts
│  ├─ Input prompt: [EditString]
│  └─ Output prompt: [EditString]
└─ Group: Windows adjustments
   └─ Hide command line output windows generated by the subprocess module [Option]
```

Visible texts:

``` text
Autocall
Autocall:
Autoreload
Use autoreload
Prompts
Input prompt:
Output prompt:
Windows adjustments
Hide command line output windows generated by the subprocess module
```

Backend links:

-   ConsolePromptPolicy
-   AutoreloadPolicy

------------------------------------------------------------------------

# 16. Profiler Page

``` text
ProfilerPage
├─ Open profiler when profiling finishes [Option]
└─ Maximum number of items displayed with large local time [EditIntSpin]
```

Visible texts:

``` text
Open profiler when profiling finishes
Maximum number of items displayed with large local time
```

Backend links:

-   ProfilerManager
-   ProfilerPane

------------------------------------------------------------------------

# 17. Run Page

``` text
RunPage
├─ title / description labels
├─ Runner: [DropList]
├─ Configuration presets table [ArrayCtrl]
│  ├─ Name
│  ├─ File extension
│  └─ Context
├─ preset buttons
│  ├─ Add
│  ├─ Edit
│  ├─ Delete
│  ├─ Duplicate
│  └─ Reset
└─ Group: Editor interactions
   ├─ Save all files before running script [Option]
   └─ Copy full cell contents to the console when running code cells [Option]
```

Visible texts:

``` text
Global presets
The following are the global configuration presets of the different runners that can execute files in Spyder.
Runner:
Configuration presets:
Name
File extension
Context
Editor interactions
Save all files before running script
Copy full cell contents to the console when running code cells
```

Backend links:

-   RunManager
-   RunPresetRegistry
-   RunConfigurationDialog

------------------------------------------------------------------------

# 18. Status bar Page

``` text
StatusBarPage
└─ Group: Display
   ├─ Show memory usage every [Option]
   ├─ memory interval [EditIntSpin]
   ├─ ms [Label]
   ├─ Show CPU usage every [Option]
   ├─ CPU interval [EditIntSpin]
   ├─ ms [Label]
   └─ Show clock [Option]
```

Visible texts:

``` text
Display
Show memory usage every
ms
Show CPU usage every
ms
Show clock
```

Backend links:

-   StatusBarController
-   MemoryUsageWidget
-   CpuUsageWidget
-   ClockWidget

------------------------------------------------------------------------

# 19. Variable explorer Page

``` text
VariableExplorerPage
├─ Group: Filter
│  ├─ Exclude private references [Option]
│  ├─ Exclude capitalized references [Option]
│  ├─ Exclude all-uppercase references [Option]
│  ├─ Exclude unsupported data types [Option]
│  └─ Exclude callables and modules [Option]
└─ Group: Display
   └─ Show arrays min/max [Option]
```

Visible texts:

``` text
Filter
Exclude private references
Exclude capitalized references
Exclude all-uppercase references
Exclude unsupported data types
Exclude callables and modules
Display
Show arrays min/max
```

Backend links:

-   VariableExplorerPane
-   VariableFilterPolicy

------------------------------------------------------------------------

# 20. Working directory Page

``` text
WorkingDirectoryPage
├─ intro label
├─ Group: Startup
│  ├─ The project (if open) or user home directory [radio]
│  ├─ The following directory: [radio]
│  ├─ directory path [EditString]
│  └─ browse button [Button]
└─ Group: New consoles
   ├─ The project (if open) or user home directory [radio]
   ├─ The working directory of the current console [radio]
   ├─ The following directory: [radio]
   ├─ directory path [EditString]
   └─ browse button [Button]
```

Visible texts:

``` text
This is the directory that will be set as the default for the IPython console and Files panes.
Startup
At startup, the working directory is:
The project (if open) or user home directory
The following directory:
New consoles
The working directory for new IPython consoles is:
The project (if open) or user home directory
The working directory of the current console
The following directory:
```

Backend links:

-   WorkingDirectoryPolicy
-   ConsoleManager
-   FilesPane
-   MainToolbar

------------------------------------------------------------------------

# 21. Plugins Page

``` text
PluginsPage
├─ intro label
├─ PluginsList [ArrayCtrl / custom list]
│  ├─ icon
│  ├─ name
│  ├─ description
│  ├─ built-in marker
│  └─ enabled checkbox
└─ Search row
   └─ Type to search [EditString]
```

Visible texts:

``` text
Disable a Spyder plugin (external or built-in) to prevent it from loading until re-enabled here, to simplify the interface or in case it causes problems.
Type to search
```

Backend links:

-   PluginManager
-   PaneRegistry
-   ActionRegistry
-   PreferencesPageRegistry

------------------------------------------------------------------------

# 22. U++ layout template suggestion

For most pages, use this repeated pattern:

``` cpp
class SomePreferencesPage : public PreferencesPage {
public:
    ParentCtrl root;
    LabelBox group_a;
    LabelBox group_b;
    LabelBox group_c;
    // controls...
};
```

------------------------------------------------------------------------

# 23. Suggested backend-link notation

Use compact annotations beside groups when useful:

``` text
[Backend: EditorManager]
[Backend: CompletionManager, LSPClientManager]
[Backend: ByteVMInterface, RunManager]
```

------------------------------------------------------------------------

# 24. Final Summary

This file answers:

> What exact controls must exist on each Preferences page, and what are
> their visible texts?

Use it as the source when building:

-   `.lay` templates
-   manual `Ctrl` construction
-   `Load/Save/Apply` bindings
-   settings structs
-   backend ownership maps
