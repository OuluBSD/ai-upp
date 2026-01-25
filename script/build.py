#!/usr/bin/env python3

import os
import re
import shutil
import subprocess
import sys
import tempfile
import platform
import datetime
from pathlib import Path


def print_help(prog):
    print(
        "\n".join(
            [
                f"Usage: {prog} [options] <target>",
                f"       {prog} tutorial",
                "",
                "Options:",
                "  --conf-release, -ConfRelease, -cr  Select Release mainconfig",
                "  --conf-debug, -ConfDebug, -cd      Select Debug mainconfig",
                "  --mainconf X, -mc X                Select mainconfig by index or name",
                "  --list-conf P, -lc P               List mainconfigs for a package",
                "  --release, -Release, -r            Use release build flags in umk",
                "  --method X, -m X                   Select build method by index, name, or path",
                "  --list-methods, -ListMethods       List build methods and exit",
                "  --android, -Android                Select Android build method (BUILDER=ANDROID)",
                "  --bootstrap, -Bootstrap, -bs       Bootstrap-build umk via Makefile",
                "  --smoketest, -Smoketest            Build a single source file (UWP smoke test)",
                "  --clean, -Clean, -c                Clean build",
                "  --jobs N, -j N, -jN                Parallel jobs",
                "  --verbose, -Verbose, -v            Verbose output",
                "  --dump-cmd                         Dump the umk command and exit",
                "  --help, -Help                      Show this help",
            ]
        )
    )


def print_tutorial(prog):
    print(
        "\n".join(
            [
                "Build tutorial",
                "",
                "Common usage:",
                f"  {prog} --conf-release --release Eon00",
                f"  {prog} -cr -r Eon00",
                f"  {prog} --conf-debug --release Eon04",
                f"  {prog} -cd -r Eon04",
                "",
                "Select a mainconfig explicitly:",
                f"  {prog} --list-conf Eon00",
                f"  {prog} --mainconf 0 Eon00",
                f"  {prog} --mainconf \"Release (Posix)\" Eon07",
                "",
                "Control method selection:",
                f"  {prog} --list-methods",
                f"  {prog} --method 0 Eon00",
                f"  {prog} --android --method ANDROID Eon00",
                "",
                "Bootstrap umk:",
                f"  {prog} --bootstrap",
            ]
        )
    )


def parse_args(argv):
    opts = {
        "conf_mode": None,
        "release": False,
        "clean": False,
        "jobs": None,
        "verbose": False,
        "method": None,
        "list_methods": False,
        "list_conf": None,
        "mainconf": None,
        "android": False,
        "bootstrap": False,
        "smoketest": False,
        "target": None,
        "dump_cmd": False,
        "help": False,
    }
    i = 0
    while i < len(argv):
        arg = argv[i]
        lower = arg.lower()
        if lower in ("--help", "-help", "-h"):
            opts["help"] = True
            i += 1
            continue
        if lower in ("--conf-release", "-confrelease", "-cr"):
            opts["conf_mode"] = "release"
            i += 1
            continue
        if lower in ("--conf-debug", "-confdebug", "-cd"):
            opts["conf_mode"] = "debug"
            i += 1
            continue
        if lower in ("--list-conf", "-list-conf", "-lc"):
            if i + 1 >= len(argv):
                raise ValueError("Missing value for list-conf")
            i += 1
            opts["list_conf"] = argv[i]
            i += 1
            continue
        if lower in ("--mainconf", "-mainconf", "-mc"):
            if i + 1 >= len(argv):
                raise ValueError("Missing value for mainconf")
            i += 1
            opts["mainconf"] = argv[i]
            i += 1
            continue
        if lower.startswith("--mainconf="):
            opts["mainconf"] = arg.split("=", 1)[1]
            i += 1
            continue
        if lower.startswith("-mc") and len(arg) > 3:
            opts["mainconf"] = arg[3:]
            i += 1
            continue
        if lower in ("--release", "-release", "-r"):
            opts["release"] = True
            i += 1
            continue
        if lower in ("--list-methods", "-listmethods"):
            opts["list_methods"] = True
            i += 1
            continue
        if lower in ("--android", "-android"):
            opts["android"] = True
            i += 1
            continue
        if lower in ("--bootstrap", "-bootstrap", "-bs"):
            opts["bootstrap"] = True
            i += 1
            continue
        if lower in ("--smoketest", "-smoketest"):
            opts["smoketest"] = True
            i += 1
            continue
        if lower in ("--method", "-method", "-m"):
            if i + 1 >= len(argv):
                raise ValueError("Missing value for method")
            i += 1
            opts["method"] = argv[i]
            i += 1
            continue
        if lower.startswith("--method="):
            opts["method"] = arg.split("=", 1)[1]
            i += 1
            continue
        if lower.startswith("-m") and len(arg) > 2:
            opts["method"] = arg[2:]
            i += 1
            continue
        if lower in ("--clean", "-clean", "-c"):
            opts["clean"] = True
            i += 1
            continue
        if lower in ("--verbose", "-verbose", "-v"):
            opts["verbose"] = True
            i += 1
            continue
        if lower in ("--dump-cmd", "-dump-cmd"):
            opts["dump_cmd"] = True
            i += 1
            continue
        if lower in ("--jobs", "-jobs", "-j"):
            if i + 1 >= len(argv):
                raise ValueError("Missing value for jobs")
            i += 1
            opts["jobs"] = parse_jobs(argv[i])
            i += 1
            continue
        if lower.startswith("--jobs="):
            opts["jobs"] = parse_jobs(arg.split("=", 1)[1])
            i += 1
            continue
        if lower.startswith("-j") and len(arg) > 2:
            opts["jobs"] = parse_jobs(arg[2:])
            i += 1
            continue
        if arg.startswith("-"):
            raise ValueError(f"Unknown option: {arg}")
        if opts["target"] is not None:
            raise ValueError("Only one target is supported")
        opts["target"] = arg
        i += 1
    return opts


