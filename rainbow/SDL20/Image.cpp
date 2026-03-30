#include <CtrlCore/CtrlCore.h>

#ifdef GUI_SDL20

NAMESPACE_UPP

#define LTIMING(x) // RTIMING(x)

void SetSurface(SystemDraw& w, int x, int y, int cx, int cy, const RGBA *pixels)
{
	GuiLock __;
	// Empty as CanSetSurface is false
}

void SetSurface(SystemDraw& w, const Rect& dest, const RGBA *pixels, Size psz, Point poff)
{
	GuiLock __;
	// Empty as CanSetSurface is false
}

#define IMAGECLASS SDL20FBImg
#define IMAGEFILE <Framebuffer/FB.iml>
#include <Draw/iml_source.h>

Image Image::Arrow() { return SDL20FBImg::arrow(); }
Image Image::Wait() { return SDL20FBImg::wait(); }
Image Image::IBeam() { return SDL20FBImg::ibeam(); }
Image Image::No() { return SDL20FBImg::no(); }
Image Image::SizeAll() { return SDL20FBImg::sizeall(); }
Image Image::SizeHorz() { return SDL20FBImg::sizehorz(); }
Image Image::SizeVert() { return SDL20FBImg::sizevert(); }
Image Image::SizeTopLeft() { return SDL20FBImg::sizetopleft(); }
Image Image::SizeTop() { return SDL20FBImg::sizetop(); }
Image Image::SizeTopRight() { return SDL20FBImg::sizetopright(); }
Image Image::SizeLeft() { return SDL20FBImg::sizeleft(); }
Image Image::SizeRight() { return SDL20FBImg::sizeright(); }
Image Image::SizeBottomLeft() { return SDL20FBImg::sizebottomleft(); }
Image Image::SizeBottom() { return SDL20FBImg::sizebottom(); }
Image Image::SizeBottomRight() { return SDL20FBImg::sizebottomright(); }
Image Image::Hand() { return SDL20FBImg::hand(); }

END_UPP_NAMESPACE

#endif
