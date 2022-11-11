#include "Compression.h"

namespace Net
{
	namespace Compression
	{
		const size_t chunk = 1024;

		struct TChunk
		{
			byte m_bufferChunk[chunk];
			size_t m_total_out;

			TChunk()
			{
				memset(this->m_bufferChunk, 0, chunk);
			}

			explicit TChunk(byte* m_bufferIn, const size_t totalOut)
			{
				memset(this->m_bufferChunk, 0, chunk);
				write(m_bufferIn, totalOut);
			}

			void write(byte* m_bufferIn, const size_t totalOut)
			{
				memcpy(this->m_bufferChunk, m_bufferIn, totalOut);
				this->m_total_out = totalOut;
			}
		};

		int ZLib::Compress(BYTE*& data, size_t& size, const ZLIB_CompressionLevel level)
		{
			z_stream m_zInfo = { Z_NULL };
			m_zInfo.next_in = data;
			m_zInfo.avail_in = size;

			auto m_result = deflateInit(&m_zInfo, (int)level);
			if (m_result != Z_OK)
			{
				deflateEnd(&m_zInfo);
				return Z_ERRNO;
			}

			BYTE m_chunkBuffer[chunk];
			std::vector<TChunk> m_vChunk = {};
			size_t m_total_out = 0;
			do
			{
				m_zInfo.next_out = m_chunkBuffer;
				m_zInfo.avail_out = chunk;
				m_zInfo.avail_in = m_zInfo.avail_out;

				m_result = deflate(&m_zInfo, Z_FINISH);
				if (m_result != Z_OK && m_result != Z_STREAM_END)
				{
					return m_result;
				}

				TChunk m_chunk = {};
				m_chunk.write(m_chunkBuffer, m_zInfo.total_out);
				m_vChunk.emplace_back(m_chunk);
				m_total_out += m_zInfo.total_out;
			} while (m_result != Z_STREAM_END);
			deflateEnd(&m_zInfo);

			/*
			* create the new buffer
			*/
			size = m_total_out;
			FREE<BYTE>(data);
			data = ALLOC<BYTE>(size + 1);

			size_t m_off = 0;
			for (const auto& m_chunk : m_vChunk)
			{
				memcpy(&data[m_off], m_chunk.m_bufferChunk, m_chunk.m_total_out);
				m_off += m_chunk.m_total_out;
			}
			data[size] = 0;

			return Z_OK;
		}

		int ZLib::Compress(BYTE*& data, BYTE*& out, size_t& size, const ZLIB_CompressionLevel level)
		{
			z_stream m_zInfo = { Z_NULL };
			m_zInfo.next_in = data;
			m_zInfo.avail_in = size;

			auto m_result = deflateInit(&m_zInfo, (int)level);
			if (m_result != Z_OK)
			{
				deflateEnd(&m_zInfo);
				return Z_ERRNO;
			}

			BYTE m_chunkBuffer[chunk];
			std::vector<TChunk> m_vChunk = {};
			size_t m_total_out = 0;
			do
			{
				m_zInfo.next_out = m_chunkBuffer;
				m_zInfo.avail_out = chunk;
				m_zInfo.avail_in = m_zInfo.avail_out;

				m_result = deflate(&m_zInfo, Z_FINISH);
				if (m_result != Z_OK && m_result != Z_STREAM_END)
				{
					return m_result;
				}

				TChunk m_chunk = {};
				m_chunk.write(m_chunkBuffer, m_zInfo.total_out);
				m_vChunk.emplace_back(m_chunk);
				m_total_out += m_zInfo.total_out;
			} while (m_result != Z_STREAM_END);
			deflateEnd(&m_zInfo);

			/*
			* create the new buffer
			*/
			size = m_total_out;
			FREE<BYTE>(out);
			out = ALLOC<BYTE>(size + 1);

			size_t m_off = 0;
			for (const auto& m_chunk : m_vChunk)
			{
				memcpy(&out[m_off], m_chunk.m_bufferChunk, m_chunk.m_total_out);
				m_off += m_chunk.m_total_out;
			}
			out[size] = 0;

			return Z_OK;
		}