def parse_jobs(value):
    try:
        jobs = int(value, 10)
    except ValueError as exc:
        raise ValueError(f"Invalid jobs value: {value}") from exc
    if jobs <= 0:
        raise ValueError("Jobs must be positive")
    return jobs


def find_repo_root():
    return Path(__file__).resolve().parent.parent


def resolve_upp_path(repo_root, target):
    target_path = Path(target)
    if target_path.is_dir():
        candidates = list(target_path.glob("*.upp"))
        if len(candidates) == 1:
            return candidates[0].resolve()
        if len(candidates) > 1:
            raise ValueError(
                f"Multiple .upp files in {target_path}: "
                + ", ".join(str(path) for path in candidates)
            )
        raise ValueError(f"No .upp file found in {target_path}")
    if target_path.suffix == ".upp":
        path = target_path
        if not path.is_absolute():
            path = repo_root / path
        if not path.exists():
            raise ValueError(f"Missing .upp file: {path}")
        return path.resolve()
    return find_upp_by_name(repo_root, f"{target}.upp")


def find_upp_by_name(repo_root, filename):
    search_roots = [
        repo_root / "upptst",
        repo_root / "uppsrc",
        repo_root / "examples",
        repo_root / "tutorial",
        repo_root / "reference",
        repo_root / "stdsrc",
        repo_root / "stdtst",
    ]
    matches = []
    for root in search_roots:
        if not root.exists():
            continue
        matches.extend(root.rglob(filename))
    if not matches:
        for root, dirs, files in os.walk(repo_root):
            if ".git" in dirs:
                dirs.remove(".git")
            if "bin" in dirs:
                dirs.remove("bin")
            if filename in files:
                matches.append(Path(root) / filename)
    if not matches:
        raise ValueError(f"Unable to locate .upp file for {filename}")
    if len(matches) > 1:
        raise ValueError(
            f"Multiple .upp files named {filename}: "
            + ", ".join(str(path) for path in matches)
        )
    return matches[0].resolve()


def read_mainconfigs(upp_path):
    text = upp_path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    start = None
    for idx, line in enumerate(lines):
        if line.strip() == "mainconfig":
            start = idx + 1
            break
    if start is None:
        return []
    block_lines = []
    for line in lines[start:]:
        if not line.strip():
            if block_lines:
                break
            continue
        if line[:1].isspace():
            block_lines.append(line)
            continue
        break
    block = "\n".join(block_lines)
    entries = re.findall(r'"([^"]*)"\s*=\s*"([^"]*)"', block)
    return [{"name": name, "flags": flags} for name, flags in entries]


def config_matches_os(name, is_windows):
    lower = name.lower()
    has_windows = "windows" in lower
    has_posix = "posix" in lower
    if has_windows and not is_windows:
        return False
    if has_posix and is_windows:
        return False
    return True


def select_config(configs, conf_mode, is_windows):
    eligible = [cfg for cfg in configs if config_matches_os(cfg["name"], is_windows)]
    if not configs:
        return None, None
    if conf_mode is None:
        if eligible:
            return eligible[0], configs.index(eligible[0])
        return None, None
    mode = conf_mode.lower()
    for cfg in eligible:
        name_lower = cfg["name"].lower()
        if mode == "release" and "release" in name_lower:
            return cfg, configs.index(cfg)
        if mode == "debug" and "debug" in name_lower:
            return cfg, configs.index(cfg)
    return None, None


def normalize_flags(flags):
    return ",".join(split_flags(flags))


def split_flags(flags):
    tokens = [token for token in re.split(r"[\s,]+", flags.strip()) if token]
    normalized = []
    for token in tokens:
        token = token.strip(",;")
        if not token:
            continue
        normalized.append(token)
    return normalized


def apply_debug_full(flags, release):
    if not flags:
        return []
    normalized = split_flags(flags) if isinstance(flags, str) else list(flags)
    filtered = []
    for flag in normalized:
        check = flag.lstrip(".")
        if check in ("DEBUG_FULL", "FULL_DEBUG"):
            continue
        filtered.append(flag)
    if not release:
        filtered.append("DEBUG_FULL")
    return filtered


def select_config_by_token(configs, token):
    token = token.strip()
    try:
        index = int(token, 10)
    except ValueError:
        index = None
    if index is not None:
        if index < 0 or index >= len(configs):
            raise ValueError(f"Mainconfig index out of range: {index}")
        return configs[index], index
    for idx, cfg in enumerate(configs):
        if cfg["name"].lower() == token.lower():
            return cfg, idx
    raise ValueError(f"Unknown mainconfig: {token}")


