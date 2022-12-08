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

#include "MD5.h"

namespace Net
{
	namespace Coding
	{
		namespace MD5
		{
			__int64 longParse(char* str)
			{
				__int64 result = 0;
				for (int i = 0; i < (signed)strlen(str); i++)
				{
					unsigned digit = str[strlen(str) - 1 - i] - '0';
					result += digit * (__int64)pow((__int64)10, i);
				}
				return result;
			}

			char* createMD5Buffer(__int64 result1, __int64 result2, char challenge[8])
			{
				char* raw_answer = (char*)calloc(MD5_SIZE, sizeof(char));
				raw_answer[0] = ((unsigned char*)&result1)[3];
				raw_answer[1] = ((unsigned char*)&result1)[2];
				raw_answer[2] = ((unsigned char*)&result1)[1];
				raw_answer[3] = ((unsigned char*)&result1)[0];
				raw_answer[4] = ((unsigned char*)&result2)[3];
				raw_answer[5] = ((unsigned char*)&result2)[2];
				raw_answer[6] = ((unsigned char*)&result2)[1];
				raw_answer[7] = ((unsigned char*)&result2)[0];
				for (int i = 0; i < 8; i++)
				{
					raw_answer[8 + i] = challenge[i];
				}

				return raw_answer;
			}

			char* createHash(const char* key1, const char* key2, char* challenge)
			{
				int spaces1 = 0;
				int spaces2 = 0;
				char* digits1 = (char*)calloc(64, sizeof(char));
				char* digits2 = (char*)calloc(64, sizeof(char));
				int d1 = 0, d2 = 0;
				//string digits1, digits2;
				__int64 result1, result2;
				for (int i = 0; i < (signed)strlen(key1); i++)
				{
					if (key1[i] == 0x20)
						spaces1++;
				}
				for (int i = 0; i < (signed)strlen(key2); i++)
				{
					if (key2[i] == 0x20)
						spaces2++;
				}

				for (int i = 0; i < (signed)strlen(key1); i++)
				{
					if (isdigit(key1[i]))
					{
						//digits1 += key1[i];
						digits1[d1++] = key1[i];
					}
				}
				for (int i = 0; i < (signed)strlen(key2); i++)
				{
					if (isdigit(key2[i]))
					{
						//digits2 += key2[i];
						digits2[d2++] = key2[i];
					}
				}

				if (spaces1 == 0)
					result1 = longParse(digits1);
				else
					result1 = longParse(digits1) / spaces1;

				if (spaces2 == 0)
					result2 = longParse(digits2);
				else
					result2 = longParse(digits2) / spaces2;

				char* raw_answer = createMD5Buffer(result1, result2, challenge);

				/* calculate the sig */
				char* sig = (char*)calloc(MD5_SIZE, sizeof(char));

				md5_buffer(raw_answer, MD5_SIZE, sig);	//sig is the MD5 hash

				// Free
				free(digits1);
				free(digits2);
				FREE<char>(raw_answer);

				return sig;
			}
		}
	}
}