#include "CtrlLib.h"

NAMESPACE_UPP

#ifndef LIBTOPSIDE

#define IMAGECLASS Images
#define IMAGEFILE <LocalCtrl/Images.iml>
#include IML_SOURCE
#undef IMAGECLASS
#undef IMAGEFILE

#define IMAGECLASS DbgImages
#define IMAGEFILE <LocalCtrl/DbgImages.iml>
#include IML_SOURCE
#undef IMAGECLASS
#undef IMAGEFILE

#endif

END_UPP_NAMESPACE
