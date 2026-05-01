#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <cstdlib>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>
#define access _access
#define F_OK 0
#define PATH_SEP '\\'
#define PATH_SEPS "\\"
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#define PATH_SEP '/'
#define PATH_SEPS "/"
#endif

using namespace std;

// --- Utility Functions ---

bool StartsWith(const string& s, const string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

bool EndsWith(const string& s, const string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string Trim(const string& s) {
    auto first = s.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

vector<string> Split(const string& s, char sep) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, sep)) {
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

string ToLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

string Replace(string s, const string& from, const string& to) {
    size_t start_pos = 0;
    while((start_pos = s.find(from, start_pos)) != string::npos) {
        s.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return s;
}

// --- Filesystem Helpers ---

bool FileExists(const string& path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
#endif
}

bool DirectoryExists(const string& path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

bool RealizeDirectory(const string& path) {
    if (DirectoryExists(path)) return true;
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}

string GetCurrentDir() {
    char buff[FILENAME_MAX];
#ifdef _WIN32
    _getcwd(buff, FILENAME_MAX);
#else
    if (!getcwd(buff, FILENAME_MAX)) return "";
#endif
    return string(buff);
}

string JoinPath(const string& p1, const string& p2) {
    if (p1.empty()) return p2;
    if (p2.empty()) return p1;
    char last = p1.back();
    if (last == '/' || last == '\\') return p1 + p2;
    return p1 + PATH_SEPS + p2;
}

string GetParentDir(const string& path) {
    size_t found = path.find_last_of("/\\");
    if (found == string::npos) return ".";
    if (found == 0) return "/";
    return path.substr(0, found);
}

string GetFileName(const string& path) {
    size_t found = path.find_last_of("/\\");
    if (found == string::npos) return path;
    return path.substr(found + 1);
}

string GetFileTitle(const string& path) {
    string name = GetFileName(path);
    size_t dot = name.find_last_of(".");
    if (dot == string::npos) return name;
    return name.substr(0, dot);
}

string GetFileExt(const string& path) {
    string name = GetFileName(path);
    size_t dot = name.find_last_of(".");
    if (dot == string::npos) return "";
    return name.substr(dot + 1);
}

vector<string> ListDir(const string& path) {
    vector<string> result;
#ifdef _WIN32
    string search_path = JoinPath(path, "*");
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                result.push_back(fd.cFileName);
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(path.c_str());
    if (dir) {
        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                result.push_back(ent->d_name);
            }
        }
        closedir(dir);
    }
#endif
    return result;
}

void Walk(const string& path, vector<string>& files, const string& ext = "") {
    vector<string> entries = ListDir(path);
    for (const string& entry : entries) {
        string full = JoinPath(path, entry);
        if (DirectoryExists(full)) {
            Walk(full, files, ext);
        } else if (FileExists(full)) {
            if (ext.empty() || EndsWith(entry, ext)) {
                files.push_back(full);
            }
        }
    }
}

string NormalizePath(string path) {
    for (char& c : path) if (c == '\\') c = '/';
    string result;
    for (char c : path) {
        if (c == '/' && !result.empty() && result.back() == '/') continue;
        result += c;
    }
    return result;
}

string GetAbsolutePath(const string& path) {
#ifdef _WIN32
    char buff[MAX_PATH];
    if (GetFullPathNameA(path.c_str(), MAX_PATH, buff, NULL)) return string(buff);
    return path;
#else
    char buff[PATH_MAX];
    char* res = realpath(path.c_str(), buff);
    return res ? string(res) : path;
#endif
}

// --- Process Helpers ---

int RunCommand(const string& cmd) {
    return system(cmd.c_str());
}

string CaptureCommand(const string& cmd) {
    string result;
    char buffer[128];
#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) return "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return result;
}

// --- Build Logic ---

struct MainConfig {
    string name;
    string flags;
};

vector<MainConfig> ReadMainConfigs(const string& upp_path) {
    vector<MainConfig> configs;
    ifstream is(upp_path);
    if (!is) return configs;

    string line;
    bool in_mainconfig = false;
    while (getline(is, line)) {
        line = Trim(line);
        if (line == "mainconfig") {
            in_mainconfig = true;
            continue;
        }
        if (in_mainconfig) {
            if (line.empty()) continue;
            if (line[0] == '\t' || line[0] == ' ' || line.find('=') != string::npos) {
                smatch m;
                regex re("\"([^\"]*)\"\\s*=\\s*\"([^\"]*)\"");
                if (regex_search(line, m, re)) {
                    string name = m[1].str();
                    string flags = m[2].str();
                    if (name.empty()) name = flags;
                    configs.push_back({name, flags});
                } else {
                    // Try without quotes on the left
                    regex re2("([^\\s=]+)\\s*=\\s*\"([^\"]*)\"");
                    if (regex_search(line, m, re2)) {
                        string name = m[1].str();
                        string flags = m[2].str();
                        if (name.empty()) name = flags;
                        configs.push_back({name, flags});
                    }
                }
            } else if (!line.empty() && line[0] != '/' && line[0] != '#') {
                break;
            }
        }
    }
    return configs;
}

struct BuildMethod {
    string name;
    string display;
    string path;
    bool generated;
    bool auto_method;
    string builder;
};

string ReadBmBuilder(const string& path) {
    ifstream is(path);
    if (!is) return "";
    string line;
    regex re("^\\s*BUILDER\\s*=\\s*\"([^\"]+)\"\\s*;");
    while (getline(is, line)) {
        smatch m;
        if (regex_search(line, m, re)) return m[1].str();
    }
    return "";
}

vector<BuildMethod> CollectMethods() {
    vector<BuildMethod> methods;
#ifdef _WIN32
    string home = string(getenv("USERPROFILE") ? getenv("USERPROFILE") : "");
    string upp_dir = JoinPath(home, "upp");
    vector<string> files = ListDir(upp_dir);
    for (const string& f : files) {
        if (EndsWith(f, ".bm")) {
            string full = JoinPath(upp_dir, f);
            methods.push_back({GetFileTitle(f), GetFileTitle(f), full, false, false, ReadBmBuilder(full)});
        }
    }
#else
    string home = string(getenv("HOME") ? getenv("HOME") : "");
    vector<string> search_dirs = {
        JoinPath(home, ".config/u++/theide"),
        JoinPath(home, ".config/u++/umk")
    };
    for (const string& dir : search_dirs) {
        if (DirectoryExists(dir)) {
            vector<string> files = ListDir(dir);
            for (const string& f : files) {
                if (EndsWith(f, ".bm")) {
                    string full = JoinPath(dir, f);
                    methods.push_back({GetFileTitle(f), GetFileTitle(f), full, false, false, ReadBmBuilder(full)});
                }
            }
        }
    }
#endif
    return methods;
}

string FindUppByName(const string& repo_root, const string& filename, const vector<string>& extra_search_roots, bool include_rainbow) {
    vector<string> search_roots = {
        JoinPath(repo_root, "upptst"),
        JoinPath(repo_root, "uppsrc"),
        JoinPath(repo_root, "examples"),
        JoinPath(repo_root, "tutorial"),
        JoinPath(repo_root, "reference"),
        JoinPath(repo_root, "stdsrc"),
        JoinPath(repo_root, "stdtst"),
        JoinPath(repo_root, "game")
    };
    if (include_rainbow) search_roots.push_back(JoinPath(repo_root, "rainbow"));
    for (const string& root : extra_search_roots) search_roots.push_back(root);

    for (const string& root : search_roots) {
        if (!DirectoryExists(root)) continue;
        vector<string> found;
        Walk(root, found, ".upp");
        for (const string& f : found) {
            if (GetFileName(f) == filename) return GetAbsolutePath(f);
        }
    }
    return "";
}

struct Options {
    string conf_mode;
    bool release = false;
    bool clean = false;
    int jobs = 0;
    bool verbose = false;
    bool rainbow = false;
    string method;
    bool list_methods = false;
    string list_mainconfigs;
    string mainconf;
    bool android = false;
    bool bootstrap = false;
    bool smoketest = false;
    vector<string> add_roots;
    vector<string> search_roots;
    string target;
    string output_dir;
    bool dump_cmd = false;
    bool help = false;
};

void ListMethods(const vector<BuildMethod>& methods) {
    if (methods.empty()) {
        cout << "No build methods found." << endl;
        return;
    }
    for (size_t i = 0; i < methods.size(); ++i) {
        cout << "[" << i << "] " << methods[i].display << ": " << methods[i].path;
        if (!methods[i].builder.empty()) cout << " [builder: " << methods[i].builder << "]";
        cout << endl;
    }
}

void ListMainConfigs(const string& upp_path) {
    vector<MainConfig> configs = ReadMainConfigs(upp_path);
    if (configs.empty()) {
        cout << "No mainconfig entries found." << endl;
        return;
    }
    for (size_t i = 0; i < configs.size(); ++i) {
        cout << "[" << i << "] " << configs[i].name << " = " << configs[i].flags << endl;
    }
}

void PrintHelp(const char* prog) {
    cout << "Usage: " << prog << " [options] <target>" << endl;
    cout << "       " << prog << " tutorial" << endl << endl;
    cout << "Options:" << endl;
    cout << "  --config X, -C X                   Select mainconfig mode: debug|release" << endl;
    cout << "  --mainconfig X, -M X               Select mainconfig by index or name" << endl;
    cout << "  --list-mainconfigs P               List mainconfigs for a package" << endl;
    cout << "  --release, -r                      Use release build flags in umk" << endl;
    cout << "  --methods X, -m X                  Select build method by index, name, or path" << endl;
    cout << "  --list-methods                     List build methods and exit" << endl;
    cout << "  --bootstrap, -bs                   Bootstrap-build umk via Makefile" << endl;
    cout << "  --clean, -c                        Clean build" << endl;
    cout << "  --jobs N, -j N, -jN                Parallel jobs" << endl;
    cout << "  --verbose, -v                      Verbose output" << endl;
    cout << "  --rainbow, -R                      Enable rainbow source root" << endl;
    cout << "  --add-root P                       Add source root passed to umk (repeatable)" << endl;
    cout << "  --search-root P                    Add package lookup root for target/mainconfig (repeatable)" << endl;
    cout << "  --dump-cmd                         Dump the umk command and exit" << endl;
    cout << "  --help, -h                         Show this help" << endl;
}

int Bootstrap(const string& repo_root, const Options& opts) {
    vector<string> packages = {
        "plugin/z", "plugin/pcre", "plugin/bz2", "plugin/lzma",
        "plugin/lz4", "plugin/zstd", "plugin/png", "Core", "Draw", "Esc",
        "ide/Core", "ide/Android", "ide/Java", "ide/Builders", "umk"
    };

#ifdef _WIN32
    cout << "Bootstrapping umk (Windows/MSVC)..." << endl;
    cout << "Note: Full MSVC bootstrap requires complex environment setup (vcvarsall.bat)." << endl;
    cout << "This C++ version currently only supports POSIX bootstrap via Makefile." << endl;
    return 1;
#else
#ifdef __APPLE__
    string makefile_name = "Makefile.macos15";
#else
    string makefile_name = "Makefile.linux64";
#endif

    string makefile = JoinPath(repo_root, "uppsrc/umk/" + makefile_name);
    
    if (!FileExists(makefile)) {
        cerr << "Bootstrap Makefile not found at " << makefile << endl;
        return 2;
    }

    string uppsrc = JoinPath(repo_root, "uppsrc");
    
    // Generate .brcc files from .brc files for POSIX bootstrap
    for (const string& pkg : packages) {
        string pkg_dir = JoinPath(uppsrc, pkg);
        vector<string> brc_files;
        Walk(pkg_dir, brc_files, ".brc");
        for (const string& brc : brc_files) {
            string brcc = brc + "c";
            if (!FileExists(brcc) || true) { // Force regeneration for safety
                if (opts.verbose) cout << "Generating " << brcc << " from " << brc << endl;
                ifstream is(brc);
                ofstream os(brcc);
                string line;
                regex re("BINARY\\s*\\(\\s*([^,]+)\\s*,\\s*\"([^\"]+)\"\\s*\\)");
                while (getline(is, line)) {
                    smatch m;
                    if (regex_search(line, m, re)) {
                        string sym = m[1].str();
                        string fn = m[2].str();
                        string fpath = JoinPath(pkg_dir, fn);
                        ifstream df(fpath, ios::binary);
                        if (df) {
                            vector<unsigned char> data((istreambuf_iterator<char>(df)), istreambuf_iterator<char>());
                            os << "static const unsigned char " << sym << "_[] = {";
                            for (size_t i = 0; i < data.size(); ++i) {
                                os << (int)data[i] << (i + 1 == data.size() ? "" : ",");
                            }
                            os << "};\n";
                            os << "const int " << sym << "_length = " << data.size() << ";\n";
                            os << "const unsigned char *" << sym << " = " << sym << "_;\n";
                        } else {
                            cerr << "Warning: BRC file not found: " << fpath << endl;
                        }
                    }
                }
            }
        }
    }

    cout << "Bootstrapping umk via " << makefile << "..." << endl;

    if (opts.clean) {
        stringstream ss_clean;
        ss_clean << "make -f " << makefile << " -C " << uppsrc << " UPPOUT=../out/ clean";
        if (opts.verbose) cout << "Command: " << ss_clean.str() << endl;
        RunCommand(ss_clean.str());
    }

    stringstream ss;
    ss << "make -f " << makefile << " -C " << uppsrc;
    ss << " UPPOUT=../out/";
    ss << " all";
    if (opts.jobs > 0) ss << " -j" << opts.jobs;
    if (opts.verbose) cout << "Command: " << ss.str() << endl;
    
    int res = RunCommand(ss.str());
#ifndef _WIN32
    if (WIFEXITED(res)) res = WEXITSTATUS(res);
    else res = -1;
#endif

    if (res == 0) {
        string built = JoinPath(uppsrc, "umk.out");
        string bin_dir = JoinPath(repo_root, "bin");
        string dest = JoinPath(bin_dir, "umk");
        RealizeDirectory(bin_dir);
        string cmd = "cp " + built + " " + dest;
        if (opts.verbose) cout << "Command: " << cmd << endl;
        RunCommand(cmd);
        cout << "Executable compiled: " << dest << endl;
    }
    return res;
#endif
}

Options ParseArgs(int argc, char* argv[]) {
    Options opts;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--verbose" || arg == "-v") {
            opts.verbose = true;
        }
    }

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            opts.help = true;
        } else if (arg == "-r" || arg == "--release") {
            opts.release = true;
        } else if (arg == "-c" || arg == "--clean") {
            opts.clean = true;
        } else if (arg == "-v" || arg == "--verbose") {
            opts.verbose = true;
        } else if (arg == "-R" || arg == "--rainbow") {
            opts.rainbow = true;
        } else if (arg == "-bs" || arg == "--bootstrap") {
            opts.bootstrap = true;
        } else if (arg == "-listmethods" || arg == "--list-methods") {
            opts.list_methods = true;
        } else if (arg == "-lc" || arg == "--list-mainconfigs") {
            if (++i < argc) opts.list_mainconfigs = argv[i];
        } else if (arg == "-C" || arg == "--config") {
            if (++i < argc) opts.conf_mode = argv[i];
        } else if (arg == "-cr" || arg == "--conf-release") {
            opts.conf_mode = "release";
        } else if (arg == "-cd" || arg == "--conf-debug") {
            opts.conf_mode = "debug";
        } else if (arg == "-M" || arg == "-mc" || arg == "--mainconfig") {
            if (++i < argc) opts.mainconf = argv[i];
        } else if (arg == "-m" || arg == "--method") {
            if (++i < argc) opts.method = argv[i];
        } else if (arg == "-j" || arg == "--jobs" || StartsWith(arg, "-j")) {
            if (arg == "-j" || arg == "--jobs") {
                if (++i < argc) opts.jobs = atoi(argv[i]);
            } else {
                opts.jobs = atoi(arg.c_str() + 2);
            }
        } else if (arg == "-sr" || arg == "--search-root") {
            if (++i < argc) opts.search_roots.push_back(argv[i]);
        } else if (arg == "-ar" || arg == "--add-root") {
            if (++i < argc) opts.add_roots.push_back(argv[i]);
        } else if (arg == "--dump-cmd") {
            opts.dump_cmd = true;
        } else if (arg[0] == '-') {
            cerr << "Unknown option: " << arg << endl;
        } else {
            if (opts.target.empty()) opts.target = arg;
            else cerr << "Only one target is supported. Ignoring: " << arg << endl;
        }
    }
    return opts;
}

