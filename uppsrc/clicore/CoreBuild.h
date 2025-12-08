#ifndef _clicore_CoreBuild_h_
#define _clicore_CoreBuild_h_

#include <Core/Core.h>

using namespace Upp;

class CoreBuild {
public:
    CoreBuild();
    ~CoreBuild();

    // Build operations
    bool BuildProject(const CoreWorkspace::Package& package, const String& config, String& log, String& error);
    bool CleanProject(const CoreWorkspace::Package& package, String& log, String& error);

    // Old methods for backward compatibility (will be deprecated)
    bool BuildProject(const String& project, const String& config, String& log, String& error);
    bool CleanProject(const String& project, String& log, String& error);
    
    // Build configuration
    void SetBuildMethod(const String& method);
    String GetBuildMethod() const { return build_method; }

private:
    String build_method;
    String output_dir;

    // Helper methods for build operations
    String GetOutputDirectory() const;
    bool ExecuteBuildCommand(const String& command, String& log, String& error);
};

// Utility function that might be used globally
String GetUppOut();

#endif