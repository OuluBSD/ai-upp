#include "clicore.h"

using namespace Upp;

// Helper function to check if a character is a valid value character
static bool IsUppValueChar(int c)
{
	return c > ' ' && c != ',' && c != ';';
}

// Helper function to read values from CParser (mimics the original function from Package.cpp)
static String ReadValue(CParser& p)
{
	p.Spaces();
	if(p.IsString())
		return p.ReadOneString();
	StringBuffer v;
	while(IsUppValueChar(p.PeekChar()))
		v.Cat(p.GetChar());
	p.Spaces();
	return String(v);
}

CoreWorkspace::CoreWorkspace() {
    // Initialize workspace
}

CoreWorkspace::~CoreWorkspace() {
    // Cleanup
}

bool CoreWorkspace::SetWorkspaceRoot(const String& root, String& error) {
    if (root.IsEmpty()) {
        error = "Workspace root cannot be empty";
        return false;
    }

    String normalized_root = NormalizePath(NativePath(root));
    if (!DirectoryExists(normalized_root)) {
        error = "Workspace root directory does not exist: " + normalized_root;
        return false;
    }

    workspace.root = normalized_root;
    return true;
}

bool CoreWorkspace::LoadWorkspace(const String& project_file, String& error) {
    // This loads a specific project file (.upp)
    String upp_file = project_file;
    if (!upp_file.EndsWith(".upp")) {
        upp_file += ".upp";
    }

    if (!FileExists(upp_file)) {
        // If it's a relative path, try with workspace root
        if (!IsFullPath(project_file) && !workspace.root.IsEmpty()) {
            upp_file = AppendFileName(workspace.root, project_file);
            if (!upp_file.EndsWith(".upp")) {
                upp_file += ".upp";
            }
            if (!FileExists(upp_file)) {
                error = "Project file does not exist: " + upp_file;
                return false;
            }
        } else {
            error = "Project file does not exist: " + upp_file;
            return false;
        }
    }

    // Extract package name from the file path
    String package_name = GetFileTitle(upp_file);

    // Load the package
    return LoadPackage(package_name, error);
}

bool CoreWorkspace::LoadPackage(const String& package_name, String& error) {
    if (package_name.IsEmpty()) {
        error = "Package name cannot be empty";
        return false;
    }

    if (workspace.root.IsEmpty()) {
        error = "Workspace root is not set. Use SetWorkspaceRoot first.";
        return false;
    }

    // Construct the .upp file path
    String upp_file;
    String package_dir = AppendFileName(workspace.root, package_name);

    // First try the package directory
    if (DirectoryExists(package_dir)) {
        upp_file = AppendFileName(package_dir, package_name + ".upp");
        if (!FileExists(upp_file)) {
            // If the default naming doesn't work, look for any .upp file in the directory
            FindFile ff(AppendFileName(package_dir, "*.upp"));
            if (ff) {
                upp_file = AppendFileName(package_dir, ff.GetName());
            } else {
                error = "Package .upp file does not exist in directory: " + package_dir;
                return false;
            }
        }
    } else {
        // If the standard directory doesn't exist, try looking for the .upp file directly in the workspace root
        upp_file = AppendFileName(workspace.root, package_name + ".upp");
        if (!FileExists(upp_file)) {
            error = "Package .upp file does not exist: " + upp_file;
            return false;
        }
        package_dir = GetFileDirectory(upp_file);
    }

    // Parse the .upp file
    Package package(package_name, package_dir, upp_file);
    if (!ParseUppFile(upp_file, package, error)) {
        return false;
    }

    // Add or update the package in the workspace
    if (workspace.packages.Find(package_name) >= 0) {
        workspace.packages.GetAdd(package_name) = pick(package);
    } else {
        workspace.packages.Add(package_name, pick(package));
    }

    return true;
}

bool CoreWorkspace::ParseUppFile(const String& upp_file, Package& package, String& error) {
    try {
        String content = LoadFile(upp_file);
        CParser p(content);
        p.NoSkipComments(); // allow file path like //home/user/project/something.cpp

        while (!p.IsEof()) {
            if (p.Id("file")) {
                // Parse file list
                do {
                    String file_path = ReadValue(p);
                    if (!file_path.IsEmpty()) {
                        // Resolve relative paths relative to package directory
                        if (!IsFullPath(file_path)) {
                            file_path = AppendFileName(package.path, file_path);
                        }
                        package.files.Add(file_path);
                    }
                } while (p.Char(','));
                p.PassChar(';');
            }
            else if (p.Id("uses")) {
                // Parse dependencies
                do {
                    String dependency = ReadValue(p);
                    if (!dependency.IsEmpty()) {
                        package.uses.Add(dependency);
                    }
                } while (p.Char(','));
                p.PassChar(';');
            }
            else if (p.Id("description")) {
                package.description = p.ReadString();
                p.PassChar(';');
            }
            else if (p.Id("include")) {
                // Parse include paths
                do {
                    String include_path = ReadValue(p);
                    if (!include_path.IsEmpty()) {
                        // Resolve relative paths relative to package directory if needed
                        if (!IsFullPath(include_path)) {
                            include_path = AppendFileName(package.path, include_path);
                        }
                        package.include_paths.Add(include_path);
                    }
                } while (p.Char(','));
                p.PassChar(';');
            }
            else if (p.Id("library")) {
                // Parse libraries
                do {
                    String library = ReadValue(p);
                    if (!library.IsEmpty()) {
                        package.libraries.Add(library);
                    }
                } while (p.Char(','));
                p.PassChar(';');
            }
            else if (p.Id("flags")) {
                // Parse flags
                do {
                    String flag = ReadValue(p);
                    if (!flag.IsEmpty()) {
                        package.flags.Add(flag);
                    }
                } while (p.Char(','));
                p.PassChar(';');
            }
            else {
                // Skip any unrecognized entries
                p.SkipTerm();
                if (p.IsChar(';')) {
                    p.PassChar(';');
                }
            }
        }
    }
    catch (CParser::Error& e) {
        error = "Error parsing .upp file '" + upp_file + "': " + e;
        return false;
    }

    return true;
}

