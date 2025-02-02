#include "LLDB.h"

#ifdef flagMAIN



CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_ELAPSED|LOG_COUT);
	if (CommandLine().IsEmpty()) {
		LOG("n	attach-name         Tells the debugger to attach to a process with the given name.");
	    LOG("p	attach-pid          Tells the debugger to attach to a process with the given pid.");
	    LOG("w	wait-for            Tells the debugger to wait for a process with the given pid or name to launch before attaching.");
	    LOG("f	file                Tells the debugger to use <filename> as the program to be debugged.");
	    LOG("S	source-before-file  Tells the debugger to read in and execute the lldb  commands  in the given file, before any file has been loaded.");
	    LOG("s	source              Tells the debugger to read in and execute the lldb commands in the given file, after any file has been loaded.");
	    LOG("W	workdir             Specify base directory of file explorer tree");
	    LOG("h	help                Print out usage information.");
	    LOG("	[] Positional arguments: these are the arguments that are entered without an option");
	    return;
	}
    
    #if 0
    // TODO: add version number to description here
    cxxopts::Options options("lldbg", "A GUI for the LLVM Debugger.");
    options.positional_help("");

    // TODO: make mutually exclusive arguments, for example attach vs. launch file
    // clang-format off
    options.add_options()
        ("n,attach-name", "Tells the debugger to attach to a process with the given name.", cxxopts::value<std::string>())
        ("p,attach-pid", "Tells the debugger to attach to a process with the given pid.", cxxopts::value<std::string>())
        ("w,wait-for", "Tells the debugger to wait for a process with the given pid or name to launch before attaching.", cxxopts::value<std::string>())
        ("f,file", "Tells the debugger to use <filename> as the program to be debugged.", cxxopts::value<std::string>())
        ("S,source-before-file", "Tells the debugger to read in and execute the lldb  commands  in the given file, before any file has been loaded.", cxxopts::value<std::string>())
        ("s,source", "Tells the debugger to read in and execute the lldb commands in the given file, after any file has been loaded.", cxxopts::value<std::string>())
        ("workdir", "Specify base directory of file explorer tree", cxxopts::value<std::string>())
        ("h,help", "Print out usage information.")
        ("positional", "Positional arguments: these are the arguments that are entered without an option", cxxopts::value<std::vector<std::string>>())
        ;
    // clang-format on

    options.parse_positional({"file", "positional"});

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return;
    }
    
    #endif
    
    UPP::String param_workdir;
    UPP::String source_path; // source-before-file;
    UPP::String file; // file;
    UPP::String source_file; // source;
    Vector<UPP::String> positional; // positional;
    
    String option;
    for (String arg : CommandLine()) {
        if (arg[0] == '-') {
            option = arg.Mid(1);
        }
        else {
            if (option.GetCount()) {
                if      (option == "w") param_workdir = arg;
                else if (option == "S") source_path = arg;
                else if (option == "f") file = arg;
                else if (option == "s") source_file = arg;
            }
            else {
                positional << arg;
            }
            option.Clear();
        }
    }
    
    if (auto lldb_error = lldb::SBDebugger::InitializeWithErrorHandling(); !lldb_error.Success()) {
        const char* lldb_error_cstr = lldb_error.GetCString();
        std::cerr << (lldb_error_cstr ? lldb_error_cstr : "Unknown LLDB error!");
        std::cerr << "Failed to initialize LLDB, exiting...";
        SetExitCode(1);
        return;
    }
    Defer(lldb::SBDebugger::Terminate());

    auto ui = UserInterface::init();
    if (!ui.has_value()) {
        Cerr() << "Failed to initialize graphics/UI.\n Exiting...";
        SetExitCode(1);
        return;
    }

    std::optional<fs::path> workdir = {};
    if (param_workdir.Is()) {
        fs::path workdir_request = fs::path(~param_workdir);
        if (fs::exists(workdir_request) && fs::is_directory(workdir_request)) {
            workdir = workdir_request;
        }
    }

    Application app(*ui, workdir);

    if (source_path.Is()) {
        auto handle = FileHandle::create(~source_path);
        if (handle.has_value()) {
            for (const std::string& line : handle->contents()) {
                auto ret = run_lldb_command(app, line.c_str());
            }
            LOG("Successfully executed commands in source file: " << source_path);
        }
        else {
            LOG("Invalid filepath passed to source-before-file argument: " << source_path);
        }
    }

    if (file.Is()) {
        // TODO: detect and open main file of specified executable
        String target_set_cmd = Format("file %s", file);
        run_lldb_command(app, ~target_set_cmd);

        if (positional) {
            String argset_command;
            argset_command = "settings set target.run-args ";
            for (const auto& arg : positional) {
                argset_command += arg;
            }
            run_lldb_command(app, ~argset_command);
        }
    }

    if (source_file.Is()) {
        auto handle = FileHandle::create(~source_file);
        if (handle.has_value()) {
            for (const std::string& line : handle->contents()) {
                auto ret = run_lldb_command(app, line.c_str());
            }
            LOG("Successfully executed commands in source file: " << source_file);
        }
        else {
            Cerr() << "Invalid filepath passed to --source argument: " << source_file << "\n";
        }
    }

    int ret = main_loop(app);
    SetExitCode(ret);
}

#endif
