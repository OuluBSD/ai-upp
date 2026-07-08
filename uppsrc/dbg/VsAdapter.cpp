#include "dbg.h"

#ifdef PLATFORM_WIN32
#ifdef VersionInfo
#undef VersionInfo
#endif
#include <windows.h>
#include <verrsrc.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

using namespace Upp;

static WString QuoteWinArg(const String& arg)
{
	WString warg = arg.ToWString();
	bool quote = warg.IsEmpty();
	for(int i = 0; !quote && i < warg.GetCount(); i++)
		if(warg[i] <= ' ' || warg[i] == '"')
			quote = true;
	if(!quote)
		return warg;

	WString out;
	out.Cat('"');
	int slashes = 0;
	for(int i = 0; i < warg.GetCount(); i++) {
		wchar c = warg[i];
		if(c == '\\') {
			slashes++;
			continue;
		}
		if(c == '"') {
			for(int j = 0; j < slashes * 2 + 1; j++)
				out.Cat('\\');
			out.Cat('"');
			slashes = 0;
			continue;
		}
		for(int j = 0; j < slashes; j++)
			out.Cat('\\');
		slashes = 0;
		out.Cat(c);
	}
	for(int j = 0; j < slashes * 2; j++)
		out.Cat('\\');
	out.Cat('"');
	return out;
}

static WString MakeCmdLine(const DbgLaunchRequest& request)
{
	WString cmd = QuoteWinArg(request.executable_path);
	for(int i = 0; i < request.arguments.GetCount(); i++) {
		cmd.Cat(' ');
		cmd.Cat(QuoteWinArg(request.arguments[i]));
	}
	return cmd;
}

static WString MakeEnvBlock(const VectorMap<String, String>& env)
{
	WString block;
	for(int i = 0; i < env.GetCount(); i++) {
		block.Cat(env.GetKey(i).ToWString());
		block.Cat('=');
		block.Cat(env[i].ToWString());
		block.Cat(0);
	}
	block.Cat(0);
	return block;
}

