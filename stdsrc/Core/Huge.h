#pragma once
#ifndef _Core_Huge_h_
#define _Core_Huge_h_

#include "Core.h"
#include <vector>
#include <cstring>

class Huge {
public:
	enum { CHUNK = 1024 * 1024 };

private:
	struct Block {
		std::vector<byte> data;
		
		Block() : data(CHUNK) {}
	};
	
	std::vector<Block> data;
	size_t             size;

public:
	byte* AddChunk() {
		data.emplace_back();
		return data.back().data.data();
	}
	
	void Finish(int last_chunk_size) {
		if (!data.empty()) {
			// Resize the last chunk to the specified size
			data.back().data.resize(last_chunk_size);
		}
		size = (data.size() - 1) * CHUNK + last_chunk_size;
	}
	
	size_t GetSize() const { return size; }
	
	void Get(void *t, size_t pos, size_t sz) const {
		byte *target = static_cast<byte*>(t);
		size_t current_pos = pos;
		size_t remaining = sz;
		
		while (remaining > 0) {
			size_t block_idx = current_pos / CHUNK;
			size_t offset_in_block = current_pos % CHUNK;
			
			if (block_idx >= data.size()) break;
			
			const Block& block = data[block_idx];
			size_t available_in_block = block.data.size() - offset_in_block;
			size_t to_copy = std::min(remaining, available_in_block);
			
			if (to_copy <= 0) break;
			
			std::memcpy(target, block.data.data() + offset_in_block, to_copy);
			
			target += to_copy;
			current_pos += to_copy;
			remaining -= to_copy;
		}
	}
	
	void Get(void *t) { Get(t, 0, GetSize()); }
	
	String Get() const {
		String result;
		result.SetCount(GetSize());
		Get(~result);
		return result;
	}
	
	Huge() : size(0) {}
};

#endif