def default_build_model(is_windows, methods):
    for env_name in ("UPP_BM", "UPP_BUILD_MODEL"):
        env_value = os.environ.get(env_name)
        if env_value:
            path = Path(env_value).expanduser()
            return {
                "name": path.stem,
                "display": path.stem,
                "path": path,
                "generated": False,
                "auto": False,
                "builder": read_bm_builder(path),
            }
    if is_windows:
        method = select_windows_default_method(methods)
        if method is not None:
            return method
        return None
    method = select_posix_default_method(methods)
    if method:
        return method
    return None


def select_windows_default_method(methods):
    priority = [
        "MSVS22x64",
        "MSVS22",
        "MSVS19x64",
        "MSVS19",
        "MSVS17x64",
        "MSVS17",
        "MSVS14x64",
        "MSVS14",
        "MSVS",
        "MSC",
    ]
    candidates_map = {method["path"].stem.lower(): method for method in methods}
    for name in priority:
        for key, method in candidates_map.items():
            if key.startswith(name.lower()):
                return method
    if methods:
        return sorted(methods, key=lambda item: str(item["path"]))[0]
    return None


def method_dirs_posix():
    base = Path("~/.config/u++").expanduser()
    return [base / "theide", base / "umk"]


def collect_methods_posix():
    auto_methods = collect_auto_methods()
    methods = []
    seen = set()
    methods.extend(auto_methods)
    seen.update(method["path"].resolve() for method in auto_methods)
    for method_dir in method_dirs_posix():
        if not method_dir.exists():
            continue
        for path in method_dir.glob("*.bm"):
            key = path.resolve()
            if key in seen:
                continue
            seen.add(key)
            methods.append(
                {
                    "name": path.stem,
                    "display": path.stem,
                    "path": path,
                    "generated": False,
                    "auto": False,
                    "builder": read_bm_builder(path),
                }
            )
    return methods


def auto_method_dir():
    return Path(tempfile.gettempdir()) / "upp_build_methods"


def collect_auto_methods():
    ensure_auto_methods()
    methods = []
    method_dir = auto_method_dir()
    if not method_dir.exists():
        return methods
    for path in sorted(method_dir.glob("*.bm")):
        name = path.stem
        display = f"{name} (auto)"
        methods.append(
            {
                "name": f"auto-{name.lower()}",
                "display": display,
                "path": path,
                "generated": True,
                "auto": True,
                "builder": None,
            }
        )
    return methods


def ensure_auto_methods():
    method_dir = auto_method_dir()
    method_dir.mkdir(parents=True, exist_ok=True)

    def ensure_method(name, compiler):
        if not shutil.which(compiler):
            return
        path = method_dir / f"{name}.bm"
        content = build_method_template(name, compiler)
        path.write_text(content, encoding="utf-8")

    ensure_method("CLANG", "clang++")
    ensure_method("GCC", "g++")


def build_method_template(name, compiler):
    paths = detect_build_paths()
    path_line = ";".join(paths["path"])
    include_line = ";".join(paths["include"])
    lib_line = ";".join(paths["lib"])
    return "\n".join(
        [
            f'BUILDER = "{name}";',
            f'COMPILER = "{compiler}";',
            'COMMON_OPTIONS = "-mpopcnt";',
            'COMMON_CPP_OPTIONS = "-std=c++17 -Wno-logical-op-parentheses";',
            'COMMON_C_OPTIONS = "";',
            'COMMON_LINK = "";',
            'COMMON_FLAGS = "";',
            'DEBUG_INFO = "2";',
            'DEBUG_BLITZ = "1";',
            'DEBUG_LINKMODE = "1";',
            'DEBUG_OPTIONS = "-O0";',
            'DEBUG_FLAGS = "";',
            'DEBUG_LINK = "";',
            'DEBUG_CUDA = "";',
            'RELEASE_BLITZ = "1";',
            'RELEASE_LINKMODE = "1";',
            'RELEASE_OPTIONS = "-O3 -ffunction-sections -fdata-sections";',
            'RELEASE_FLAGS = "";',
            'RELEASE_LINK = "-Wl,--gc-sections";',
            'RELEASE_CUDA = "";',
            'DEBUGGER = "gdb";',
            'ALLOW_PRECOMPILED_HEADERS = "0";',
            'DISABLE_BLITZ = "0";',
            f'PATH = "{path_line}";',
            f'INCLUDE = "{include_line}";',
            f'LIB = "{lib_line}";',
            'LINKMODE_LOCK = "0";',
            "",
        ]
    )


