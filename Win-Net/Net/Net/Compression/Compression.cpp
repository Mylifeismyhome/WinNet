#include "Compression.h"

namespace Net
{
	namespace Compression
	{
		const size_t chunk = 1024;

		struct ChunkVector
		{
			byte data[chunk];

			explicit ChunkVector(byte* data, const int err, const size_t listSize, const size_t totalOut)
			{
				memset(this->data, NULL, chunk);
				memcpy(this->data, data, err == Z_STREAM_END ? totalOut - listSize * chunk : chunk);
			}
		};

		int ZLib::Compress(BYTE*& data, size_t& size, const ZLIB_CompressionLevel level)
		{
			z_stream zInfo = { 0 };
			zInfo.total_in = zInfo.avail_in = size;
			zInfo.total_out = zInfo.avail_out = size;
			zInfo.next_in = data;
			zInfo.next_out = data;

			int nErr, nRet = -1;
			nErr = deflateInit(&zInfo, Z_DEFAULT_COMPRESSION);
			if (nErr == Z_OK) {
				nErr = deflate(&zInfo, Z_FINISH);
				if (nErr == Z_STREAM_END) {
					nRet = zInfo.total_out;
				}
			}
			deflateEnd(&zInfo);
			return(nRet);
		}

		int ZLib::Compress(BYTE*& data, BYTE*& out, size_t& size, const ZLIB_CompressionLevel level, const bool skip_free)
		{
			z_stream zInfo = { 0 };
			zInfo.total_in = zInfo.avail_in = size;
			zInfo.total_out = zInfo.avail_out = size;
			zInfo.next_in = data;
			zInfo.next_out = out;

			int nErr, nRet = -1;
			nErr = deflateInit(&zInfo, Z_DEFAULT_COMPRESSION);
			if (nErr == Z_OK) {
				nErr = deflate(&zInfo, Z_FINISH);
				if (nErr == Z_STREAM_END) {
					nRet = zInfo.total_out;
				}
			}
			deflateEnd(&zInfo);

			if (!skip_free) FREE(data);

			return(nRet);
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
			if (!skip_free) FREE(data);
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
	}
}