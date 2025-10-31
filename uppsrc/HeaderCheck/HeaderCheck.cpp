#include "HeaderCheck.h"

NAMESPACE_UPP

// Helper function to find all files in a directory recursively with specific extensions
Vector<String> FindFilesRecursive(const String& dir, const Vector<String>& extensions) {
	Vector<String> result;
	FindFile ff(AppendFileName(dir, "*.*"));
	
	while(ff) {
		String name = ff.GetName();
		String path = ff.GetPath();
		
		if (ff.IsFolder()) {
			if (name != "." && name != "..") {
				Vector<String> sub_result = FindFilesRecursive(path, extensions);
				result.Append(sub_result);
			}
		} else {
			String ext = ToLower(GetFileExt(name));
			for (const auto& e : extensions) {
				if (ext == e) {
					result.Add(path);
					break;
				}
			}
		}
		ff.Next();
	}
	return result;
}

bool HeaderAnalysis::IsSystemInclude(const String& include) {
	// Check if the include is surrounded by angle brackets
	return include.StartsWith("<") && include.EndsWith(">");
}

String HeaderAnalysis::GetPackageNameForFile(const String& file) {
	for (const auto& pkg : packages) {
		if (file.StartsWith(pkg.path)) {
			return pkg.name;
		}
	}
	return String(); // Return empty string if not found
}

bool HeaderAnalysis::IsMainHeader(const String& include, const String& package) {
	// Check if the include is the main header of the given package
	String main_header = package + ".h";
	String include_file = GetFileName(include);
	if (include_file == main_header) {
		return true;
	}
	
	// If include has path separators, extract just the filename
	if (include.Find('/') >= 0 || include.Find('\\') >= 0) {
		include_file = GetFileName(include);
		return include_file == main_header;
	}
	
	return false;
}

void HeaderAnalysis::AnalyzeIncludes() {
	int success_count = 0;
	int total_count = 0;
	bool has_errors = false;
	
	// Check each file's includes
	for (int i = 0; i < file_includes.GetCount(); i++) {
		String file = file_includes.GetKey(i);
		const Vector<String>& includes = file_includes[i];
		String file_package = GetPackageNameForFile(file);
		String file_ext = ToLower(GetFileExt(file));
		
		for (const auto& include : includes) {
			total_count++;
			
			// Check if it's a system include - these are generally OK anywhere
			if (IsSystemInclude(include)) {
				success_count++;
				continue;
			}
			
			// Check if it's a main header from another package - these are OK in headers and sources
			String include_package = GetPackageNameForFile(include);
			if (!include_package.IsEmpty() && IsMainHeader(include, include_package) && include_package != file_package) {
				success_count++;
				continue;
			}
			
			// Check if it's a single file from another package (not main header)
			if (!include_package.IsEmpty() && include_package != file_package && !IsMainHeader(include, include_package)) {
				errors.Add("ERROR: File '" + file + "' includes single file from other package: '" + include + "'");
				has_errors = true;
				continue;
			}
			
			// Check includes in headers vs source files
			bool is_header = (file_ext == ".h" || file_ext == ".hpp");
			
			// If it's a header file but includes something other than main header of same package
			if (is_header && !IsMainHeader(include, file_package)) {
				// Check if it's another package's main header in a header file
				if (!include_package.IsEmpty() && include_package != file_package) {
					// This is allowed for main headers of other packages
					if (IsMainHeader(include, include_package)) {
						success_count++;
						continue;
					} else {
						errors.Add("ERROR: Header file '" + file + "' includes non-main header from another package: '" + include + "'");
						has_errors = true;
						continue;
					}
				}
				
				// Headers should generally only include main package headers
				if (include_package != file_package) {
					errors.Add("ERROR: Header file '" + file + "' includes file from different package: '" + include + "'");
					has_errors = true;
				} else {
					// Same package but not main header - this is allowed
					success_count++;
				}
			} 
			// If it's a source file (.cpp, .c, etc.), it can include other package main headers
			else if (!is_header) {
				// Source files can include main headers from other packages
				if (!include_package.IsEmpty() && IsMainHeader(include, include_package) && include_package != file_package) {
					success_count++;
				} else {
					// Source file including non-main header from same package is OK
					if (include_package == file_package) {
						success_count++;
					}
					// But source file including non-main header from another package is not OK
					else if (!include_package.IsEmpty() && !IsMainHeader(include, include_package)) {
						errors.Add("ERROR: Source file '" + file + "' includes single file from other package: '" + include + "'");
						has_errors = true;
					}
				}
			}
		}
	}
	
	// Determine final status based on analysis
	if (!has_errors) {
		if (success_count == total_count) {
			PrintResult();
			Cout() << "SUCCESS: All includes follow proper dependency rules." << EOL;
		} else {
			PrintResult();
			Cout() << "PARTIAL SUCCESS: Most includes follow proper dependency rules, but some may need review." << EOL;
		}
	} else {
		PrintResult();
		Cout() << "FAILURE: Found dependency issues that need fixing." << EOL;
	}
}