def detect_build_paths():
    path_entries = []
    include_entries = []
    lib_entries = []

    def add_path(entry, collection):
        if entry and entry.exists():
            value = str(entry)
            if value not in collection:
                collection.append(value)

    def add_path_str(entry, collection):
        if entry and entry not in collection:
            collection.append(entry)

    add_path(Path("/usr/bin"), path_entries)

    llvm_roots = find_llvm_roots()
    for root in llvm_roots:
        add_path(root / "bin", path_entries)
        add_path(root / "include", include_entries)
        add_path(root / "lib", lib_entries)
        add_path(root / "lib64", lib_entries)

    include_candidates = [
        Path("/usr/include/opencv4"),
        Path("/usr/include/gtk-4.0"),
        Path("/usr/include/glib-2.0"),
        Path("/usr/lib64/glib-2.0/include"),
        Path("/usr/lib/glib-2.0/include"),
        Path("/usr/include/cairo"),
        Path("/usr/local/include/opencv4"),
        Path("/usr/local/include/gtk-4.0"),
        Path("/usr/local/include/glib-2.0"),
        Path("/usr/local/lib/glib-2.0/include"),
        Path("/usr/local/lib64/glib-2.0/include"),
        Path("/usr/local/include/cairo"),
    ]
    for entry in include_candidates:
        add_path(entry, include_entries)

    lib_candidates = [
        Path("/usr/lib64"),
        Path("/usr/lib"),
        Path("/usr/local/lib"),
        Path("/usr/local/lib64"),
    ]
    for entry in lib_candidates:
        add_path(entry, lib_entries)

    if platform.system().lower().startswith("freebsd"):
        add_path(Path("/usr/local/bin"), path_entries)

    return {"path": path_entries, "include": include_entries, "lib": lib_entries}


def find_llvm_roots():
    roots = []
    candidates = [
        Path("/usr/lib"),
        Path("/usr/lib64"),
        Path("/usr/local/lib"),
        Path("/usr/local/lib64"),
    ]
    for base in candidates:
        if not base.exists():
            continue
        for path in base.glob("llvm*"):
            if path.is_dir():
                roots.append(path)
    return sorted(roots, key=llvm_root_sort_key, reverse=True)


def llvm_root_sort_key(path):
    match = re.search(r"llvm[-/]?(\d+)", path.name)
    if not match:
        return 0
    return int(match.group(1))


def select_posix_default_method(methods):
    if not methods:
        return None
    non_auto = [method for method in methods if not method.get("auto")]
    by_name = {method["name"].lower(): method for method in non_auto}
    for name in ("clang", "gcc"):
        if name in by_name:
            return by_name[name]
    by_auto_name = {method["name"].lower(): method for method in methods}
    for name in ("auto-clang", "auto-gcc"):
        if name in by_auto_name:
            return by_auto_name[name]
    return methods[0]


def is_android_method(method):
    builder = method.get("builder") or ""
    return "android" in builder.lower()


def select_android_method(methods):
    for method in methods:
        if is_android_method(method):
            return method
    return None


def collect_methods_windows():
    home = Path.home()
    methods = []
    for path in (home / "upp").glob("*.bm"):
        methods.append(
            {
                "name": path.stem,
                "display": path.stem,
                "path": path,
                "generated": False,
                "auto": False,
                "builder": read_bm_builder(path),
            }
        )
    return methods


def read_bm_builder(path):
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None
    match = re.search(r'^\s*BUILDER\s*=\s*"([^"]+)"\s*;', text, re.MULTILINE)
    if not match:
        return None
    return match.group(1).strip()


def resolve_method(methods, method_arg):
    if method_arg is None:
        return None
    try:
        index = int(method_arg, 10)
    except ValueError:
        index = None
    if index is not None:
        if index < 0 or index >= len(methods):
            raise ValueError(f"Build method index out of range: {index}")
        return methods[index]
    method_path = Path(method_arg).expanduser()
    if method_path.suffix == ".bm" or method_path.is_absolute() or os.sep in method_arg:
        if method_path.exists():
            return {
                "name": method_path.stem,
                "display": method_path.stem,
                "path": method_path,
                "generated": False,
                "auto": False,
                "builder": read_bm_builder(method_path),
            }
        method_arg = method_path.stem
    for method in methods:
        if method["name"].lower() == method_arg.lower():
            return method
        if method.get("display") and method["display"].lower() == method_arg.lower():
            return method
    lowered = method_arg.lower()
    if lowered in ("clang", "gcc"):
        for method in methods:
            if method["name"].lower() == lowered:
                return method
        for method in methods:
            if method["name"].lower() == f"auto-{lowered}":
                return method
    raise ValueError(f"Unknown build method: {method_arg}")


def list_methods(methods):
    if not methods:
        print("No build methods found.")
        return
    for idx, method in enumerate(methods):
        suffix = " (generated)" if method.get("generated") else ""
        display = method.get("display") or method["name"]
        builder = method.get("builder")
        builder_note = f" [builder: {builder}]" if builder else ""
        print(f"[{idx}] {display}: {method['path']}{suffix}{builder_note}")


def build_command(
    umk_path, roots, target, build_model, build_flags, flags, output_path, jobs, verbose
):
    args = [umk_path, roots, target, str(build_model), build_flags]
    if jobs:
        args.append(f"-H{jobs}")
    if flags:
        args.append(f"+{flags}")
    args.append(str(output_path))
    if verbose:
        print("Command:", " ".join(args))
    return args


def resolve_umk_path():
    bin_dir = Path("bin")
    candidates = [bin_dir / "umk", bin_dir / "umk.exe"]
    for path in candidates:
        if path.exists():
            return str(path)
    raise FileNotFoundError(
        "umk executable not found. Please ensure 'umk.exe' (Windows) or 'umk' (Posix) "
        "is in the 'bin' directory, or available in your system's PATH. "
        "Refer to COMPILING.md for instructions on setting up the U++ environment."
    )


