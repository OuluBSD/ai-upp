#include "Pkg.h"

NAMESPACE_UPP

bool ParseCommandLine(const Vector<String>& args, CliOptions& opts, String& error)
{
	for (int i = 0; i < args.GetCount(); i++) {
		String arg = args[i];
		if (arg.StartsWith("--")) {
			if (arg == "--help") {
				opts.help = true;
			} else if (arg == "--version") {
				opts.version = true;
			} else if (arg == "--info") {
				opts.info = true;
			} else if (arg == "--ask") {
				opts.ask = true;
			} else if (arg == "--pretend") {
				opts.pretend = true;
			} else if (arg == "--verbose") {
				opts.verbose = true;
			} else if (arg == "--update") {
				opts.update = true;
			} else if (arg == "--deep") {
				opts.deep = true;
			} else if (arg == "--newuse") {
				opts.newuse = true;
			} else if (arg == "--changed-use") {
				opts.changed_use = true;
			} else if (arg == "--depclean") {
				opts.depclean = true;
			} else if (arg == "--list-sets") {
				opts.list_sets = true;
			} else if (arg == "--sync") {
				opts.sync = true;
			} else if (arg == "--metadata") {
				opts.metadata = true;
			} else if (arg == "--audit-acceptflags") {
				opts.audit_acceptflags = true;
			} else if (arg == "--eselect") {
				opts.eselect = true;
			} else if (arg == "--search") {
				opts.search = true;
				if (i + 1 < args.GetCount()) {
					opts.search_query = args[++i];
				} else {
					error = "Missing argument for --search";
					return false;
				}
			} else if (arg.StartsWith("--color=")) {
				opts.color = arg.Mid(8);
				if (opts.color != "y" && opts.color != "n" && opts.color != "auto") {
					error = "Invalid value for --color: " + opts.color;
					return false;
				}
			} else if (arg == "--color") {
				if (i + 1 < args.GetCount()) {
					opts.color = args[++i];
					if (opts.color != "y" && opts.color != "n" && opts.color != "auto") {
						error = "Invalid value for --color: " + opts.color;
						return false;
					}
				} else {
					error = "Missing argument for --color";
					return false;
				}
			} else if (arg.StartsWith("--jobs=")) {
				opts.jobs = StrInt(arg.Mid(7));
			} else if (arg == "--jobs") {
				if (i + 1 < args.GetCount()) {
					opts.jobs = StrInt(args[++i]);
				} else {
					error = "Missing argument for --jobs";
					return false;
				}
			} else if (arg.StartsWith("--target=")) {
				opts.target = arg.Mid(9);
			} else if (arg == "--target") {
				if (i + 1 < args.GetCount()) {
					opts.target = args[++i];
				} else {
					error = "Missing argument for --target";
					return false;
				}
			} else if (arg.StartsWith("--provider=")) {
				opts.provider = arg.Mid(11);
			} else if (arg == "--provider") {
				if (i + 1 < args.GetCount()) {
					opts.provider = args[++i];
				} else {
					error = "Missing argument for --provider";
					return false;
				}
			} else {
				error = "Unknown option: " + arg;
				return false;
			}
		} else if (arg.StartsWith("-") && arg.GetLength() > 1) {
			// Clustered short options
			for (int j = 1; j < arg.GetLength(); j++) {
				char c = arg[j];
				switch (c) {
				case 'h': opts.help = true; break;
				case 'a': opts.ask = true; break;
				case 'p': opts.pretend = true; break;
				case 'v': opts.verbose = true; break;
				case 'u': opts.update = true; break;
				case 'D': opts.deep = true; break;
				case 'N': opts.newuse = true; break;
				case 'U': opts.changed_use = true; break;
				case 's':
					opts.search = true;
					// If there are remaining characters in the cluster, they are the query (e.g. -ssqlite)
					if (j + 1 < arg.GetLength()) {
						opts.search_query = arg.Mid(j + 1);
						j = arg.GetLength(); // Stop parsing this cluster
					} else if (i + 1 < args.GetCount()) {
						opts.search_query = args[++i];
					} else {
						error = "Missing argument for -s";
						return false;
					}
					break;
				default:
					error = String("Unknown short option: -") + c;
					return false;
				}
			}
		} else {
			// Positional argument or action
			if (arg == "eselect") {
				opts.eselect = true;
				// Consume all remaining arguments as eselect arguments
				while (i + 1 < args.GetCount()) {
					opts.eselect_args.Add(args[++i]);
				}
			} else {
				// Normalize "world" to "@world"
				if (arg == "world") {
					arg = "@world";
				}
				opts.atoms.Add(arg);
			}
		}
	}
	return true;
}

END_UPP_NAMESPACE