static void AppendResolvedFrame(HANDLE hProcess, DWORD64 address, DbgRunResult& result)
{
	DbgCallStackFrame& frame = result.call_stack.Add();
	frame.address = Format64Hex(address);

	ULONG64 buffer[(sizeof(SYMBOL_INFO) + 1024 + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
	SYMBOL_INFO *symbol = (SYMBOL_INFO *)buffer;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = 1024;

	DWORD64 displacement = 0;
	if(SymFromAddr(hProcess, address, &displacement, symbol))
		frame.function = symbol->Name;

	IMAGEHLP_LINE64 line;
	memset(&line, 0, sizeof(line));
	line.SizeOfStruct = sizeof(line);
	DWORD line_displacement = 0;
	if(SymGetLineFromAddr64(hProcess, address, &line_displacement, &line)) {
		frame.source_file = line.FileName;
		frame.line = (int)line.LineNumber;
	}

	IMAGEHLP_MODULE64 module;
	memset(&module, 0, sizeof(module));
	module.SizeOfStruct = sizeof(module);
	if(SymGetModuleInfo64(hProcess, address, &module))
		frame.module = module.ModuleName;
}

static void AppendStackTrace64(HANDLE hProcess, HANDLE hThread, CONTEXT ctx, DbgRunResult& result)
{
	STACKFRAME64 stack = {};
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Mode = AddrModeFlat;
	stack.AddrPC.Offset = ctx.Rip;
	stack.AddrFrame.Offset = ctx.Rbp;
	stack.AddrStack.Offset = ctx.Rsp;

	DWORD64 last_pc = 0;
	for(int depth = 0; depth < 64; depth++) {
		BOOL ok = StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &stack, &ctx,
		                      (PREAD_PROCESS_MEMORY_ROUTINE64)ReadProcessMemory,
		                      SymFunctionTableAccess64, SymGetModuleBase64, NULL);
		if(!ok)
			break;
		if(stack.AddrPC.Offset == 0)
			break;
		if(stack.AddrPC.Offset == last_pc)
			break;
		last_pc = stack.AddrPC.Offset;
		AppendResolvedFrame(hProcess, stack.AddrPC.Offset, result);
	}
}

#ifdef CPU_64
static void AppendStackTrace32(HANDLE hProcess, HANDLE hThread, WOW64_CONTEXT ctx, DbgRunResult& result)
{
	STACKFRAME64 stack = {};
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Mode = AddrModeFlat;
	stack.AddrPC.Offset = ctx.Eip;
	stack.AddrFrame.Offset = ctx.Ebp;
	stack.AddrStack.Offset = ctx.Esp;

	DWORD64 last_pc = 0;
	for(int depth = 0; depth < 64; depth++) {
		BOOL ok = StackWalk64(IMAGE_FILE_MACHINE_I386, hProcess, hThread, &stack, &ctx,
		                      (PREAD_PROCESS_MEMORY_ROUTINE64)ReadProcessMemory,
		                      SymFunctionTableAccess64, SymGetModuleBase64, NULL);
		if(!ok)
			break;
		if(stack.AddrPC.Offset == 0)
			break;
		if(stack.AddrPC.Offset == last_pc)
			break;
		last_pc = stack.AddrPC.Offset;
		AppendResolvedFrame(hProcess, stack.AddrPC.Offset, result);
	}
}
#endif

static void CaptureCallStack(HANDLE hProcess, HANDLE hThread, DbgRunResult& result)
{
#ifdef CPU_64
	BOOL wow64 = FALSE;
	if(IsWow64Process(hProcess, &wow64) && wow64) {
		WOW64_CONTEXT ctx = {};
		ctx.ContextFlags = WOW64_CONTEXT_FULL;
		if(Wow64GetThreadContext(hThread, &ctx))
			AppendStackTrace32(hProcess, hThread, ctx, result);
		else
			result.transcript << "note: Wow64GetThreadContext failed: " << GetLastErrorMessage() << "\n";
		return;
	}
#endif
	CONTEXT ctx = {};
	ctx.ContextFlags = CONTEXT_ALL;
	if(GetThreadContext(hThread, &ctx))
		AppendStackTrace64(hProcess, hThread, ctx, result);
	else
		result.transcript << "note: GetThreadContext failed: " << GetLastErrorMessage() << "\n";
}

static String PathFromFileHandle(HANDLE file)
{
	if(!file)
		return String();

	char path[MAX_PATH * 4];
	DWORD len = GetFinalPathNameByHandleA(file, path, (DWORD)sizeof(path), FILE_NAME_NORMALIZED);
	if(len == 0 || len >= sizeof(path))
		return String();

	String out(path, (int)len);
	if(out.StartsWith("\\\\?\\"))
		out = out.Mid(4);
	return out;
}

static bool LoadDebugModule(HANDLE sym_process, HANDLE file, DWORD64 base, const String& path)
{
	if(path.IsEmpty())
		return true;

	String sys_path = ToSystemCharset(path);
	return !!SymLoadModuleEx(sym_process, file, sys_path, NULL, base, 0, NULL, 0);
}

String VsBackendSession::GetBackendName() const
{
	return "vs";
}

DbgRunResult VsBackendSession::Run(const DbgLaunchRequest& request)
{
	DbgRunResult result;
	result.backend_name = "vs";
	result.started = false;
	AppendDbgLaunchRequestTranscript(result.transcript, request);

#ifdef PLATFORM_WIN32
	WString cmdline = MakeCmdLine(request);
	WString envblock = MakeEnvBlock(request.environment);
	Vector<WCHAR> command = ToSystemCharsetW(cmdline);
	command.Add(0);
	Vector<WCHAR> cwd;
	if(!request.working_directory.IsEmpty()) {
		cwd = ToSystemCharsetW(request.working_directory.ToWString());
		cwd.Add(0);
	}
	Vector<WCHAR> env = ToSystemCharsetW(envblock);
	env.Add(0);

	STARTUPINFOW si = {};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {};

	DWORD flags = DEBUG_ONLY_THIS_PROCESS | CREATE_UNICODE_ENVIRONMENT;
	BOOL ok = CreateProcessW(NULL,
	                         command.begin(),
	                         NULL, NULL,
	                         FALSE,
	                         flags,
	                         envblock.GetCount() > 1 ? (LPVOID)env.begin() : NULL,
	                         request.working_directory.IsEmpty() ? NULL : cwd.begin(),
	                         &si, &pi);
	if(!ok) {
		result.error = "CreateProcessW failed: " + GetLastErrorMessage();
		result.exit_code = 1;
		return result;
	}

	result.started = true;
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS);
	HANDLE target_process = pi.hProcess;
	if(!SymInitialize(target_process, NULL, FALSE)) {
		result.error = "SymInitialize failed: " + GetLastErrorMessage();
		result.exit_code = 1;
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return result;
	}
	{
		String sym_path = GetFileDirectory(GetFullPath(request.executable_path));
		if(!request.working_directory.IsEmpty()) {
			if(!sym_path.IsEmpty())
				sym_path << ';';
			sym_path << GetFullPath(request.working_directory);
		}
		if(!sym_path.IsEmpty()) {
			sym_path << ";.";
			SymSetSearchPath(target_process, sym_path);
			result.transcript << "debug: symbol search path = " << sym_path << "\n";
		}
	}

	bool saw_process_exit = false;
	for(;;) {
		DEBUG_EVENT event;
		if(!WaitForDebugEvent(&event, INFINITE)) {
			result.error = "WaitForDebugEvent failed: " + GetLastErrorMessage();
			break;
		}

		DWORD continue_status = DBG_CONTINUE;
		switch(event.dwDebugEventCode) {
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			String module_path = PathFromFileHandle(event.u.CreateProcessInfo.hFile);
			DWORD64 image_base = (DWORD64)event.u.CreateProcessInfo.lpBaseOfImage;
			if(!LoadDebugModule(target_process, event.u.CreateProcessInfo.hFile, image_base, module_path))
				result.transcript << "debug: failed to load image symbols for " << module_path << ": " << GetLastErrorMessage() << "\n";
			SymRefreshModuleList(target_process);
			IMAGEHLP_MODULE64 module;
			memset(&module, 0, sizeof(module));
			module.SizeOfStruct = sizeof(module);
			if(SymGetModuleInfo64(target_process, image_base, &module))
				result.transcript << "debug: image symtype=" << (int)module.SymType << " pdb=" << module.LoadedPdbName
				                  << " image=" << module.LoadedImageName << "\n";
			result.transcript << "debug: load image " << module_path
			                  << " base=" << Format64Hex(image_base) << "\n";
			if(event.u.CreateProcessInfo.hFile)
				CloseHandle(event.u.CreateProcessInfo.hFile);
			if(event.u.CreateProcessInfo.hThread)
				CloseHandle(event.u.CreateProcessInfo.hThread);
			result.transcript << "debug: process created (pid=" << event.dwProcessId << ")\n";
			break;
		}

		case CREATE_THREAD_DEBUG_EVENT:
			if(event.u.CreateThread.hThread)
				CloseHandle(event.u.CreateThread.hThread);
			break;

		case LOAD_DLL_DEBUG_EVENT:
		{
			String module_path = PathFromFileHandle(event.u.LoadDll.hFile);
			DWORD64 dll_base = (DWORD64)event.u.LoadDll.lpBaseOfDll;
			if(!LoadDebugModule(target_process, event.u.LoadDll.hFile, dll_base, module_path))
				result.transcript << "debug: failed to load dll symbols for " << module_path << ": " << GetLastErrorMessage() << "\n";
			SymRefreshModuleList(target_process);
			IMAGEHLP_MODULE64 module;
			memset(&module, 0, sizeof(module));
			module.SizeOfStruct = sizeof(module);
			if(SymGetModuleInfo64(target_process, dll_base, &module))
				result.transcript << "debug: dll symtype=" << (int)module.SymType << " pdb=" << module.LoadedPdbName
				                  << " image=" << module.LoadedImageName << "\n";
			result.transcript << "debug: load dll " << module_path
			                  << " base=" << Format64Hex(dll_base) << "\n";
			if(event.u.LoadDll.hFile)
				CloseHandle(event.u.LoadDll.hFile);
			break;
		}

		case UNLOAD_DLL_DEBUG_EVENT:
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			break;

		case EXCEPTION_DEBUG_EVENT:
		{
			if(event.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT ||
			   event.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP) {
				continue_status = DBG_CONTINUE;
				break;
			}

			result.crashed = true;
			result.transcript << "crash: exception=0x"
			                  << FormatIntHex((int64)event.u.Exception.ExceptionRecord.ExceptionCode)
			                  << " firstChance=" << event.u.Exception.dwFirstChance << "\n";
			HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, event.dwThreadId);
			if(thread) {
				CaptureCallStack(target_process, thread, result);
				CloseHandle(thread);
			}
			else
				result.error = "OpenThread failed while collecting stack: " + GetLastErrorMessage();
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		}

		case EXIT_PROCESS_DEBUG_EVENT:
			result.exit_code = (int)event.u.ExitProcess.dwExitCode;
			result.transcript << "debug: process exited with code " << result.exit_code << "\n";
			saw_process_exit = true;
			break;

		default:
			break;
		}

		ContinueDebugEvent(event.dwProcessId, event.dwThreadId, continue_status);
		if(saw_process_exit)
			break;
		if(result.crashed)
			break;
	}

	SymCleanup(target_process);
	if(IsNull(result.exit_code))
		result.exit_code = result.crashed ? 1 : 0;
	if(!result.crashed && result.exit_code != 0 && result.error.IsEmpty())
		result.error = "process exited with code " + AsString(result.exit_code);
	CloseHandle(pi.hThread);
	CloseHandle(target_process);
#else
	result.error = "vs backend is only available on Windows";
	result.exit_code = 1;
#endif

	return result;
}
