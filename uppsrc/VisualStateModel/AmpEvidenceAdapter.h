#ifndef _VisualStateModel_AmpEvidenceAdapter_h_
#define _VisualStateModel_AmpEvidenceAdapter_h_

namespace Upp {

struct VsmAmpEvidenceAdapter {
	static bool Process(const VsmImageBuffer& frame,
	                   const VsmImageBuffer& crop_map,
	                   const VsmShaderTemplateManifest& manifest,
	                   int threshold,
	                   const String& backend,
	                   const String& device_path,
	                   VsmShaderEvidence& evidence,
	                   String& error);
};

}

#endif