def copy_eon_files(upp_path, bin_dir, verbose):
    eon_files = list(upp_path.parent.glob("*.eon"))
    if not eon_files:
        return
    bin_dir.mkdir(parents=True, exist_ok=True)
    for path in eon_files:
        dest = bin_dir / path.name
        shutil.copy2(path, dest)
        if verbose:
            print(f"Copied: {path} -> {dest}")


def find_msvc():
    vswhere_path = Path(os.environ.get("ProgramFiles(x86)", "C:\\Program Files (x86)")) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if not vswhere_path.exists():
        return None
    try:
        res = subprocess.run([str(vswhere_path), "-latest", "-property", "installationPath"], capture_output=True, text=True, check=True)
        vs_path = Path(res.stdout.strip())
        vcvarsall = vs_path / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat"
        if vcvarsall.exists():
            return vcvarsall
    except Exception:
        pass
    return None


def get_msvc_env():
    if shutil.which("cl.exe"):
        return os.environ.copy()
    vcvarsall = find_msvc()
    if not vcvarsall:
        return None
    cmd = f'"{vcvarsall}" x64 && set'
    res = subprocess.run(cmd, capture_output=True, text=True, shell=True)
    if res.returncode != 0:
        return None
    env = os.environ.copy()
    for line in res.stdout.splitlines():
        if "=" in line:
            parts = line.split("=", 1)
            if len(parts) == 2:
                key, val = parts
                env[key.upper()] = val
    return env


def get_upp_sources(repo_root, package):
    pkg_path = repo_root / "uppsrc" / package
    upp_file = pkg_path / f"{pkg_path.name}.upp"
    if not upp_file.exists():
        # Fallback for nested packages like ide/Builders
        upp_file = pkg_path / f"{Path(package).name}.upp"
    
    if not upp_file.exists():
        return []
        
    sources = []
    try:
        text = upp_file.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return []
        
    lines = text.splitlines()
    
    in_file_section = False
    for line in lines:
        line = line.strip()
        if not line: continue
        if line == "file":
            in_file_section = True
            continue
        if in_file_section:
            if line.endswith(";"):
                in_file_section = False
                line = line[:-1].strip()
            if not line: continue
            if "readonly separator" in line: continue
            
            # Match filename at start of line, handle optional quotes and trailing comma/options
            match = re.match(r'^"?([^",\s\)]+)"?', line)
            if match:
                fname = match.group(1).replace('\\', os.sep)
                if fname.lower().endswith(('.cpp', '.c', '.icpp', '.brcc')):
                    sources.append(fname)
    return sources


