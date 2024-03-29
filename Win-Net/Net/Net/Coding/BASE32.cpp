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

#include "BASE32.h"

namespace Net
{
	namespace Coding
	{
		static int GetEncode32Length(int bytes)
		{
			int bits = bytes * 8;
			int length = bits / 5;
			if ((bits % 5) > 0)
			{
				length++;
			}
			return length;
		}

		static int GetDecode32Length(int bytes)
		{
			int bits = bytes * 5;
			int length = bits / 8;
			return length;
		}

		static bool Encode32Block(unsigned char* in5, unsigned char* out8)
		{
			// pack 5 bytes
			unsigned __int64 buffer = 0;
			for (int i = 0; i < 5; i++)
			{
				if (i != 0)
				{
					buffer = (buffer << 8);
				}
				buffer = buffer | in5[i];
			}
			// output 8 bytes
			for (int j = 7; j >= 0; j--)
			{
				buffer = buffer << (24 + (7 - j) * 5);
				buffer = buffer >> (24 + (7 - j) * 5);
				unsigned char c = (unsigned char)(buffer >> (j * 5));
				// self check
				if (c >= 32) return false;
				out8[7 - j] = c;
			}
			return true;
		}

		static bool Encode32(unsigned char* in, int inLen, unsigned char* out)
		{
			if ((in == 0) || (inLen <= 0) || (out == 0)) return false;

			int d = inLen / 5;
			int r = inLen % 5;

			unsigned char outBuff[8];

			for (int j = 0; j < d; j++)
			{
				if (!Encode32Block(&in[j * 5], &outBuff[0])) return false;
				memmove(&out[j * 8], &outBuff[0], sizeof(unsigned char) * 8);
			}

			unsigned char padd[5];
			memset(padd, 0, sizeof(unsigned char) * 5);
			for (int i = 0; i < r; i++)
			{
				padd[i] = in[inLen - r + i];
			}
			if (!Encode32Block(&padd[0], &outBuff[0])) return false;
			memmove(&out[d * 8], &outBuff[0], sizeof(unsigned char) * GetEncode32Length(r));

			return true;
		}

		static bool Decode32Block(unsigned char* in8, unsigned char* out5)
		{
			// pack 8 bytes
			unsigned __int64 buffer = 0;
			for (int i = 0; i < 8; i++)
			{
				// input check
				if (in8[i] >= 32) return false;
				if (i != 0)
				{
					buffer = (buffer << 5);
				}
				buffer = buffer | in8[i];
			}
			// output 5 bytes
			for (int j = 4; j >= 0; j--)
			{
				out5[4 - j] = (unsigned char)(buffer >> (j * 8));
			}
			return true;
		}

		static bool Decode32(unsigned char* in, int inLen, unsigned char* out)
		{
			if ((in == 0) || (inLen <= 0) || (out == 0)) return false;

			int d = inLen / 8;
			int r = inLen % 8;

			unsigned char outBuff[5];

			for (int j = 0; j < d; j++)
			{
				if (!Decode32Block(&in[j * 8], &outBuff[0])) return false;
				memmove(&out[j * 5], &outBuff[0], sizeof(unsigned char) * 5);
			}

			unsigned char padd[8];
			memset(padd, 0, sizeof(unsigned char) * 8);
			for (int i = 0; i < r; i++)
			{
				padd[i] = in[inLen - r + i];
			}
			if (!Decode32Block(&padd[0], &outBuff[0])) return false;
			memmove(&out[d * 5], &outBuff[0], sizeof(unsigned char) * GetDecode32Length(r));

			return true;
		}

		static bool Map32(unsigned char* inout32, int inout32Len, unsigned char* alpha32)
		{
			if ((inout32 == 0) || (inout32Len <= 0) || (alpha32 == 0)) return false;
			for (int i = 0; i < inout32Len; i++)
			{
				if (inout32[i] >= 32) return false;
				inout32[i] = alpha32[inout32[i]];
			}
			return true;
		}

		static void ReverseMap(unsigned char* inAlpha32, unsigned char* outMap)
		{
			memset(outMap, 0, sizeof(unsigned char) * 256);
			for (int i = 0; i < 32; i++)
			{
				outMap[(int)inAlpha32[i]] = i;
			}
		}

		static bool Unmap32(unsigned char* inout32, int inout32Len, unsigned char* alpha32)
		{
			if ((inout32 == 0) || (inout32Len <= 0) || (alpha32 == 0)) return false;
			unsigned char rmap[256];
			ReverseMap(alpha32, rmap);
			for (int i = 0; i < inout32Len; i++)
			{
				inout32[i] = rmap[(int)inout32[i]];
			}
			return true;
		}

		bool Base32::encode(byte*& buffer, size_t& len)
		{
			// create a new buffer
			size_t encodeLength = (size_t)GetEncode32Length(static_cast<int>(len));
			byte* data32 = ALLOC<byte>(encodeLength + 1);
			data32[encodeLength] = '\0';
			if (Encode32(buffer, static_cast<int>(len), data32))
			{
				if (Map32(data32, static_cast<int>(encodeLength), NET_BASE32_PATTERN))
				{
					FREE<byte>(buffer);
					buffer = data32;
					len = encodeLength;

					return true;
				}
			}

			FREE<byte>(data32);
			return false;
		}

		bool Base32::encode(byte* buffer, byte*& out, size_t& len)
		{
			// create a new buffer
			size_t encodeLength = (size_t)GetEncode32Length(static_cast<int>(len));
			byte* data32 = ALLOC<byte>(encodeLength + 1);
			data32[encodeLength] = '\0';
			if (Encode32(buffer, static_cast<int>(len), data32))
			{
				if (Map32(data32, static_cast<int>(encodeLength), NET_BASE32_PATTERN))
				{
					FREE<byte>(out);
					out = data32;
					len = encodeLength;

					return true;
				}
			}

			FREE<byte>(data32);
			return false;
		}

		bool Base32::decode(byte*& buffer, size_t& len)
		{
			// create a new buffer
			size_t decodedLength = (size_t)GetDecode32Length(static_cast<int>(len));
			byte* data256 = ALLOC<byte>(decodedLength + 1);
			data256[decodedLength] = '\0';
			if (Unmap32(buffer, static_cast<int>(len), NET_BASE32_PATTERN))
			{
				if (Decode32(buffer, static_cast<int>(len), data256))
				{
					FREE<byte>(buffer);
					buffer = data256;
					len = decodedLength;

					return true;
				}
			}

			FREE<byte>(data256);
			return false;
		}

		bool Base32::decode(byte* buffer, byte*& out, size_t& len)
		{
			// create a new buffer
			size_t decodedLength = (size_t)GetDecode32Length(static_cast<int>(len));
			byte* data256 = ALLOC<byte>(decodedLength + 1);
			data256[decodedLength] = '\0';
			if (Unmap32(buffer, static_cast<int>(len), NET_BASE32_PATTERN))
			{
				if (Decode32(buffer, static_cast<int>(len), data256))
				{
					FREE<byte>(out);
					out = data256;
					len = decodedLength;

					return true;
				}
			}

			FREE<byte>(data256);
			return false;
		}
	}
}