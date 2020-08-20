#pragma once
#include <Net/Net.h>

#define NET_ZLIB Net::Compression::ZLib

#include <ZLib/zlib.h>

enum CompressionLevel
{
	NO_COMPRESSION = Z_NO_COMPRESSION,
	BEST_SPEED = Z_BEST_SPEED,
	BEST_COMPRESSION = Z_BEST_COMPRESSION,
	DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION
};

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Compression)
BEGIN_CLASS(ZLib)
CLASS_PUBLIC
void Compress(BYTE**, size_t&, int = BEST_COMPRESSION) const;
void Decompress(BYTE**, size_t&, size_t = (128 * 1024)) const;
END_CLASS
END_NAMESPACE
END_NAMESPACE