def bootstrap_build_umk_windows(repo_root, opts):
    env = get_msvc_env()
    if not env:
        print("MSVC (cl.exe) not found. Please run from a Developer Command Prompt or install Visual Studio.", file=sys.stderr)
        return 2
    
    out_dir = repo_root / "_out"
    if opts["clean"] and out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    
    # Generate build_info.h
    build_info = out_dir / "build_info.h"
    now = datetime.datetime.now()
    info_content = [
        f'#define bmYEAR    {now.year % 100}',
        f'#define bmMONTH   {now.month}',
        f'#define bmDAY     {now.day}',
        f'#define bmHOUR    {now.hour}',
        f'#define bmMINUTE  {now.minute}',
        f'#define bmSECOND  {now.second}',
        f'#define bmTIME    Time({now.year}, {now.month}, {now.day}, {now.hour}, {now.minute}, {now.second})',
        f'#define bmMACHINE "{platform.node()}"',
        f'#define bmUSER    "{os.environ.get("USERNAME", "unknown")}"',
    ]
    # Try to get git info
    try:
        git_hash = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True, cwd=str(repo_root)).strip()
        git_rev = subprocess.check_output(["git", "rev-list", "--count", "HEAD"], text=True, cwd=str(repo_root)).strip()
        git_branch = subprocess.check_output(["git", "rev-parse", "--abbrev-ref", "HEAD"], text=True, cwd=str(repo_root)).strip()
        info_content.extend([
            f'#define bmGIT_REVCOUNT "{git_rev}"',
            f'#define bmGIT_HASH "{git_hash}"',
            f'#define bmGIT_BRANCH "{git_branch}"',
        ])
    except Exception:
        pass
    build_info.write_text("\n".join(info_content) + "\n", encoding="utf-8")

    packages = [
        "plugin/z", "plugin/pcre", "plugin/bz2", "plugin/lzma",
        "plugin/lz4", "plugin/zstd", "plugin/png", "Core", "Draw", "Esc",
        "ide/Core", "ide/Android", "ide/Java", "ide/Builders", "umk"
    ]
    
    # MSVC flags
    common_flags = ["/nologo", "/bigobj", "/D_CRT_SECURE_NO_WARNINGS", "/O2", "/std:c++17", "/EHsc", "/MT"]
    common_macros = [
        "/DflagMSC", "/DflagWIN32", "/DflagBLITZ", "/DflagRELEASE",
        "/DflagSTATIC_Z", "/DflagSTATIC_PNG", "/DflagSTATIC_BZ2",
        "/DDYNAMIC_LIBCLANG"
    ]
    
    includes = [f"/I{repo_root / 'uppsrc'}", f"/I{out_dir}"]
    
    cl_path = shutil.which("cl.exe", path=env.get("PATH"))
    link_path = shutil.which("link.exe", path=env.get("PATH"))
    if not cl_path or not link_path:
        print("cl.exe or link.exe not found in captured environment.", file=sys.stderr)
        return 2

    obj_files = []
    
    print("Bootstrapping umk (Windows/MSVC)...")
    
    for pkg in packages:
        pkg_out = out_dir / pkg.replace('/', '_').replace('\\', '_')
        pkg_out.mkdir(parents=True, exist_ok=True)
        pkg_src_root = repo_root / "uppsrc" / pkg
        
        # Handle .brc files by converting them to .cpp
        brc_files = list(pkg_src_root.glob("*.brc"))
        for brc in brc_files:
            brc_cpp = pkg_out / (brc.name + ".cpp")
            if not brc_cpp.exists() or brc.stat().st_mtime > brc_cpp.stat().st_mtime:
                if opts["verbose"]:
                    print(f"Converting {brc} to {brc_cpp}...")
                content = brc.read_text(encoding="utf-8", errors="replace")
                cpp_content = []
                for line in content.splitlines():
                    m = re.match(r'BINARY\s*\(\s*([^,]+)\s*,\s*"([^"]+)"\s*\)', line)
                    if m:
                        sym = m.group(1).strip()
                        fn = m.group(2).strip()
                        fpath = pkg_src_root / fn
                        if fpath.exists():
                            data = fpath.read_bytes()
                            cpp_content.append(f'static const unsigned char {sym}_[] = {{')
                            cpp_content.append(", ".join(str(b) for b in data))
                            cpp_content.append("};")
                            cpp_content.append(f'extern "C" const int {sym}_length = {len(data)};')
                            cpp_content.append(f'extern "C" const unsigned char *{sym} = {sym}_;')
                brc_cpp.write_text("\n".join(cpp_content), encoding="utf-8")
            
            obj_name = brc.name + "_brc.obj"
            obj_path = pkg_out / obj_name
            if not obj_path.exists() or brc_cpp.stat().st_mtime > obj_path.stat().st_mtime:
                cmd = [cl_path, "/c"] + common_flags + ["/DflagMSC", "/DflagWIN32"] + includes + [f"/Fo{obj_path}", f"/Tp{brc_cpp}"]
                subprocess.run(cmd, env=env)
            obj_files.append(str(obj_path))

        sources = get_upp_sources(repo_root, pkg)
        if not sources:
            print(f"Warning: No source files found for package {pkg}")
            continue
            
        pkg_macro = list(common_macros)
        if pkg == "umk":
            pkg_macro.append("-DflagMAIN")
            
        for src in sources:
            src_path = pkg_src_root / src
            if not src_path.exists():
                if opts["verbose"]:
                    print(f"Skipping missing source: {src_path}")
                continue

            # Replace all possible separators to keep obj files flat in pkg_out
            obj_name = src.replace('/', '_').replace('\\', '_').replace('.', '_') + ".obj"
            obj_path = pkg_out / obj_name
            
            if obj_path.exists() and src_path.stat().st_mtime < obj_path.stat().st_mtime:
                obj_files.append(str(obj_path))
                continue
            
            cmd = [cl_path, "/c"] + common_flags + pkg_macro + includes + [f"/Fo{obj_path}"]
            
            # For C files or .brcc, use /Tc (Compile as C), otherwise /Tp (Compile as C++)
            if src.lower().endswith(('.c', '.brcc')):
                cmd.append(f"/Tc{src_path}")
            else:
                cmd.append(f"/Tp{src_path}")
            
            if opts["verbose"]:
                print(" ".join(cmd))
            else:
                print(f"Compiling {pkg}/{src}...")
                
            res = subprocess.run(cmd, env=env)
            if res.returncode != 0:
                print(f"Compilation failed for {src_path}", file=sys.stderr)
                return res.returncode
            obj_files.append(str(obj_path))
            
    bin_dir = repo_root / "bin"
    bin_dir.mkdir(parents=True, exist_ok=True)
    out_file = bin_dir / "umk.exe"
    
    temp_out_file = bin_dir / f"umk_tmp_{os.getpid()}.exe"
    
    libs = [
        "kernel32.lib", "user32.lib", "gdi32.lib", "ole32.lib", "oleaut32.lib",
        "uuid.lib", "ws2_32.lib", "advapi32.lib", "shell32.lib", "winmm.lib",
        "mpr.lib", "crypt32.lib", "usp10.lib"
    ]
    
    link_cmd = [link_path, "/nologo", "/OUT:" + str(temp_out_file), "/STACK:20000000", "/OPT:REF", "/OPT:ICF"] + obj_files + libs
    print(f"Linking {out_file}...")
    if opts["verbose"]:
        print(" ".join(link_cmd))
    res = subprocess.run(link_cmd, env=env)
    if res.returncode != 0:
        print("Linking failed", file=sys.stderr)
        if temp_out_file.exists():
            try:
                temp_out_file.unlink()
            except OSError:
                pass
        return res.returncode
    
    if out_file.exists():
        try:
            out_file.unlink()
        except OSError:
            old_path = out_file.with_suffix(".old")
            if old_path.exists():
                try:
                    old_path.unlink()
                except OSError:
                    pass
            try:
                out_file.rename(old_path)
            except OSError:
                print(f"Warning: Could not replace {out_file}. It may be in use.", file=sys.stderr)
                return 1

    try:
        temp_out_file.rename(out_file)
    except OSError as exc:
        print(f"Error: Failed to move {temp_out_file} to {out_file}: {exc}", file=sys.stderr)
        return 1
        
    print(f"umk.exe successfully bootstrapped to {out_file}")
    return 0


