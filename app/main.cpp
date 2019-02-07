#include <nitro/nitro.h>
#include <cstdint>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <utility>
#include <memory>

using namespace std;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

/*
 * nitro application
 *
 * nitro has 3 required inputs:
 *  - compress or decompress (-c, -x respectively)
 *  - input file name
 * 	- output file name
 * nitro has 1 one optional:
 * 	- optional flag of compressions method (default being block, -b) only valid if compressing (otherwise ignored)
 *
 */

struct cmd_args
{
	bool		compress;
	const char* infile;
	const char* outfile;
	NitroEncoderType encode_method {BLOCK};
};

void abort_nitro()
{
	printf("Aborting\n");
	exit(-1);
}

void print_help()
{
	printf("Usage:   nitro [-cx] [FILE] [FILE]\n");
	printf("Example: nitro -c genome.txt compressed.bin\n");
	printf("         nitro -x compressed.bin genome.txt\n");
	printf("Flags:\n");
	printf("  -c	 compress\n");
	printf("  -x	 decompress\n");
}

bool parse_cmd_args(int argc, char** argv, cmd_args& cmd)
{
	if(argc < 3)
		return false;
	// compress or decompress?
	const char* cp = *argv++;
	if(strncmp(cp, "-c", 2) == 0)
		cmd.compress = true;
	else if(strncmp(cp, "-x", 2) == 0)
		cmd.compress = false;
	else {
		return false;
	}
	// parse file names
	cp = *(argv++);
	cmd.infile = cp;
	cp = *(argv++);
	cmd.outfile = cp;
	// parse compress method
	if(argc > 3 && cmd.compress) {
		if(strnlen(*argv, 2) < 2)
			return false;
		char method = (*argv)[1];
		switch(method) {
		case 'b':
			cmd.encode_method = NitroEncoderType::BLOCK;
			break;
		default:
			printf("Unsupported compression method.\n");
			break;
		}
	}
	return true;
}

void emit_statistics(const NitroData& nitrodata, u64 original_length)
{
	const char* method;
	switch(nitrodata.enctype) {
	case BLOCK:
		method = "BLOCK";
		break;
	default:
		method = "N/A";
		break;
	}
	double ratio = ((double)nitrodata.len) / original_length;
	printf("--------------------------------------\n");
	printf("Compression statistics:\n");
	printf("Method: %s\n", method);
	printf("Bytes written: %llu\n", nitrodata.len);
	printf("Compressed/Original Ratio: %.1f%%\n", 100.0 * ratio);
	printf("--------------------------------------\n");
}

pair<unique_ptr<u8>, u64>  read_file(const char* filename)
{
	pair<unique_ptr<u8>, u64> result {nullptr, 0};
	printf("Reading input file: %s\n", filename);
	ifstream infile(filename, ifstream::in | ifstream::binary);
	if(!infile.is_open()) {
		fprintf(stderr, "Failed to open input file: %s\n", filename);
		return result;
	}
	// determine size of the file
	infile.seekg(0, infile.end);
	u64 length = infile.tellg();
	// rewind
	infile.seekg(0, infile.beg);
	// allocate memory to hold the contents - can fail if we run out of memory
	unique_ptr<u8> buffer { nullptr };
	try {
		buffer = unique_ptr<u8> { new u8[length] };
	}
	catch(const bad_alloc& memfail) {
		fprintf(stderr, "Failed to allocate enough memory to hold the input file: %s with size: %llu\n",
									filename, length);
		return result;
	}
	infile.read((char*)(buffer.get()), length);
	infile.close();
	result.first = move(buffer);
	result.second = length;
	return result;
}

bool write_file(const char* filename, u8* data, u64 len)
{
	printf("Writing result to %s\n", filename);
	std::ios_base::sync_with_stdio(false);
	ofstream outfile(filename, ios::out | ios::binary);
	if(!outfile.is_open()) {
		fprintf(stderr, "Failed to open file to write results: %s\n", filename);
		return false;
	}
	outfile.write((char*)data, len);
	outfile.close();
	return true;
}

pair<unique_ptr<u8>, u64> read_data(const char* filename)
{
	auto contents = read_file(filename);
	if(!contents.first){  	// if pointer to data is null - exit
		fprintf(stderr, "Error during reading file: %s\n", filename);
		abort_nitro();
	}
	return contents;
}

void compress(const char* infile_name, const char* outfile_name, NitroEncoderType method)
{
	auto contents = read_data(infile_name);
	auto& data = contents.first;
	auto& len = contents.second;

	printf("Compressing...\n");
	NitroData result = nitro_compress(data.get(), len, method);
	bool good = false;
	if(result.data && result.len) {
		good = write_file(outfile_name, result.data, result.len);
	}
	else {
		fprintf(stderr, "Failed compression. Output file will not be written\n");
	}
	free(result.data);		// need to use free to deallocate the memory returned from nitro library
	if(good)
		emit_statistics(result, len);
}

void decompress(const char* infile_name, const char* outfile_name)
{
	auto contents = read_data(infile_name);
	auto& data = contents.first;
	auto& len = contents.second;
	printf("Decompressing...\n");
	NitroData result = nitro_decompress(data.get(), len);
	bool good = false;
	if(result.data && result.len) {
		good = write_file(outfile_name, result.data, result.len);
	}
	else {
		fprintf(stderr, "Failed compression. Output file will not be written\n");
	}
	free(result.data);		// need to use free to deallocate the memory returned from nitro library
}

int main(int argc, char** argv)
{
	cmd_args cmd;
	if(!parse_cmd_args(argc - 1, argv + 1, cmd)) {
		print_help();
		exit(-1);
	}
	if(cmd.compress) {
		compress(cmd.infile, cmd.outfile, cmd.encode_method);
	}
	else {
		decompress(cmd.infile, cmd.outfile);
	}
    return 0;
}


