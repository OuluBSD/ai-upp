#ifndef _Maestro_SemanticIntegrity_h_
#define _Maestro_SemanticIntegrity_h_

struct SemanticResult : Moveable<SemanticResult> {
	String         semantic_equivalence = "unknown"; // "high", "medium", "low"
	double         confidence = 0.0;
	Vector<String> preserved_concepts;
	Vector<String> changed_concepts;
	Vector<String> lost_concepts;
	Vector<String> assumptions;
	Vector<String> risk_flags;
	bool           requires_human_review = false;

	void Jsonize(JsonIO& jio) {
		jio("semantic_equivalence", semantic_equivalence)("confidence", confidence)
		   ("preserved_concepts", preserved_concepts)("changed_concepts", changed_concepts)
		   ("lost_concepts", lost_concepts)("assumptions", assumptions)
		   ("risk_flags", risk_flags)("requires_human_review", requires_human_review);
	}
};

struct SemanticSummary : Moveable<SemanticSummary> {
	int      total_files_checked = 0;
	ValueMap equivalence_counts;
	ValueMap cumulative_risk_flags;
	int      unresolved_semantic_warnings = 0;
	Time     last_updated;

	void Jsonize(JsonIO& jio) {
		jio("total_files_checked", total_files_checked)("equivalence_counts", equivalence_counts)
		   ("cumulative_risk_flags", cumulative_risk_flags)
		   ("unresolved_semantic_warnings", unresolved_semantic_warnings)
		   ("last_updated", last_updated);
	}
};

class SemanticIntegrityChecker {
	String base_path;
	String summary_path;
	String issues_path;

public:
	SemanticIntegrityChecker(const String& maestro_root = ".");
	
	SemanticResult RunCheck(const WorkGraphTask& task, const String& source_root, const String& target_root);
	SemanticResult RunCheck(const String& task_id, const Vector<String>& inputs, const Vector<String>& outputs, const String& source_root, const String& target_root, const String& intent);
	SemanticSummary GetSummary();
	
private:
	SemanticResult AnalyzeEquivalence(const String& source_content, const String& target_content, const String& intent);
	void SaveResult(const String& task_id, const SemanticResult& res);
	void UpdateSummary(const SemanticResult& res);
};

#endif
