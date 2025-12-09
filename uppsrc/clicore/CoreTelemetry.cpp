#include "clicore.h"

CoreTelemetry::CoreTelemetry() {
}

Value CoreTelemetry::GetWorkspaceStats(const CoreWorkspace& ws) const {
    ValueMap result;

    // Number of packages
    int package_count = ws.GetPackages().GetCount();
    result.Set("packages", package_count);

    // Total source files and distribution by extension
    int total_files = 0;
    ValueMap extension_count;
    String largest_file_path;
    int64 largest_file_size = 0;
    int64 total_size = 0;

    const VectorMap<String, CoreWorkspace::Package>& all_packages_map = ws.GetPackages();
    Vector<String> all_packages;
    for(int i = 0; i < all_packages_map.GetCount(); i++) {
        all_packages.Add(all_packages_map.GetKey(i));
    }
    for(int i = 0; i < all_packages.GetCount(); i++) {
        String pkg_name = all_packages[i];
        const CoreWorkspace::Package* pkg = ws.GetPackage(pkg_name);
        if(pkg) {
            for(int j = 0; j < pkg->files.GetCount(); j++) {
                const String& file_path = pkg->files[j];
                total_files++;

                // Get file extension
                String ext = GetFileExt(file_path);
                if(ext.GetCount() > 0) {
                    ext = ToLower(ext.Mid(1)); // Remove the dot and convert to lowercase
                    int current_count = (int)extension_count.Get(ext, 0);
                    extension_count.Set(ext, current_count + 1);
                }

                // Calculate file size
                int64 file_size = GetFileLength(file_path);
                total_size += file_size;

                // Track largest file
                if (file_size > largest_file_size) {
                    largest_file_size = file_size;
                    largest_file_path = file_path;
                }
            }
        }
    }

    result.Set("files", total_files);
    result.Set("extensions", extension_count);

    if (total_files > 0) {
        result.Set("average_size", total_size / total_files);
    } else {
        result.Set("average_size", 0);
    }

    if (largest_file_size > 0) {
        ValueMap largest_info;
        largest_info.Set("path", largest_file_path);
        largest_info.Set("bytes", largest_file_size);
        result.Set("largest_file", largest_info);
    }

    // Main package name (if available)
    String main_pkg_name = ws.GetMainPackage();
    if (main_pkg_name.GetCount() > 0) {
        result.Set("main_package", main_pkg_name);
    } else {
        result.Set("main_package", String());
    }

    return result;
}

Value CoreTelemetry::GetPackageStats(const CoreWorkspace::Package& pkg) const {
    ValueMap result;

    // Number of files
    result.Set("files", pkg.files.GetCount());

    // Uses count
    result.Set("uses_count", pkg.uses.GetCount());

    // Include count
    int include_count = 0;
    int total_lines = 0;
    Vector<int> file_sizes;

    for (int j = 0; j < pkg.files.GetCount(); j++) {
        const String& file_path = pkg.files[j];
        String content = LoadFile(file_path);
        if (content.GetCount() > 0) {
            // Count lines
            int lines = 0;
            for (int i = 0; i < content.GetCount(); i++) {
                if (content[i] == '\n') lines++;
            }
            if (content.GetCount() > 0 && content[content.GetCount()-1] != '\n') lines++; // Last line without newline
            total_lines += lines;

            // Count includes
            for (int i = 0; i < content.GetCount(); i++) {
                if (content.Mid(i, 8) == "#include") {
                    include_count++;
                    i += 7; // Skip past "include"
                }
            }

            // Record file size for histogram
            file_sizes.Add(content.GetCount());
        }
    }

    result.Set("include_count", include_count);
    result.Set("total_lines", total_lines);

    // Histogram of file sizes
    ValueMap histogram;
    for (int j = 0; j < file_sizes.GetCount(); j++) {
        int size = file_sizes[j];
        int bucket = (size / 1000) * 1000; // Group by 1KB increments
        String bucket_str = IntStr(bucket) + "-" + IntStr(bucket + 999);
        int idx = histogram.Find(bucket_str);
        if(idx >= 0) {
            histogram.Set(bucket_str, (int)histogram[idx] + 1);
        } else {
            histogram.Set(bucket_str, 1);
        }
    }
    result.Set("file_size_histogram", histogram);

    return result;
}