		int ZLib::Decompress(BYTE*& data, size_t& size)
		{
			z_stream m_zInfo = { 0 };
			m_zInfo.zalloc = Z_NULL;
			m_zInfo.zfree = Z_NULL;
			m_zInfo.opaque = Z_NULL;

			std::vector<TChunk> m_vChunk;
			BYTE m_chunkBuffer[chunk];

			auto m_result = inflateInit(&m_zInfo);
			if (m_result != Z_OK)
			{
				return m_result;
			}

			size_t m_total_out = 0;
			do
			{
				m_zInfo.avail_in = size;
				m_zInfo.next_in = data;

				do
				{
					m_zInfo.next_out = m_chunkBuffer;
					m_zInfo.avail_out = chunk;
					m_result = inflate(&m_zInfo, Z_NO_FLUSH);
					if (m_result != Z_OK
						&& m_result != Z_STREAM_END)
					{
						return m_result;
					}

					TChunk m_chunk;
					m_chunk.write(m_chunkBuffer, m_zInfo.total_out);
					m_vChunk.emplace_back(m_chunk);
					m_total_out += m_zInfo.total_out;
				} while (m_zInfo.avail_out == 0);
			} while (m_result != Z_STREAM_END);

			auto m_prevSize = size;
			size = m_total_out;

			m_result = inflateEnd(&m_zInfo);
			if (m_result != Z_OK)
			{
				size = m_prevSize;
				return m_result;
			}

			FREE<Byte>(data);
			data = ALLOC<BYTE>(size + 1);
			size_t m_off = 0;
			for (const auto& m_chunk : m_vChunk)
			{
				memcpy(&data[m_off], m_chunk.m_bufferChunk, m_chunk.m_total_out);
				m_off += m_chunk.m_total_out;
			}
			data[size] = 0;

			return Z_OK;
		}

		int ZLib::Decompress(BYTE*& data, BYTE*& out, size_t& size)
		{
			z_stream m_zInfo = { 0 };
			m_zInfo.zalloc = Z_NULL;
			m_zInfo.zfree = Z_NULL;
			m_zInfo.opaque = Z_NULL;

			std::vector<TChunk> m_vChunk;
			BYTE m_chunkBuffer[chunk];

			auto m_result = inflateInit(&m_zInfo);
			if (m_result != Z_OK)
			{
				return m_result;
			}

			size_t m_total_out = 0;
			do
			{
				m_zInfo.avail_in = size;
				m_zInfo.next_in = data;

				do
				{
					m_zInfo.next_out = m_chunkBuffer;
					m_zInfo.avail_out = chunk;
					m_result = inflate(&m_zInfo, Z_NO_FLUSH);
					if (m_result != Z_OK
						&& m_result != Z_STREAM_END)
					{
						return m_result;
					}

					TChunk m_chunk;
					m_chunk.write(m_chunkBuffer, m_zInfo.total_out);
					m_vChunk.emplace_back(m_chunk);
					m_total_out += m_zInfo.total_out;
				} while (m_zInfo.avail_out == 0);
			} while (m_result != Z_STREAM_END);

			auto m_prevSize = size;
			size = m_total_out;

			m_result = inflateEnd(&m_zInfo);
			if (m_result != Z_OK)
			{
				size = m_prevSize;
				return m_result;
			}

			FREE<Byte>(out);
			out = ALLOC<BYTE>(size + 1);
			size_t m_off = 0;
			for (const auto& m_chunk : m_vChunk)
			{
				memcpy(&out[m_off], m_chunk.m_bufferChunk, m_chunk.m_total_out);
				m_off += m_chunk.m_total_out;
			}
			out[size] = 0;
			return Z_OK;
		}
	}
}