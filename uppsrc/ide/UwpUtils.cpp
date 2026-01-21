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

bool IsUwpApp(const String& path)
{
	String path_norm = NormalizePath(path);
	String folder = GetFileFolder(path_norm);
	
	// Check 1: Manifest in the same folder (e.g. running from AppxLayout)
	if(FileExists(AppendFileName(folder, "AppxManifest.xml"))) return true;
	
	// Check 2: Manifest in AppxLayout subdirectory (standard U++ build)
	if(FileExists(AppendFileName(AppendFileName(folder, "AppxLayout"), "AppxManifest.xml"))) return true;

	return false;
}

String GetUwpPackageName(const String& folder)
{
	String manifestPath = AppendFileName(folder, "AppxManifest.xml");
	if(!FileExists(manifestPath)) {
		// Check for AppxLayout structure
		if(GetFileTitle(folder) == "AppxLayout") // We might be inside AppxLayout/
			manifestPath = AppendFileName(folder, "AppxManifest.xml");
		else
		if (GetFileTitle(folder) == "AppxLayout") // path passed might be the exe
			manifestPath = AppendFileName(folder, "AppxManifest.xml");
		else
		{
			// Try parent folder if we are in AppxLayout
			String parent = GetFileFolder(folder);
			if(GetFileTitle(parent) == "AppxLayout")
				manifestPath = AppendFileName(parent, "AppxManifest.xml");
			else // Or maybe we are in bin directory and AppxLayout is subdirectory?
			    manifestPath = AppendFileName(AppendFileName(folder, "AppxLayout"), "AppxManifest.xml");
		}
	}
	
	if(!FileExists(manifestPath)) {
		// Try one more: if path is the executable, folder is its dir.
		// If structure is bin/out/Package/AppxLayout/App.exe
		// manifest is in AppxLayout/AppxManifest.xml.
		return Null;
	}

	try {
		XmlNode n = ParseXMLFile(manifestPath);
		if(n.IsVoid()) {
			Exclamation("UWPUtils: ParseXMLFile returned empty node for:\n" + DeQtf(manifestPath));
			return Null;
		}
		
		const XmlNode* packageNode = &n;
		if(n.GetType() == XML_DOC) {
			const XmlNode& p = n["Package"];
			if(p.IsVoid()) {
				Exclamation("UWPUtils: <Package> tag not found in:\n" + DeQtf(manifestPath));
				return Null;
			}
			packageNode = &p;
		} else if(n.GetTag() != "Package") {
			Exclamation("UWPUtils: Root tag is '" + DeQtf(n.GetTag()) + "', expected 'Package' in:\n" + DeQtf(manifestPath));
			return Null;
		}

		const XmlNode& identity = (*packageNode)["Identity"];
		if(!identity.IsVoid())
			return identity.Attr("Name");
		else
			Exclamation("UWPUtils: <Identity> tag not found in:\n" + DeQtf(manifestPath));
	}
	catch(XmlError e) {
		Exclamation("UWPUtils: XML Parse Error: " + DeQtf(e) + "\nFile: " + DeQtf(manifestPath));
	}
	catch(...) {
		Exclamation("UWPUtils: Unknown exception while parsing:\n" + DeQtf(manifestPath));
	}
	return Null;
}

String GetUwpPackageFamilyName(const String& pkgName)
{
	String cmd;
	cmd << "powershell -Command \"(Get-AppxPackage -Name '" << pkgName << "').PackageFamilyName\"";
	String out;
	if(Sys(cmd, out) == 0) {
		return TrimBoth(out);
	}
	return Null;
}

void RegisterUwpApp(const String& folder)
{
	String manifest = AppendFileName(folder, "AppxManifest.xml");
	if(!FileExists(manifest)) return;
	
	String cmd;
	cmd << "powershell -Command \"Add-AppxPackage -Register '" << manifest << "' -ForceApplicationShutdown\"";
	String out;
	Sys(cmd, out);
}

bool LaunchUwpApp(const String& path, const String& args, bool debug, DWORD& pid)
{
	String folder = GetFileFolder(path);
	String name = GetUwpPackageName(folder);
	if(IsNull(name)) {
		Exclamation("UWP: Could not determine Package Name from manifest.");
		return false;
	}
	
	RegisterUwpApp(folder); // Ensure the app is registered from this location
	
	String pfn = GetUwpPackageFamilyName(name);
	if(IsNull(pfn)) {
		Exclamation("UWP: Could not find Package Family Name for " + name + ".\nEnsure the package is registered (deploy/build successfully).");
		return false;
	}
	
	String aumid = pfn + "!App"; // Hardcoded "App" ID for now, matches MscBuilder template

	HRESULT hr = E_FAIL;
	CoInitialize(NULL);

	if(debug) {
		IPackageDebugSettings *pds = NULL;
		hr = CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_INPROC_SERVER, IID_IPackageDebugSettings, (void**)&pds);
		if(SUCCEEDED(hr) && pds) {
			hr = pds->EnableDebugging(ToSystemCharsetW(pfn), NULL, NULL);
			if(FAILED(hr)) Exclamation("UWP: Failed to EnableDebugging. HRESULT: " + FormatIntHex(hr));
			pds->Release();
		} else {
			Exclamation("UWP: Failed to create IPackageDebugSettings. HRESULT: " + FormatIntHex(hr));
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