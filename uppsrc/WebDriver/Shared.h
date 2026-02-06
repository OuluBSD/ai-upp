#ifndef _WebDriver_Shared_h_
#define _WebDriver_Shared_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

struct Shared_object_base {
private:
	volatile int ref_count_;

public:
	Shared_object_base() : ref_count_(0) {}
	virtual ~Shared_object_base() {}
	
	void Add_ref() { ref_count_++; }
	void Release() {
		if (--ref_count_ == 0) {
			delete this;
		}
	}
};

template<typename T>
class Shared {
public:
	Shared() : ptr_(nullptr) {}
	
	explicit Shared(T* ptr) : ptr_(ptr) {}
	
	Shared(const Shared& other) : ptr_(other.ptr_) {
		if (ptr_) {
			ptr_->Add_ref();
		}
	}
	
	~Shared() {
		if (ptr_) {
			ptr_->Release();
		}
	}
	
	Shared& operator=(const Shared& other) {
		if (this != &other) {
			if (ptr_) {
				ptr_->Release();
			}
			ptr_ = other.ptr_;
			if (ptr_) {
				ptr_->Add_ref();
			}
		}
		return *this;
	}
	
	T* operator->() const { return ptr_; }
	T& operator*() const { return *ptr_; }
	T* Get() const { return ptr_; }
	bool Is_empty() const { return ptr_ == nullptr; }
	
private:
	T* ptr_;
};

} // namespace detail

END_UPP_NAMESPACE

#endif