string ResolveUmkPath() {
    string p = JoinPath("bin", "umk");
#ifdef _WIN32
    p += ".exe";
#endif
    if (FileExists(p)) return p;
    return "umk"; // Fallback to PATH
}

string FindRepoRoot(string current) {
    while (!current.empty() && current != "/" && current != "\\") {
        if (DirectoryExists(JoinPath(current, "uppsrc"))) {
            return current;
        }
        current = GetParentDir(current);
    }
    return GetCurrentDir(); // fallback
}

int main(int argc, char* argv[]) {
    Options opts = ParseArgs(argc, argv);
    if (opts.help || (argc == 1 && opts.target.empty() && !opts.list_methods && opts.list_mainconfigs.empty())) {
        PrintHelp(argv[0]);
        return 0;
    }

    string repo_root = FindRepoRoot(GetAbsolutePath(GetCurrentDir()));
    
    if (opts.list_methods) {
        vector<BuildMethod> methods = CollectMethods();
        ListMethods(methods);
        return 0;
    }

    if (!opts.list_mainconfigs.empty()) {
        string upp_path = FindUppByName(repo_root, opts.list_mainconfigs + ".upp", opts.search_roots, opts.rainbow);
        if (upp_path.empty()) {
            cerr << "Unable to locate .upp file for " << opts.list_mainconfigs << endl;
            return 1;
        }
        ListMainConfigs(upp_path);
        return 0;
    }

    if (opts.bootstrap) {
        return Bootstrap(repo_root, opts);
    }
    
    if (opts.target.empty()) {
        cerr << "Target missing." << endl;
        return 1;
    }
    
    if (ToLower(opts.target) == "tutorial") {
        cout << "Build tutorial" << endl << endl;
        cout << "Common usage:" << endl;
        cout << "  build --conf-release --release Eon00" << endl;
        cout << "  build -cr -r Eon00" << endl;
        return 0;
    }

    string upp_path;
    if (EndsWith(opts.target, ".upp")) {
        upp_path = GetAbsolutePath(opts.target);
    } else {
        upp_path = FindUppByName(repo_root, opts.target + ".upp", opts.search_roots, opts.rainbow);
    }

    if (upp_path.empty()) {
        cerr << "Unable to locate .upp file for " << opts.target << endl;
        return 1;
    }

    vector<MainConfig> configs = ReadMainConfigs(upp_path);
    MainConfig selected = {"", ""};
    if (!opts.mainconf.empty()) {
        if (isdigit(opts.mainconf[0])) {
            int idx = atoi(opts.mainconf.c_str());
            if (idx >= 0 && (size_t)idx < configs.size()) {
                selected = configs[idx];
            }
        } else {
            for (const auto& c : configs) {
                if (ToLower(c.name) == ToLower(opts.mainconf)) {
                    selected = c;
                    break;
                }
            }
        }
    } else {
        string preferred = opts.conf_mode.empty() ? "debug" : opts.conf_mode;
        for (const auto& c : configs) {
            string lower = ToLower(c.name);
            bool win = lower.find("windows") != string::npos;
            bool posix = lower.find("posix") != string::npos;
#ifdef _WIN32
            if (posix && !win) continue;
#else
            if (win && !posix) continue;
#endif
            if (lower.find(preferred) != string::npos) {
                selected = c;
                break;
            }
        }
        if (selected.name.empty() && !configs.empty()) selected = configs[0];
    }

    vector<BuildMethod> methods = CollectMethods();
    BuildMethod method = {"", "", "", false, false, ""};
    if (!opts.method.empty()) {
        if (isdigit(opts.method[0])) {
            int idx = atoi(opts.method.c_str());
            if (idx >= 0 && (size_t)idx < methods.size()) {
                method = methods[idx];
            }
        } else {
            string target_method = ToLower(opts.method);
            // 1. Try exact match on name
            for (const auto& m : methods) {
                if (ToLower(m.name) == target_method) {
                    method = m;
                    break;
                }
            }
            // 2. Try partial match on name or exact on path
            if (method.path.empty()) {
                for (const auto& m : methods) {
                    if (ToLower(m.name).find(target_method) != string::npos || ToLower(m.path) == target_method) {
                        method = m;
                        break;
                    }
                }
            }
        }
    } else {
        if (!methods.empty()) {
            // Try to find a sensible default
            for (const auto& m : methods) {
                string b = ToLower(m.builder);
                if (b.find("clang") != string::npos || b.find("gcc") != string::npos || b.find("msc") != string::npos) {
                    method = m;
                    break;
                }
            }
            if (method.path.empty()) method = methods[0];
        }
    }

    if (method.path.empty()) {
        cerr << "No suitable build method found." << endl;
        if (!methods.empty()) {
            cerr << "Available methods:" << endl;
            ListMethods(methods);
        }
        return 1;
    }

    if (opts.verbose) {
        cout << "Selected mainconfig: " << selected.name << " = " << selected.flags << endl;
        cout << "Selected method: " << method.name << " (" << method.path << ")" << endl;
    }

    string umk = ResolveUmkPath();
    stringstream ss;
    ss << umk << " ";
    
    vector<string> roots = {
        JoinPath(repo_root, "upptst"),
        JoinPath(repo_root, "uppsrc"),
        JoinPath(repo_root, "examples"),
        JoinPath(repo_root, "tutorial"),
        JoinPath(repo_root, "reference"),
        JoinPath(repo_root, "game")
    };
    if (opts.rainbow) roots.insert(roots.begin() + 2, JoinPath(repo_root, "rainbow"));
    for (const string& r : opts.add_roots) roots.push_back(r);
    
    for (size_t i = 0; i < roots.size(); ++i) {
        ss << roots[i] << (i == roots.size() - 1 ? "" : 
#ifdef _WIN32
        ";"
#else
        ","
#endif
        );
    }
    
    string target_name = GetFileTitle(upp_path);
    ss << " " << target_name << " " << method.path;
    ss << " " << (opts.release ? "-rbsH1" : "-bsdH1");
    if (opts.clean) ss << "a";
    if (opts.jobs > 0) ss << " -H" << opts.jobs;
    if (!selected.flags.empty()) ss << " +" << Replace(selected.flags, " ", ",");
    
    string out_exe = JoinPath("bin", target_name);
#ifdef _WIN32
    out_exe += ".exe";
#endif
    ss << " " << out_exe;

    if (opts.dump_cmd) {
        cout << ss.str() << endl;
        return 0;
    }

    if (opts.verbose) cout << "Command: " << ss.str() << endl;
    return RunCommand(ss.str());
}
