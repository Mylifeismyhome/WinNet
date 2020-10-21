#include "Compression.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Compression)
int ZLib::Compress(BYTE*& data, size_t& size, const CompressionLevel level) const
{
	z_stream stream;
	const auto max = (uInt)-1;

	stream.zalloc = (alloc_func)nullptr;
	stream.zfree = (free_func)nullptr;
	stream.opaque = (voidpf)nullptr;

	auto err = deflateInit(&stream, (int)level);
	if (err != Z_OK) return err;

	stream.next_out = data;
	stream.avail_out = max;
	stream.next_in = (z_const Bytef*)data;
	stream.avail_in = size;

	do {
		stream.avail_out = max;
		stream.avail_in = size;
		err = deflate(&stream, Z_FINISH);
	} while (err == Z_OK);

	size = stream.total_out;
	deflateEnd(&stream);

	return err == Z_STREAM_END ? Z_OK : err;
}

const size_t chunk = 1024;

struct ChunkVector
{
	byte data[chunk];

	explicit ChunkVector(byte* data)
	{
		memcpy(this->data, data, chunk);
	}
};

int ZLib::Decompress(BYTE*& data, size_t& size) const
{
	z_stream stream;
	stream.zalloc = (alloc_func)nullptr;
	stream.zfree = (free_func)nullptr;
	stream.opaque = (voidpf)nullptr;

	auto err = inflateInit(&stream);
	if (err != Z_OK) return err;

	byte out[chunk];
	std::vector<ChunkVector> chunkList;

	stream.next_in = (z_const Bytef*)data;
	stream.avail_in = size;

	do {
		stream.next_out = out;
		stream.avail_out = chunk;
		err = inflate(&stream, Z_FINISH);
		if (err != Z_STREAM_END)
			chunkList.emplace_back(ChunkVector(out));
	} while (err != Z_STREAM_END);

	FREE(data);
	size = stream.total_out;
	data = ALLOC<BYTE>(size + 1);

	size_t curPart = NULL;
	for (const auto& entry : chunkList)
	{

		memcpy(&data[curPart], entry.data, chunk);
		curPart += chunk;
	}

	inflateEnd(&stream);

	return Z_OK;
}
NET_NAMESPACE_END
NET_NAMESPACE_END
