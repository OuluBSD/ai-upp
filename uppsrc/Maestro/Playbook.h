
#ifndef _Maestro_Playbook_h_
#define _Maestro_Playbook_h_

struct PlaybookAppliesTo : Moveable<PlaybookAppliesTo> {
	String source_language;
	String target_language;

	void Jsonize(JsonIO& jio) {
		jio("source_language", source_language)("target_language", target_language);
	}
};

struct PlaybookCheckpointPolicy : Moveable<PlaybookCheckpointPolicy> {
	int  after_files = 5;
	bool on_semantic_loss = true;

	void Jsonize(JsonIO& jio) {
		jio("after_files", after_files)("on_semantic_loss", on_semantic_loss);
	}
};

struct PlaybookValidationPolicy : Moveable<PlaybookValidationPolicy> {
	String mode = "full"; // "vectors_only", "full", "minimal"
	bool   require_behavior_envelope = true;

	void Jsonize(JsonIO& jio) {
		jio("mode", mode)("require_behavior_envelope", require_behavior_envelope);
	}
};

struct Playbook : Moveable<Playbook> {
	String                   id;
	String                   title;
	String                   version;
	PlaybookAppliesTo        applies_to;
	String                   intent;
	Vector<String>           principles;
	Vector<String>           required_losses;
	ValueMap                 forbidden_constructs;
	Vector<String>           preferred_patterns;
	PlaybookCheckpointPolicy checkpoint_policy;
	PlaybookValidationPolicy validation_policy;
	ValueMap                 glossary;
	ValueMap                 constraints;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("title", title)("version", version)("applies_to", applies_to)
		   ("intent", intent)("principles", principles)("required_losses", required_losses)
		   ("forbidden_constructs", forbidden_constructs)("preferred_patterns", preferred_patterns)
		   ("checkpoint_policy", checkpoint_policy)("validation_policy", validation_policy)
		   ("glossary", glossary)("constraints", constraints);
	}
	
	String GetVersionHash() const;
};

struct PlaybookBinding : Moveable<PlaybookBinding> {
	String playbook_id;
	String playbook_version;
	String version_hash;
	int64  bound_at;
	String bound_by;

	void Jsonize(JsonIO& jio) {
		jio("playbook_id", playbook_id)("playbook_version", playbook_version)
		   ("version_hash", version_hash)("bound_at", bound_at)("bound_by", bound_by);
	}
};

struct PlaybookOverride : Moveable<PlaybookOverride> {
	String task_id;
	String violation_type;
	String reason;
	int64  timestamp;
	String overridden_by;

	void Jsonize(JsonIO& jio) {
		jio("task_id", task_id)("violation_type", violation_type)("reason", reason)
		   ("timestamp", timestamp)("overridden_by", overridden_by);
	}
};

class PlaybookManager {
	String base_path;
	String playbooks_dir;
	String binding_file;
	String overrides_file;

public:
	PlaybookManager(const String& maestro_root = ".");
	
	Array<Playbook>  ListPlaybooks();
	Playbook*        LoadPlaybook(const String& id);
	bool             SavePlaybook(const Playbook& pb);
	
	bool             BindPlaybook(const String& id);
	PlaybookBinding  GetActiveBinding();
	
	bool             RecordOverride(const String& task_id, const String& violation_type, const String& reason);
	Array<PlaybookOverride> GetOverrides();
	
	static bool      Validate(const Playbook& pb);
};

#endif

