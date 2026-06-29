#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

void VsmRegionFingerprint::Jsonize(JsonIO& json)
{
	json("hash", hash)
	    ("file", file);
}

void VsmChangedRect::Jsonize(JsonIO& json)
{
	json("x", x)("y", y)("w", w)("h", h)
	    ("score", score);
}

void VsmChangeEvent::Jsonize(JsonIO& json)
{
	json("frame",   frame)
	    ("ts",      ts)
	    ("regions", regions);
}

void VsmRegionNode::Jsonize(JsonIO& json)
{
	json("region_id",   id)
	    ("parent_id",   parent_id)
	    ("x",           x)
	    ("y",           y)
	    ("w",           w)
	    ("h",           h)
	    ("action",      action)
	    ("fingerprint", fingerprint)
	    ("label",       label)
	    ("frame",       frame)
	    ("ts",          ts);
}

void VsmFrameRef::Jsonize(JsonIO& json)
{
	json("frame",      frame)
	    ("ts",         ts)
	    ("image_file", image_file);
}

void VsmOcrObservation::Jsonize(JsonIO& json)
{
	json("frame",         frame)
	    ("ts",            ts)
	    ("region_id",     region_id)
	    ("trigger_frame", trigger_frame)
	    ("text",          text)
	    ("confidence",    confidence)
	    ("engine",        engine)
	    ("crop_file",     crop_file);
}

void VsmTemplateObservation::Jsonize(JsonIO& json)
{
	json("frame",         frame)
	    ("ts",            ts)
	    ("region_id",     region_id)
	    ("template_name", template_name)
	    ("score",         score)
	    ("mx",            mx)
	    ("my",            my)
	    ("mw",            mw)
	    ("mh",            mh)
	    ("crop_file",     crop_file);
}

void VsmModelStateRef::Jsonize(JsonIO& json)
{
	json("frame",      frame)
	    ("ts",         ts)
	    ("state_json", state_json);
}

void VsmDivergence::Jsonize(JsonIO& json)
{
	json("frame",         frame)
	    ("ts",            ts)
	    ("severity",      severity)
	    ("message",       message)
	    ("region_id",     region_id)
	    ("expected_json", expected_json)
	    ("observed_json", observed_json);
}

} // namespace Upp
