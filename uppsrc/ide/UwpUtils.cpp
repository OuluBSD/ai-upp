#include "ide.h"
#include "UwpUtils.h"

#ifdef PLATFORM_WIN32

#ifdef Ptr
#undef Ptr
#endif
#define Ptr Ptr_ 

#ifdef byte
#undef byte
#endif
#define byte byte_ 

#ifdef CY
#undef CY
#endif
#define CY win32_CY_ 

#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif

#include <shobjidl.h>
#include <appmodel.h>
#include <shlwapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "shlwapi.lib")

// IPackageDebugSettings is in shobjidl.h (Windows 8+)
// IApplicationActivationManager is in shobjidl.h

String FindUwpManifest(const String& folder)
{
	String manifestPath = AppendFileName(folder, "AppxManifest.xml");
	if(FileExists(manifestPath)) return manifestPath;
	
	// Check for AppxLayout subdirectory
	manifestPath = AppendFileName(AppendFileName(folder, "AppxLayout"), "AppxManifest.xml");
	if(FileExists(manifestPath)) return manifestPath;
	
	// Check if we are inside AppxLayout
	if(GetFileTitle(folder) == "AppxLayout") {
		manifestPath = AppendFileName(folder, "AppxManifest.xml");
		if(FileExists(manifestPath)) return manifestPath;
	}
	
	// Check parent if it's AppxLayout
	String parent = GetFileFolder(folder);
	if(GetFileTitle(parent) == "AppxLayout") {
		manifestPath = AppendFileName(parent, "AppxManifest.xml");
		if(FileExists(manifestPath)) return manifestPath;
	}

	return Null;
}

bool IsUwpApp(const String& path)
{
	return !IsNull(FindUwpManifest(GetFileFolder(path)));
}

struct UwpManifestInfo {
	String name;
	String appId;
};

UwpManifestInfo GetUwpManifestInfo(const String& folder)
{
	UwpManifestInfo info;
	String manifestPath = FindUwpManifest(folder);
	if(IsNull(manifestPath)) return info;

	try {
		XmlNode n = ParseXMLFile(manifestPath);
		if(n.IsVoid()) return info;
		
		const XmlNode* packageNode = &n;
		if(n.GetType() == XML_DOC) {
			const XmlNode& p = n["Package"];
			if(p.IsVoid()) return info;
			packageNode = &p;
		} else if(n.GetTag() != "Package") {
			return info;
		}

		const XmlNode& identity = (*packageNode)["Identity"];
		if(!identity.IsVoid())
			info.name = identity.Attr("Name");
		
		const XmlNode& apps = (*packageNode)["Applications"];
		if(!apps.IsVoid()) {
			const XmlNode& app = apps["Application"];
			if(!app.IsVoid())
				info.appId = app.Attr("Id");
		}
	}
	catch(...) {}
	return info;
}

String GetUwpPackageName(const String& folder)
{
	return GetUwpManifestInfo(folder).name;
}

String GetUwpPackageFamilyName(const String& pkgName)
{
	String cmd;
	cmd << "powershell -NoProfile -Command \"(Get-AppxPackage -Name '" << pkgName << "').PackageFamilyName\"";
	String out;
	if(Sys(cmd, out) == 0) {
		return TrimBoth(out);
	}
	return Null;
}

String GetUwpPackageFullName(const String& pkgName)
{
	String cmd;
	cmd << "powershell -NoProfile -Command \"(Get-AppxPackage -Name '" << pkgName << "').PackageFullName\"";
	String out;
	if(Sys(cmd, out) == 0) {
		return TrimBoth(out);
	}
	return Null;
}

void RegisterUwpApp(const String& folder)
{
	String manifest = FindUwpManifest(folder);
	if(IsNull(manifest)) return;
	
	PutConsole("UWP: Registering application from " + manifest);
	String cmd;
	// Use -ErrorAction SilentlyContinue to avoid noise, but 2>&1 to catch errors in 'out'
	cmd << "powershell -NoProfile -Command \"Add-AppxPackage -Register '" << manifest << "' -ForceApplicationShutdown 2>&1\"";
	String out;
	int res = Sys(cmd, out);
	if(!IsNull(out) && out.Find("Deployment failed") >= 0) {
		PutConsole("UWP: Registration output:\n" + out);
	}
	if(res == 0) {
		Sleep(500); // Give Windows a moment to process the registration
	}
}

