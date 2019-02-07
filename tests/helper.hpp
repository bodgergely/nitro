
#pragma once

#include <gtest/gtest.h>
#include <nitro/nitro.h>

#include <fstream>
#include <memory>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unordered_set>
#include <cstdint>
#include <cstring>


using namespace std;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

void test_compress_decompress(u8* text, u64 len)
{
	NitroData compressed = nitro_compress(text, len, NitroEncoderType::BLOCK);
	NitroData decompressed = nitro_decompress(compressed.data, compressed.len);
	ASSERT_EQ(decompressed.len, len);
	ASSERT_EQ(memcmp(decompressed.data, text, len), 0);
	free(decompressed.data);
}

unique_ptr<u8> get_some_input(const vector<u8>& alphabet, uint64_t input_len)
{
    u8* buff = new u8[input_len];
    srand(time(NULL));

    int alen = alphabet.size();
    for(uint64_t i=0;i<input_len;i++) {
        buff[i] = alphabet[rand() % alen];
    }

    return unique_ptr<u8>(buff);
}

void debug_print_string(void* ptr, int len)
{
	if (!ptr) {
		printf("debug_print_string error: Null ptr passed!\n");
		return;
	}
	char* start = (char*)ptr;
	char* end = start + len;
	for (char* p = start; p < end; p++)
		printf("%c", *p);
	printf("\n");
}

vector<u8>	generate_big_alphabet(u16 alpha_size)
{
	vector<u8> alphabet;
	for (u16 i = 0; i < alpha_size; i++)
		alphabet.push_back((u8)i);
	return alphabet;
}

vector<u8>	generate_regular_alphabet(u8 alpha_size)
{
	unordered_set<char> alphabet;
	while (alphabet.size() < alpha_size) {
		u8 sym = 'A' + (rand() % ('A' - 'Z'));
		if (alphabet.find(sym) == alphabet.end())
			alphabet.insert(sym);
	}
	return vector<u8>(alphabet.begin(), alphabet.end());
}

unsigned bits_needed(unsigned count)
{
	unsigned bits_needed = 0;
	while (count > (0x1 << bits_needed)) {
		bits_needed++;
	}
	return bits_needed;
}


