#include "Extensions.h"

NAMESPACE_UPP


void FloatImage::operator=(const FloatImage& i) {
	TODO
}

void FloatImage::Clear() {
	if (data) {
		free(data);
		sz = Size(0, 0);
		pitch = 0;
		channels = 0;
		size = 0;
	}
}

void FloatImage::FlipVert() {
	int half = sz.cy / 2;
	for(int y = 0; y < half; y++) {
		int yb = sz.cy - 1 - y;
		float* a = data + y * pitch;
		float* b = data + yb * pitch;
		for(int i = 0; i != pitch; i++)
			Swap(*a++, *b++);
	}
}

void FloatImage::Set(const Image& img) {
	Size sz = img.GetSize();
	Set(sz.cx, sz.cy, 4, sz.cx, (const byte*)img.Begin());
}

void FloatImage::Set(int w, int h, int stride, int src_pitch, const byte* src_data) {
	Clear();
	
	sz = Size(w,h);
	channels = stride;
	pitch = sz.cx * channels;
	size = pitch * sz.cy;
	
	ASSERT(channels >= 1 && channels <= 4);
	data = (float*)malloc(sizeof(float) * size);
	
	float* f = data;
	const byte* it = src_data;
	const byte* end = src_data + sz.cy * pitch;
	
	if (src_pitch == pitch) {
		int exp_size = (int)(end - it);
		ASSERT(exp_size == size);
		while (it != end) {
			double v = *it++ / 255.0;
			ASSERT(IsFin(v));
			*f++ = (float)v;
		}
	}
	else {
		for(int y = 0; y < sz.cy; y++) {
			const byte* it0 = it + y * src_pitch;
			const byte* end0 = it0 + pitch;
			while (it0 != end0)
				*f++ = (float)(*it0++ / 255.0);
		}
	}
}

void FloatImage::Set(Size sz, int channels) {
	Clear();
	
	this->sz = sz;
	this->channels = channels;
	pitch = sz.cx * channels;
	size = pitch * sz.cy;
	
	ASSERT(channels >= 1 && channels <= 4);
	data = (float*)malloc(sizeof(float) * size);
}





hash_t ByteImage::GetHashValue() const {
    CombineHash c;
    c.Put(size);
    c.Put(pitch);
    c.Put(channels);
    const byte* it = data;
    const byte* end = it + size;
    while (it != end)
        c.Put(*it++);
    return c;
}

void ByteImage::Visit(Vis& v) {
	v	("sz",sz)
		("pitch",pitch)
		("channels",channels)
		("size",size)
		("lock_channels",lock_channels);
	if (v.IsLoading()) {
		if (data) delete data;
		data = (byte*)malloc(sizeof(byte) * size);
	}
	v.VisitBinary("data", data, size);
}

void ByteImage::operator=(const ByteImage& i) {
	if (i.data)
		Set(i.sz.cx, i.sz.cy, i.channels, i.pitch, i.data);
	else
		Clear();
	faces <<= i.faces;
}

void ByteImage::FlipVert() {
	int half = sz.cy / 2;
	for(int y = 0; y < half; y++) {
		int yb = sz.cy - 1 - y;
		byte* a = data + y * pitch;
		byte* b = data + yb * pitch;
		for(int i = 0; i != pitch; i++)
			Swap(*a++, *b++);
	}
}

void ByteImage::Set(const Image& img) {
	Size sz = img.GetSize();
	Set(sz.cx, sz.cy, 4, sz.cx * 4, (const byte*)img.Begin());
}

void ByteImage::Set(int w, int h, int stride, int src_pitch, const byte* src_data) {
	Clear();
	sz = Size(w,h);
	channels = stride;
	pitch = sz.cx * channels;
	size = pitch * sz.cy;
	ASSERT(channels >= 1 && channels <= 4);
	
	data = (byte*)malloc(sizeof(byte) * size);
	
	if (src_data) {
		byte* b = data;
		const byte* it = src_data;
		//const byte* end = src_data + size;
		
		if (src_pitch == pitch) {
			#if 1
			memcpy(b, it, size);
			#else
			while (it != end)
				*b++ = *it++;
			#endif
		}
		else {
			for(int y = 0; y < sz.cy; y++) {
				const byte* it0 = it + y * src_pitch;
				const byte* end0 = it0 + pitch;
				while (it0 != end0)
					*b++ = *it0++;
			}
		}
	}
}

