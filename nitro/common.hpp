#pragma once

#include <nitro/nitro.h>

#include <cstdint>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <iostream> // debugging
#include <cassert>
#include <memory>
#include <exception>

//#define DEBUG

#ifdef DEBUG
#include <iostream>
#endif // DEBUG


using std::vector;
using std::unordered_map;
using std::unique_ptr;
using std::cerr;
using std::runtime_error;
using std::endl;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


namespace protocol
{
	extern const int	sizeof_encoder_type;
	extern const u64	sizeof_table_entry_size;
}

/*
	 * Symbol table to hold code mappings
	 */
class SymbolTable
{
private:
	unsigned bits_needed(unsigned count) const
	{
		unsigned bits_needed = 0;
		while (count > (0x1 << bits_needed)) {
			bits_needed++;
		}
		return bits_needed;
	}
public:
	u8	operator[](u8 sym)
	{
		assert(find(sym));	// replce with exception
		return _table[sym];
	}
	bool find(u8 sym) const { return !(_table.find(sym) == _table.end()); }
	bool insert(u8 sym, u8 code)
	{
		if (find(sym))
			return false;
		_table[sym] = code;
		return true;
	}
	size_t size() const
	{
		return _table.size();
	}

	size_t raw_size() const
	{
		// how many bytes we need to save it - one entry is (sym, encoded_sym) == 2 bytes
		return size() * 2;
	}

	const auto& get() { return _table; }

	unsigned bits_per_block() const { return bits_needed(_table.size()); }

	void debug_print(bool reverse)
	{
		printf("Symbol table:\n");
		printf("Symbol count: %llu\n", _table.size());
		for (const auto& entry : _table) {
			auto sym = entry.first;
			auto code = entry.second;
			if(!reverse)
				printf("Entry:%c:%u\n", sym, code);
			else
				printf("Entry:%u:%c\n", sym, code);
		}
		printf("End\n");
	}

private:
	unordered_map<u8, u8>		_table;
};


class BitStream
{
public:
	virtual ~BitStream() {}
	virtual void init(u8* buf, u64 size)
	{
		_init_common(buf, size);
	}
	u8*		begin() const { return _data; }
	u8*		pointer() const { return _data; }
	u64		size() const { return _data_size; }
	u64		remaining_bytes() const { return (_data + _data_size) - _pointer; }
	bool	valid() const { return _data && _data_size; }

	void release()
	{
		free(_data);
		_data = nullptr;
		_data_size = 0;
	}
protected:
	void	_init_common(u8* buf, u64 size)
	{
		_data = buf;
		_data_size = size;
		_pointer = buf;
	}
	u8		_bit_buffer{ 0 };				// current bit buffer
	u8		_idx_bit_buf{ 0 };			// current bit index to read/write
	u8*		_data{ nullptr };		// points to the beginning of the stream data
	u64		_data_size{ 0 };		// allocated memory for the stream
	u8*		_pointer{ nullptr };	// current pointer in the stream
};




class InputBitStream : public BitStream
{
public:
	virtual ~InputBitStream() {}
	virtual void init(u8* buf, u64 size) override
	{
		_init_common(buf, size);
		fetch_next_byte_to_buf();	// we need to fill the bit buffer
	}
	u8		read()
	{
		return read_bytes<u8>();
	}
	u16		read_16()
	{
		return read_bytes<u16>();
	}
	template<typename Size>
	Size	read_bytes()
	{
		Size r = *reinterpret_cast<Size*>(_pointer);
		_pointer += sizeof(Size);
		if (_pointer < _data + _data_size)	// do not read past last byte!
			fetch_next_byte_to_buf();
		return r;
	}

	u8		read_bit()
	{
		if (_idx_bit_buf == 8) {
			_pointer++;
			fetch_next_byte_to_buf();
		}
		u8 res = 0x1 & (_bit_buffer >> _idx_bit_buf++);	  // grab the next bit from the 8-bit buffer
		return res;
	}
private:
	void fetch_next_byte_to_buf()
	{
		_bit_buffer = *_pointer;
		_idx_bit_buf = 0;
	}
};




class OutputBitStream : public BitStream
{
public:
	virtual ~OutputBitStream() {}
	
	void	write_bytes(const void* start, int count)
	{
		auto byte = reinterpret_cast<const u8*>(start);
		while (count--)
			*_pointer++ = *byte++;
	}
	void	write_bit(u8 bit)
	{
#ifdef DEBUG
		// expensive check below
		if (bit != 0 && bit != 1)
			throw runtime_error("Attempt to write not 1 nor 0");
#endif
		_bit_buffer |= (bit << _idx_bit_buf++);
		if (_idx_bit_buf == 8) {
			write_buffer();
		}
	}
	void flush()
	{
		if (_idx_bit_buf != 0) {
			write_buffer();
		}
#ifdef DEBUG
		if (_pointer - _data != _data_size)
			throw runtime_error("Encoded data size does not match with allocated mem size!");
#endif
	}


private:
	void	write_buffer()
	{
		*_pointer++ = _bit_buffer;	// write out the bit buffer to memory
		_bit_buffer = 0;			// clear bit buffer
		_idx_bit_buf = 0;			// reset to 0 the 'next bit pos' index
	}
};