void CoreWorkspace::EnumeratePackages(const String& root, Index<String>& packages) {
    // This looks for .upp files in the workspace root and subdirectories
    FindFile ff0(AppendFileName(root, "*.upp"));
    while (ff0) {
        if (ff0.IsFile()) {
            String package_name = GetFileTitle(ff0.GetName());
            if (package_name.GetCount() > 0 && packages.Find(package_name) < 0) {
                packages.Add(package_name);
            }
        }
        ff0.Next();
    }

    // Also look in subdirectories that might be package directories
    FindFile ff1(AppendFileName(root, "*"));
    while (ff1) {
        if (ff1.IsFolder() && *ff1.GetName() != '.') {
            String subdir = AppendFileName(root, ff1.GetName());
            FindFile upp_file_finder(AppendFileName(subdir, "*.upp"));
            if (upp_file_finder) {
                String package_name = GetFileTitle(upp_file_finder.GetName());
                if (package_name.GetCount() > 0 && packages.Find(package_name) < 0) {
                    packages.Add(package_name);
                }
            }
        }
        ff1.Next();
    }
}

bool CoreWorkspace::SetMainPackage(const String& package_name, String& error) {
    if (package_name.IsEmpty()) {
        error = "Package name cannot be empty";
        return false;
    }

    if (workspace.root.IsEmpty()) {
        error = "Workspace root is not set. Use --workspace-root PATH.";
        return false;
    }

    // Check if the package exists in the workspace
    if (!HasPackage(package_name)) {
        // Try to load the package if it doesn't exist yet
        if (!LoadPackage(package_name, error)) {
            return false;
        }
    }

    // Verify that the package has been loaded
    if (!HasPackage(package_name)) {
        error = "Package '" + package_name + "' could not be loaded";
        return false;
    }

    workspace.main_package = package_name;
    return true;
}

bool CoreWorkspace::GetMainPackage(Package& out, String& error) const {
    if (workspace.main_package.IsEmpty()) {
        error = "No main package has been set";
        return false;
    }

    const Package* pkg = GetPackage(workspace.main_package);
    if (!pkg) {
        error = "Main package '" + workspace.main_package + "' not found";
        return false;
    }

    // Since we can't copy due to vector types, let's manually copy the data
    // This approach manually constructs the output package
    out.name = pkg->name;
    out.path = pkg->path;
    out.upp_file = pkg->upp_file;
    out.files <<= pkg->files;  // This should work for vector assignment in UPP
    out.uses <<= pkg->uses;
    out.description = pkg->description;
    out.include_paths <<= pkg->include_paths;
    out.libraries <<= pkg->libraries;
    out.flags <<= pkg->flags;

    return true;
}

bool CoreWorkspace::AddPackage(const String& package_name, const String& directory, String& error) {
    if (HasPackage(package_name)) {
        error = "Package '" + package_name + "' already exists";
        return false;
    }

    // Create a package and try to load its .upp file
    String upp_file = AppendFileName(directory, package_name + ".upp");
    Package pkg(package_name, directory, upp_file);

    // If the .upp file exists, parse it
    if (FileExists(upp_file)) {
        if (!ParseUppFile(upp_file, pkg, error)) {
            return false;
        }
    }

    workspace.packages.Add(package_name, pick(pkg));
    return true;
}

bool CoreWorkspace::HasPackage(const String& package_name) const {
    return workspace.packages.Find(package_name) >= 0;
}

const CoreWorkspace::Package* CoreWorkspace::GetPackage(const String& package_name) const {
    int index = workspace.packages.Find(package_name);
    if (index >= 0) {
        return &workspace.packages[index];
    }
    return nullptr;
}

String CoreWorkspace::GetPackageDirectory(const String& package_name) const {
    const Package* pkg = GetPackage(package_name);
    if (pkg) {
        return pkg->path;
    }
    return String();
}

String CoreWorkspace::GetSourcePath(const String& package, const String& file) const {
    if (IsFullPath(file)) return NativePath(file);
    String pkg_dir = GetPackageDirectory(package);
    if (pkg_dir.IsEmpty()) return String();
    return NormalizePath(AppendFileName(pkg_dir, file));
}

String CoreWorkspace::GetPackageOfFile(const String& filepath) const {
    String normalized_filepath = NormalizePath(NativePath(filepath));

    for(int i = 0; i < workspace.packages.GetCount(); i++) {
        const Package& pkg = workspace.packages[i];
        for(int j = 0; j < pkg.files.GetCount(); j++) {
            String pkg_file = NormalizePath(NativePath(pkg.files[j]));
            if(pkg_file == normalized_filepath) {
                return pkg.name;
            }
        }
    }

    return String();  // Empty string if file is not found in any package
}

Vector<String> CoreWorkspace::GetPackageUses(const String& package_name) const {
    const Package* pkg = GetPackage(package_name);
    if (pkg) {
        Vector<String> result;
        result <<= pkg->uses; // Use pick assignment to copy the vector
        return result;
    }
    return Vector<String>();  // Return empty vector if package not found
}