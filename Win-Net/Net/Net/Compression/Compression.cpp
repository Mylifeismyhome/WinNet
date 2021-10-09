#include "Compression.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Compression)
int ZLib::Compress(BYTE*& data, size_t& size, const CompressionLevel level, const bool skip_free)
{
	z_stream stream;
	const auto max = (uInt)-1;

	stream.zalloc = (alloc_func)nullptr;
	stream.zfree = (free_func)nullptr;
	stream.opaque = (voidpf)nullptr;

	auto err = deflateInit(&stream, (int)level);
	if (err != Z_OK) return err;

	const auto out = ALLOC<BYTE>(size + 1);

	stream.next_out = out;
	stream.avail_out = max;
	stream.next_in = (z_const Bytef*)data;
	stream.avail_in = static_cast<uInt>(size);

	do {
		stream.avail_out = max;
		stream.avail_in = static_cast<uInt>(size);
		err = deflate(&stream, Z_FINISH);
	} while (err == Z_OK);

	size = stream.total_out;
	deflateEnd(&stream);

	if (!skip_free) FREE(data);
	data = out;

	return err == Z_STREAM_END ? Z_OK : err;
}

int ZLib::Compress(BYTE*& data, BYTE*& out, size_t& size, const CompressionLevel level, const bool skip_free)
{
	z_stream stream;
	const auto max = (uInt)-1;

	stream.zalloc = (alloc_func)nullptr;
	stream.zfree = (free_func)nullptr;
	stream.opaque = (voidpf)nullptr;

	auto err = deflateInit(&stream, (int)level);
	if (err != Z_OK) return err;

	out = ALLOC<BYTE>(size + 1);

	stream.next_out = out;
	stream.avail_out = max;
	stream.next_in = (z_const Bytef*)data;
	stream.avail_in = static_cast<uInt>(size);

	do {
		stream.avail_out = max;
		stream.avail_in = static_cast<uInt>(size);
		err = deflate(&stream, Z_FINISH);
	} while (err == Z_OK);

	size = stream.total_out;
	deflateEnd(&stream);

	if(!skip_free) FREE(data);

	return err == Z_STREAM_END ? Z_OK : err;
}

int ZLib::Decompress(BYTE*& data, size_t& size, const bool skip_free)
{
	z_stream stream;
	stream.zalloc = (alloc_func)nullptr;
	stream.zfree = (free_func)nullptr;
	stream.opaque = (voidpf)nullptr;

	auto err = inflateInit(&stream);
	if (err != Z_OK) return err;

	auto out = ALLOC<BYTE>(chunk + 1);

	std::vector<ChunkVector> chunkList;

	stream.next_out = out;
	stream.avail_out = NULL;
	stream.next_in = (z_const Bytef*)data;
	stream.avail_in = static_cast<uInt>(size);

	do {
		stream.next_out = out;
		stream.avail_out = chunk;
		stream.avail_in = static_cast<uInt>(size);
		err = inflate(&stream, Z_NO_FLUSH);

		if (err == Z_OK
			|| err == Z_STREAM_END)
			chunkList.emplace_back(ChunkVector(out, err, chunkList.size(), stream.total_out));
	} while (err == Z_OK);

	size = stream.total_out;
	inflateEnd(&stream);

	FREE(out);
	if (!skip_free) FREE(data);
	data = ALLOC<BYTE>(size + 1);
	size_t curPart = NULL;
	size_t it = NULL;
	for (const auto& entry : chunkList)
	{
		memcpy(&data[curPart], entry.data, it == chunkList.size() - 1 ? size - curPart : chunk);
		curPart += chunk;
		++it;
	}
	data[size] = '\0';

	return err == Z_STREAM_END ? Z_OK : err;
}

int ZLib::Decompress(BYTE*& data, BYTE*& out, size_t& size, const bool skip_free)
{
	z_stream stream;
	stream.zalloc = (alloc_func)nullptr;
	stream.zfree = (free_func)nullptr;
	stream.opaque = (voidpf)nullptr;

	auto err = inflateInit(&stream);
	if (err != Z_OK) return err;

	out = ALLOC<BYTE>(chunk + 1);

	std::vector<ChunkVector> chunkList;

	stream.next_out = out;
	stream.avail_out = NULL;
	stream.next_in = (z_const Bytef*)data;
	stream.avail_in = static_cast<uInt>(size);

	do {
		stream.next_out = out;
		stream.avail_out = chunk;
		stream.avail_in = static_cast<uInt>(size);
		err = inflate(&stream, Z_NO_FLUSH);

		if (err == Z_OK
			|| err == Z_STREAM_END)
			chunkList.emplace_back(ChunkVector(out, err, chunkList.size(), stream.total_out));
	} while (err == Z_OK);

	size = stream.total_out;
	inflateEnd(&stream);

	FREE(out);
	if(!skip_free) FREE(data);
	out = ALLOC<BYTE>(size + 1);
	size_t curPart = NULL;
	size_t it = NULL;
	for (const auto& entry : chunkList)
	{
		memcpy(&out[curPart], entry.data, it == chunkList.size() - 1 ? size - curPart : chunk);
		curPart += chunk;
		++it;
	}
	out[size] = '\0';

	return err == Z_STREAM_END ? Z_OK : err;
}
NET_NAMESPACE_END
NET_NAMESPACE_END
