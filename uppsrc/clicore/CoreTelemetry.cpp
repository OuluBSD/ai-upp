#include "clicore.h"

CoreTelemetry::CoreTelemetry() {
}

Value CoreTelemetry::GetWorkspaceStats(const CoreWorkspace& ws) const {
    Value result = CreateMap();
    
    // Number of packages
    int package_count = ws.packages.GetCount();
    result("packages") = package_count;
    
    // Total source files and distribution by extension
    int total_files = 0;
    Map<String, int> extension_count;
    String largest_file_path;
    int largest_file_size = 0;
    int64 total_size = 0;
    
    for (int i = 0; i < ws.packages.GetCount(); i++) {
        const CoreWorkspace::Package& pkg = ws.packages[i];
        for (const String& file_path : pkg.files) {
            total_files++;
            
            // Get file extension
            String ext = GetFileExt(file_path).ToLower();
            if (ext.GetCount() > 0) {
                ext = ext.Mid(1); // Remove the dot
                extension_count.GetAdd(ext, 0)++;
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
    
    result("files") = total_files;
    result("extensions") = extension_count;
    
    if (total_files > 0) {
        result("average_size") = total_size / total_files;
    } else {
        result("average_size") = 0;
    }
    
    if (largest_file_size > 0) {
        Value largest_info = CreateMap();
        largest_info("path") = largest_file_path;
        largest_info("bytes") = largest_file_size;
        result("largest_file") = largest_info;
    }
    
    // Main package name (if available)
    if (ws.main_package.GetCount() > 0) {
        result("main_package") = ws.main_package;
    } else {
        result("main_package") = String();
    }
    
    return result;
}

Value CoreTelemetry::GetPackageStats(const CoreWorkspace::Package& pkg) const {
    Value result = CreateMap();
    
    // Number of files
    result("files") = pkg.files.GetCount();
    
    // Uses count
    result("uses_count") = pkg.uses.GetCount();
    
    // Include count
    int include_count = 0;
    int total_lines = 0;
    Vector<int> file_sizes;
    
    for (const String& file_path : pkg.files) {
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
    
    result("include_count") = include_count;
    result("total_lines") = total_lines;
    
    // Histogram of file sizes
    Value histogram = CreateMap();
    for (int size : file_sizes) {
        int bucket = (size / 1000) * 1000; // Group by 1KB increments
        String bucket_str = IntStr(bucket) + "-" + IntStr(bucket + 999);
        histogram(bucket_str) = histogram.IsKey(bucket_str) ? ToInt(histogram[bucket_str]) + 1 : 1;
    }
    result("file_size_histogram") = histogram;
    
    return result;
}

Value CoreTelemetry::ComputeFileComplexity(const String& path, const String& contents) const {
    Value result = CreateMap();
    
    // Lines of code
    int lines = 0;
    for (int i = 0; i < contents.GetCount(); i++) {
        if (contents[i] == '\n') lines++;
    }
    if (contents.GetCount() > 0 && contents[contents.GetCount()-1] != '\n') lines++; // Last line without newline
    result("lines") = lines;
    
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
    result("functions") = function_count;
    
    // Number of includes
    int include_count = 0;
    for (int i = 0; i < contents.GetCount(); i++) {
        if (contents.Mid(i, 8) == "#include") {
            include_count++;
            i += 7; // Skip past "include"
        }
    }
    result("includes") = include_count;
    
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
    result("nesting") = nesting_depth;
    
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
    result("comment_ratio") = comment_ratio;
    
    return result;
}

Value CoreTelemetry::GetGraphStats(const CoreGraph& graph) const {
    Value result = CreateMap();
    
    // Node count
    result("nodes") = graph.GetNodeCount();
    
    // Edge count
    int edge_count = 0;
    for (int i = 0; i < graph.GetNodeCount(); i++) {
        edge_count += graph.GetOutgoingCount(i);
    }
    result("edges") = edge_count;
    
    // Average outdegree
    if (graph.GetNodeCount() > 0) {
        result("avg_out") = (double)edge_count / graph.GetNodeCount();
    } else {
        result("avg_out") = 0.0;
    }
    
    // Check for cycles
    bool has_cycles = graph.HasCycles();
    result("cycles") = has_cycles;
    
    // Largest dependency chain length (approx using longest path on DAG)
    int max_chain_length = graph.GetLongestPathLength();
    result("max_chain") = max_chain_length;
    
    // Most depended-on package
    String most_depended_on_pkg;
    int max_incoming_count = 0;
    for (int i = 0; i < graph.GetNodeCount(); i++) {
        int incoming_count = graph.GetIncomingCount(i);
        if (incoming_count > max_incoming_count) {
            max_incoming_count = incoming_count;
            most_depended_on_pkg = graph.GetNodeName(i);
        }
    }
    result("most_depended_on") = most_depended_on_pkg;
    
    return result;
}

void CoreTelemetry::RecordEdit(const String& path, int bytes_before, int bytes_after) {
    EditRecord record;
    record.path = path;
    record.delta = bytes_after - bytes_before;
    record.timestamp = GetSysTime();
    edits.Add(std::move(record));
}

Value CoreTelemetry::GetEditHistory() const {
    Value result = CreateArray();
    
    for (const EditRecord& record : edits) {
        Value record_value = CreateMap();
        record_value("path") = record.path;
        record_value("delta") = record.delta;
        record_value("timestamp") = record.timestamp;
        result.Add(record_value);
    }
    
    return result;
}