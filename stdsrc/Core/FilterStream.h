#pragma once
#ifndef _Core_FilterStream_h_
#define _Core_FilterStream_h_

#include "Core.h"
#include <vector>
#include <functional>

class InFilterStream : public Stream {
public:
	virtual   bool  IsOpen() const override;

protected:
	virtual   int   _Term() override;
	virtual   int   _Get() override;
	virtual   dword _Get(void *data, dword size) override;

	std::vector<byte> buffer;
	bool         eof;
	int          buffersize = 4096;
	std::vector<int>  inbuffer;
	byte        *t; // target pointer for block _Get
	dword        todo; // target count

	void   Init();
	void   Fetch();
	void   SetRd();

private:
	void   SetSize(int64 size)  { NEVER(); } // removed
	int64  GetSize() const      { NEVER(); return 0; }

public:
	Stream                      *in;
	std::function<void(const void *, int)>     Filter;
	std::function<void()>                       FilterEof;
	std::function<void()>                       End;
	std::function<void()>                       More;
	void                         Out(const void *ptr, int size);
	
	std::function<void()>                      WhenOut;
	
	template <class F>
	void Set(Stream& in_, F& filter) {
		Init();
		in = &in_;
		filter.WhenOut = [this](const void *ptr, int size) { Out(ptr, size); };
		Filter = [&filter](const void *ptr, int size) { filter.Put(ptr, size); };
		End = [&filter] { filter.End(); };
	}
	
	void SetBufferSize(int size) { buffersize = size; inbuffer.clear(); }
	
	InFilterStream();
	template <class F> InFilterStream(Stream& in, F& filter) { Set(in, filter); }
};

class OutFilterStream : public Stream {
public:
	virtual   void  Close() override;
	virtual   bool  IsOpen() const override;

protected:
	virtual   void  _Put(int w) override;
	virtual   void  _Put(const void *data, dword size) override;

	std::vector<byte> buffer;
	int64        count;

	void   FlushOut();
	dword  Avail()               { return static_cast<dword>(4096 - (ptr - ~buffer)); }
	void   Init();

public:
	Stream                      *out;
	std::function<void(const void *, int)>     Filter;
	std::function<void()>                       End;
	void                         Out(const void *ptr, int size);
	
	int64                        GetCount() const             { return count; }
	
	std::function<int64()>                 WhenPos;

	template <class F>
	void Set(Stream& out_, F& filter) {
		Init();
		out = &out_;
		filter.WhenOut = [this](const void *ptr, int size) { Out(ptr, size); };
		Filter = [&filter](const void *ptr, int size) { filter.Put(ptr, size); };
		End = [&filter] { filter.End(); };
		count = 0;
	}
	
	OutFilterStream();
	template <class F> OutFilterStream(Stream& in, F& filter) { Set(in, filter); }
	~OutFilterStream();
};

#endif