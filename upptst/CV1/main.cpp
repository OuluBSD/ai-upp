#include <ComputerVision/ComputerVision.h>

using namespace Upp;

static void FillSynthetic(ByteMat& img) {
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			img.data[y * img.cols + x] = (byte)((x * 17 + y * 29 + (x * y) % 31) & 0xFF);
		}
	}
}

static void CopyPatch(const ByteMat& src, ByteMat& dst, int sx, int sy) {
	for (int y = 0; y < dst.rows; y++) {
		for (int x = 0; x < dst.cols; x++) {
			dst.data[y * dst.cols + x] = src.data[(sy + y) * src.cols + (sx + x)];
		}
	}
}

CONSOLE_APP_MAIN {
	StdLogSetup(LOG_COUT | LOG_FILE);

	const int expected_x = 19;
	const int expected_y = 11;

	ByteMat image(64, 48, 1);
	FillSynthetic(image);

	ByteMat templ(9, 7, 1);
	CopyPatch(image, templ, expected_x, expected_y);

	TemplateMatchMethod methods[] = {
		TM_CCOEFF,
		TM_CCOEFF_NORMED,
		TM_CCORR,
		TM_CCORR_NORMED,
		TM_SQDIFF,
		TM_SQDIFF_NORMED,
	};
	const char* names[] = {
		"TM_CCOEFF",
		"TM_CCOEFF_NORMED",
		"TM_CCORR",
		"TM_CCORR_NORMED",
		"TM_SQDIFF",
		"TM_SQDIFF_NORMED",
	};

	for (int i = 0; i < 6; i++) {
		FloatMat result;
		MatchTemplate(image, templ, result, methods[i]);

		double min_v = 0.0;
		double max_v = 0.0;
		Point min_p;
		Point max_p;
		MinMaxLoc(result, &min_v, &max_v, &min_p, &max_p);

		Point best = (methods[i] == TM_SQDIFF || methods[i] == TM_SQDIFF_NORMED) ? min_p : max_p;
		bool ok = (best.x == expected_x && best.y == expected_y);
		bool strict = methods[i] != TM_CCORR;

		Cout() << names[i]
			<< ": best=" << best
			<< " min=" << min_v
			<< " max=" << max_v
			<< " status=" << (ok ? "OK" : (strict ? "FAIL" : "EXPECTED_VARIANCE")) << "\n";

		if (strict && !ok)
			SetExitCode(1);
	}
}