// Enable UWP prelaunch debugging - this sets up debug mode before app starts
bool EnableUwpPrelaunch(const String& path, const String& debuggerPath)
{
	String folder = GetFileFolder(path);
	String name = GetUwpPackageName(folder);
	if(IsNull(name)) {
		PutConsole("UWP Prelaunch: Could not determine package name");
		return false;
	}

	String pfull = GetUwpPackageFullName(name);
	if(IsNull(pfull)) {
		PutConsole("UWP Prelaunch: Could not find Package Full Name for " + name);
		return false;
	}

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	bool co_uninit = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;

	IPackageDebugSettings *pds = NULL;
	hr = CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, IID_IPackageDebugSettings, (void**)&pds);
	if(SUCCEEDED(hr) && pds) {
		PutConsole("UWP Prelaunch: Terminating existing processes for " + pfull);
		pds->TerminateAllProcesses(ToSystemCharsetW(pfull).begin());

		PutConsole("UWP Prelaunch: Disabling any previous debugging");
		pds->DisableDebugging(ToSystemCharsetW(pfull).begin());

		// EnableDebugging with debugger path (can be NULL for just enabling debug mode)
		LPCWSTR dbgPath = debuggerPath.IsEmpty() ? NULL : ToSystemCharsetW(debuggerPath).begin();
		PutConsole("UWP Prelaunch: Enabling debugging for package");
		hr = pds->EnableDebugging(ToSystemCharsetW(pfull).begin(), dbgPath, NULL);
		if(FAILED(hr)) {
			PutConsole("UWP Prelaunch: EnableDebugging failed: " + FormatIntHex((dword)hr));
			if(hr == 0x80070005) PutConsole("  (E_ACCESSDENIED - run as administrator or check developer mode)");
		} else {
			PutConsole("UWP Prelaunch: Debug mode enabled successfully");
		}
		pds->Release();
	} else {
		PutConsole("UWP Prelaunch: Failed to create PackageDebugSettings: " + FormatIntHex((dword)hr));
	}

	if(co_uninit) CoUninitialize();
	return SUCCEEDED(hr);
}

// Launch UWP app via direct process creation (for debugging)
bool LaunchUwpAppDirect(const String& path, DWORD& pid)
{
	String folder = GetFileFolder(path);
	String manifestFolder = folder;

	// Find the AppxLayout folder
	String manifestPath = FindUwpManifest(folder);
	if(!IsNull(manifestPath)) {
		manifestFolder = GetFileFolder(manifestPath);
	}

	// The exe should be in the layout folder
	String exePath = AppendFileName(manifestFolder, GetFileName(path));
	if(!FileExists(exePath)) {
		exePath = path; // Fallback to original path
	}

	PutConsole("UWP Direct Launch: Starting " + exePath);

	STARTUPINFOW si = {};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {};

	auto w_path = ToSystemCharsetW(exePath);

	if(CreateProcessW(w_path.begin(), NULL, NULL, NULL, FALSE,
	                  CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
		pid = pi.dwProcessId;
		PutConsole("UWP Direct Launch: Process created with PID " + AsString(pid) + " (suspended)");

		// Resume the process
		ResumeThread(pi.hThread);
		PutConsole("UWP Direct Launch: Process resumed");

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return true;
	} else {
		DWORD err = GetLastError();
		PutConsole("UWP Direct Launch: CreateProcess failed with error " + AsString(err));
		return false;
	}
}

// Launch UWP app for debugging - keeps process suspended, returns handles
// Caller is responsible for resuming thread and closing handles
bool LaunchUwpAppForDebug(const String& path, DWORD& pid, HANDLE& hProcess, HANDLE& hThread)
{
	String folder = GetFileFolder(path);
	UwpManifestInfo info = GetUwpManifestInfo(folder);

	// First register the app
	RegisterUwpApp(folder);

	// Enable debugging via IPackageDebugSettings
	if(!IsNull(info.name)) {
		String pfull = GetUwpPackageFullName(info.name);
		if(!IsNull(pfull)) {
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
			bool co_uninit = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;

			IPackageDebugSettings *pds = NULL;
			hr = CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER,
			                      IID_IPackageDebugSettings, (void**)&pds);
			if(SUCCEEDED(hr) && pds) {
				PutConsole("UWP Debug: Terminating existing processes...");
				pds->TerminateAllProcesses(ToSystemCharsetW(pfull).begin());

				PutConsole("UWP Debug: Enabling debug mode...");
				hr = pds->EnableDebugging(ToSystemCharsetW(pfull).begin(), NULL, NULL);
				if(FAILED(hr)) {
					PutConsole("UWP Debug: EnableDebugging failed: " + FormatIntHex((dword)hr));
				} else {
					PutConsole("UWP Debug: Debug mode enabled");
				}
				pds->Release();
			}
			if(co_uninit) CoUninitialize();
		}
	}

	// Find the exe in AppxLayout folder
	String manifestFolder = folder;
	String manifestPath = FindUwpManifest(folder);
	if(!IsNull(manifestPath)) {
		manifestFolder = GetFileFolder(manifestPath);
	}

	String exePath = AppendFileName(manifestFolder, GetFileName(path));
	if(!FileExists(exePath)) {
		exePath = path;
	}

	PutConsole("UWP Debug: Creating process (suspended): " + exePath);

	STARTUPINFOW si = {};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {};

	auto w_path = ToSystemCharsetW(exePath);

	// Use DEBUG_ONLY_THIS_PROCESS - process starts in debug mode, debugger receives events
	// Don't use CREATE_SUSPENDED as the debug event loop handles synchronization
	if(CreateProcessW(w_path.begin(), NULL, NULL, NULL, FALSE,
	                  DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi)) {
		pid = pi.dwProcessId;
		hProcess = pi.hProcess;
		hThread = pi.hThread;
		PutConsole("UWP Debug: Process created with PID " + AsString(pid) + " (debug attached)");
		return true;
	} else {
		DWORD err = GetLastError();
		PutConsole("UWP Debug: CreateProcess failed with error " + AsString(err));
		return false;
	}
}

