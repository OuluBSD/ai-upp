#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <regex>
#include <filesystem>

// Simple API extractor to extract class/function declarations from U++ headers
// This extracts declarations that can be mapped to STL equivalents

class APIExtractor {
public:
    struct Declaration {
        std::string type;      // "class", "struct", "function", "typedef", etc.
        std::string name;      // name of the entity
        std::string signature; // full signature
        std::string file;      // file where it's defined
        int line;             // line number
    };

    // Extract declarations from a header file
    std::vector<Declaration> ExtractDeclarations(const std::string& filepath) {
        std::vector<Declaration> declarations;
        std::ifstream file(filepath);
        std::string line;
        int line_num = 0;

        if (!file.is_open()) {
            std::cerr << "Could not open file: " << filepath << std::endl;
            return declarations;
        }

        std::string content;
        while (std::getline(file, line)) {
            content += line + "\n";
            line_num++;
        }

        // Extract class declarations
        std::regex class_regex(R"(class\s+(\w+))");
        std::sregex_iterator class_begin(content.begin(), content.end(), class_regex);
        std::sregex_iterator class_end;
        
        for (std::sregex_iterator i = class_begin; i != class_end; ++i) {
            std::smatch match = *i;
            declarations.push_back({
                "class",
                match[1].str(),
                match[0].str(),
                filepath,
                0 // line number not precisely extracted in this simple version
            });
        }

        // Extract struct declarations
        std::regex struct_regex(R"(struct\s+(\w+))");
        std::sregex_iterator struct_begin(content.begin(), content.end(), struct_regex);
        std::sregex_iterator struct_end;
        
        for (std::sregex_iterator i = struct_begin; i != struct_end; ++i) {
            std::smatch match = *i;
            declarations.push_back({
                "struct", 
                match[1].str(),
                match[0].str(),
                filepath,
                0
            });
        }

        // Extract function declarations
        std::regex func_regex(R"((\w+::)?(\w+)\s*\([^;]*\)\s*(const)?\s*;)");
        std::sregex_iterator func_begin(content.begin(), content.end(), func_regex);
        std::sregex_iterator func_end;
        
        for (std::sregex_iterator i = func_begin; i != func_end; ++i) {
            std::smatch match = *i;
            declarations.push_back({
                "function",
                match[2].str(), // function name
                match[0].str(),
                filepath,
                0
            });
        }

        return declarations;
    }

    // Create a mapping from U++ types to STL equivalents
    std::map<std::string, std::string> CreateStdMapping() {
        std::map<std::string, std::string> mapping = {
            // Core containers
            {"Vector", "std::vector"},
            {"Array", "std::vector with unique_ptr<T> elements"},
            {"Index", "std::unordered_set or std::set"},
            {"VectorMap", "std::unordered_map or std::map"},
            {"ArrayMap", "std::unordered_map or std::map with unique_ptr<T> values"},
            
            // Strings 
            {"String", "std::string"},
            {"WString", "std::wstring"},
            {"StringBuffer", "std::string with reserve/capacity operations"},
            
            // Smart pointers
            {"One", "std::unique_ptr"},
            {"Ptr", "std::shared_ptr or raw pointer depending on context"},
            
            // Utilities
            {"Value", "std::any or std::variant"},
            {"Function", "std::function"},
            {"Callback", "std::function or callback pattern"},
            {"Tuple", "std::tuple"},
            
            // Threading
            {"Thread", "std::thread"},
            {"Mutex", "std::mutex"},
            {"Semaphore", "std::counting_semaphore (C++20) or custom implementation"},
            {"Atomic", "std::atomic<T>"},
            
            // Time/date
            {"Time", "std::chrono::time_point"},
            {"Date", "Custom date type or std::chrono::day/month/year"},
            
            // Algorithms
            {"Find", "std::find"},
            {"FindIndex", "std::find + std::distance"},
            {"Sort", "std::sort"},
            {"IndexOf", "std::find + std::distance"},
            
            // I/O
            {"Stream", "std::iostream hierarchy"},
            {"FileIn", "std::ifstream"},
            {"FileOut", "std::ofstream"},
            {"StringStream", "std::stringstream"}
        };
        
        return mapping;
    }
    
