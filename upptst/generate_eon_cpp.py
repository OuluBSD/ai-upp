#!/usr/bin/env python3
"""
Script to generate .cpp files from .eon files for Eon test projects.
"""

import os
import sys
import re
from pathlib import Path

def snake_to_pascal(name):
    """Convert snake_case to PascalCase, e.g., 03a_x11_video -> Run03aX11Video"""
    parts = name.split('_')
    return 'Run' + ''.join(p.capitalize() for p in parts)

def generate_header(project_dir, eon_files):
    """Generate header file for a project."""
    project_name = os.path.basename(project_dir)
    header_guard = f"_{project_name}_{project_name}_h_"

    functions = []
    for eon_file in sorted(eon_files):
        base = os.path.splitext(os.path.basename(eon_file))[0]
        func_name = snake_to_pascal(base)
        functions.append(f"void {func_name}(Engine& eng, int method);")

    content = f"""#ifndef {header_guard}
#define {header_guard}

#include <Shell/Shell.h>

NAMESPACE_UPP

{chr(10).join(functions)}

END_UPP_NAMESPACE

#endif
"""

    header_path = os.path.join(project_dir, f"{project_name}.h")
    with open(header_path, 'w') as f:
        f.write(content)
    print(f"Created: {header_path}")

def read_eon_content(eon_file):
    """Read .eon file content."""
    with open(eon_file, 'r') as f:
        return f.read()

def generate_cpp(project_dir, eon_file):
    """Generate .cpp file for a .eon file."""
    project_name = os.path.basename(project_dir)
    base = os.path.splitext(os.path.basename(eon_file))[0]
    func_name = snake_to_pascal(base)

    eon_content = read_eon_content(eon_file)
    eon_basename = os.path.basename(eon_file)

    content = f"""#include "{project_name}.h"

/*
{eon_content}
*/

NAMESPACE_UPP

void {func_name}(Engine& eng, int method) {{
\tauto sys = eng.GetAdd<Eon::ScriptLoader>();
\tsys->SetEagerChainBuild(true);

\tswitch(method) {{
\tcase 1:
\tcase 2:
\t\tLOG(Format("warning: {func_name}: method %d not implemented yet", method));
\tcase 0:
\t\tsys->PostLoadFile(GetDataFile("{eon_basename}"));
\t\tbreak;
\tdefault:
\t\tthrow Exc(Format("{func_name}: unknown method %d", method));
\t}}
}}

END_UPP_NAMESPACE
"""

    cpp_path = os.path.join(project_dir, f"{base}.cpp")
    with open(cpp_path, 'w') as f:
        f.write(content)
    print(f"Created: {cpp_path}")

def generate_main(project_dir, eon_files):
    """Generate Main.cpp for a project."""
    project_name = os.path.basename(project_dir)

    test_cases = []
    for eon_file in sorted(eon_files):
        base = os.path.splitext(os.path.basename(eon_file))[0]
        func_name = snake_to_pascal(base)
        test_cases.append(f'\t{{ {func_name}, "{func_name}" }},')

    content = f"""#include "{project_name}.h"

using namespace Upp;

namespace {{

struct EngineGuard {{
\tEngine* eng = nullptr;
\t~EngineGuard() {{
\t\tif (eng)
\t\t\tEngine::Uninstall(true, eng);
\t}}
}};

struct TestCase {{
\tvoid (*runner)(Engine&, int);
\tconst char* label;
}};

void ConfigureEngine(Engine& eng, void (*runner)(Engine&, int), int method) {{
\teng.ClearCallbacks();
\teng.WhenInitialize << callback(MachineEcsInit);
\teng.WhenPreFirstUpdate << callback(DefaultStartup);
\teng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
\teng.WhenUserInitialize << callback1(runner, method);
}}

void RunScenario(void (*runner)(Engine&, int), int method, const char* label) {{
\tEngineGuard guard;
\tguard.eng = &ShellMainEngine();
\tEngine& eng = *guard.eng;

\tConfigureEngine(eng, runner, method);

\tValueMap args;
\targs.Add("MACHINE_TIME_LIMIT", 3);

\tif (!eng.StartLoad("Shell", String(), args))
\t\tthrow Exc(String().Cat() << label << ": engine failed to start");

\teng.MainLoop();

\tEngine::Uninstall(true, guard.eng);
\tguard.eng = nullptr;
}}

const TestCase kTests[] = {{
{chr(10).join(test_cases)}
}};

constexpr int kTestCount = int(sizeof(kTests) / sizeof(kTests[0]));

void PrintTestCatalog() {{
\tCout() << "Tests:\\n";
\tCout() << "  -1 runs all tests.\\n";
\tfor(int i = 0; i < kTestCount; i++)
\t\tCout() << "  " << i << ": " << kTests[i].label << '\\n';
}}

void RunSingleTest(int index, int method) {{
\tif (index < 0 || index >= kTestCount)
\t\tthrow Exc(String().Cat() << "invalid test index " << index);
\tRunScenario(kTests[index].runner, method, kTests[index].label);
}}

void RunAllTests(int method) {{
\tfor(int i = 0; i < kTestCount; i++)
\t\tRunScenario(kTests[i].runner, method, kTests[i].label);
}}

}} // namespace

CONSOLE_APP_MAIN {{
\tCommandLineArguments cmd;
\tcmd.AddPositional("test number", INT_V, -1);
\tcmd.AddPositional("method number", INT_V, 0);
\tcmd.AddArg('h', "Show usage information", false);
\tif (!cmd.Parse() || cmd.IsArg('h')) {{
\t\tcmd.PrintHelp();
\t\tPrintTestCatalog();
\t\treturn;
\t}}

\tint test_number = (int)cmd.GetPositional(0);
\tint method_number = (int)cmd.GetPositional(1);

\tif (test_number < -1 || test_number >= kTestCount) {{
\t\tCerr() << "Test number out of range: " << test_number << '\\n';
\t\tcmd.PrintHelp();
\t\tPrintTestCatalog();
\t\treturn;
\t}}

\ttry {{
\t\tif (test_number == -1)
\t\t\tRunAllTests(method_number);
\t\telse
\t\t\tRunSingleTest(test_number, method_number);
\t}}
\tcatch (Exc e) {{
\t\tCout() << "error: " << e << '\\n';
\t\tthrow;
\t}}
}}
"""

    main_path = os.path.join(project_dir, "Main.cpp")
    with open(main_path, 'w') as f:
        f.write(content)
    print(f"Created: {main_path}")

def process_project(project_dir):
    """Process a single project directory."""
    print(f"\nProcessing {project_dir}...")

    # Find all .eon files
    eon_files = sorted(Path(project_dir).glob("*.eon"))

    if not eon_files:
        print(f"No .eon files found in {project_dir}")
        return

    print(f"Found {len(eon_files)} .eon files")

    # Generate header
    generate_header(project_dir, eon_files)

    # Generate .cpp files
    for eon_file in eon_files:
        generate_cpp(project_dir, str(eon_file))

    # Generate Main.cpp
    generate_main(project_dir, eon_files)

def main():
    if len(sys.argv) < 2:
        print("Usage: generate_eon_cpp.py <project_dir1> [project_dir2] ...")
        sys.exit(1)

    for project_dir in sys.argv[1:]:
        if not os.path.isdir(project_dir):
            print(f"Error: {project_dir} is not a directory")
            continue
        process_project(project_dir)

if __name__ == "__main__":
    main()