def parse_makefile_vars(makefile_path):
    vars_map = {}
    for line in makefile_path.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line or line.startswith("\t") or line.startswith("#"):
            continue
        match = re.match(r"^([A-Za-z0-9_]+)\s*=\s*(.*)$", line)
        if not match:
            continue
        key = match.group(1).strip()
        value = match.group(2).strip()
        vars_map[key] = value
    return vars_map


def bootstrap_build_umk(repo_root, opts):
    if os.name == "nt":
        return bootstrap_build_umk_windows(repo_root, opts)
    makefile = Path("/home/sblo/umk/Makefile")
    if not makefile.exists():
        print(f"Missing bootstrap Makefile: {makefile}", file=sys.stderr)
        return 2
    make_vars = parse_makefile_vars(makefile)
    out_file = make_vars.get("OutFile", "umk.out").strip().strip('"')
    uppsrc_dir = repo_root / "uppsrc"
    if not uppsrc_dir.exists():
        print(f"Missing uppsrc directory: {uppsrc_dir}", file=sys.stderr)
        return 2
    if opts["clean"]:
        clean_cmd = ["make", "-f", str(makefile), "-C", str(uppsrc_dir), "clean"]
        if opts["verbose"]:
            print("Command:", " ".join(clean_cmd))
        result = subprocess.run(clean_cmd)
        if result.returncode != 0:
            return result.returncode
    build_cmd = ["make", "-f", str(makefile), "-C", str(uppsrc_dir)]
    build_cmd.append("UPPOUT=../out/")
    make_cinc = make_vars.get("CINC")
    if make_cinc and platform.system().lower().startswith("freebsd"):
        build_cmd.append(f"CINC={make_cinc} -I/usr/local/include")
    if platform.system().lower().startswith("freebsd"):
        macro = make_vars.get("Macro")
        if macro:
            macro = macro.replace("-DflagLINUX", "-DflagFREEBSD")
            if "-DflagFREEBSD" not in macro:
                macro = f"{macro} -DflagFREEBSD"
            if "-DflagPOSIX" not in macro:
                macro = f"{macro} -DflagPOSIX"
            build_cmd.append(f"Macro={macro}")
    if opts["jobs"]:
        build_cmd.append(f"-j{opts['jobs']}")
    if opts["verbose"]:
        print("Command:", " ".join(build_cmd))
    result = subprocess.run(build_cmd)
    if result.returncode != 0:
        return result.returncode
    built_path = uppsrc_dir / out_file
    if not built_path.exists():
        print(f"Bootstrap build did not produce {built_path}", file=sys.stderr)
        return 2
    bin_dir = repo_root / "bin"
    bin_dir.mkdir(parents=True, exist_ok=True)
    output_path = bin_dir / "umk"
    shutil.copy2(built_path, output_path)
    print(f"Executable compiled: {output_path}")
    return 0