Value CoreTelemetry::ComputeFileComplexity(const String& path, const String& contents) const {
    ValueMap result;

    // Lines of code
    int lines = 0;
    for (int i = 0; i < contents.GetCount(); i++) {
        if (contents[i] == '\n') lines++;
    }
    if (contents.GetCount() > 0 && contents[contents.GetCount()-1] != '\n') lines++; // Last line without newline
    result.Set("lines", lines);

    // Number of functions found by regex (simple heuristic)
    int function_count = 0;
    // Look for patterns like "return_type function_name(" or "function_name("
    // This is a simple heuristic and doesn't handle all cases
    for (int i = 0; i < contents.GetCount(); i++) {
        // Check for function name pattern: word + space + word + parentheses
        // Or just word + parentheses
        if (contents.Mid(i, 2) == " {") { // Start of function body
            bool is_function = false;
            // Look backward to see if there's a function signature
            int start = i;
            while (start > 0 && start > i - 100) { // Limit backward search
                if (contents[start] == ';') break; // End of declaration, not function
                if (contents[start] == '{') { // Nested block, not function start
                    is_function = false;
                    break;
                }
                if (contents[start] == ')') { // Found closing parenthesis of function parameters
                    is_function = true;
                    break;
                }
                start--;
            }
            if (is_function) function_count++;
        }
    }
    result.Set("functions", function_count);

    // Number of includes
    int include_count = 0;
    for (int i = 0; i < contents.GetCount(); i++) {
        if (contents.Mid(i, 8) == "#include") {
            include_count++;
            i += 7; // Skip past "include"
        }
    }
    result.Set("includes", include_count);

    // Nesting depth approximation (count of '{' - '}' transitions)
    int nesting_depth = 0;
    int current_depth = 0;
    for (int i = 0; i < contents.GetCount(); i++) {
        if (contents[i] == '{') {
            current_depth++;
            if (current_depth > nesting_depth) nesting_depth = current_depth;
        } else if (contents[i] == '}') {
            current_depth--;
        }
    }
    result.Set("nesting", nesting_depth);

    // Comment ratio
    int total_chars = contents.GetCount();
    int comment_chars = 0;

    for (int i = 0; i < contents.GetCount(); i++) {
        if (contents.Mid(i, 2) == "//") {
            // Count rest of line as comment
            while (i < contents.GetCount() && contents[i] != '\n') {
                comment_chars++;
                i++;
            }
        } else if (contents.Mid(i, 2) == "/*") {
            // Count until end of block comment
            comment_chars += 2; // For the /*
            i += 2;
            while (i < contents.GetCount() - 1) {
                if (contents.Mid(i, 2) == "*/") {
                    comment_chars += 2; // For the */
                    i++;
                    break;
                } else {
                    comment_chars++;
                }
                i++;
            }
        } else {
            i++;
        }
    }

    double comment_ratio = total_chars > 0 ? (double)comment_chars / total_chars : 0.0;
    result.Set("comment_ratio", comment_ratio);

    return result;
}

Value CoreTelemetry::GetGraphStats(const CoreGraph& graph) const {
    ValueMap result;

    // Node count
    result.Set("nodes", graph.GetNodeCount());

    // Edge count
    int edge_count = 0;
    const auto& adj_list = graph.GetAdjacencyList();
    for(int i = 0; i < adj_list.GetCount(); i++) {
        edge_count += adj_list[i].GetCount();
    }
    result.Set("edges", edge_count);

    // Average outdegree
    if (graph.GetNodeCount() > 0) {
        result.Set("avg_out", (double)edge_count / graph.GetNodeCount());
    } else {
        result.Set("avg_out", 0.0);
    }

    // Check for cycles
    bool has_cycles = graph.HasCycles();
    result.Set("cycles", has_cycles);

    // Largest dependency chain length (approx using longest path)
    int max_chain_length = graph.GetLongestPathLength();
    result.Set("max_chain", max_chain_length);

    // Most depended-on package
    String most_depended_on_pkg;
    int max_incoming_count = 0;
    for(int i = 0; i < adj_list.GetCount(); i++) {
        const String& pkg = adj_list.GetKey(i);
        int incoming_count = graph.GetIncomingCount(pkg);
        if (incoming_count > max_incoming_count) {
            max_incoming_count = incoming_count;
            most_depended_on_pkg = pkg;
        }
    }
    result.Set("most_depended_on", most_depended_on_pkg);

    return result;
}

void CoreTelemetry::RecordEdit(const String& path, int bytes_before, int bytes_after) {
    EditRecord record;
    record.path = path;
    record.delta = bytes_after - bytes_before;
    record.timestamp = GetSysTime();
    edits.Add(record);
}

Value CoreTelemetry::GetEditHistory() const {
    ValueArray result;

    for(int i = 0; i < edits.GetCount(); i++) {
        const EditRecord& record = edits[i];
        ValueMap record_value;
        record_value.Set("path", record.path);
        record_value.Set("delta", record.delta);
        record_value.Set("timestamp", record.timestamp);
        result.Add(record_value);
    }

    return result;
}