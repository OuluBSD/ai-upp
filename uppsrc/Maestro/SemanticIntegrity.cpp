#include "Maestro.h"

namespace Upp {

SemanticIntegrityChecker::SemanticIntegrityChecker(const String& maestro_root)
{
	base_path = AppendFileName(maestro_root, ".maestro/convert/semantics");
	summary_path = AppendFileName(base_path, "summary.json");
	issues_path = AppendFileName(base_path, "open_issues.json");
	
	RealizeDirectory(base_path);
}

SemanticResult SemanticIntegrityChecker::RunCheck(const WorkGraphTask& task, const String& source_root, const String& target_root)
{
	String source_content;
	for(const auto& f : task.inputs)
		source_content << LoadFile(AppendFileName(source_root, f)) << "\n";
	
	String target_content;
	for(const auto& f : task.outputs)
		target_content << LoadFile(AppendFileName(target_root, f)) << "\n";
	
	SemanticResult res = AnalyzeEquivalence(source_content, target_content, task.intent);
	
	SaveResult(task.id, res);
	UpdateSummary(res);
	
	return res;
}

SemanticResult SemanticIntegrityChecker::RunCheck(const String& task_id, const Vector<String>& inputs, const Vector<String>& outputs, const String& source_root, const String& target_root, const String& intent)
{
	String source_content;
	for(const auto& f : inputs)
		source_content << LoadFile(AppendFileName(source_root, f)) << "\n";
	
	String target_content;
	for(const auto& f : outputs)
		target_content << LoadFile(AppendFileName(target_root, f)) << "\n";
	
	SemanticResult res = AnalyzeEquivalence(source_content, target_content, intent);
	
	SaveResult(task_id, res);
	UpdateSummary(res);
	
	return res;
}

SemanticResult SemanticIntegrityChecker::AnalyzeEquivalence(const String& source_content, const String& target_content, const String& intent)
{
	SemanticResult res;
	
	// Basic heuristic analysis
	if(target_content.IsEmpty() && !source_content.IsEmpty()) {
		res.semantic_equivalence = "low";
		res.confidence = 1.0;
		res.lost_concepts.Add("all_content");
		res.requires_human_review = true;
		return res;
	}
	
	// Check for common risk keywords
	static const char* risks[] = { "if", "for", "while", "malloc", "free", "thread", "mutex", "socket" };
	for(const char* r : risks) {
		if(target_content.Find(r) >= 0)
			res.risk_flags.Add(r);
	}
	
	// Simple length-based heuristic
	double ratio = (double)target_content.GetCount() / max(1, source_content.GetCount());
	if(ratio > 0.5 && ratio < 2.0) {
		res.semantic_equivalence = "high";
		res.confidence = 0.7;
	} else {
		res.semantic_equivalence = "medium";
		res.confidence = 0.5;
		res.requires_human_review = true;
	}
	
	return res;
}

void SemanticIntegrityChecker::SaveResult(const String& task_id, const SemanticResult& res)
{
	String path = AppendFileName(base_path, "task_" + task_id + ".json");
	StoreAsJsonFile(res, path, true);
}

void SemanticIntegrityChecker::UpdateSummary(const SemanticResult& res)
{
	SemanticSummary s = GetSummary();
	s.total_files_checked++;
	
	Value& count = s.equivalence_counts.GetAdd(res.semantic_equivalence);
	if(IsNumber(count)) count = (int)count + 1;
	else count = 1;
	
	for(const auto& rf : res.risk_flags) {
		Value& rcount = s.cumulative_risk_flags.GetAdd(rf);
		if(IsNumber(rcount)) rcount = (int)rcount + 1;
		else rcount = 1;
	}
	
	if(res.requires_human_review)
		s.unresolved_semantic_warnings++;
	s.last_updated = GetSysTime();
	
	StoreAsJsonFile(s, summary_path, true);
}

SemanticSummary SemanticIntegrityChecker::GetSummary()
{
	SemanticSummary s;
	if(FileExists(summary_path))
		LoadFromJsonFile(s, summary_path);
	return s;
}

} // namespace Upp