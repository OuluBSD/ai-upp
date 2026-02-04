#include "Maestro.h"

namespace Upp {

LogManager::LogManager(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
	scans_dir = AppendFileName(base_path, "docs/maestro/log_scans");
	RealizeDirectory(scans_dir);
}

String LogManager::CreateScan(const String& source_path, const String& log_text, const String& kind)
{
	Time t = GetSysTime();
	String ts = Format("%04d%02d%02d_%02d%02d%02d", t.year, t.month, t.day, t.hour, t.minute, t.second);
	String scan_id = Format("%s_%s", ts, kind);
	
	LogScan scan;
	scan.meta.scan_id = scan_id;
	scan.meta.timestamp = GetSysTime();
	scan.meta.source_path = source_path;
	scan.meta.kind = kind;
	scan.meta.cwd = GetCurrentDirectory();
	
	String text = log_text;
	if(text.IsEmpty() && !source_path.IsEmpty())
		text = LoadFile(source_path);
	
	scan.findings = ExtractFindings(text, kind);
	scan.meta.finding_count = scan.findings.GetCount();
	
	String dir = AppendFileName(scans_dir, scan_id);
	RealizeDirectory(dir);
	
	StoreAsJsonFile(scan.meta, AppendFileName(dir, "meta.json"), true);
	StoreAsJsonFile(scan.findings, AppendFileName(dir, "findings.json"), true);
	SaveFile(AppendFileName(dir, "raw.txt"), text);
	
	return scan_id;
}

LogScan LogManager::LoadScan(const String& scan_id)
{
	LogScan scan;
	String dir = AppendFileName(scans_dir, scan_id);
	LoadFromJsonFile(scan.meta, AppendFileName(dir, "meta.json"));
	LoadFromJsonFile(scan.findings, AppendFileName(dir, "findings.json"));
	return scan;
}

Array<LogScanMeta> LogManager::ListScans()
{
	Array<LogScanMeta> list;
	FindFile ff(AppendFileName(scans_dir, "*"));
	while(ff) {
		if(ff.IsDirectory()) {
			LogScanMeta meta;
			if(LoadFromJsonFile(meta, AppendFileName(ff.GetPath(), "meta.json")))
				list.Add(meta);
		}
		ff.Next();
	}
	return list;
}

Array<LogFinding> LogManager::ExtractFindings(const String& log_text, const String& kind_filter)
{
	Array<LogFinding> findings;
	
	// Simple regex patterns (converted to string find for now to avoid Pcre complexity if possible,
	// but for robustness we really need Regex)
	
	// Let's use simple string search for MVP to avoid Pcre link issues if any
	// Real implementation should use RegExp
	
	Vector<String> lines = Split(log_text, '\n');
	for(const String& line : lines) {
		String l = line; // Copy
		l = TrimBoth(l);
		if(l.IsEmpty()) continue;
		
		String kind, severity;
		if(l.Find("error:") >= 0 || l.Find("fatal error:") >= 0) {
			kind = "error"; severity = "blocker";
		}
		else if(l.Find("warning:") >= 0) {
			kind = "warning"; severity = "warning";
		}
		else if(l.Find("Segmentation fault") >= 0) {
			kind = "crash"; severity = "critical";
		}
		else continue;
		
		if(kind_filter != "any" && kind != kind_filter) continue;
		
		LogFinding& f = findings.Add();
		f.kind = kind;
		f.severity = severity;
		f.message = l;
		f.raw_line = line;
		
		// Fingerprint
		f.fingerprint = GenerateFingerprint(l, "", "");
	}
	
	return findings;
}

String LogManager::GenerateFingerprint(const String& message, const String& tool, const String& file)
{
	// normalize
	String s = message;
	// Remove numbers to dedupe line changes
	// s = RegExp(":\d+:").GlobalReplace(s, ":<LINE>:"); // Requires Pcre
	return SHA256String(s);
}

} 