void ByteImage::Resize(Size sz) {
	ASSERT(channels > 0);
	if (channels)
		Set(sz, channels);
}

void ByteImage::Set(Size sz, int channels) {
	if (this->sz == sz && this->channels == channels)
		return;
	if (lock_channels && this->channels != 0 && this->channels != channels) {
		Panic("ByteImage: trying to change channel number, but the number is locked");
	}
	Clear();
	this->sz = sz;
	this->channels = channels;
	pitch = sz.cx * channels;
	size = pitch * sz.cy;
	ASSERT(channels >= 1 && channels <= 4);
	
	data = (byte*)malloc(sizeof(byte) * size);
}

void ByteImage::Clear() {
	if (data) {
		free(data);
		sz = Size(0, 0);
		pitch = 0;
		channels = 0;
		size = 0;
		data = 0;
	}
	faces.Clear();
}

void ByteImage::Zero(RGBA clr) {
	if (clr.r == 0 && clr.g == 0 && clr.b == 0 && clr.a == 0)
		Zero();
	else if (data) {
		ASSERT(channels >= 1 && channels <= 4);
		if (!((channels >= 1 && channels <= 4))) return;
		
		for(int y = 0; y < sz.cy; y++) {
			byte* dst = data + y * pitch;
			for (int x = 0; x < sz.cx; x++) {
				const byte* src = &clr.r;
				for(int i = 0; i < channels; i++)
					*dst++ = src[i];
			}
		}
	}
}

void ByteImage::Zero() {
	if (data)
		memset(data, 0, size);
}

void ByteImage::SwapRedBlue() {
	byte* it = data;
	byte* end = data + size;
	int stride = this->channels;
	if (stride >= 3 && (size % stride) == 0) {
		while (it != end) {
			byte s = it[0];
			it[0] = it[2];
			it[2] = s;
			it += stride;
		}
	}
}

void ByteImage::ToGrayscaleRGB() {
	if (!data || channels >= 3)
		return;
	int old_channels = this->channels;
	int old_pitch = this->pitch;
	this->channels = 3;
	pitch = sz.cx * 3;
	size = pitch * sz.cy;
	ASSERT(channels >= 1 && channels <= 4);
	
	byte* old_data = data;
	data = (byte*)malloc(sizeof(byte) * size);
	
	for(int y = 0; y < sz.cy; y++) {
		byte* from = old_data + y * old_pitch;
		byte* to = data + y * pitch;
		for(int x = 0; x < sz.cx; x++) {
			byte gray = *from;
			for(int i = 0; i < old_channels; i++)
				*to++ = *from++;
			for(int i = old_channels; i < 3; i++)
				*to++ = gray;
		}
	}
	
	free(old_data);
}

void ByteImage::SetSwapRedBlue(const ByteImage& i, bool add_alpha_ch) {
	int new_ch = i.GetChannels();
	if (new_ch == 3 && add_alpha_ch)
		new_ch = 4;
	
	if (this->sz != i.sz || this->channels != new_ch) {
		Clear();
		this->sz = i.sz;
		this->channels = new_ch;
		pitch = sz.cx * channels;
		size = pitch * sz.cy;
		ASSERT(channels >= 1 && channels <= 4);
		
		data = (byte*)malloc(sizeof(byte) * size);
	}
	
	if (channels == 4 && i.channels == 4) {
		ASSERT(pitch == i.pitch);
		const dword* src = (const dword*)i.data;
		union {
			dword* dst;
			byte* b;
		};
		dst = (dword*)data;
		dword* end = dst + size / 4;
		while (dst != end) {
			*dst = *src++;
			byte s = b[0];
			b[0] = b[2];
			b[2] = s;
			dst++;
		}
	}
	else if (channels == 4 && i.channels == 3) {
		ASSERT(pitch / 4 == i.pitch / 3);
		const byte* src = (const byte*)i.data;
		byte* dst = (byte*)data;
		byte* end = dst + size;
		ASSERT(size % 4 == 0);
		while (dst != end) {
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = 255;
			dst += 4;
			src += 3;
		}
	}
	else {
		TODO
	}
}

void ByteImage::Randomize() {
	byte* it = data;
	byte* end = data + size;
	while (it != end)
		*it++ = (byte)Random(256);
}

