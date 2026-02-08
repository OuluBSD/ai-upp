
#ifndef _Maestro_ConversionMemory_h_
#define _Maestro_ConversionMemory_h_

struct ConversionDecision : Moveable<ConversionDecision> {
	String   id;
	String   category;
	String   description;
	Value    value;
	String   justification;
	String   status = "active";
	Time     timestamp;
	String   created_by = "ai";

	void Jsonize(JsonIO& jio) {
		jio("id", id)("category", category)("description", description)("value", value)
		   ("justification", justification)("status", status)("timestamp", timestamp)
		   ("created_by", created_by);
	}
};

struct ConversionConvention : Moveable<ConversionConvention> {
	String id;
	String category;
	String rule;
	String applies_to;
	Time   timestamp;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("category", category)("rule", rule)("applies_to", applies_to)("timestamp", timestamp);
	}
};

struct ConversionIssue : Moveable<ConversionIssue> {
	String         id;
	String         severity; // "low", "medium", "high"
	String         description;
	String         status = "open";
	Vector<String> related_tasks;
	String         resolution;
	Time           timestamp;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("severity", severity)("description", description)("status", status)
		   ("related_tasks", related_tasks)("resolution", resolution)("timestamp", timestamp);
	}
};

class ConversionMemory {
	String path;

public:
	Array<ConversionDecision>   decisions;
	Array<ConversionConvention> conventions;
	Array<ConversionIssue>      issues;
	ArrayMap<String, String>    glossary;
	Vector<ValueMap>            summary_log;

	void Load(const String& maestro_root);
	void Save();
	
	void AddDecision(const String& cat, const String& desc, const Value& val, const String& justification);
	void AddConvention(const String& cat, const String& rule, const String& applies_to);
	void AddIssue(const String& severity, const String& desc, const Vector<String>& related_tasks);
	
	String ComputeDecisionFingerprint() const;
	
	void Jsonize(JsonIO& jio) {
		jio("decisions", decisions)("conventions", conventions)("issues", issues)
		   ("glossary", glossary)("summary_log", summary_log);
	}
};

#endif
