#include <nitro/nitro.h>
#include "encoder.hpp"
#include "decoder.hpp"

#include <memory>
#include <exception>

using namespace std;

static void unknown_decoder_type(NitroEncoderType type)
{
	fprintf(stderr, "Error- Unknown decoder type detected: %d\n", type);
}

NitroData nitro_compress(const uint8_t* input, uint64_t len, NitroEncoderType type)
{
    unique_ptr<Encoder> encoder {nullptr};
    switch(type) {
        case BLOCK:
            encoder = make_unique<BlockEncoder>(input, len);
            break;
        default:
			unknown_decoder_type(type);
            break;
    }
	NitroData data{ nullptr, 0, type};
    // check for nullptr
    if(!encoder || !len) 
        return data;

	try
	{
		data = encoder->encode();
	}
	catch (const exception& err)
	{
		data.data = nullptr;
		data.len = 0;
	}
	return data;
}

NitroData nitro_decompress(const uint8_t * encoded, uint64_t len)
{
	NitroData data{ nullptr, 0, (NitroEncoderType)0 };
	NitroEncoderType type;
	try
	{
		if(!encoded || !len)
			throw runtime_error("Invalid input to decoder!");
		unique_ptr<Decoder> decoder{ nullptr };
		type = determine_type(encoded);
		switch (type) {
		case BLOCK:
			decoder = make_unique<BlockDecoder>(encoded, len);
			break;
		default:
			unknown_decoder_type(type);
			break;
		}

		// check for nullptr
		if (!decoder)
			throw runtime_error("Failed to obtain decoder object!");

		data = decoder->decode();	// throws
	}
	catch (runtime_error& err)
	{
		cerr << err.what() << endl;
		data.enctype = type;
	}
	return data;
}




