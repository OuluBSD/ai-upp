#include "VisualStateModel.h"

namespace Upp {

bool VsmShaderEvidenceAdapter::Slice(const VsmImageBuffer& source,
	                                   const VsmPackedWindow& window,
	                                   VsmImageBuffer& output, String& error)
{
	if(window.source_x < 0 || window.source_y < 0 || window.width <= 0 || window.height <= 0 ||
	   window.source_x + window.width > source.width || window.source_y + window.height > source.height) {
		error = "packed window is outside source";
		return false;
	}
	output.Create(window.width, window.height, source.channels);
	for(int y = 0; y < window.height; y++)
		for(int x = 0; x < window.width; x++)
			for(int c = 0; c < source.channels; c++)
				output.Set(x, y, source.Get(window.source_x + x, window.source_y + y, c), c);
	return true;
}

bool VsmShaderEvidenceAdapter::ProcessPacked(const VsmImageBuffer& packed,
	                                           const Vector<VsmPackedWindow>& windows,
	                                           Vector<VsmShaderWindowEvidence>& output,
	                                           String& error) const
{
	return ProcessSource(packed, windows, output, error);
}

bool VsmShaderEvidenceAdapter::ProcessSource(const VsmImageBuffer& source,
	                                           const Vector<VsmPackedWindow>& windows,
	                                           Vector<VsmShaderWindowEvidence>& output,
	                                           String& error) const
{
	output.Clear();
	if(source.IsEmpty()) { error = "source frame is empty"; return false; }
	if(windows.IsEmpty()) { error = "no source windows"; return false; }
	if(!service_) { error = "shader recognition service is not set"; return false; }
	for(const VsmPackedWindow& window : windows) {
		VsmShaderWindowEvidence& result = output.Add();
		result.id = window.id;
		result.timestamp_ms = window.timestamp_ms;
		VsmImageBuffer local;
		if(!Slice(source, window, local, result.error)) {
			error = result.error;
			return false;
		}
		if(!service_->Process(local, result.evidence, result.runs, result.error)) {
			error = result.error;
			return false;
		}
	}
	return true;
}

} // namespace Upp
