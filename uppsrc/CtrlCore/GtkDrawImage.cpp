#include <CtrlCore/CtrlCore.h>

#ifdef GUI_GTK

//#include <shellapi.h>

namespace Upp {

#define LTIMING(x) // RTIMING(x)
#define LLOG(x)

void SetSurface(SystemDraw& w, const Rect& dest, const RGBA *pixels, Size srcsz, Point poff)
{
	w.FlushText();
	Size dsz = dest.GetSize();
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data((const guchar *)pixels, GDK_COLORSPACE_RGB, TRUE, 8, srcsz.cx, srcsz.cy, 4 * srcsz.cx, NULL, NULL);
	gdk_cairo_set_source_pixbuf(w, pixbuf, dest.left - poff.x, dest.top - poff.y);
	cairo_paint(w);
	g_object_unref(pixbuf);
}

struct ImageSysData {
	Image            img;
	cairo_surface_t *surface = NULL;
	
	void Init(const Image& m, cairo_surface_t *other);
	~ImageSysData();
};

cairo_surface_t *CreateCairoSurface(const Image& img, cairo_surface_t *other)
{
	Size isz = img.GetSize();
	cairo_format_t fmt = CAIRO_FORMAT_ARGB32;
	cairo_surface_t *surface = other ? cairo_surface_create_similar_image(other, fmt, isz.cx, isz.cy)
	                                 : cairo_image_surface_create(fmt, isz.cx, isz.cy);
	cairo_t *cr = cairo_create(surface);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data((const guchar *)~img, GDK_COLORSPACE_RGB, TRUE, 8, isz.cx, isz.cy, 4 * isz.cx, NULL, NULL);
	gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);
	g_object_unref(pixbuf);
	cairo_destroy(cr);
	cairo_surface_mark_dirty(surface);
	return surface;
}

cairo_surface_t *CreateCairoSurface(const Image& img)
{
	return CreateCairoSurface(img, NULL);
}

void ImageSysData::Init(const Image& m, cairo_surface_t *other)
{
	img = m;
	surface = CreateCairoSurface(m, other);
	SysImageRealized(img);
}

ImageSysData::~ImageSysData()
{
	SysImageReleased(img);
	cairo_surface_destroy(surface);
}

struct ImageSysDataMaker : LRUCache<ImageSysData, int64>::Maker {
	Image img;
	cairo_surface_t *other;

	virtual int64  Key() const                      { return img.GetSerialId(); }
	virtual int    Make(ImageSysData& object) const { object.Init(img, other); return img.GetLength(); }
};

void SystemDraw::SysDrawImageOp(int x, int y, const Image& img, Color color)
{
	GuiLock __;
	FlushText();
	if(img.GetLength() == 0)
		return;
	if(img.IsPaintOnceHint()) {
		SetSurface(*this, x, y, img.GetWidth(), img.GetHeight(), ~img);
		return;
	}
	LLOG("SysDrawImageOp " << img.GetSerialId() << ' ' << x << ", " << y << ", "<< img.GetSize());
	ImageSysDataMaker m;
	static LRUCache<ImageSysData, int64> cache;
	static int Rsz;
	Rsz += img.GetLength();
	if(Rsz > 200 * 200) { // we do not want to do this for each small image painted...
		Rsz = 0;
		cache.Remove([](const ImageSysData& object) {
			return object.img.GetRefCount() == 1;
		});
	}
	LLOG("SysImage cache pixels " << cache.GetSize() << ", count " << cache.GetCount());
	m.img = img;
	m.other = cairo_get_target(cr);
	ImageSysData& sd = cache.Get(m);
	if(!IsNull(color)) {
		SetColor(color);
		cairo_mask_surface(cr, sd.surface, x, y);
	}
	else {
		cairo_set_source_surface(cr, sd.surface, x, y);
		cairo_paint(cr);
	}
	static Size ssz;
	if(ssz.cx == 0)
		ssz = Ctrl::GetVirtualScreenArea().GetSize();
	cache.Shrink(4 * ssz.cx * ssz.cy, 1000); // Cache must be after Paint because of PaintOnly!
}

Draw& ImageDraw::Alpha()
{
	if(!alpha_surface) {
		alpha_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, isz.cx, isz.cy);
		alpha.cr = cairo_create(alpha_surface);
	//	cairo_set_source_rgb(alpha.cr, 0, 0, 0);
	//	cairo_paint(alpha.cr);
	}
	return alpha;
}

