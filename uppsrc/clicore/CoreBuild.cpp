#include "clicore.h"

using namespace Upp;

CoreBuild::CoreBuild() {
    build_method = "GCC"; // Default build method
    // Use standard Ultimate++ output directory
    output_dir = GetHomeDirFile(".config/upp.out");
}

CoreBuild::~CoreBuild() {
    // Cleanup
}

bool CoreBuild::BuildProject(const CoreWorkspace::Package& package, const String& config, String& log, String& error) {
    // SIMPLIFIED FOR CLI CORE v1 - simulate a build process using real package data
    String output_dir = GetOutputDirectory();
    if (output_dir.IsEmpty()) {
        error = "Could not determine output directory";
        return false;
    }

    // Create the output directory if it doesn't exist
    if (!DirectoryExists(output_dir)) {
        if (!RealizeDirectory(output_dir)) {
            error = "Could not create output directory: " + output_dir;
            return false;
        }
    }

    // In a real implementation, this would call the actual build system
    // For now, we'll simulate by creating a build log and checking if the package files exist
    log = "Build started for package: " + package.name + "\n";
    log += "Configuration: " + config + "\n";
    log += "Package directory: " + package.path + "\n";
    log += "Upp file: " + package.upp_file + "\n";
    log += "Output directory: " + output_dir + "\n";
    log += "Dependencies: " + Join(package.uses, ", ") + "\n";
    log += "Files to compile:\n";

    // Log information about files in the package
    if (package.files.GetCount() == 0) {
        error = "Package '" + package.name + "' has no files to build";
        return false;
    }

    int cpp_count = 0;
    for (const String& file_path : package.files) {
        log += "  Compiling file: " + file_path + "\n";
        String lext = ToLower(GetFileExt(file_path));
        if (lext == ".cpp"  ||
            lext == ".c"    ||
            lext == ".cc") {
            cpp_count++;
        }
    }

    // Simulate compilation process
    log += "\n" + AsString(cpp_count) + " C++ files processed\n";
    log += "Linking package: " + package.name + " (config: " + config + ")\n";
    log += "SIMULATED BUILD PROCESS COMPLETED\n";
    log += "Build for package '" + package.name + "' completed successfully\n";
    return true;
}

bool CoreBuild::BuildProject(const String& project, const String& config, String& log, String& error) {
    // For backward compatibility - create a minimal package for the string-based approach
    CoreWorkspace::Package pkg;
    pkg.name = project;
    pkg.path = project;  // Use project name as path for backward compatibility
    pkg.upp_file = AppendFileName(project, project + ".upp");  // Assume file is in project dir

    // Try to load the actual .upp file if it exists to populate files
    String upp_content = LoadFile(pkg.upp_file);
    if (!upp_content.IsEmpty()) {
        // This is a simplified approach for backward compatibility
        // In a proper implementation, we'd use the same parsing as in CoreWorkspace
        pkg.files.Add("dummy.cpp");  // Add a dummy file to simulate
    }

    return BuildProject(pkg, config, log, error);
}

bool CoreBuild::CleanProject(const CoreWorkspace::Package& package, String& log, String& error) {
    // SIMPLIFIED FOR CLI CORE v1 - simulate a clean process
    String output_dir = GetOutputDirectory();
    if (output_dir.IsEmpty()) {
        error = "Could not determine output directory";
        return false;
    }

    log = "Clean started for package: " + package.name + "\n";
    log += "Package directory: " + package.path + "\n";
    log += "Output directory: " + output_dir + "\n";
    log += "Files to clean: " + AsString(package.files.GetCount()) + "\n";

    // In a real implementation, this would call the clean functions
    // For now, we'll just check if output directory exists and log what would be cleaned
    if (DirectoryExists(output_dir)) {
        // This is a simulation, so we won't actually delete anything
        log += "Found output directory to clean: " + output_dir + "\n";
        log += "SIMULATED CLEAN PROCESS COMPLETED\n";
        log += "Clean for package '" + package.name + "' completed successfully\n";
        return true;
    } else {
        log += "Output directory does not exist: " + output_dir + "\n";
        error = "Output directory does not exist: " + output_dir;
        return false;
    }
}

bool CoreBuild::CleanProject(const String& project, String& log, String& error) {
    // For backward compatibility - create a minimal package for the string-based approach
    CoreWorkspace::Package pkg;
    pkg.name = project;
    pkg.path = project;  // Use project name as path for backward compatibility
    pkg.upp_file = AppendFileName(project, project + ".upp");  // Assume file is in project dir

    return CleanProject(pkg, log, error);
}

String GetUppOut() {
    // Implementation based on the ide/Assembly.cpp logic
    // GetVar is not available in this context, so we'll use a hardcoded approach

    // Try .config directory first
    String s = GetHomeDirFile(".config/upp.out");
    if (DirectoryExists(s))
        return s;

    // Fallback to .upp directory
    s = GetHomeDirFile(".upp/out");
    if (DirectoryExists(s))
        return s;

    // Create .config/upp.out if it doesn't exist
    if (RealizeDirectory(GetHomeDirFile(".config")))
        return GetHomeDirFile(".config/upp.out");

    // Create .upp.out as final fallback
    if (RealizeDirectory(GetHomeDirFile(".upp")))
        return GetHomeDirFile(".upp/out");

    return String(); // Return empty if all attempts fail
}

void CoreBuild::SetBuildMethod(const String& method) {
    build_method = method;
}

String CoreBuild::GetOutputDirectory() const {
    // For now, return a default output directory
    // In the future, this would be determined by TheIDE's logic
    String uppOut = GetUppOut();
    if (uppOut.IsEmpty()) {
        // If GetUppOut() is not available or empty, try common directories
        uppOut = GetHomeDirFile(".config/upp.out");
    }
    return uppOut;
}

bool CoreBuild::ExecuteBuildCommand(const String& command, String& log, String& error) {
    // This would execute the actual build command
    // For now, we just simulate it
    log = "Executing build command: " + command;
    error = "TODO: Execute actual build command";
    return false;
}