void HeaderAnalysis::PrintResult() {
	Cout() << "\n=== Header Dependency Analysis Results ===" << EOL;
	Cout() << "Packages processed: " << packages.GetCount() << EOL;
	
	int total_files = 0;
	for (const auto& pkg : packages) {
		total_files += pkg.files.GetCount();
	}
	Cout() << "Total files analyzed: " << total_files << EOL;
	Cout() << "Total include statements: " << file_includes.GetCount() << EOL;
	
	if (errors.GetCount() > 0) {
		Cout() << "\nErrors found:" << EOL;
		for (const auto& error : errors) {
			Cout() << "- " << error << EOL;
		}
	}
}

// Method to find a package by name in assembly directories
bool HeaderAnalysis::FindPackage(const String& name, PackageInfo& out_pkg) {
	for (const auto& dir : assembly_dirs) {
		// First try the direct path: assembly_dir/package_name.upp
		String upp_file = AppendFileName(dir, name + ".upp");
		if (FileExists(upp_file)) {
			out_pkg.name = name;
			out_pkg.path = GetFileDirectory(upp_file);
			// Load files in this package
			out_pkg.files = FindFilesRecursive(out_pkg.path, {".h", ".hpp", ".cpp", ".cc", ".cxx", ".c"});
			return true;
		}
		
		// Then try the subdirectory path: assembly_dir/package_name/package_name.upp
		String subdir_upp = AppendFileName(AppendFileName(dir, name), name + ".upp");
		if (FileExists(subdir_upp)) {
			out_pkg.name = name;
			out_pkg.path = GetFileDirectory(subdir_upp);
			// Load files in this package
			out_pkg.files = FindFilesRecursive(out_pkg.path, {".h", ".hpp", ".cpp", ".cc", ".cxx", ".c"});
			return true;
		}
	}
	return false;
}