int ByteImage::GetPitch() const {
	return pitch;
}

int ByteImage::GetWidth() const {
	return sz.cx;
}

int ByteImage::GetHeight() const {
	return sz.cy;
}

int ByteImage::GetChannels() const {
	return channels;
}

int ByteImage::GetLength() const {
	return size;
}

Size ByteImage::GetSize() const {
	return sz;
}

byte* ByteImage::GetIter(int x, int y) {
	ASSERT(x >= 0 && y >= 0 && x < sz.cx && y < sz.cy);
	ASSERT(sz.cx && sz.cy && data && pitch && channels);
	return data + y * pitch + x * channels;
}

const byte* ByteImage::GetIter(int x, int y) const {
	ASSERT(x >= 0 && y >= 0 && x < sz.cx && y < sz.cy);
	ASSERT(sz.cx && sz.cy && data && pitch && channels);
	return data + y * pitch + x * channels;
}








void DataFromImage(const Image& img, Vector<byte>& out) {
	Size sz = img.GetSize();
	int bytes = sz.cx * sz.cy * sizeof(RGBA);
	out.SetCount(bytes);
	if (!bytes) return;

	const RGBA* src = img.Begin();
	memcpy(out.Begin(), src, bytes);
}

Image MirrorVertical(const Image& img) {
	if (img.IsEmpty())
		return img;
	const RGBA* src = img.Begin();
	Size sz = img.GetSize();
	
	ImageBuffer ib(sz);
	int line_sz = sz.cx * sizeof(RGBA);
	RGBA* dst = ib.Begin() + (int)((sz.cy-1) * sz.cx);
	
	for(int i = 0; i < sz.cy; i++) {
		memcpy(dst, src, line_sz);
		dst -= sz.cx;
		src += sz.cx;
	}
	
	return ib;
}









DescriptorImage::DescriptorImage() {
	
}

void DescriptorImage::AddDescriptor(float x, float y, float angle, void* descriptor) {
	ASSERT(resolution.cx > 0 && resolution.cy > 0);
	if (x >= 0 && x < resolution.cx &&
		y >= 0 && y < resolution.cy) {
		angle = fmodf(angle, 2*(float)M_PI);
		Descriptor32& d = descriptors.Add();
		d.x = x;
		d.y = y;
		d.angle = (byte)(angle / (2*M_PI) * 0x100);
		uint32* u = (uint32*)descriptor;
		for(int i = 0; i < 8; i++)
			d.u[i] = u[i];
	}
}

String DescriptorImage::ToString() const {
	String s;
	int i = 0;
	for(const Descriptor32& d : descriptors) {
		s << i++ << ": " << d.x << ", " << d.y << ", " << (int)d.angle << "\n";
	}
	return s;
}





int GetDescriptor8HammingDistance(const uint32* a, const uint32* b) {
	int dist = 0;
	for(int i = 0; i < 8; i++)
		dist += PopCount32(a[i] ^ b[i]);
	return dist;
}

namespace {

#pragma pack(push, 1)
struct DdsPixelFormat {
	uint32 size;
	uint32 flags;
	uint32 fourCC;
	uint32 rgbBitCount;
	uint32 rMask;
	uint32 gMask;
	uint32 bMask;
	uint32 aMask;
};

struct DdsHeader {
	uint32 size;
	uint32 flags;
	uint32 height;
	uint32 width;
	uint32 pitchOrLinearSize;
	uint32 depth;
	uint32 mipMapCount;
	uint32 reserved1[11];
	DdsPixelFormat ddspf;
	uint32 caps;
	uint32 caps2;
	uint32 caps3;
	uint32 caps4;
	uint32 reserved2;
};

struct DdsHeaderDX10 {
	uint32 dxgiFormat;
	uint32 resourceDimension;
	uint32 miscFlag;
	uint32 arraySize;
	uint32 miscFlags2;
};
#pragma pack(pop)

static const uint32 DDS_MAGIC = 0x20534444; // "DDS "
static const uint32 DDS_FOURCC = 0x00000004;
static const uint32 DDS_RGB = 0x00000040;
static const uint32 DDS_RGBA = 0x00000041;
static const uint32 DDS_CUBEMAP = 0x00000200;

#define DXGI_FORMAT_BC1_UNORM 71
#define DXGI_FORMAT_BC3_UNORM 77
#define DXGI_FORMAT_BC6H_UF16 95
#define DXGI_FORMAT_BC6H_SF16 96
#define DXGI_FORMAT_BC7_UNORM 98

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) | \
	((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24))
