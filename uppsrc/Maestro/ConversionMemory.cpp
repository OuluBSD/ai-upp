#include "Maestro.h"

namespace Upp {

void ConversionMemory::Load(const String& maestro_root)
{
	path = AppendFileName(maestro_root, ".maestro/convert/memory.json");
	if(FileExists(path)) {
		LoadFromJsonFile(*this, path);
	}
}

void ConversionMemory::Save()
{
	RealizeDirectory(GetFileDirectory(path));
	StoreAsJsonFile(*this, path, true);
}

void ConversionMemory::AddDecision(const String& cat, const String& desc, const Value& val, const String& justification)
{
	ConversionDecision& d = decisions.Add();
	d.id = "D-" + Format("%03d", decisions.GetCount());
	d.category = cat;
	d.description = desc;
	d.value = val;
	d.justification = justification;
	d.timestamp = GetSysTime();
	Save();
}

void ConversionMemory::AddConvention(const String& cat, const String& rule, const String& applies_to)
{
	ConversionConvention& c = conventions.Add();
	c.id = "C-" + Format("%03d", conventions.GetCount());
	c.category = cat;
	c.rule = rule;
	c.applies_to = applies_to;
	c.timestamp = GetSysTime();
	Save();
}

void ConversionMemory::AddIssue(const String& severity, const String& desc, const Vector<String>& related_tasks)
{
	ConversionIssue& i = issues.Add();
	i.id = "I-" + Format("%03d", issues.GetCount());
	i.severity = severity;
	i.description = desc;
	i.related_tasks = clone(related_tasks);
	i.timestamp = GetSysTime();
	Save();
}

String ConversionMemory::ComputeDecisionFingerprint() const
{
	String s;
	for(const auto& d : decisions) {
		if(d.status == "active")
			s << d.id << ":" << d.value << ";";
	}
	return SHA256String(s);
}

}