bool LaunchUwpApp(const String& path, const String& args, bool debug, DWORD& pid)
{
	String folder = GetFileFolder(path);
	UwpManifestInfo info = GetUwpManifestInfo(folder);
	if(IsNull(info.name)) {
		String msg = "UWP: Could not determine Package Name from manifest at " + folder;
		PutConsole(msg);
		return false;
	}

	RegisterUwpApp(folder);

	String pfn = GetUwpPackageFamilyName(info.name);
	String pfull = GetUwpPackageFullName(info.name);

	if(IsNull(pfn) || IsNull(pfull)) {
		String msg = "UWP: Could not find Package Family/Full Name for " + info.name + ". Ensure it built and registered correctly.";
		PutConsole(msg);
		return false;
	}

	String appId = Nvl(info.appId, "App");
	String aumid = pfn + "!" + appId;

	PutConsole("UWP: Package Full Name: " + pfull);
	PutConsole("UWP: Activating AUMID: " + aumid);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	bool co_uninit = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;

	if(debug) {
		IPackageDebugSettings *pds = NULL;
		hr = CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, IID_IPackageDebugSettings, (void**)&pds);
		if(SUCCEEDED(hr) && pds) {
			PutConsole("UWP: Terminating existing processes...");
			pds->TerminateAllProcesses(ToSystemCharsetW(pfull).begin());

			PutConsole("UWP: Disabling previous debugging...");
			pds->DisableDebugging(ToSystemCharsetW(pfull).begin());

			PutConsole("UWP: Enabling debug mode...");
			hr = pds->EnableDebugging(ToSystemCharsetW(pfull).begin(), NULL, NULL);
			if(FAILED(hr)) {
				PutConsole("UWP: EnableDebugging failed: " + FormatIntHex((dword)hr));
				if(hr == 0x80070005) PutConsole("  (E_ACCESSDENIED - run as administrator or enable developer mode)");
				PutConsole("UWP: Continuing without debug settings...");
			} else {
				PutConsole("UWP: Debug mode enabled for package");
			}
			pds->Release();
		} else {
			PutConsole("UWP: Could not create PackageDebugSettings: " + FormatIntHex((dword)hr));
		}
	}

	IApplicationActivationManager *aam = NULL;
	hr = CoCreateInstance(CLSID_ApplicationActivationManager, NULL, CLSCTX_LOCAL_SERVER, IID_IApplicationActivationManager, (void**)&aam);
	if(SUCCEEDED(hr) && aam) {
		auto w_aumid = ToSystemCharsetW(aumid);
		auto w_args = ToSystemCharsetW(args);

		// Try with AO_NOERRORUI first to avoid UI blocking
		ACTIVATEOPTIONS opts = debug ? AO_NONE : AO_NOERRORUI;
		hr = aam->ActivateApplication(w_aumid.begin(),
		                              args.IsEmpty() ? (LPCWSTR)NULL : w_args.begin(),
		                              opts, &pid);

		if(FAILED(hr)) {
			String msg = "UWP: ActivateApplication failed: " + FormatIntHex((dword)hr);
			// Decode common HRESULT values
			if(hr == 0x80004005) msg << " (E_FAIL - generic failure, app may have crashed on startup)";
			else if(hr == 0x80070057) msg << " (E_INVALIDARG - AUMID may be wrong or app not registered)";
			else if(hr == 0x80073D54) msg << " (ERROR_PACKAGES_IN_USE - package is being used)";
			else if(hr == 0x80073CF0) msg << " (ERROR_INSTALL_OPEN_PACKAGE_FAILED)";
			else if(hr == 0x80073D5A) msg << " (ERROR_INSTALL_RESOLVE_DEPENDENCY_FAILED)";
			else if(hr == 0x80073CFB) msg << " (ERROR_PACKAGE_ALREADY_EXISTS)";

			PutConsole(msg);
			PutConsole("UWP: Attempting fallback via direct process launch...");

			// Try direct launch for debugging
			if(debug && LaunchUwpAppDirect(path, pid)) {
				PutConsole("UWP: Direct launch successful, PID: " + AsString(pid));
				hr = S_OK;
			} else {
				// Try ShellExecute fallback
				PutConsole("UWP: Attempting fallback via ShellExecute...");
				String shellCmd = "shell:AppsFolder\\" + aumid;
				HINSTANCE hInst = ShellExecuteW(NULL, L"open", ToSystemCharsetW(shellCmd).begin(), NULL, NULL, SW_SHOWNORMAL);
				if((intptr_t)hInst > 32) {
					PutConsole("UWP: ShellExecute launch initiated.");

					// For debugging, we need to find the process ID
					Sleep(2000); // Give more time for UWP app to start

					// Find process by exe name
					String exeName = GetFileName(path);
					HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
					if(hSnap != INVALID_HANDLE_VALUE) {
						PROCESSENTRY32 pe;
						pe.dwSize = sizeof(pe);
						if(Process32First(hSnap, &pe)) {
							do {
								if(ToLower(String(pe.szExeFile)) == ToLower(exeName)) {
									pid = pe.th32ProcessID;
									PutConsole("UWP: Found process " + exeName + " with PID: " + AsString(pid));
									hr = S_OK;
									break;
								}
							} while(Process32Next(hSnap, &pe));
						}
						CloseHandle(hSnap);
					}

					if(pid == 0) {
						PutConsole("UWP: Warning - Could not find process ID for " + exeName);
						PutConsole("UWP: The app may have failed to start or is not running as a desktop process");
					}
				} else {
					PutConsole("UWP: ShellExecute also failed");
				}
			}
		} else {
			PutConsole("UWP: App activated successfully, PID: " + AsString(pid));
		}
		aam->Release();
	} else {
		PutConsole("UWP: Failed to create IApplicationActivationManager: " + FormatIntHex((dword)hr));
	}

	if(co_uninit) CoUninitialize();
	return SUCCEEDED(hr);
}

void StopUwpDebug(const String& path)
{
	String folder = GetFileFolder(path);
	String name = GetUwpPackageName(folder);
	if(IsNull(name)) return;
	String pfull = GetUwpPackageFullName(name);
	if(IsNull(pfull)) return;
	
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	bool co_uninit = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;
	
	IPackageDebugSettings *pds = NULL;
	if(SUCCEEDED(CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, IID_IPackageDebugSettings, (void**)&pds))) {
		pds->DisableDebugging(ToSystemCharsetW(pfull).begin());
		pds->Release();
	}
	
	if(co_uninit) CoUninitialize();
}

#endif
