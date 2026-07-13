#include <VisualStateLogic/LogicCompare.h>

using namespace Upp;

struct TestHarness {
	int total = 0;
	int passed = 0;

	void Check(bool condition, const String& description)
	{
		total++;
		if(condition) {
			passed++;
			Cout() << "PASS: " << description << "\n";
		}
		else {
			Cout() << "FAIL: " << description << "\n";
		}
	}
};

static TestHarness harness;
#define CHECK(condition, description) harness.Check((condition), (description))

static bool LoadTexasHoldemGroundTruthRows(const String& session_dir,
                                           Vector<TexasHoldemGroundTruthRecord>& out)
{
	out.Clear();
	String path = AppendFileName(session_dir, "groundtruth.jsonl");
	if(!FileExists(path))
		return false;
	Vector<String> rows = Split(LoadFile(path), '\n', false);
	for(String row : rows) {
		row = TrimBoth(row);
		if(row.IsEmpty())
			continue;
		TexasHoldemGroundTruthRecord rec;
		if(!LoadFromJson(rec, row))
			return false;
		out.Add(pick(rec));
	}
	return true;
}

static bool CopyVideoOnlySession(const String& source, const String& target, int max_frames)
{
	if(DirectoryExists(target))
		DeleteFolderDeep(target);
	RealizeDirectory(AppendFileName(target, "frames"));
	String metadata_path = AppendFileName(source, "metadata.json");
	String metadata = LoadFile(metadata_path);
	Value metadata_value;
	try {
		metadata_value = ParseJSON(metadata);
	}
	catch(...) {
		return false;
	}
	if(!metadata_value.Is<ValueMap>())
		return false;
	ValueMap metadata_map = metadata_value;
	metadata_map.Set("frame_count", max_frames);
	metadata_map.Set("status", "complete");
	if(!SaveFile(AppendFileName(target, "metadata.json"), AsJSON(metadata_map)))
		return false;
	for(int i = 0; i < max_frames; i++) {
		String name = Format("%08d.png", i);
		String source_frame = AppendFileName(AppendFileName(source, "frames"), name);
		if(!FileExists(source_frame))
			return false;
		if(!FileCopy(source_frame, AppendFileName(AppendFileName(target, "frames"), name)))
			return false;
	}
	DeleteFile(AppendFileName(target, "groundtruth.jsonl"));
	return true;
}

static const TexasHoldemGroundTruthRecord *FindGroundTruthFrame(
	const Vector<TexasHoldemGroundTruthRecord>& ground_truth, int frame_id)
{
	for(const TexasHoldemGroundTruthRecord& record : ground_truth)
		if(record.frame_id == frame_id)
			return &record;
	return NULL;
}

