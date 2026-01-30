#include "StereoCalibrationSynthetic.h"

CONSOLE_APP_MAIN {
	Upp::StdLogSetup(Upp::LOG_COUT|Upp::LOG_FILE);
	Upp::String dataset_path = Upp::AppendFileName("upptst/StereoCalibrationSynthetic", "WmrCaseDataset.txt");
	Upp::String gt_path = Upp::AppendFileName("upptst/StereoCalibrationSynthetic", "WmrCaseGroundTruth.txt");
	Upp::WmrCaseData data;
	if (!Upp::GenerateWmrCase(data, 12345)) {
		Upp::Cout() << "Failed to generate WMR dataset.\n";
		Upp::SetExitCode(1);
		return;
	}
	Upp::SaveWmrCase(data, dataset_path);
	Upp::SaveWmrCase(data, gt_path);

	bool pass = Upp::RunWmrCaseTest(dataset_path);
	Upp::Cout() << "StereoCalibrationSynthetic: " << (pass ? "PASS" : "FAIL") << "\n";
	Upp::SetExitCode(pass ? 0 : 1);
}
