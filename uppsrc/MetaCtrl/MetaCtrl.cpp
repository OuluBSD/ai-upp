#include "MetaCtrl.h"

#define IMAGECLASS MetaImgs
#define IMAGEFILE <MetaCtrl/Images.iml>
#include <Draw/iml_source.h>

NAMESPACE_UPP


MetaNodeExt& MetaExtCtrl::GetExt() {return *ext;}
MetaNode& MetaExtCtrl::GetNode() {ASSERT(ext); return ext->node;}
const MetaNode& MetaExtCtrl::GetNode() const {return ext->node;}
String MetaExtCtrl::GetFilePath() const {
	if (owner)
		return owner->GetFilePath();
	else
		return String();
}

VfsPath MetaExtCtrl::GetCursorPath() const {
	if (ext)
		return ext->node.GetPath();
	if (owner)
		return StrVfs(owner->GetFilePath());
	ASSERT_(0, "invalid cursor path");
	return VfsPath();
}

END_UPP_NAMESPACE