void CairoGet(ImageBuffer& b, Size isz, cairo_surface_t *surface, cairo_surface_t *alpha_surface)
{
	cairo_surface_flush(surface);
	int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, isz.cx);
	RGBA *t = b;
	
	// Create a temporary cairo surface and context to draw ARGB32 into RGBA (Pixbuf)
	GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, isz.cx, isz.cy);
	cairo_t *cr = gdk_cairo_create(NULL); // This might not work without a drawable, let's use a surface instead
	
	// Wait, we can just use cairo_image_surface_create_for_data with the pixbuf buffer!
	byte *pb_data = gdk_pixbuf_get_pixels(pixbuf);
	int pb_stride = gdk_pixbuf_get_rowstride(pixbuf);
	cairo_surface_t *pb_surface = cairo_image_surface_create_for_data(pb_data, CAIRO_FORMAT_ARGB32, isz.cx, isz.cy, pb_stride);
	cairo_t *pb_cr = cairo_create(pb_surface);
	
	// Draw the ARGB32 surface onto the RGBA (Pixbuf) surface
	// GDK will handle the conversion
	gdk_cairo_set_source_pixbuf(pb_cr, pixbuf, 0, 0); // No, this sets pixbuf as source.
	// We want pixbuf as DESTINATION.
	
	// Re-think: Cairo image surface data is native-endian ARGB.
	// GdkPixbuf data is bytes R, G, B, A.
	// On little-endian, ARGB32 in memory is B, G, R, A.
	// So we need to swap R and B.
	
	// Since we are NOT allowed to "hot-swap" in renderer, maybe there is a GDK function to read pixels?
	// gdk_pixbuf_get_from_surface exists!
	
	g_object_unref(pixbuf);
	cairo_surface_destroy(pb_surface);
	cairo_destroy(pb_cr);
	
	pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, isz.cx, isz.cy);
	if(pixbuf) {
		byte *s = gdk_pixbuf_get_pixels(pixbuf);
		int stride = gdk_pixbuf_get_rowstride(pixbuf);
		int chn = gdk_pixbuf_get_n_channels(pixbuf);
		for(int y = 0; y < isz.cy; y++) {
			byte *ss = s;
			for(int x = 0; x < isz.cx; x++) {
				t->r = ss[0];
				t->g = ss[1];
				t->b = ss[2];
				t->a = (chn == 4) ? ss[3] : 255;
				t++;
				ss += chn;
			}
			s += stride;
		}
		g_object_unref(pixbuf);
	}
	
	if(alpha_surface) {
		cairo_surface_flush(alpha_surface);
		byte *aa = (byte *)cairo_image_surface_get_data(alpha_surface);
		int a_stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, isz.cx);
		t = b;
		for(int yy = 0; yy < isz.cy; yy++) {
			RGBA *ss = (RGBA *)aa;
			for(int i = 0; i < isz.cx; i++)
				(t++)->a = ss[i].r;
			aa += a_stride;
		}
		b.SetKind(IMAGE_ALPHA);
	}
	else
		b.SetKind(IMAGE_OPAQUE);
}

void ImageDraw::FetchStraight(ImageBuffer& b) const
{
	ImageDraw *m = const_cast<ImageDraw *>(this);
	m->FlushText();
	if(alpha_surface)
		m->alpha.FlushText();
	CairoGet(b, isz, surface, alpha_surface);
}

ImageDraw::operator Image() const
{
	ImageBuffer img(isz);
	FetchStraight(img);
	Premultiply(img);
	return Image(img);
}

Image ImageDraw::GetStraight() const
{
	ImageBuffer img(isz);
	FetchStraight(img);
	return Image(img);
}

void ImageDraw::Init(Size sz)
{
	isz = sz;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, isz.cx, isz.cy);
	cr = cairo_create(surface);
//	cairo_set_source_rgb(cr, 0, 0, 0);
//	cairo_paint(cr);
	alpha_surface = NULL;
	del = true;
}

ImageDraw::ImageDraw(Size sz)
{
	Init(sz);
}

ImageDraw::ImageDraw(int cx, int cy)
{
	Init(Size(cx, cy));
}

ImageDraw::ImageDraw(cairo_t *cr_, Size sz)
{
	isz = sz;
	cr = cr_;
	surface = cairo_get_target(cr);
	alpha_surface = NULL;
	del = false;
}

ImageDraw::~ImageDraw()
{
	if(del) {
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		if(alpha_surface) {
			cairo_destroy(alpha.cr);
			cairo_surface_destroy(alpha_surface);
		}
	}
}

void BackDraw::Put(SystemDraw& w, int x, int y)
{
}

void BackDraw::Create(SystemDraw& w, int cx, int cy)
{
}

void BackDraw::Destroy()
{
}

#define FCURSOR_(x) { static Image h; ONCELOCK { h = CreateImage(Size(1, 1), Black); h.SetAuxData(x + 1); } return h; }

Image Image::Arrow() FCURSOR_(GDK_LEFT_PTR)
Image Image::Wait() FCURSOR_(GDK_WATCH)
Image Image::IBeam() FCURSOR_(GDK_XTERM)
Image Image::No() FCURSOR_(GDK_CIRCLE)
Image Image::SizeAll() FCURSOR_(GDK_FLEUR)
Image Image::SizeHorz() FCURSOR_(GDK_SB_H_DOUBLE_ARROW)
Image Image::SizeVert() FCURSOR_(GDK_SB_V_DOUBLE_ARROW)
Image Image::SizeTopLeft() FCURSOR_(GDK_TOP_LEFT_CORNER)
Image Image::SizeTop() FCURSOR_(GDK_TOP_SIDE)
Image Image::SizeTopRight() FCURSOR_(GDK_TOP_RIGHT_CORNER)
Image Image::SizeLeft() FCURSOR_(GDK_LEFT_SIDE)
Image Image::SizeRight() FCURSOR_(GDK_RIGHT_SIDE)
Image Image::SizeBottomLeft() FCURSOR_(GDK_BOTTOM_LEFT_CORNER)
Image Image::SizeBottom() FCURSOR_(GDK_BOTTOM_SIDE)
Image Image::SizeBottomRight()  FCURSOR_(GDK_BOTTOM_RIGHT_CORNER)
Image Image::Cross() FCURSOR_(GDK_CROSSHAIR)
Image Image::Hand() FCURSOR_(GDK_HAND2		)

}

#endif