#endif

static int CountBits(uint32 v) {
	int n = 0;
	while (v) {
		n += (v & 1u) != 0;
		v >>= 1u;
	}
	return n;
}

static int CountTrailingZeros(uint32 v) {
	if (v == 0)
		return 32;
	int n = 0;
	while ((v & 1u) == 0) {
		n++;
		v >>= 1u;
	}
	return n;
}

static byte ExtractMasked(uint32 value, uint32 mask) {
	if (mask == 0)
		return 255;
	int shift = CountTrailingZeros(mask);
	int bits = CountBits(mask);
	uint32 v = (value & mask) >> shift;
	if (bits <= 0)
		return 0;
	if (bits >= 8)
		return (byte)(v >> (bits - 8));
	uint32 maxv = (1u << bits) - 1u;
	return (byte)((v * 255u) / maxv);
}

static float HalfToFloat(uint16 h) {
	uint32 s = (h >> 15) & 0x00000001u;
	uint32 e = (h >> 10) & 0x0000001fu;
	uint32 f = h & 0x000003ffu;
	if (e == 0) {
		if (f == 0)
			return s ? -0.0f : 0.0f;
		return (s ? -1.0f : 1.0f) * ldexpf((float)f, -24);
	}
	if (e == 31) {
		if (f == 0)
			return s ? -INFINITY : INFINITY;
		return NAN;
	}
	return (s ? -1.0f : 1.0f) * ldexpf((float)(f | 0x400), (int)e - 25);
}

static byte FloatToByte(float v) {
	if (v < 0.0f)
		v = 0.0f;
	else if (v > 1.0f)
		v = 1.0f;
	return (byte)(v * 255.0f + 0.5f);
}

static RGBA MakeRGBA(byte r, byte g, byte b, byte a) {
	RGBA c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return c;
}

static void Decode565(uint16 v, byte& r, byte& g, byte& b) {
	r = (byte)(((v >> 11) & 0x1f) * 255 / 31);
	g = (byte)(((v >> 5) & 0x3f) * 255 / 63);
	b = (byte)((v & 0x1f) * 255 / 31);
}

static void DecodeBc1Block(const byte* src, RGBA colors[16]) {
	uint16 c0 = (uint16)(src[0] | (src[1] << 8));
	uint16 c1 = (uint16)(src[2] | (src[3] << 8));
	byte r0, g0, b0, r1, g1, b1;
	Decode565(c0, r0, g0, b0);
	Decode565(c1, r1, g1, b1);

	RGBA palette[4];
	palette[0] = MakeRGBA(r0, g0, b0, 255);
	palette[1] = MakeRGBA(r1, g1, b1, 255);
	if (c0 > c1) {
		palette[2] = MakeRGBA((byte)((2 * r0 + r1) / 3), (byte)((2 * g0 + g1) / 3), (byte)((2 * b0 + b1) / 3), 255);
		palette[3] = MakeRGBA((byte)((r0 + 2 * r1) / 3), (byte)((g0 + 2 * g1) / 3), (byte)((b0 + 2 * b1) / 3), 255);
	}
	else {
		palette[2] = MakeRGBA((byte)((r0 + r1) / 2), (byte)((g0 + g1) / 2), (byte)((b0 + b1) / 2), 255);
		palette[3] = MakeRGBA(0, 0, 0, 0);
	}

	uint32 idx = (uint32)(src[4] | (src[5] << 8) | (src[6] << 16) | (src[7] << 24));
	for (int i = 0; i < 16; i++) {
		int code = (int)(idx & 0x3u);
		colors[i] = palette[code];
		idx >>= 2;
	}
}

