#ifndef _NITRO_H
#define _NITRO_H

#include <cstdint>

enum NitroEncoderType {
	BLOCK = 0xC4
};

struct NitroData
{
	uint8_t*			data;
	uint64_t			len;
	enum NitroEncoderType	enctype;
};

/*
 *
 *	args:
 *		input:	data to be encoded
 *		len:	number bytes to encode
 *		type:	one of the enum EncoderType values
 *	returns:
 *		NitroData structure holding a malloc-ed output of the encoded text
 *		use free to release the memory!
 */
extern "C" NitroData nitro_compress(const uint8_t* input, uint64_t len, enum NitroEncoderType type);

/*
 *
 *	args:
 *		input:	data to be decoded
 *		len:	number bytes to decode
 *	returns:
 *		NitroData structure holding a malloc-ed output of the decoded text
 *		use free to release the memory!
 */
extern "C" NitroData nitro_decompress(const uint8_t* encoded, uint64_t len);


#endif  //_NITRO_H
