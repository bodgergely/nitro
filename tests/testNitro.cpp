
#include <gtest/gtest.h>
#include "helper.hpp"

/*
 * Test!:
 * 	- Test if compression ratio is achieved
 * 	- Encoding typical use case
 * 	- Encoding/decoding with 1-256 alphabet size
 * 	- Encoding text with only one symbol
 * 	- Encoding empty text (zero length)
 * 	- Encoding when nullptr is passed
 * 	- Encoding with high number of symbol (max 256)
 * 	- Decoding well formed input
 * 	- Decoding when nullptr is passed
 * 	- Decoding malformed input
 * 	- Decoding malicious input - TODO
 */

TEST(NitroEncode, symbolCounts)
{
	uint64_t length = 300;
	vector<u16> alpha_sizes = { 1,2,3,4,5,6,7,8,9,10,11,12,15,16,19,32,33,64,128,256 };
	for (auto alpha_size : alpha_sizes) {
		for (int i = 0; i < 1000; i++) {
			auto alphabet = generate_big_alphabet(alpha_size);
			auto input = get_some_input(alphabet, length);
			test_compress_decompress(input.get(), length);
		}
	}
}

TEST(NitroEncode, randomized)
{
	for (int i=0;i<1000; i++) {
		srand(time(NULL));
		int symcount = (rand() % 16) + 1;
		if(rand() % 2 == 0)
			symcount = max(16 + (rand() % 256), 256);
		u64 len = 100 + (rand() % 200);	// average input len case
		if(rand() % 3 == 0)
			len = rand() % 10;		// small input len case
		if(rand() % 4 == 0)
			len = 10000 + (rand() % 200);

		auto alphabet = generate_big_alphabet(symcount);
		auto input = get_some_input(alphabet, len);
		test_compress_decompress(input.get(), len);
	}
}

TEST(NitroEncode, compressionRatio)
{
	int length = 10000;

	for(auto symcount=1;symcount<=256;symcount++) {
		auto alphabet = generate_big_alphabet(symcount);
		auto text = get_some_input(alphabet, length);
		auto res = nitro_compress(text.get(), length, NitroEncoderType::BLOCK);
		double ratio = (double)res.len / length;
		double ideal = bits_needed(symcount) / (double)8;
		ASSERT_TRUE(ratio - ideal <  0.06);			//  acceptable error depends on input len
		free(res.data);
	}
}


TEST(NitroEncode, inputLenEqualsOne)
{
	char c = 'a';
	auto enc = nitro_compress((const u8*)&c, 1, NitroEncoderType::BLOCK);
	auto dec = nitro_decompress(enc.data, enc.len);
	ASSERT_EQ(dec.len, 1);
	ASSERT_EQ(*(dec.data), c);
}

TEST(NitroEncode, inputHasOnlyOneTypeOfSymbol)
{
	u64 len = 2000;
	char sym = 'a';
	char* p = (char*)malloc(len);
	memset(p, 'a', len);
	test_compress_decompress((u8*)p, len);
	free(p);
}

TEST(NitroEncode, nullPtrPassed)
{
	auto enc = nitro_compress(NULL, 43434432323, NitroEncoderType::BLOCK);
	ASSERT_EQ(enc.data, nullptr);
	ASSERT_EQ(enc.len, 0);
}

TEST(NitroEncode, inputLenEqualZero)
{
	char c = 'a';
	auto enc = nitro_compress((const u8*)&c, 0, NitroEncoderType::BLOCK);
	ASSERT_EQ(enc.data, nullptr);
	ASSERT_EQ(enc.len, 0);
}


TEST(NitroDecode, nullPtrPassed)
{
	auto res = nitro_decompress(nullptr, 1000000);
	ASSERT_EQ(res.data, nullptr);
	ASSERT_EQ(res.len, 0);
}

TEST(NitroDecode, malformHeaderType)
{
	int len = 300;
	auto text = get_some_input({'A', 'T', 'V','Z'}, len);
	auto enc = nitro_compress(text.get(), len, NitroEncoderType::BLOCK);
	u8* code = enc.data;
	*code = 0xDE;
	auto dec = nitro_decompress(code, enc.len);
	ASSERT_EQ(dec.data, nullptr);
	if(dec.data)
		free(dec.data);
}

TEST(NitroDecode, badPointerPassed)
{
	int len = 300;
	auto text = get_some_input({'A', 'T', 'V','Z'}, len);
	auto enc = nitro_compress(text.get(), len, NitroEncoderType::BLOCK);
	auto dec = nitro_decompress((u8*)&len, 2000);		// pass 'bad' pointer
	ASSERT_EQ(dec.data, nullptr);
	if(dec.data)
		free(dec.data);
}