static void DecodeBc3Block(const byte* src, RGBA colors[16]) {
	byte a0 = src[0];
	byte a1 = src[1];
	byte alpha[8];
	alpha[0] = a0;
	alpha[1] = a1;
	if (a0 > a1) {
		alpha[2] = (byte)((6 * a0 + 1 * a1) / 7);
		alpha[3] = (byte)((5 * a0 + 2 * a1) / 7);
		alpha[4] = (byte)((4 * a0 + 3 * a1) / 7);
		alpha[5] = (byte)((3 * a0 + 4 * a1) / 7);
		alpha[6] = (byte)((2 * a0 + 5 * a1) / 7);
		alpha[7] = (byte)((1 * a0 + 6 * a1) / 7);
	}
	else {
		alpha[2] = (byte)((4 * a0 + 1 * a1) / 5);
		alpha[3] = (byte)((3 * a0 + 2 * a1) / 5);
		alpha[4] = (byte)((2 * a0 + 3 * a1) / 5);
		alpha[5] = (byte)((1 * a0 + 4 * a1) / 5);
		alpha[6] = 0;
		alpha[7] = 255;
	}

	uint64 abits = 0;
	for (int i = 0; i < 6; i++)
		abits |= (uint64)src[2 + i] << (8 * i);

	RGBA block_colors[16];
	DecodeBc1Block(src + 8, block_colors);
	for (int i = 0; i < 16; i++) {
		int aidx = (int)(abits & 0x7u);
		block_colors[i].a = alpha[aidx];
		abits >>= 3;
		colors[i] = block_colors[i];
	}
}

