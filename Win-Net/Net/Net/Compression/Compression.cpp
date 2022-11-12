#include "Compression.h"

namespace Net
{
	namespace Compression
	{
		int ZLib::Compress(BYTE* m_pUncompressed, size_t m_iSizeUncompressed, BYTE*& m_pCompressed, size_t& m_iSizeCompressed, const ZLIB_CompressionLevel level)
		{
			z_stream m_zInfo = { Z_NULL };
			m_zInfo.zalloc = Z_NULL;
			m_zInfo.zfree = Z_NULL;
			m_zInfo.opaque = Z_NULL;

			auto m_result = deflateInit(&m_zInfo, (int)level);
			if (m_result != Z_OK)
			{
				return m_result;
			}

			m_zInfo.next_in = m_pUncompressed;
			m_zInfo.avail_in = m_iSizeUncompressed;

			/*
			* determinate the required space for the compressed buffer
			*/
			m_iSizeCompressed = deflateBound(&m_zInfo, m_iSizeUncompressed);

			/*
			* just in-case if m_pUncompressed is allocated
			* free it before we leak memory
			*/
			FREE<BYTE>(m_pCompressed);
			m_pCompressed = ALLOC<BYTE>(m_iSizeCompressed + 1);
			m_pCompressed[m_iSizeCompressed] = 0;

			m_zInfo.next_out = m_pCompressed;
			m_zInfo.avail_out = m_iSizeCompressed;

			m_result = deflate(&m_zInfo, Z_FINISH);

			/*
			* deflate should report Z_STREAM_END
			*/
			if (m_result != Z_STREAM_END)
			{
				return Z_DATA_ERROR;
			}

			/*
			* check if deflate is not greedy
			*/
			if (m_zInfo.avail_in != 0)
			{
				return Z_DATA_ERROR;
			}

			m_result = deflateEnd(&m_zInfo);
			if (m_result != Z_OK)
			{
				return m_result;
			}

			return Z_OK;
		}

		int ZLib::Decompress(BYTE* m_pCompressed, size_t m_iSizeCompressed, BYTE*& m_pUncompressed, size_t m_iSizeUncompressed)
		{
			z_stream m_zInfo = { Z_NULL };
			m_zInfo.zalloc = Z_NULL;
			m_zInfo.zfree = Z_NULL;
			m_zInfo.opaque = Z_NULL;

			m_zInfo.next_in = m_pCompressed;
			m_zInfo.avail_in = m_iSizeCompressed;

			auto m_result = inflateInit(&m_zInfo);
			if (m_result != Z_OK)
			{
				return m_result;
			}

			/*
			* just in-case if m_pUncompressed is allocated
			* free it before we leak memory
			*/
			FREE<BYTE>(m_pUncompressed);
			m_pUncompressed = ALLOC<BYTE>(m_iSizeUncompressed + 1);
			m_pUncompressed[m_iSizeUncompressed] = 0;

			for (;;)
			{
				m_zInfo.next_out = m_pUncompressed;
				m_zInfo.avail_out = m_iSizeUncompressed;
				m_result = inflate(&m_zInfo, Z_NO_FLUSH);
				if (m_result == Z_STREAM_END)
				{
					break;
				}
				else if (m_result != Z_OK)
				{
					return m_result;
				}
			}

			m_result = inflateEnd(&m_zInfo);
			if (m_result != Z_OK)
			{
				return m_result;
			}

			/*
			* check for bad inflate
			*/
			if (m_zInfo.total_out != 2 * m_iSizeUncompressed + m_iSizeCompressed / 2) 
			{
				return Z_DATA_ERROR;
			}

			return Z_OK;
		}
	}
}