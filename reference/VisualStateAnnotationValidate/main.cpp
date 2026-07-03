#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	Cout() << "=== VisualStateModel Annotation Validator Demo ===\n\n";

	// Test frame dimensions
	const int FRAME_WIDTH = 640;
	const int FRAME_HEIGHT = 480;

	Cout() << "Frame dimensions: " << FRAME_WIDTH << "x" << FRAME_HEIGHT << "\n\n";

	// --- Create valid annotation layer ---
	Cout() << "Creating valid annotation layer...\n";
	VsmAnnotationLayer valid_layer;
	valid_layer.session_id = "test-valid";
	valid_layer.schema = 1;

	{
		VsmRegionAnnotation& root = valid_layer.annotations.Add();
		root.id = "ann-root";
		root.name = "Root Annotation";
		root.parent_id = "";
		root.x = 10;
		root.y = 10;
		root.w = 200;
		root.h = 150;
		root.relative_to_parent = false;

		VsmRegionAnnotation& child = valid_layer.annotations.Add();
		child.id = "ann-child";
		child.name = "Child Annotation";
		child.parent_id = "ann-root";
		child.x = 20;
		child.y = 20;
		child.w = 100;
		child.h = 80;
		child.relative_to_parent = true;
	}
	Cout() << "  Created with 2 annotations (root + child)\n";

	// Validate hierarchy
	auto hier_errs = valid_layer.Validate();
	Cout() << "  Hierarchy validation: " << hier_errs.GetCount() << " issues\n";
	for(const auto& err : hier_errs) {
		Cout() << "    [" << err.annotation_id << "] " << err.message << "\n";
	}

	// Validate bounds
	auto bounds_errs = valid_layer.ValidateBounds(FRAME_WIDTH, FRAME_HEIGHT);
	Cout() << "  Bounds validation: " << bounds_errs.GetCount() << " issues\n";
	for(const auto& err : bounds_errs) {
		Cout() << "    [" << err.annotation_id << "] " << err.message << "\n";
	}

	if(hier_errs.GetCount() != 0)
		{ Fail("Valid layer should have 0 hierarchy issues"); return; }
	if(bounds_errs.GetCount() != 0)
		{ Fail("Valid layer should have 0 bounds issues"); return; }

	Cout() << "  OK: Valid layer passed both checks\n\n";

	// --- Create broken annotation layer ---
	Cout() << "Creating broken annotation layer (cycle + out-of-bounds rect)...\n";
	VsmAnnotationLayer broken_layer;
	broken_layer.session_id = "test-broken";
	broken_layer.schema = 1;

	{
		// Annotation A that will point to C (creating a cycle: A -> C -> B -> A)
		VsmRegionAnnotation& ann_a = broken_layer.annotations.Add();
		ann_a.id = "ann-a";
		ann_a.name = "Annotation A";
		ann_a.parent_id = "ann-c";  // Points to C, which will point back to A
		ann_a.x = 50;
		ann_a.y = 50;
		ann_a.w = 100;
		ann_a.h = 100;
		ann_a.relative_to_parent = false;

		// Annotation B that points to A
		VsmRegionAnnotation& ann_b = broken_layer.annotations.Add();
		ann_b.id = "ann-b";
		ann_b.name = "Annotation B";
		ann_b.parent_id = "ann-a";  // Points to A
		ann_b.x = 60;
		ann_b.y = 60;
		ann_b.w = 80;
		ann_b.h = 80;
		ann_b.relative_to_parent = true;

		// Annotation C that points to B, completing the cycle: A -> C -> B -> A
		VsmRegionAnnotation& ann_c = broken_layer.annotations.Add();
		ann_c.id = "ann-c";
		ann_c.name = "Annotation C";
		ann_c.parent_id = "ann-b";  // Points to B, completing cycle
		ann_c.x = 700;  // Out of bounds (FRAME_WIDTH=640)
		ann_c.y = 500;  // Out of bounds (FRAME_HEIGHT=480)
		ann_c.w = 100;
		ann_c.h = 100;
		ann_c.relative_to_parent = false;
	}
	Cout() << "  Created with 3 annotations (A -> C -> B -> A cycle + C out of bounds)\n";

	// Validate hierarchy
	hier_errs = broken_layer.Validate();
	Cout() << "  Hierarchy validation: " << hier_errs.GetCount() << " issues\n";
	for(const auto& err : hier_errs) {
		Cout() << "    [" << err.annotation_id << "] " << err.message << "\n";
	}

	// Validate bounds
	bounds_errs = broken_layer.ValidateBounds(FRAME_WIDTH, FRAME_HEIGHT);
	Cout() << "  Bounds validation: " << bounds_errs.GetCount() << " issues\n";
	for(const auto& err : bounds_errs) {
		Cout() << "    [" << err.annotation_id << "] " << err.message << "\n";
	}

	bool has_hier_issue = hier_errs.GetCount() > 0;
	bool has_bounds_issue = bounds_errs.GetCount() > 0;

	if(!has_hier_issue)
		{ Fail("Broken layer should have at least 1 hierarchy issue"); return; }
	if(!has_bounds_issue)
		{ Fail("Broken layer should have at least 1 bounds issue"); return; }

	Cout() << "  OK: Broken layer detected both cycle and bounds issues\n\n";

	Cout() << "=== All validation checks passed ===\n";
}