// Method to load package dependencies recursively
void HeaderAnalysis::LoadDependenciesRecursively(const PackageInfo& start_pkg) {
	Index<String> processed_pkgs;
	processed_pkgs.Add(start_pkg.name);
	
	// Use a queue-like approach to process dependencies
	Vector<PackageInfo> to_process;
	// Use a direct way to add the start package to the vector
	to_process.Add();
	// Instead of assignment, let's swap or use other move operations
	// Actually, the best approach is to use pick() when adding to vector
	// But we have the issue that we can't copy from const reference
	// So we'll create a new object with the same data but using pick() from individual elements
	
	// Create new PackageInfo by moving individual components
	PackageInfo new_pkg;
	new_pkg.name = start_pkg.name;
	new_pkg.path = start_pkg.path;
	new_pkg.files <<= start_pkg.files; // Use <<= operator to move the vector
	
	// Replace the last element (which was default constructed) with our new_pkg using pick
	to_process[to_process.GetCount()-1] = pick(new_pkg);
	
	while(to_process.GetCount() > 0) {
		PackageInfo current = pick(to_process[0]);
		to_process.Remove(0);
		
		// Find the .upp file for this package
		String upp_file;
		for (const auto& dir : assembly_dirs) {
			String test_upp = AppendFileName(dir, current.name + ".upp");
			if (FileExists(test_upp)) {
				upp_file = test_upp;
				break;
			}
		}
		
		if (upp_file.IsEmpty()) continue;
		
		// Read the .upp file and find dependencies in the "uses" section
		Vector<String> deps = FindPackageDependencies(upp_file);
		
		for (const auto& dep : deps) {
			if (processed_pkgs.Find(dep) < 0) {  // Changed from Contains to Find (returns -1 if not found)
				PackageInfo dep_pkg;
				if (FindPackage(dep, dep_pkg)) {
					// Add to packages vector
					packages.Add();
					packages[packages.GetCount()-1] = pick(dep_pkg);
					
					// Add to to_process vector
					to_process.Add();
					to_process[to_process.GetCount()-1] = pick(dep_pkg);
				}
			}
		}
	}
}

// Method to find dependencies in a .upp file
Vector<String> HeaderAnalysis::FindPackageDependencies(const String& upp_file) {
	Index<String> temp_deps;  // Use Index for checking if already exists
	Vector<String> dependencies; // Use Vector for return value
	
	FileIn in(upp_file);
	if (!in.IsOpen()) return dependencies;
	
	String content = LoadFile(upp_file);
	
	// Find the 'uses' section - look for 'uses' keyword followed by package names
	// This is a simplified approach; in U++ .upp files, dependencies might be listed in different formats
	const char *ptr = content.Begin();
	const char *end = content.End();
	
	while (ptr < end - 4) {
		// Look for "uses" keyword or "assembly" sections that may contain dependencies
		if (memcmp(ptr, "uses", 4) == 0 && (ptr == content.Begin() || !IsAlpha(*(ptr-1)) || IsSpace(*(ptr-1)))) {
			// Skip to after the 'uses' keyword
			ptr += 4;
			// Skip whitespace
			while (ptr < end && (IsSpace(*ptr) || *ptr == '=')) ptr++;
			
			// Parse comma-separated package names
			while (ptr < end && *ptr != '\n' && *ptr != '{' && *ptr != ';' && *ptr != '/') {
				// Skip whitespace
				while (ptr < end && IsSpace(*ptr)) ptr++;
				if (ptr >= end) break;
				
				if (*ptr == '{') {
					// Handle block format: uses { pkg1, pkg2, pkg3 }
					ptr++; // Skip '{'
					while (ptr < end && *ptr != '}') {
						// Skip whitespace
						while (ptr < end && IsSpace(*ptr)) ptr++;
						if (ptr >= end || *ptr == '}') break;
						
						// Find the package name
						const char *start = ptr;
						while (ptr < end && (*ptr != ',' && *ptr != '\n' && *ptr != '}' && !IsSpace(*ptr) && *ptr != '/' && *ptr != '{')) {
							ptr++;
						}
						if (start < ptr) {
							String pkg_name = TrimLeft(TrimRight(String(start, ptr)));
							if (!pkg_name.IsEmpty() && temp_deps.Find(pkg_name) < 0) {  // Find is valid for Index
								temp_deps.Add(pkg_name);
								dependencies.Add(pkg_name);
							}
						}
						
						// Skip to next package name
						while (ptr < end && *ptr != ',' && *ptr != '}' && *ptr != '/' && *ptr != '{') ptr++;
						if (ptr < end && *ptr == ',') ptr++; // Skip comma if present
					}
					break; // Exit the outer loop after processing the block
				} else {
					// Find the package name
					const char *start = ptr;
					while (ptr < end && (*ptr != ',' && *ptr != '\n' && *ptr != '{' && *ptr != ';' && !IsSpace(*ptr) && *ptr != '/' && *ptr != '{')) {
						ptr++;
					}
					if (start < ptr) {
						String pkg_name = TrimLeft(TrimRight(String(start, ptr)));
						if (!pkg_name.IsEmpty() && temp_deps.Find(pkg_name) < 0) {  // Find is valid for Index
							temp_deps.Add(pkg_name);
							dependencies.Add(pkg_name);
						}
					}
					
					// Skip to next package name
					while (ptr < end && *ptr != ',' && *ptr != '\n' && *ptr != '{' && *ptr != ';' && *ptr != '/') ptr++;
					if (ptr < end && *ptr == ',') ptr++; // Skip comma
				}
			}
		}
		ptr++;
	}
	
	return dependencies;
}