static void RunVideoOnlyParityRegression()
{
	struct Fixture {
		const char *name;
		int max_frames;
	};
	static const Fixture fixtures[] = {
		{ "task0126_seedA", 12 },
		{ "task0126_seedB", 12 },
		{ "task0127_seedA", 12 },
		{ "task0127_seedB", 12 },
	};

	String form_path = "game/TexasHoldem/GameTable_PS_6p.form";
	int fixtures_checked = 0;
	int frames_checked = 0;
	int board_matches = 0;
	int board_mismatches = 0;
	int board_pending = 0;
	int confidence_records = 0;
	bool any_known_board = false;

	for(const Fixture& fixture : fixtures) {
		String source = AppendFileName("var/vsm_fixtures", fixture.name);
		CHECK(DirectoryExists(source), Format("fixture exists: %s", fixture.name));
		if(!DirectoryExists(source))
			continue;

		Vector<TexasHoldemGroundTruthRecord> ground_truth;
		bool ground_truth_ok = LoadTexasHoldemGroundTruthRows(source, ground_truth);
		CHECK(ground_truth_ok, Format("harness reads groundtruth separately for %s", fixture.name));
		if(!ground_truth_ok)
			continue;

		String scratch = AppendFileName(GetCurrentDirectory(),
		                                Format("var/vsm_fixtures/task0140_video_only_%s", fixture.name));
		bool copied = CopyVideoOnlySession(source, scratch, fixture.max_frames);
		CHECK(copied && !FileExists(AppendFileName(scratch, "groundtruth.jsonl")),
		      Format("parser scratch for %s has frames+metadata but no groundtruth.jsonl", fixture.name));
		if(!copied) {
			DeleteFolderDeep(scratch);
			continue;
		}

		Vector<VsmLogicCompareRecordOut> records;
		String error;
		bool derived = VsmDeriveSessionLogicStates(scratch, form_path, records, error, true);
		CHECK(derived,
		      Format("production-like derivation succeeds without groundtruth for %s: %s",
		             fixture.name, error));
		if(!derived) {
			DeleteFolderDeep(scratch);
			continue;
		}
		CHECK(records.GetCount() == fixture.max_frames,
		      Format("%s derived %d/%d video-only frames",
		             fixture.name, records.GetCount(), fixture.max_frames));

		for(const VsmLogicCompareRecordOut& record : records) {
			const TexasHoldemGroundTruthRecord *truth = FindGroundTruthFrame(ground_truth, record.frame_id);
			CHECK(truth != NULL,
			      Format("%s frame %d has harness groundtruth row",
			             fixture.name, record.frame_id));
			if(!truth)
				continue;
			CHECK(record.logic_state.frame_id == truth->frame_id,
			      Format("%s frame %d logic_state.frame_id matches GT",
			             fixture.name, record.frame_id));
			CHECK(record.logic_state.board_cards.GetCount() == 5,
			      Format("%s frame %d has five derived board-card slots",
			             fixture.name, record.frame_id));
			CHECK(truth->board_cards.GetCount() == 5,
			      Format("%s frame %d has five GT board-card slots",
			             fixture.name, record.frame_id));

			int slots = min(record.logic_state.board_cards.GetCount(), truth->board_cards.GetCount());
			for(int i = 0; i < slots; i++) {
				int got = record.logic_state.board_cards[i];
				int expected = truth->board_cards[i];
				if(got < 0) {
					board_pending++;
					continue;
				}
				any_known_board = true;
				if(got == expected)
					board_matches++;
				else
					board_mismatches++;
			}

			for(const VsmFieldConfidence& confidence : record.field_confidence) {
				confidence_records++;
				CHECK(confidence.confidence >= 0.0 && confidence.confidence <= 1.0,
				      Format("%s frame %d confidence %s in [0,1]",
				             fixture.name, record.frame_id, confidence.field));
			}
			frames_checked++;
		}

		fixtures_checked++;
		DeleteFolderDeep(scratch);
	}

	CHECK(fixtures_checked == __countof(fixtures),
	      Format("video-only parity checked %d/%d fixtures",
	             fixtures_checked, (int)__countof(fixtures)));
	CHECK(frames_checked == 48,
	      Format("video-only parity checked 48 frames (got %d)", frames_checked));
	CHECK(any_known_board,
	      "video-only parity observed at least one known derived board-card value");
	CHECK(board_mismatches == 0,
	      Format("known derived board cards have zero GT contradictions "
	             "(matches=%d pending=%d mismatches=%d)",
	             board_matches, board_pending, board_mismatches));
	CHECK(confidence_records > 0,
	      Format("production-like records include confidence diagnostics (got %d)",
	             confidence_records));
	Cout() << "M07-04 video-only parity summary: fixtures=" << fixtures_checked
	       << " frames=" << frames_checked
	       << " board_matches=" << board_matches
	       << " board_pending=" << board_pending
	       << " board_mismatches=" << board_mismatches
	       << " confidence_records=" << confidence_records << "\n";
}

GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT);
	Cout() << "=== VisualState M07 Video-Only Parity Tests (task 0140) ===\n";
	RunVideoOnlyParityRegression();
	Cout() << "\n=== Summary ===\n";
	Cout() << harness.passed << "/" << harness.total << " checks passed\n";
	if(harness.passed == harness.total) {
		Cout() << "ALL CHECKS PASSED\n";
		SetExitCode(0);
	}
	else {
		Cout() << (harness.total - harness.passed) << " CHECK(S) FAILED\n";
		SetExitCode(1);
	}
}