static bool LoadDdsImagesInternal(const String& path, Vector<Image>& out) {
	out.Clear();

	String data = LoadFile(path);
	if (data.GetCount() < (int)(4 + sizeof(DdsHeader)))
		return false;

	const byte* ptr = (const byte*)data.Begin();
	uint32 magic;
	memcpy(&magic, ptr, sizeof(magic));
	if (magic != DDS_MAGIC)
		return false;

	DdsHeader hdr;
	memcpy(&hdr, ptr + 4, sizeof(hdr));
	if (hdr.size != 124 || hdr.ddspf.size != 32)
		return false;

	bool has_dx10 = false;
	if ((hdr.ddspf.flags & DDS_FOURCC) && hdr.ddspf.fourCC == MAKEFOURCC('D','X','1','0'))
		has_dx10 = true;

	int width = (int)hdr.width;
	int height = (int)hdr.height;
	if (width <= 0 || height <= 0)
		return false;

	int mip_count = hdr.mipMapCount ? (int)hdr.mipMapCount : 1;
	bool is_cubemap = (hdr.caps2 & DDS_CUBEMAP) != 0;
	int face_count = is_cubemap ? 6 : 1;

	DdsHeaderDX10 dx10;
	if (has_dx10) {
		if ((size_t)data.GetCount() < 4 + sizeof(DdsHeader) + sizeof(DdsHeaderDX10))
			return false;
		memcpy(&dx10, ptr + 4 + sizeof(DdsHeader), sizeof(DdsHeaderDX10));
	}

	size_t offset = 4 + sizeof(DdsHeader) + (has_dx10 ? sizeof(DdsHeaderDX10) : 0);
	if (offset >= (size_t)data.GetCount())
		return false;

	int bytes_per_pixel = 0;
	int block_bytes = 0;
	bool is_float = false;
	bool is_half = false;
	bool is_bc1 = false;
	bool is_bc3 = false;
	bool is_bc6h = false;
	bool is_bc7 = false;
	if (hdr.ddspf.flags & DDS_FOURCC) {
		if (hdr.ddspf.fourCC == 0x74) { // D3DFMT_A32B32G32R32F
			is_float = true;
			bytes_per_pixel = 16;
		}
		else if (hdr.ddspf.fourCC == 0x71) { // D3DFMT_A16B16G16R16F
			is_half = true;
			bytes_per_pixel = 8;
		}
		else if (hdr.ddspf.fourCC == MAKEFOURCC('D','X','T','1')) {
			is_bc1 = true;
			block_bytes = 8;
		}
		else if (hdr.ddspf.fourCC == MAKEFOURCC('D','X','T','5')) {
			is_bc3 = true;
			block_bytes = 16;
		}
		else {
			return false;
		}
	}
	else if (hdr.ddspf.flags & (DDS_RGB | DDS_RGBA)) {
		if (hdr.ddspf.rgbBitCount == 32)
			bytes_per_pixel = 4;
		else
			return false;
	}
	else {
		return false;
	}

	if (has_dx10) {
		if (dx10.dxgiFormat == DXGI_FORMAT_BC1_UNORM) {
			is_bc1 = true;
			block_bytes = 8;
		}
		else if (dx10.dxgiFormat == DXGI_FORMAT_BC3_UNORM) {
			is_bc3 = true;
			block_bytes = 16;
		}
		else if (dx10.dxgiFormat == DXGI_FORMAT_BC6H_UF16 || dx10.dxgiFormat == DXGI_FORMAT_BC6H_SF16) {
			is_bc6h = true;
			block_bytes = 16;
		}
		else if (dx10.dxgiFormat == DXGI_FORMAT_BC7_UNORM) {
			is_bc7 = true;
			block_bytes = 16;
		}
	}

	if (is_bc6h || is_bc7) {
		#ifdef flagDEBUG
		ASSERT_(0, "DDS BC6H/BC7 decompression not implemented");
		#endif
		return false;
	}

	size_t face_size = 0;
	int mip_w = width;
	int mip_h = height;
	for (int mip = 0; mip < mip_count; mip++) {
		if (is_bc1 || is_bc3) {
			int bw = (mip_w + 3) / 4;
			int bh = (mip_h + 3) / 4;
			face_size += (size_t)bw * (size_t)bh * (size_t)block_bytes;
		}
		else {
			face_size += (size_t)mip_w * (size_t)mip_h * (size_t)bytes_per_pixel;
		}
		mip_w = max(1, mip_w >> 1);
		mip_h = max(1, mip_h >> 1);
	}

	size_t required = offset + face_size * (size_t)face_count;
	if (required > (size_t)data.GetCount())
		return false;

	for (int face = 0; face < face_count; face++) {
		const byte* face_ptr = ptr + offset + face_size * (size_t)face;
		ImageBuffer ib(width, height);
		if (is_bc1 || is_bc3) {
			int bw = (width + 3) / 4;
			int bh = (height + 3) / 4;
			const byte* p = face_ptr;
			for (int by = 0; by < bh; by++) {
				for (int bx = 0; bx < bw; bx++) {
					RGBA block[16];
					if (is_bc1)
						DecodeBc1Block(p, block);
					else
						DecodeBc3Block(p, block);
					for (int py = 0; py < 4; py++) {
						int y = by * 4 + py;
						if (y >= height)
							continue;
						RGBA* dst = ib[y];
						for (int px = 0; px < 4; px++) {
							int x = bx * 4 + px;
							if (x >= width)
								continue;
							dst[x] = block[py * 4 + px];
						}
					}
					p += block_bytes;
				}
			}
		}
		else if (is_float || is_half) {
			const byte* p = face_ptr;
			for (int y = 0; y < height; y++) {
				RGBA* dst = ib[y];
				for (int x = 0; x < width; x++) {
					float r, g, b, a;
					if (is_half) {
						const uint16* hp = (const uint16*)p;
						r = HalfToFloat(hp[0]);
						g = HalfToFloat(hp[1]);
						b = HalfToFloat(hp[2]);
						a = HalfToFloat(hp[3]);
					}
					else {
						const float* fp = (const float*)p;
						r = fp[0];
						g = fp[1];
						b = fp[2];
						a = fp[3];
					}
					dst[x] = MakeRGBA(FloatToByte(r), FloatToByte(g), FloatToByte(b), FloatToByte(a));
					p += bytes_per_pixel;
				}
			}
		}
		else {
			const uint32* p = (const uint32*)face_ptr;
			for (int y = 0; y < height; y++) {
				RGBA* dst = ib[y];
				for (int x = 0; x < width; x++) {
					uint32 v = *p++;
					byte r = ExtractMasked(v, hdr.ddspf.rMask);
					byte g = ExtractMasked(v, hdr.ddspf.gMask);
					byte b = ExtractMasked(v, hdr.ddspf.bMask);
					byte a = ExtractMasked(v, hdr.ddspf.aMask);
					dst[x] = MakeRGBA(r, g, b, a);
				}
			}
		}
		out.Add(ib);
	}

	return true;
}

}

bool LoadDdsImage(const String& path, Image& out) {
	Vector<Image> faces;
	if (!LoadDdsImagesInternal(path, faces))
		return false;
	if (faces.GetCount() != 1)
		return false;
	out = faces[0];
	return true;
}

bool LoadDdsImages(const String& path, Vector<Image>& out) {
	return LoadDdsImagesInternal(path, out);
}


END_UPP_NAMESPACE
