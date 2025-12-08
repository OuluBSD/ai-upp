#ifndef _clicore_CoreWorkspace_h_
#define _clicore_CoreWorkspace_h_

#include <Core/Core.h>

using namespace Upp;

class CoreWorkspace {
public:
    struct Package : Moveable<Package> {
        String name;
        String path;          // directory path
        String upp_file;      // full path to .upp file
        Vector<String> files;
        Vector<String> uses;  // dependencies
        // Additional fields as needed based on the original Package structure
        String description;
        Vector<String> include_paths;
        Vector<String> libraries;
        Vector<String> flags;

        Package() {}
        Package(const String& n, const String& pkg_path, const String& upp)
            : name(n), path(pkg_path), upp_file(upp) {}
    };

    struct Workspace {
        String root;          // workspace root directory
        VectorMap<String, Package> packages;  // map from package name to Package
        String main_package;

        Workspace() {}
    };

private:
    Workspace workspace;

public:
    CoreWorkspace();
    ~CoreWorkspace();

    // Workspace management
    bool SetWorkspaceRoot(const String& root, String& error);
    const String& GetWorkspaceRoot() const { return workspace.root; }

    bool LoadWorkspace(const String& project_file, String& error);  // loads a specific project file
    bool LoadPackage(const String& package, String& error);         // loads a specific .upp package
    bool SetMainPackage(const String& package_name, String& error);
    bool GetMainPackage(Package& out, String& error) const;
    String GetMainPackage() const { return workspace.main_package; }

    // Package management
    bool AddPackage(const String& package_name, const String& directory, String& error);
    bool HasPackage(const String& package_name) const;
    const Package* GetPackage(const String& package_name) const;
    const VectorMap<String, Package>& GetPackages() const { return workspace.packages; }

    // File operations
    String GetPackageDirectory(const String& package_name) const;
    String GetSourcePath(const String& package, const String& file) const;
    String GetPackageOfFile(const String& filepath) const;

private:
    // Helper methods for parsing .upp files
    bool ParseUppFile(const String& upp_file, Package& package, String& error);
    void EnumeratePackages(const String& root, Index<String>& packages);
    bool ResolveDependencies(const String& package_name, String& error);
};

#endif