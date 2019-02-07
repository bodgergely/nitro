#pragma once

#include "common.hpp"


class Encoder
{
public:
	virtual ~Encoder() {}
    virtual NitroData	encode() = 0;
	NitroEncoderType			get_my_type() const { return this->_type; }
protected:
	NitroEncoderType			_type;
};

class BlockEncoder : public Encoder
{
public:
	BlockEncoder(const u8* input, uint64_t len) : 
		_input(input),
		_len_of_input(len)
	{
		_type = NitroEncoderType::BLOCK;
	}
	virtual ~BlockEncoder() {}
	virtual NitroData encode() override
	{
		if (!_input || !_len_of_input)
			throw runtime_error("invalid input (input nullptr or 0 length)");
#ifdef DEBUG
		printf("------------ENCODING-----------------\n");
#endif // DEBUG
		build_symtable();
		alloc_output();	// throws
#ifdef DEBUG
		printf("Allocated bytes for encoding: %llu\n", _output.size());
		printf("Writing metadata\n");
#endif // DEBUG
		write_metadata();
#ifdef DEBUG
		printf("Compressing...\n");
#endif // DEBUG

		compress();
#ifdef DEBUG
		printf("Done.\n");
#endif // DEBUG

		auto data = _output.begin();
		auto len = _output.size();
		return NitroData{ data, len, get_my_type() };
	}

private:
	void write_encoder_type()
	{
		u8 type = static_cast<u8>(get_my_type());
		_output.write_bytes(&type, protocol::sizeof_encoder_type);
	}
	void write_metadata()
	{
		write_encoder_type();
		auto& table = _symtable;
		assert(table.size() <= 256);
		u16 entry_count = (u16)table.size();			// cast should be safe now
		_output.write_bytes(&entry_count, protocol::sizeof_table_entry_size);		// write how many entries we have in the table
		// now write each entry (sym - code)
		for (const auto& entry : table.get()) {
			u8 sym = entry.first;
			u8 code = entry.second;
			_output.write_bytes(&code, 1);
			_output.write_bytes(&sym, 1);
		}
		// now write the length of the input - use 8 bytes
		_output.write_bytes(&_len_of_input, sizeof(_len_of_input));
	}
	void compress()
	{
		try
		{
			// encode and write the input
			unsigned blocksize = _symtable.bits_per_block();
			const u8* pinput = _input;
			// iterate over the input and write the code bit blocks to the bitstream
			for (uint64_t i = 0; i < _len_of_input; i++) {
				u8 sym = *pinput++;
				u8 code = _symtable[sym];
				// write the code block 
				for (int bitidx = 0; bitidx < blocksize; bitidx++) {
					_output.write_bit(0x1 & (code >> bitidx));
				}
			}
			// we need to flush the last 'buffer' byte which might hold the remaining bits
			// when decoding this is an EDGE CASE
			_output.flush();
		}
		catch (const runtime_error& err)
		{
			cerr << err.what() << endl;
			_output.release();
			throw err;
		}
	}
	void build_symtable()
	{
		auto& _table = _symtable;
		u8 encoding = 0;
		for (size_t i = 0; i < _len_of_input; i++) {
			u8 ch = _input[i];
			if (!_table.find(ch))
				_table.insert(ch, encoding++);
		}
#ifdef DEBUG
		//_symtable.debug_print(false);
#endif // DEBUG

		assert(_symtable.size() <= 256);
	}

	void alloc_output()
	{
		/**
		 * Store:
		 * 	- symbol table len (symbol table raw size)
		 *  - symbol table (each entry is 2 bytes)
		 *              one byte for the encoded num,1 byte
		 *              for the original symbol
		 *  - data len (8 bytes - can be a large number)
		 *  - data
		 */
		// calculate the number of bits required to be able to
		// represent all the symbols - there are max 256 symbols
		// since in one byte in the input can max have 256 values
		auto bits_per_block = _symtable.bits_per_block();
		assert(bits_per_block <= 8);
		u64 total_bits = bits_per_block * _len_of_input;
		u64 bytes_for_data = total_bits / 8;
		if (total_bits % 8)
			bytes_for_data++;

		u64 space_required =  header_size() + bytes_for_data;
		u8* buffer = (u8*) malloc(space_required);
		if (!buffer) {
			fprintf(stderr,
					"Allocation failed for %llu bytes for holding the encoded result.\n",
					space_required);
			throw runtime_error("Memory allocation failed");
		}
		_output.init(buffer, space_required);
	}

	inline unsigned header_size() const
	{
		return	protocol::sizeof_encoder_type + protocol::sizeof_table_entry_size + 
				_symtable.raw_size() + sizeof(_len_of_input);
	}

private:
	const u8*			_input;
	OutputBitStream		_output;
	const u64			_len_of_input;
	SymbolTable			_symtable;
};