int AnalyzeHeaderDependencies(const Vector<String>& assembly_dirs, const String& package_name) {
	HeaderAnalysis analysis;
	
	// Add assembly directories to the analysis
	for (const auto& dir : assembly_dirs) {
		analysis.assembly_dirs.Add(dir);
	}
	
	// Load the specified package and its dependencies recursively
	PackageInfo main_pkg;
	if (!analysis.FindPackage(package_name, main_pkg)) {
		Cout() << "ERROR: Package '" << package_name << "' not found in assembly directories." << EOL;
		return 1;
	}
	
	// Add main package using the safe move approach
	analysis.packages.Add();
	PackageInfo new_pkg;
	new_pkg.name = main_pkg.name;
	new_pkg.path = main_pkg.path;
	new_pkg.files <<= main_pkg.files; // Use <<= operator to move the vector
	analysis.packages[analysis.packages.GetCount()-1] = pick(new_pkg);
	analysis.LoadDependenciesRecursively(main_pkg);
	
	// For each file in each package, find all include statements
	for (auto& pkg : analysis.packages) {
		for (const auto& file : pkg.files) {
			Vector<String> includes = analysis.FindIncludeStatements(file);
			analysis.file_includes.Add(file, pick(includes));
		}
	}
	
	// Analyze the includes
	analysis.AnalyzeIncludes();
	
	return 0;
}

// Method to find all include statements in a file
Vector<String> HeaderAnalysis::FindIncludeStatements(const String& file_path) {
	Vector<String> includes;
	
	FileIn in(file_path);
	if (!in.IsOpen()) return includes;
	
	String content = LoadStream(in);
	
	const char *ptr = content.Begin();
	const char *end = content.End();
	
	while (ptr < end - 7) { // "include" has 7 chars
		// Look for "#include" pattern
		if (memcmp(ptr, "#include", 8) == 0) {
			// Ensure the previous character is start of line or whitespace
			bool valid_start = (ptr == content.Begin() || *(ptr-1) == '\n' || IsSpace(*(ptr-1)));
			if (valid_start) {
				ptr += 8; // Skip "#include"
				
				// Skip whitespace
				while (ptr < end && IsSpace(*ptr)) ptr++;
				
				if (ptr < end) {
					char delimiter = *ptr;
					if (delimiter == '"' || delimiter == '<') {
						ptr++; // Skip opening delimiter
						const char *start = ptr;
						
						// Find closing delimiter
						char closing_delim = (delimiter == '"') ? '"' : '>';
						while (ptr < end && *ptr != closing_delim && *ptr != '\n') ptr++;
						
						if (ptr < end && *ptr == closing_delim) {
							String include = TrimLeft(TrimRight(String(start, ptr)));
							if (!include.IsEmpty()) {
								includes.Add(delimiter + include + closing_delim);
							}
							ptr++; // Skip closing delimiter
						}
					}
				}
			}
		}
		ptr++;
	}
	
	return includes;
}

END_UPP_NAMESPACE