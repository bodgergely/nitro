#pragma once

#include "common.hpp"


/* Determines the the type of the encoder
*  that was used to encode the data
*	The first byte describes the EncoderType
*/
NitroEncoderType determine_type(const u8* data)
{
	u8 type = *data;
	return (NitroEncoderType)type;		// TODO dangerous c style cast - we store in one byte the type but it can be actually an int
}

class Decoder
{
public:
	virtual ~Decoder() {}
	virtual NitroData decode() = 0;
};


class BlockDecoder : public Decoder
{
public:
	BlockDecoder(const u8* encoded, uint64_t len)
	{
		// ignoe the first byte which is the encoder type
		_input.init(const_cast<u8*>(encoded), len);
	}
	virtual ~BlockDecoder() {}
	virtual NitroData decode() override
	{
		if (!_input.valid())
			throw runtime_error("invalid input (input nullptr or 0 length)");
#ifdef DEBUG
		printf("------------DECODING-----------------\n");
#endif // DEBUG
		
		read_metadata(); // throws
		alloc_space();	 // throws
		decompress();

		return NitroData{ _output, _orig_symbol_count, NitroEncoderType::BLOCK };

	}
private:
	void decompress()
	{
		unsigned blocksize = _symtable.bits_per_block();
		assert(blocksize <= 8);
		u8* pout = _output;		// current output location
		// try to decode all the symbols
		for (u64 i = 0; i < _orig_symbol_count; i++) {
			u8 code = fetch_next_code(blocksize);
			u8 sym =  _symtable[code];
			*pout++ = sym;
		}
	}
	u8	 fetch_next_code(const unsigned& blocksize)
	{
		u8 code = 0;
		for (int bitidx = 0; bitidx < blocksize; bitidx++) {
			u8 bit = _input.read_bit();
			assert(bit == 1 || bit == 0);
			code |= bit << bitidx;
		}
		return code;
	}
	void parse_symtable(u16 entrycount)
	{
		if (entrycount > 256)
			throw runtime_error("Symbol table size can be max 256.");
		if (_input.remaining_bytes() < entrycount * protocol::sizeof_table_entry_size)
			throw runtime_error("Malformed metadata - symbol table size indicated is bigger than the number of bytes left in the stream.");

		for (auto i = 0; i < entrycount; i++) {
			u8 code = _input.read();
			u8 sym = _input.read();
#ifdef DEBUG
			if (_symtable.find(code))
				throw runtime_error("Malformed symbol table. Coded value occurs multiple times.");
#endif // DEBUG
			_symtable.insert(code, sym);
		}
#ifdef DEBUG
		//_symtable.debug_print(true);
#endif // DEBUG

		if (_symtable.size() != entrycount)
			throw runtime_error("Symbol table size does not match with indicated count after parsing.");
	}
	void read_metadata()
	{
		// we will need to sanitize the user input whether this data still represents
		// a valid protocol
		// be careful - user input can be 'anything' even malicious
		// TODO - create a hash when encoding possibly and check if the hash matches
		// at the end of the decoding
		// 1. check encoder type again
		if ((NitroEncoderType)_input.read() != NitroEncoderType::BLOCK)
			throw runtime_error("Wrong decoder for indicated encoder.");
		// 2. read symbol table entry count (stored on the next 2 bytes)
		u16 table_entry_count = _input.read_bytes<u16>();
		parse_symtable(table_entry_count);
		parse_orig_symbol_count();
		validate_orig_symbol_count();	// throws
	}
	void parse_orig_symbol_count()
	{
		if (_input.remaining_bytes() < sizeof(u64))
			throw runtime_error("Malformed protocol - not enough bytes remains to read the original text length");
		_orig_symbol_count = _input.read_bytes<u64>();
	}

	void validate_orig_symbol_count()
	{
		auto total_bits_needed = _symtable.bits_per_block() * _orig_symbol_count;
		auto bytes_needed = total_bits_needed / 8;
		if (total_bits_needed % 8)
			bytes_needed++;
		if (bytes_needed != _input.remaining_bytes())
			throw runtime_error("Malformed data - stream does not match with required amount based on the recovered original symbol count and bits per block calculation.");
	}

	void alloc_space()
	{
		_output = reinterpret_cast<u8*>(malloc(_orig_symbol_count));
		if (!_output)
			throw runtime_error("Could not allocate enough space to hold decoded result");
	}

private:
	InputBitStream		_input;
	SymbolTable			_symtable;
	u64					_orig_symbol_count{ 0 };
	u8*					_output{ nullptr };
};
