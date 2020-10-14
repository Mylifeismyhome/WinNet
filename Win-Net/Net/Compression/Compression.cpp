#include "Compression.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Compression)
void ZLib::Compress(BYTE*& bytes, size_t& size, const int level) const
{
	std::vector<uint8_t> buffer;
	auto compressed = ALLOC<BYTE>(size + 1);

	z_stream strm;
	strm.zalloc = nullptr;
	strm.zfree = nullptr;
	strm.next_in = bytes;
	strm.avail_in = static_cast<uInt>(size);
	strm.next_out = compressed;
	strm.avail_out = static_cast<uInt>(size);

	deflateInit(&strm, level);

	while (strm.avail_in != 0)
	{
		if (deflate(&strm, Z_NO_FLUSH) != Z_OK)
		{
			FREE(compressed);
			return;
		}

		if (strm.avail_out == 0)
		{
			buffer.insert(buffer.end(), compressed, compressed + size);
			strm.next_out = compressed;
			strm.avail_out = static_cast<uInt>(size);
		}
	}

	auto result = Z_OK;
	do
	{
		if (strm.avail_out == 0)
		{
			buffer.insert(buffer.end(), compressed, compressed + size);
			strm.next_out = compressed;
			strm.avail_out = static_cast<uInt>(size);
		}
	} while ((result = deflate(&strm, Z_FINISH)) == Z_OK);

	if (result != Z_STREAM_END)
	{
		FREE(compressed);
		return;
	}

	buffer.insert(buffer.end(), compressed, compressed + size - strm.avail_out);
	deflateEnd(&strm);

	FREE(bytes);

	size = buffer.size();
	bytes = compressed; // pointer swap
	bytes[size] = '\0';
}

void ZLib::Decompress(BYTE*& bytes, size_t& size, const size_t maxAvailOut) const
{
	auto decompressed = ALLOC<BYTE>(maxAvailOut + 1);

	z_stream strm = { nullptr };
	strm.total_in = strm.avail_in = static_cast<uInt>(size);
	strm.total_out = strm.avail_out = static_cast<uInt>(maxAvailOut);
	strm.next_in = bytes;
	strm.next_out = decompressed;
	strm.zalloc = nullptr;
	strm.zfree = nullptr;
	strm.opaque = nullptr;

	/* 15 window bits, and the +32 tells zlib to to detect if using gzip or zlib */
	if (inflateInit2(&strm, (15 + 32)) == Z_OK)
	{
		if (inflate(&strm, Z_FINISH) != Z_STREAM_END)
		{
			inflateEnd(&strm);
			FREE(decompressed);
			return;
		}
	}
	else
	{
		inflateEnd(&strm);
		FREE(decompressed);
		return;
	}

	inflateEnd(&strm);

	FREE(bytes);

	size = strm.total_out;
	bytes = decompressed;
	bytes[size] = '\0'; // pointer swap
}
NET_NAMESPACE_END
NET_NAMESPACE_END
