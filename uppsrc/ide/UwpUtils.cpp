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

String GetUwpPackageName(const String& folder)
{
	String manifestPath = FindUwpManifest(folder);
	if(IsNull(manifestPath)) return Null;

	try {
		XmlNode n = ParseXMLFile(manifestPath);
		if(n.IsVoid()) {
			PutConsole("UWPUtils: ParseXMLFile returned empty node for: " + manifestPath);
			return Null;
		}
		
		const XmlNode* packageNode = &n;
		if(n.GetType() == XML_DOC) {
			const XmlNode& p = n["Package"];
			if(p.IsVoid()) {
				PutConsole("UWPUtils: <Package> tag not found in: " + manifestPath);
				return Null;
			}
			packageNode = &p;
		} else if(n.GetTag() != "Package") {
			PutConsole("UWPUtils: Root tag is '" + n.GetTag() + "', expected 'Package' in: " + manifestPath);
			return Null;
		}

		const XmlNode& identity = (*packageNode)["Identity"];
		if(!identity.IsVoid())
			return identity.Attr("Name");
		else
			PutConsole("UWPUtils: <Identity> tag not found in: " + manifestPath);
	}
	catch(XmlError e) {
		PutConsole("UWPUtils: XML Parse Error: " + e + "\nFile: " + manifestPath);
	}
	catch(...)
	{
		PutConsole("UWPUtils: Unknown exception while parsing: " + manifestPath);
	}
	return Null;
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
	if(IsNull(manifest)) {
		PutConsole("UWP: Could not find manifest to register in " + folder);
		return;
	}
	
	PutConsole("UWP: Registering application from " + manifest);
	String cmd;
	cmd << "powershell -NoProfile -Command \"Add-AppxPackage -Register '" << manifest << "' -ForceApplicationShutdown\"";
	String out;
	if(Sys(cmd, out) != 0) {
		PutConsole("UWP: Registration failed:\n" + out);
	}
}

bool LaunchUwpApp(const String& path, const String& args, bool debug, DWORD& pid)
{
	String folder = GetFileFolder(path);
	String name = GetUwpPackageName(folder);
	if(IsNull(name)) {
		Exclamation("UWP: Could not determine Package Name from manifest at " + folder);
		return false;
	}
	
	RegisterUwpApp(folder); // Ensure the app is registered from this location
	
	String pfn = GetUwpPackageFamilyName(name);
	String pfull = GetUwpPackageFullName(name);
	
	if(IsNull(pfn) || IsNull(pfull)) {
		Exclamation("UWP: Could not find Package Family/Full Name for " + name + ".\nEnsure the package is registered correctly.");
		return false;
	}
	
	String aumid = pfn + "!App"; // Hardcoded "App" ID for now, matches MscBuilder template

	HRESULT hr = E_FAIL;
	CoInitialize(NULL);

	if(debug) {
		IPackageDebugSettings *pds = NULL;
		hr = CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, IID_IPackageDebugSettings, (void**)&pds);
		if(SUCCEEDED(hr) && pds) {
			hr = pds->EnableDebugging(ToSystemCharsetW(pfull), NULL, NULL);
			if(FAILED(hr)) PutConsole("UWP: Failed to EnableDebugging. HRESULT: " + FormatIntHex(hr) + "\nPackage: " + pfull);
			pds->Release();
		} else {
			PutConsole("UWP: Failed to create IPackageDebugSettings. HRESULT: " + FormatIntHex(hr));
		}
	}

	IApplicationActivationManager *aam = NULL;
	hr = CoCreateInstance(CLSID_ApplicationActivationManager, NULL, CLSCTX_LOCAL_SERVER, IID_IApplicationActivationManager, (void**)&aam);
	if(SUCCEEDED(hr) && aam) {
		Vector<wchar_t> w_aumid = ToSystemCharsetW(aumid);
		Vector<wchar_t> w_args;
		if(!IsNull(args)) w_args = ToSystemCharsetW(args);

		hr = aam->ActivateApplication(w_aumid, 
		                              IsNull(args) ? (LPCWSTR)NULL : w_args, 
		                              AO_NONE, &pid);
		if(FAILED(hr)) {
			String msg = "UWP: ActivateApplication failed. HRESULT: " + FormatIntHex(hr);
			if(hr == 0x8027025b) msg << "\n(The app didn't start in the required time)";
			PutConsole(msg);
			Exclamation(msg);
		}
		aam->Release();
	} else {
		Exclamation("UWP: Failed to create IApplicationActivationManager. HRESULT: " + FormatIntHex(hr));
	}
	
	CoUninitialize();
	return SUCCEEDED(hr);
}

void StopUwpDebug(const String& path)
{
	String folder = GetFileFolder(path);
	String name = GetUwpPackageName(folder);
	if(IsNull(name)) return;
	String pfn = GetUwpPackageFamilyName(name);
	if(IsNull(pfn)) return;
	
	CoInitialize(NULL);
	IPackageDebugSettings *pds = NULL;
	HRESULT hr = CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, IID_IPackageDebugSettings, (void**)&pds);
	if(SUCCEEDED(hr) && pds) {
		pds->DisableDebugging(ToSystemCharsetW(pfn));
		pds->Release();
	}
	CoUninitialize();
}

#endif