    // Generate a comparison report between U++ and STL equivalents
    void GenerateComparisonReport(const std::string& output_file) {
        auto mapping = CreateStdMapping();
        
        std::ofstream out(output_file);
        out << "# U++ to STL C++ Mapping Report\n\n";
        out << "This report documents the mapping between U++ types/functions and their STL equivalents.\n\n";
        
        out << "## Container Mapping\n\n";
        out << "| U++ Type | STL Equivalent | Status | Notes |\n";
        out << "|----------|----------------|--------|-------|\n";
        
        for (const auto& [upp, std] : mapping) {
            if (upp == "Vector" || upp == "Array" || upp == "Index" || 
                upp == "VectorMap" || upp == "ArrayMap") {
                out << "| " << upp << " | " << std << " | ? | Direct mapping possible |\n";
            }
        }
        
        out << "\n## String Handling Mapping\n\n";
        out << "| U++ Type | STL Equivalent | Status | Notes |\n";
        out << "|----------|----------------|--------|-------|\n";
        
        for (const auto& [upp, std] : mapping) {
            if (upp == "String" || upp == "WString" || upp == "StringBuffer") {
                out << "| " << upp << " | " << std << " | ? | Direct mapping possible |\n";
            }
        }
        
        out << "\n## Smart Pointer Mapping\n\n";
        out << "| U++ Type | STL Equivalent | Status | Notes |\n";
        out << "|----------|----------------|--------|-------|\n";
        
        for (const auto& [upp, std] : mapping) {
            if (upp == "One" || upp == "Ptr") {
                out << "| " << upp << " | " << std << " | ? | Direct mapping possible |\n";
            }
        }
        
        out << "\n## Utility Types Mapping\n\n";
        out << "| U++ Type | STL Equivalent | Status | Notes |\n";
        out << "|----------|----------------|--------|-------|\n";
        
        for (const auto& [upp, std] : mapping) {
            if (upp == "Value" || upp == "Function" || upp == "Callback" || upp == "Tuple") {
                out << "| " << upp << " | " << std << " | ? | Direct mapping possible |\n";
            }
        }
        
        out << "\n## Threading Mapping\n\n";
        out << "| U++ Type | STL Equivalent | Status | Notes |\n";
        out << "|----------|----------------|--------|-------|\n";
        
        for (const auto& [upp, std] : mapping) {
            if (upp == "Thread" || upp == "Mutex" || upp == "Semaphore" || upp == "Atomic") {
                out << "| " << upp << " | " << std << " | ? | Direct mapping possible |\n";
            }
        }
        
        out << "\n## Time/Date Mapping\n\n";
        out << "| U++ Type | STL Equivalent | Status | Notes |\n";
        out << "|----------|----------------|--------|-------|\n";
        
        for (const auto& [upp, std] : mapping) {
            if (upp == "Time" || upp == "Date") {
                out << "| " << upp << " | " << std << " | ? | Direct mapping possible |\n";
            }
        }
        
        out << "\n## Algorithm Mapping\n\n";
        out << "| U++ Function | STL Equivalent | Status | Notes |\n";
        out << "|--------------|----------------|--------|-------|\n";
        
        for (const auto& [upp, std] : mapping) {
            if (upp == "Find" || upp == "FindIndex" || upp == "Sort" || upp == "IndexOf") {
                out << "| " << upp << " | " << std << " | ? | Direct mapping possible |\n";
            }
        }
        
        out << "\n## I/O Mapping\n\n";
        out << "| U++ Type | STL Equivalent | Status | Notes |\n";
        out << "|----------|----------------|--------|-------|\n";
        
        for (const auto& [upp, std] : mapping) {
            if (upp == "Stream" || upp == "FileIn" || upp == "FileOut" || upp == "StringStream") {
                out << "| " << upp << " | " << std << " | ? | Direct mapping possible |\n";
            }
        }
        
        out.close();
    }
};

int main() {
    APIExtractor extractor;
    
    // Generate the comparison report
    extractor.GenerateComparisonReport("stdmap_comparison_report.md");
    
    std::cout << "API extractor tool created and comparison report generated." << std::endl;
    return 0;
}