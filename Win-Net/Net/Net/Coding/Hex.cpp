/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <Net/Coding/Hex.h>

namespace Net
{
	namespace Coding
	{
		static bool ProcessEncode(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
		{
			try
			{
				const auto hexSize = (size * 2);
				CryptoPP::HexEncoder encoder;
				encoder.Put(data, size);
				encoder.MessageEnd();
				const auto encoded = ALLOC<byte>(hexSize + 1);
				encoder.Get(encoded, hexSize);
				encoded[hexSize] = '\0';

				FREE<byte>(out);
				out = encoded; // pointer swap
				out[hexSize] = '\0';

				size = hexSize;
				return true;
			}
			catch (const CryptoPP::Exception& ex)
			{
				NET_LOG_ERROR(CSTRING("[NET_HEX][ENCODE] - %s"), ex.what());
				return false;
			}
		}

		static bool ProcessEncode(CryptoPP::byte*& data, size_t& size)
		{
			try
			{
				const auto hexSize = (size * 2);
				CryptoPP::HexEncoder encoder;
				encoder.Put(data, size);
				encoder.MessageEnd();
				const auto encoded = ALLOC<byte>(hexSize + 1);
				encoder.Get(encoded, hexSize);
				encoded[hexSize] = '\0';

				FREE<byte>(data);
				data = encoded; // pointer swap
				data[hexSize] = '\0';

				size = hexSize;
				return true;
			}
			catch (const CryptoPP::Exception& ex)
			{
				NET_LOG_ERROR(CSTRING("[NET_HEX][ENCODE] - %s"), ex.what());
				return false;
			}
		}

		static bool ProcessDecode(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
		{
			try
			{
				const auto originalSize = (size / 2);
				CryptoPP::HexDecoder decoder;
				decoder.Put(data, size);
				decoder.MessageEnd();
				const auto decoded = ALLOC<byte>(originalSize + 1);
				decoder.Get(decoded, originalSize);
				decoded[originalSize] = '\0';

				FREE<byte>(data);
				data = decoded; // pointer swap
				data[originalSize] = '\0';

				size = originalSize;
				return true;
			}
			catch (const CryptoPP::Exception& ex)
			{
				NET_LOG_ERROR(CSTRING("[NET_HEX][DECODE] - %s"), ex.what());
				return false;
			}
		}

		static bool ProcessDecode(CryptoPP::byte*& data, size_t& size)
		{
			try
			{
				const auto originalSize = (size / 2);
				CryptoPP::HexDecoder decoder;
				decoder.Put(data, size);
				decoder.MessageEnd();
				const auto decoded = ALLOC<byte>(originalSize + 1);
				decoder.Get(decoded, originalSize);
				decoded[originalSize] = '\0';

				FREE<byte>(data);
				data = decoded; // pointer swap
				data[originalSize] = '\0';

				size = originalSize;
				return true;
			}
			catch (const CryptoPP::Exception& ex)
			{
				NET_LOG_ERROR(CSTRING("[NET_HEX][DECODE] - %s"), ex.what());
				return false;
			}
		}

		bool Hex::encode(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
		{
			return ProcessEncode(data, out, size);
		}

		bool Hex::encode(CryptoPP::byte*& data, size_t& size)
		{
			return ProcessEncode(data, size);
		}

		bool Hex::decode(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
		{
			return ProcessDecode(data, out, size);
		}

		bool Hex::decode(CryptoPP::byte*& data, size_t& size)
		{
			return ProcessDecode(data, size);
		}
	}
}