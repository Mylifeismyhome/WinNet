#define CHUNK 16384
#include "Compression.h"

#include <elzma/easylzma/compress.h>
#include <elzma/easylzma/decompress.h>

struct dataStream
{
	const unsigned char* inData;
	size_t inLen;

	unsigned char* outData;
	size_t outLen;
};

static int inputCallback(void* ctx, void* buf, size_t* size)
{
	size_t rd = 0;
	struct dataStream* ds = (struct dataStream*)ctx;

	rd = (ds->inLen < *size) ? ds->inLen : *size;

	if (rd > 0)
	{
		memcpy(buf, &ds->inData, rd);
		ds->inData += rd;
		ds->inLen -= rd;
	}

	*size = rd;

	return 0;
}

static size_t outputCallback(void* ctx, const void* buf, size_t size)
{
	struct dataStream* ds = (struct dataStream*)ctx;

	if (size > 0) {
		const auto rel = (unsigned char*)realloc(ds->outData, ds->outLen + size);
		if (!rel) return 0;
		ds->outData = rel;
		memcpy(&ds->outData[ds->outLen], buf, size);
		ds->outLen += size;
	}

	return size;
}

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Compression)
int LZMA::Compress(BYTE*& data, size_t& size, const LZMA_CompressionLevel format)
{
	/* allocate compression handle */
	auto hand = elzma_compress_alloc();

	auto rc = elzma_compress_config(hand, ELZMA_LC_DEFAULT, ELZMA_LP_DEFAULT, ELZMA_PB_DEFAULT, 5, (1 << 20) /* 1mb */, (elzma_file_format)format, size);
	if (rc != ELZMA_E_OK) 
	{
		elzma_compress_free(&hand);
		return rc;
	}

	/* now run the compression */
	{
		struct dataStream ds;
		ds.inData = data;
		ds.inLen = size;
		ds.outData = NULL;
		ds.outLen = 0;

		rc = elzma_compress_run(hand, inputCallback, (void*)&ds,
			outputCallback, (void*)&ds,
			NULL, NULL);

		if (rc != ELZMA_E_OK) {
			if (ds.outData != NULL) free(ds.outData);
			elzma_compress_free(&hand);
			return rc;
		}

		data = ds.outData;
		size = ds.outLen;
	}
	elzma_compress_free(&hand);

	return rc;
}

int LZMA::Compress(BYTE*& data, BYTE*& out, size_t& size, const LZMA_CompressionLevel format, const bool skip_free)
{
	/* allocate compression handle */
	auto hand = elzma_compress_alloc();

	auto rc = elzma_compress_config(hand, ELZMA_LC_DEFAULT, ELZMA_LP_DEFAULT, ELZMA_PB_DEFAULT, 5, (1 << 20) /* 1mb */, (elzma_file_format)format, size);
	if (rc != ELZMA_E_OK)
	{
		elzma_compress_free(&hand);
		return rc;
	}

	/* now run the compression */
	{
		struct dataStream ds;
		ds.inData = data;
		ds.inLen = size;
		ds.outData = NULL;
		ds.outLen = 0;

		rc = elzma_compress_run(hand, inputCallback, (void*)&ds,
			outputCallback, (void*)&ds,
			NULL, NULL);

		if (rc != ELZMA_E_OK) {
			if (ds.outData != NULL) free(ds.outData);
			elzma_compress_free(&hand);
			return rc;
		}

		if(!skip_free) FREE(data);
		out = ds.outData;
		size = ds.outLen;
	}
	elzma_compress_free(&hand);

	return rc;
}

int LZMA::Decompress(BYTE*& data, size_t& size, const LZMA_CompressionLevel format)
{
	int rc = 0;

	auto hand = elzma_decompress_alloc();

	/* now run the compression */
	{
		struct dataStream ds;
		ds.inData = data;
		ds.inLen = size;
		ds.outData = NULL;
		ds.outLen = 0;

		rc = elzma_decompress_run(hand, inputCallback, (void*)&ds, outputCallback, (void*)&ds, (elzma_file_format)format);

		if (rc != ELZMA_E_OK) {
			if (ds.outData != NULL) free(ds.outData);
			elzma_decompress_free(&hand);
			return rc;
		}

		data = ds.outData;
		size = ds.outLen;
	}
	elzma_decompress_free(&hand);

	return rc;
}

int LZMA::Decompress(BYTE*& data, BYTE*& out, size_t& size, const LZMA_CompressionLevel format, const bool skip_free)
{
	int rc = 0;

	auto hand = elzma_decompress_alloc();

	/* now run the compression */
	{
		struct dataStream ds;
		ds.inData = data;
		ds.inLen = size;
		ds.outData = NULL;
		ds.outLen = 0;

		rc = elzma_decompress_run(hand, inputCallback, (void*)&ds, outputCallback, (void*)&ds, (elzma_file_format)format);

		if (rc != ELZMA_E_OK) {
			if (ds.outData != NULL) free(ds.outData);
			elzma_decompress_free(&hand);
			return rc;
		}

		if(!skip_free) FREE(data);
		out = ds.outData;
		size = ds.outLen;
	}
	elzma_decompress_free(&hand);

	return rc;
}

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
