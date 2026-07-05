Scope: uppsrc/dbg

Purpose
- Headless debugger CLI.
- Direct integration with `ide/DebuggersCore` to run programs, capture crash signals, and output structured stack backtraces.
- Supports local debugging (GDB and LLDB) and remote Android debugging (LLDB over ADB).

Current Commands
- `--help`
- `--backends`
- `--backend <name> --help`
- `--backend <name> check` (probes toolchain/environment readiness)
- `--backend <name> run <program> [args...]`

Backends Status
- `vs`: Visual Studio backend (planned/supported on Windows; unsupported on POSIX/Linux).
- `gdb`: GNU Debugger (fully supported on Linux/Windows).
- `lldb`: LLVM Debugger (fully supported on Linux/Windows).

Build
- Linux: `python3 script/build.py -m GCC dbg` -> Binary: `bin/dbg`
- Windows: `bin/build.exe -m MSVS26x64 -j12 dbg` -> Binary: `bin/dbg.exe`

Local Debugger Toolchain Notes
- Both GDB and LLDB adapters run targets in headless batch mode.
- If a target crashes (e.g. SIGSEGV), the adapters capture the raw debugger backtrace and format it into a structured Call Stack output on stdout.
- On Linux, the CLI probes for `gdb` and `lldb` on `PATH`, and verifies Python 3 environment.
- On Windows, LLVM is expected under `C:\Program Files\LLVM\bin` and Python 3.11 at `C:\Python311`.

Android Remote Debugging via ADB (LLDB)
To debug an arm64 target running on a physical Android device or virtual machine:
1. **Locate lldb-server**:
   - Get the precompiled `lldb-server` from Android Studio (e.g., under `plugins/android-ndk/resources/lldb/android/arm64-v8a/lldb-server`).
2. **Deploy to Device**:
   - Push the server and target executable to the device's tmp folder:
     `adb push lldb-server /data/local/tmp/`
     `adb push target_exe /data/local/tmp/`
     `adb shell "chmod 755 /data/local/tmp/lldb-server /data/local/tmp/target_exe"`
3. **Configure Port Forwarding**:
   - Forward the debugger port (default 5039):
     `adb forward tcp:5039 tcp:5039`
4. **Launch lldb-server**:
   - Start the remote platform server on the device. **Critical**: You must `cd` to `/data/local/tmp` first so that `lldb-server` can write cache/tmp files without raising "Read-only file system" errors:
     `adb shell "cd /data/local/tmp && ./lldb-server platform --server --listen \"*:5039\""`
5. **Connect and Debug from Host**:
   - Launch `lldb` on host and connect:
     `platform select remote-android`
     `platform settings -w /data/local/tmp`
     `platform connect connect://localhost:5039`
     `target create ./local_exe -r /data/local/tmp/target_exe`
     `run`

Android Java Debugging via JDWP (jdb)
To debug a Java-only Android application (such as `android/JavaCrashSmoke`):
1. **Ensure app is debuggable**:
   - Set `android:debuggable="true"` in the application manifest.
2. **Build and Deploy**:
   - Compile the APK, install it on the device (`adb install app.apk`), and launch it.
3. **Get target process PID**:
   - Probe the running process PID on the device:
     `adb shell pidof org.upp.JavaCrashSmoke`
4. **Configure Port Forwarding**:
   - Forward host port 5005 to JDWP on the device:
     `adb forward tcp:5005 jdwp:<PID>`
5. **Connect and Debug from Host**:
   - Attach host `jdb` to JDWP port:
     `jdb -connect com.sun.jdi.SocketAttach:hostname=localhost,port=5005`
   - Use `where` to print stack traces or `catch java.lang.Throwable` to intercept crashes.

Package Notes
- `TcpProxy.*` are legacy MCP proxy files.
- They are not part of `dbg.upp` and must stay out of the package manifest.

File Map
- `dbg.h` : umbrella header
- `Backend.h/cpp` : backend registry, planned backend metadata, CLI dispatch
- `Toolchain.h/cpp` : backend toolchain diagnostics
- `Session.h/cpp` : debugger session/request definitions
- `GdbAdapter.h/cpp` : adapter for GdbEngine
- `LldbAdapter.h/cpp` : adapter for LldbEngine
- `VsAdapter.h/cpp` : VS adapter skeleton
- `JavaAdapter.h/cpp` : adapter for JavaEngine
- `main.cpp` : console entrypoint, delegates to `RunDbgCli`