def main():
    try:
        opts = parse_args(sys.argv[1:])
    except ValueError as exc:
        print(exc, file=sys.stderr)
        return 2
    if not sys.argv[1:] or opts["help"]:
        print_help(Path(sys.argv[0]).name)
        return 0
    if opts["list_methods"]:
        is_windows = os.name == "nt"
        methods = (
            collect_methods_windows()
            if is_windows
            else collect_methods_posix()
        )
        list_methods(methods)
        return 0
    if opts["list_conf"]:
        repo_root = find_repo_root()
        os.chdir(repo_root)
        try:
            upp_path = resolve_upp_path(repo_root, opts["list_conf"])
        except ValueError as exc:
            print(exc, file=sys.stderr)
            return 2
        configs = read_mainconfigs(upp_path)
        if not configs:
            print("No mainconfig entries found.")
            return 0
        for idx, cfg in enumerate(configs):
            print(f"[{idx}] {cfg['name']} = {cfg['flags']}")
        return 0
    if opts["bootstrap"]:
        if not opts["target"]:
            opts["target"] = "umk"
        if opts["target"].lower() not in ("umk", "umk.upp", "uppsrc/umk/umk.upp"):
            print(
                "--bootstrap can only be used to build umk.",
                file=sys.stderr,
            )
            return 2
        repo_root = find_repo_root()
        return bootstrap_build_umk(repo_root, opts)

    if not opts["target"]:
        print("Missing target.", file=sys.stderr)
        print_help(Path(sys.argv[0]).name)
        return 2
    if opts["target"].lower() == "tutorial":
        print_tutorial(Path(sys.argv[0]).name)
        return 0
    if opts["conf_mode"] and opts["mainconf"]:
        print("Use either --conf-* or --mainconf, not both.", file=sys.stderr)
        return 2

    repo_root = find_repo_root()
    os.chdir(repo_root)

    try:
        upp_path = resolve_upp_path(repo_root, opts["target"])
    except ValueError as exc:
        print(exc, file=sys.stderr)
        return 2

    target = upp_path.stem
    is_windows = os.name == "nt"
    methods = collect_methods_windows() if is_windows else collect_methods_posix()
    configs = read_mainconfigs(upp_path)

    if opts["mainconf"]:
        try:
            selected, index = select_config_by_token(configs, opts["mainconf"])
        except ValueError as exc:
            print(exc, file=sys.stderr)
            return 2
    else:
        if not opts["release"] and opts["conf_mode"] is None:
            # Automatically prefer 'debug' mode if available
            selected, index = select_config(configs, "debug", is_windows)
            if selected:
                opts["conf_mode"] = "debug"
            else:
                selected, index = select_config(configs, None, is_windows)
        else:
            selected, index = select_config(configs, opts["conf_mode"], is_windows)
    if not opts["conf_mode"] and not opts["mainconf"] and configs and not selected:
        available = ", ".join(
            f"[{idx}] {cfg['name']}" for idx, cfg in enumerate(configs)
        )
        print(
            f"No mainconfig matches this OS. Available: {available}",
            file=sys.stderr,
        )
        return 2
    if opts["conf_mode"] and not selected:
        available = ", ".join(
            f"[{idx}] {cfg['name']}" for idx, cfg in enumerate(configs)
        )
        print(
            f"No matching mainconfig for {opts['conf_mode']}. "
            f"Available: {available}",
            file=sys.stderr,
        )
        return 2

    flags = ""
    if selected:
        flags = normalize_flags(selected["flags"])

    if flags:
        flags = ",".join(apply_debug_full(flags, opts["release"]))
    elif not opts["release"]:
        flags = "DEBUG_FULL"
    if opts["smoketest"]:
        flags = f"{flags},UWP_SMOKETEST" if flags else "UWP_SMOKETEST"

    try:
        resolved_method = resolve_method(methods, opts["method"])
    except ValueError as exc:
        print(exc, file=sys.stderr)
        return 2

    if opts["android"]:
        if resolved_method and not is_android_method(resolved_method):
            print("Selected method is not an Android builder.", file=sys.stderr)
            return 2
        resolved_method = resolved_method or select_android_method(methods)
        if resolved_method is None:
            print("No Android build method found.", file=sys.stderr)
            return 2

    build_method = resolved_method or default_build_model(is_windows, methods)
    if build_method is None:
        print("No suitable build method found.", file=sys.stderr)
        return 2
    build_model = build_method["path"]
    if not build_model.exists():
        print(f"Build model not found: {build_model}", file=sys.stderr)
        return 2

    build_flags = "-bsH1" if opts["release"] else "-bsdH1"
    if opts["clean"]:
        build_flags += "a"
    if opts["jobs"]:
        build_flags = re.sub(r"H\\d+", "", build_flags)

    roots = "./upptst,./rainbow,./uppsrc,./examples,./tutorial,./reference"
    output_name = f"{target}.exe" if is_windows else target
    output_path = Path("bin") / output_name
    output_path.parent.mkdir(parents=True, exist_ok=True)

    real_output_path = output_path
    temp_output_path = None
    if is_windows and target.lower() == "umk":
        # Build to a temporary file to avoid overwriting the running umk.exe
        temp_output_path = output_path.with_name(f"{output_path.stem}_tmp_{os.getpid()}.exe")
        output_path = temp_output_path

    if opts["verbose"]:
        if selected:
            print(f"Mainconfig: [{index}] {selected['name']}")
        else:
            print("Mainconfig: (none)")
        print(f"Flags: {flags if flags else '(none)'}")
        builder = build_method.get("builder")
        builder_note = f" [{builder}]" if builder else ""
        print(f"Build model: {build_model}{builder_note}")
        print(f"Build flags: {build_flags}")

    try:
        umk_path = resolve_umk_path()
    except FileNotFoundError as exc:
        print(exc, file=sys.stderr)
        return 2
    args = build_command(
        umk_path,
        roots,
        target,
        build_model,
        build_flags,
        flags,
        output_path,
        opts["jobs"],
        opts["verbose"],
    )
    if opts["dump_cmd"]:
        print(" ".join(args))
        return 0
    result = subprocess.run(args)

    if result.returncode == 0 and temp_output_path:
        if temp_output_path.exists():
            if real_output_path.exists():
                try:
                    real_output_path.unlink()
                except OSError:
                    # If we cannot delete it, it might still be in use.
                    # On Windows, we can often rename it even if it's in use.
                    old_path = real_output_path.with_suffix(".old")
                    if old_path.exists():
                        try:
                            old_path.unlink()
                        except OSError:
                            pass
                    try:
                        real_output_path.rename(old_path)
                    except OSError:
                        print(f"Warning: Could not replace {real_output_path}. It may be in use.", file=sys.stderr)
                        return 1
            try:
                temp_output_path.rename(real_output_path)
                output_path = real_output_path
            except OSError as exc:
                print(f"Error: Failed to move {temp_output_path} to {real_output_path}: {exc}", file=sys.stderr)
                return 1

    if result.returncode != 0:
        if temp_output_path and temp_output_path.exists():
            try:
                temp_output_path.unlink()
            except OSError:
                pass
        return result.returncode

    copy_eon_files(upp_path, output_path.parent, opts["verbose"])
    if output_path.exists():
        print(f"Executable compiled: {output_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
