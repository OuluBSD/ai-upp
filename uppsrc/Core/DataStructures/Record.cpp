#include "DataStructures.h"


NAMESPACE_UPP
using namespace UPP;





Record::Record() {
	
}

Record::Record(const Record& r) {
	data = r.data;
	if(data)
		data->Retain();
}

Record::~Record() {
	Clear();
}


void Record::operator=(const Record& r) {
	if(data == r.data)
		return;
	if(r.data)
		r.data->Retain();
	if(data)
		data->Release();
	data = r.data;
}

void Record::Clear() {
	if (data) {
		data->Release();
		data = 0;
	}
}

int Record::GetSampleRate() const {
	return data ? data->buffer.GetSampleRate() : 0;
}

float Record::Get(int ch, int i) const {
	return data ? data->buffer.Get(ch, i) : 0.0f;
}

int Record::GetCount() const {
	return data ? data->buffer.GetCount() : 0;
}

int Record::GetChannels() const {
	return data ? data->buffer.GetChannels() : 0;
}

bool Record::IsEmpty() const {
	return !data;
}









Record StreamRecord::LoadAny(String path) {
	TODO
	return Record();
}


END_UPP_NAMESPACE
