



#ifndef clau_parser11_CPP_11_H
#define clau_parser11_CPP_11_H

#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <string>
#include <cstring>
#include <fstream>

#include <algorithm>
#include <utility>
#include <thread>

#ifdef USE_SIMD
#include <intrin.h> // windows, todo : linux - x86intrin
#endif

namespace clau_parser11 {

	namespace LoadDataOption
	{
		constexpr char LineComment = '#';	// # 
		constexpr char Left = '{';  // { [
		constexpr char Right = '}';	// } ]
		constexpr char Assignment = '=';	// = :
	}


	class Utility {
	private:
		class BomInfo
		{
		public:
			size_t bom_size;
			char seq[5];
		};

		const static size_t BOM_COUNT = 1;


		static const BomInfo bomInfo[1];

	public:
		enum class BomType { UTF_8, ANSI };

		static BomType ReadBom(FILE* file) {
			char btBom[5] = { 0, };
			size_t readSize = fread(btBom, sizeof(char), 5, file);


			if (0 == readSize) {
				clearerr(file);
				fseek(file, 0, SEEK_SET);

				return BomType::ANSI;
			}

			BomInfo stBom = { 0, };
			BomType type = ReadBom(btBom, readSize, stBom);

			if (type == BomType::ANSI) { // ansi
				clearerr(file);
				fseek(file, 0, SEEK_SET);
				return BomType::ANSI;
			}

			clearerr(file);
			fseek(file, static_cast<long>(stBom.bom_size * sizeof(char)), SEEK_SET);
			return type;
		}

		static BomType ReadBom(const char* contents, size_t length, BomInfo& outInfo) {
			char btBom[5] = { 0, };
			size_t testLength = length < 5 ? length : 5;
			memcpy(btBom, contents, testLength);

			size_t i, j;
			for (i = 0; i < BOM_COUNT; ++i) {
				const BomInfo& bom = bomInfo[i];

				if (bom.bom_size > testLength) {
					continue;
				}

				bool matched = true;

				for (j = 0; j < bom.bom_size; ++j) {
					if (bom.seq[j] == btBom[j]) {
						continue;
					}

					matched = false;
					break;
				}

				if (!matched) {
					continue;
				}

				outInfo = bom;

				return (BomType)i;
			}

			return BomType::ANSI;
		}


	public:
		static inline bool isWhitespace(const char ch)
		{
			switch (ch)
			{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '\v':
			case '\f':
				return true;
				break;
			}
			return false;
		}


		static inline int Equal(const int64_t x, const int64_t y)
		{
			if (x == y) {
				return 0;
			}
			return -1;
		}

	public:

		// todo - rename.
		static int64_t Get(int64_t position, int64_t length, char ch) {
			int64_t x = (position << 32) + (length << 3) + 0;

			if (length != 1) {
				return x;
			}

			if (LoadDataOption::Left == ch) {
				x += 2; // 010
			}
			else if (LoadDataOption::Right == ch) {
				x += 4; // 100
			}
			else if (LoadDataOption::Assignment == ch) {
				x += 6;
			}

			return x;
		}

		static int64_t GetIdx(int64_t x) {
			return (x >> 32) & 0x00000000FFFFFFFF;
		}
		static int64_t GetLength(int64_t x) {
			return (x & 0x00000000FFFFFFF8) >> 3;
		}
		static int64_t GetType(int64_t x) { //to enum or enum class?
			return (x & 6) >> 1;
		}
		static bool IsToken2(int64_t x) {
			return (x & 1);
		}

		static void PrintToken(const char* buffer, int64_t token) {
			//std::cout << Utility::GetIdx(token) << " " << Utility::GetLength(token) << "\n";
			//std::cout << std::string_view(buffer + Utility::GetIdx(token), Utility::GetLength(token));
		}
	};

	class InFileReserver
	{
	private:

		// use simd - experimental..  - has bug : 2020.10.04
		static void _ScanningWithSimd(char* text, int64_t num, const int64_t length,
			int64_t*& token_arr, size_t& _token_arr_size) {

			size_t token_arr_size = 0;

			{
				int state = 0;

				int64_t token_first = 0;
				int64_t token_last = -1;

				size_t token_arr_count = 0;

				int64_t _i = 0;

#ifdef USE_SIMD
				__m256i temp;
				__m256i _1st, _2nd, _3rd, _4th, _5th, _6th, _7th, _8th, _9th, _10th, _11th, _12th, _13th;

				char ch1 = '\"';
				_1st = _mm256_set_epi8(ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1,
					ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1, ch1);
				char ch2 = '\\';
				_2nd = _mm256_set_epi8(ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2,
					ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2, ch2);
				char ch3 = '\n';
				_3rd = _mm256_set_epi8(ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3,
					ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3, ch3);
				char ch4 = '\0';
				_4th = _mm256_set_epi8(ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4,
					ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4, ch4);
				char ch5 = '#';
				_5th = _mm256_set_epi8(ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5,
					ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5, ch5);
				char ch6 = ' ';
				_6th = _mm256_set_epi8(ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6,
					ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6, ch6);
				char ch7 = '\t';
				_7th = _mm256_set_epi8(ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7,
					ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7, ch7);
				char ch8 = '\r';
				_8th = _mm256_set_epi8(ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8,
					ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8, ch8);
				char ch9 = '\v';
				_9th = _mm256_set_epi8(ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9,
					ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9, ch9);
				char ch10 = '\f';
				_10th = _mm256_set_epi8(ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10,
					ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10, ch10);

				char ch11 = '{';
				_11th = _mm256_set_epi8(ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11,
					ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11, ch11);

				char ch12 = '}';
				_12th = _mm256_set_epi8(ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12,
					ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12, ch12);

				char ch13 = '=';
				_13th = _mm256_set_epi8(ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13,
					ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13, ch13);



				__m256i mask1, mask2, mask3, mask4, mask5;
				int val = -7; // 111
				mask1 = _mm256_set_epi8(val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val,
					val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val);

				val = -2; // 010
				mask2 = _mm256_set_epi8(val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val,
					val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val);

				val = -5; // 101
				mask3 = _mm256_set_epi8(val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val,
					val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val);

				val = -10; // 1010
				mask4 = _mm256_set_epi8(val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val,
					val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val);

				val = -15; // 1111
				mask5 = _mm256_set_epi8(val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val,
					val, val, val, val, val, val, val, val, val, val, val, val, val, val, val, val);

				for (; _i + 32 < length; _i = _i + 32) {
					temp = _mm256_setr_epi8(text[_i], text[_i + 1], text[_i + 2], text[_i + 3], text[_i + 4], text[_i + 5], text[_i + 6], text[_i + 7],
						text[_i + 8], text[_i + 9], text[_i + 10], text[_i + 11], text[_i + 12], text[_i + 13], text[_i + 14], text[_i + 15], text[_i + 16],
						text[_i + 17], text[_i + 18], text[_i + 19], text[_i + 20], text[_i + 21], text[_i + 22], text[_i + 23], text[_i + 24], text[_i + 25],
						text[_i + 26], text[_i + 27], text[_i + 28], text[_i + 29], text[_i + 30], text[_i + 31]);

					__m256i x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13;

					x1 = _mm256_cmpeq_epi8(temp, _1st);
					x2 = _mm256_cmpeq_epi8(temp, _2nd);
					x3 = _mm256_cmpeq_epi8(temp, _3rd);
					x4 = _mm256_cmpeq_epi8(temp, _4th);
					x5 = _mm256_cmpeq_epi8(temp, _5th);
					x6 = _mm256_cmpeq_epi8(temp, _6th);
					x7 = _mm256_cmpeq_epi8(temp, _7th);
					x8 = _mm256_cmpeq_epi8(temp, _8th);
					x9 = _mm256_cmpeq_epi8(temp, _9th);
					x10 = _mm256_cmpeq_epi8(temp, _10th);
					x11 = _mm256_cmpeq_epi8(temp, _11th);
					x12 = _mm256_cmpeq_epi8(temp, _12th);
					x13 = _mm256_cmpeq_epi8(temp, _13th);

					x1 = _mm256_blendv_epi8(x1, mask5, x1);
					x2 = _mm256_blendv_epi8(x2, mask4, x2);
					x3 = _mm256_blendv_epi8(x3, mask5, x3);
					x4 = _mm256_blendv_epi8(x4, mask5, x4);
					x5 = _mm256_blendv_epi8(x5, mask5, x5);
					x6 = _mm256_blendv_epi8(x6, mask3, x6);
					x7 = _mm256_blendv_epi8(x7, mask3, x7);
					x8 = _mm256_blendv_epi8(x8, mask3, x8);
					x9 = _mm256_blendv_epi8(x9, mask3, x9);
					x10 = _mm256_blendv_epi8(x10, mask3, x10);
					x11 = _mm256_blendv_epi8(x11, mask1, x11);
					x12 = _mm256_blendv_epi8(x12, mask1, x12);
					x13 = _mm256_blendv_epi8(x13, mask1, x13);


					x1 = _mm256_add_epi8(x1, x2);
					x3 = _mm256_add_epi8(x3, x4);
					x5 = _mm256_add_epi8(x5, x6);
					x7 = _mm256_add_epi8(x7, x8);
					x9 = _mm256_add_epi8(x9, x10);
					x11 = _mm256_add_epi8(x11, x12);

					x1 = _mm256_add_epi8(x1, x3);
					x5 = _mm256_add_epi8(x5, x7);
					x9 = _mm256_add_epi8(x9, x11);

					x1 = _mm256_add_epi8(x1, x5);
					x9 = _mm256_add_epi8(x9, x13);

					x1 = _mm256_add_epi8(x1, x9);

					int start = 0;
					int r = _mm256_movemask_epi8(x1);

					while (r != 0) {
						{
							int a = _tzcnt_u32(r); // 

							r = r & (r - 1);

							start = a;

							{
								const int64_t i = _i + start;

								if (((-x1.m256i_i8[start]) & 0b100) != 0) {
									token_last = i - 1;
									if (token_last - token_first + 1 > 0) {
										token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
										token_arr_count++;
									}

									token_first = i;
									token_last = i;
								}
								if (((-x1.m256i_i8[start]) & 0b010) != 0) {
									{//
										if (((-x1.m256i_i8[start]) & 0b1000) != 0) {
											token_arr[num + token_arr_count] = 1;
										}
										else {
											token_arr[num + token_arr_count] = 0;
										}
										const char ch = text[i];
										token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
										token_arr_count++;
									}
								}
								if (((-x1.m256i_i8[start]) & 0b001) != 0) {
									token_first = i + 1;
									token_last = i + 1;
								}


								continue;
							}
						}
					}
				}

#endif

				//default?
				for (; _i < length; _i = _i + 1) {
					int64_t i = _i;
					const char ch = text[i];

					switch (ch) {
					case '\"':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}

						token_first = i;
						token_last = i;

						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
							token_arr_count++;
						}
						break;
					case '\\':
					{//
						token_arr[num + token_arr_count] = 1;
						token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
						token_arr_count++;
					}
					break;
					case '\n':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
							token_arr_count++;
						}
						break;
					case '\0':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
							token_arr_count++;
						}
						break;
					case LoadDataOption::LineComment:
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
							token_arr_count++;
						}

						break;
					case ' ':
					case '\t':
					case '\r':
					case '\v':
					case '\f':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						break;
					case LoadDataOption::Left:
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}

						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					case LoadDataOption::Right:
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					case LoadDataOption::Assignment:
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					}

				}

				if (length - 1 - token_first + 1 > 0) {
					token_arr[num + token_arr_count] = Utility::Get(token_first + num, length - 1 - token_first + 1, text[token_first]);
					token_arr_count++;
				}
				token_arr_size = token_arr_count;
			}

			{
				_token_arr_size = token_arr_size;
			}
		}

		static void _Scanning(char* text, int64_t num, const int64_t length,
			int64_t*& token_arr, size_t& _token_arr_size) {

			size_t token_arr_size = 0;

			{
				int state = 0;

				int64_t token_first = 0;
				int64_t token_last = -1;

				size_t token_arr_count = 0;

				for (int64_t i = 0; i < length; ++i) {

					const char ch = text[i];

					switch (ch) {
					case '\"':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}

						token_first = i;
						token_last = i;

						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
							token_arr_count++;
						}
						break;
					case '\\':
					{//
						token_arr[num + token_arr_count] = 1;
						token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
						token_arr_count++;
					}
					break;
					case '\n':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
							token_arr_count++;
						}
						break;
					case '\0':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
							token_arr_count++;
						}
						break;
					case  LoadDataOption::LineComment:
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Utility::Get(i + num, 1, ch);
							token_arr_count++;
						}

						break;
					case ' ':
					case '\t':
					case '\r':
					case '\v':
					case '\f':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						break;
					case LoadDataOption::Left:
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}

						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					case LoadDataOption::Right:
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					case LoadDataOption::Assignment:
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
							token_arr_count++;
						}
						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Utility::Get(token_first + num, token_last - token_first + 1, text[token_first]);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					}

				}

				if (length - 1 - token_first + 1 > 0) {
					token_arr[num + token_arr_count] = Utility::Get(token_first + num, length - 1 - token_first + 1, text[token_first]);
					token_arr_count++;
				}
				token_arr_size = token_arr_count;
			}

			{
				_token_arr_size = token_arr_size;
			}
		}


		static void ScanningNew(char* text, const size_t length, const int thr_num,
			int64_t*& _token_arr, size_t& _token_arr_size, bool use_simd)
		{
			std::vector<std::thread> thr(thr_num);
			std::vector<size_t> start(thr_num);
			std::vector<size_t> last(thr_num);

			{
				start[0] = 0;

				for (int i = 1; i < thr_num; ++i) {
					start[i] = length / thr_num * i;

					for (size_t x = start[i]; x <= length; ++x) {
						if (Utility::isWhitespace(text[x]) || '\0' == text[x] ||
							LoadDataOption::Left == text[x] || LoadDataOption::Right == text[x] || LoadDataOption::Assignment == text[x]) {
							start[i] = x;
							break;
						}
					}
				}
				for (int i = 0; i < thr_num - 1; ++i) {
					last[i] = start[i + 1];
					for (size_t x = last[i]; x <= length; ++x) {
						if (Utility::isWhitespace(text[x]) || '\0' == text[x] ||
							LoadDataOption::Left == text[x] || LoadDataOption::Right == text[x] || LoadDataOption::Assignment == text[x]) {
							last[i] = x;
							break;
						}
					}
				}
				last[thr_num - 1] = length + 1;
			}
			size_t real_token_arr_count = 0;

			int64_t* tokens = new int64_t[length + 1];
			int64_t token_count = 0;

			std::vector<size_t> token_arr_size(thr_num);
			auto a = std::chrono::steady_clock::now();
			for (int i = 0; i < thr_num; ++i) {
				if (use_simd) {
					thr[i] = std::thread(_ScanningWithSimd, text + start[i], start[i], last[i] - start[i], std::ref(tokens), std::ref(token_arr_size[i]));
				}
				else {
					thr[i] = std::thread(_Scanning, text + start[i], start[i], last[i] - start[i], std::ref(tokens), std::ref(token_arr_size[i]));
				}
			}

			for (int i = 0; i < thr_num; ++i) {
				thr[i].join();
			}
			auto b = std::chrono::steady_clock::now();
			int state = 0;
			int64_t qouted_start;
			int64_t slush_start;

			for (size_t t = 0; t < thr_num; ++t) {
				for (size_t j = 0; j < token_arr_size[t]; ++j) {

					const int64_t i = start[t] + j;

					//std::cout << tokens[i] << "\n";
					//Utility::PrintToken(text, tokens[i]);
					//
					const int64_t len = Utility::GetLength(tokens[i]);
					const char ch = text[Utility::GetIdx(tokens[i])];
					const int64_t idx = Utility::GetIdx(tokens[i]);
					const bool isToken2 = Utility::IsToken2(tokens[i]);

					if (isToken2) {
						if (0 == state && '\"' == ch) {
							state = 1;
							qouted_start = i;
						}
						else if (0 == state && LoadDataOption::LineComment == ch) {
							state = 2;
						}
						else if (1 == state && '\\' == ch) {
							state = 3;
							slush_start = idx;
						}
						else if (1 == state && '\"' == ch) {
							state = 0;

							{
								int64_t idx = Utility::GetIdx(tokens[qouted_start]);
								int64_t len = Utility::GetLength(tokens[qouted_start]);

								len = Utility::GetIdx(tokens[i]) - idx + 1;

								tokens[real_token_arr_count] = Utility::Get(idx, len, text[idx]);

								//	Utility::PrintToken(text, tokens[real_token_arr_count]);


								real_token_arr_count++;
							}
						}
						else if (3 == state) {
							if (idx != slush_start + 1) {
								--j; // --i;
							}
							state = 1;
						}
						else if (2 == state && ('\n' == ch || '\0' == ch)) {
							state = 0;
						}
					}
					else if (0 == state) { // 
						tokens[real_token_arr_count] = tokens[i];

						//	Utility::PrintToken(text, tokens[real_token_arr_count]);

						real_token_arr_count++;
					}
				}
			}
			//auto c = std::chrono::steady_clock::now();
			//auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(b - a);
			//auto dur2 = std::chrono::duration_cast<std::chrono::milliseconds>(c - b);

			//std::cout << dur.count() << "ms\n";
			//std::cout << dur2.count() << "ms\n";

			{
				if (0 != state) {
					std::cout << "[ERROR] state [" << state << "] is not zero \n";
				}
			}


			{
				_token_arr = tokens;
				_token_arr_size = real_token_arr_count;
			}
		}


		static void Scanning(char* text, const size_t length,
			int64_t*& _token_arr, size_t& _token_arr_size) {

			int64_t* token_arr = new int64_t[length + 1];
			size_t token_arr_size = 0;

			{
				int state = 0;

				int64_t token_first = 0;
				int64_t token_last = -1;

				size_t token_arr_count = 0;

				for (size_t i = 0; i <= length; ++i) {
					const char ch = text[i];

					if (0 == state) {
						if (LoadDataOption::LineComment == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
								token_arr_count++;
							}

							state = 3;
						}
						else if ('\"' == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
								token_arr_count++;
							}

							token_first = i;
							token_last = i;

							state = 1;
						}
						else if (Utility::isWhitespace(ch) || '\0' == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
								token_arr_count++;
							}
							token_first = i + 1;
							token_last = i + 1;
						}
						else if (LoadDataOption::Left == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
								token_arr_count++;
							}

							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;
						}
						else if (LoadDataOption::Right == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
								token_arr_count++;
							}
							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;

						}
						else if (LoadDataOption::Assignment == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
								token_arr_count++;
							}
							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;
						}
					}
					else if (1 == state) {
						if ('\\' == ch) {
							state = 2;
						}
						else if ('\"' == ch) {
							token_last = i;

							token_arr[token_arr_count] = Utility::Get(token_first, token_last - token_first + 1, text[token_first]);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;

							state = 0;
						}
					}
					else if (2 == state) {
						state = 1;
					}
					else if (3 == state) {
						if ('\n' == ch || '\0' == ch) {
							state = 0;

							token_first = i + 1;
							token_last = i + 1;
						}
					}
				}

				token_arr_size = token_arr_count;

				if (0 != state) {
					std::cout << "[" << state << "] state is not zero.\n";
				}
			}

			{
				_token_arr = token_arr;
				_token_arr_size = token_arr_size;
			}
		}


		static std::pair<bool, int> Scan(FILE* inFile, int thr_num,
			char*& _buffer, size_t* _buffer_len, int64_t*& _token_arr, size_t* _token_arr_len, bool use_simd)
		{
			if (inFile == nullptr) {
				return { false, 0 };
			}

			int64_t* arr_count = nullptr; //
			size_t arr_count_size = 0;

			std::string temp;
			char* buffer = nullptr;
			size_t file_length;

			{
				fseek(inFile, 0, SEEK_END);
				size_t length = ftell(inFile);
				fseek(inFile, 0, SEEK_SET);

				Utility::BomType x = Utility::ReadBom(inFile);

				//	clau_parser11::Out << "length " << length << "\n";
				if (x == Utility::BomType::UTF_8) {
					length = length - 3;
				}

				file_length = length;
				buffer = new char[file_length + 1]; // 

				//int a = clock();
				// read data as a block:
				fread(buffer, sizeof(char), file_length, inFile);
				//int b = clock();
				//std::cout << b - a << " " << file_length <<"\n";

				buffer[file_length] = '\0';

				{
					int64_t* token_arr;
					size_t token_arr_size;

					if (thr_num == 1) {
						Scanning(buffer, file_length, token_arr, token_arr_size);
					}
					else {
						ScanningNew(buffer, file_length, thr_num, token_arr, token_arr_size, use_simd);
					}

					_buffer = buffer;
					_token_arr = token_arr;
					*_token_arr_len = token_arr_size;
					*_buffer_len = file_length;
				}
			}

			return{ true, 1 };
		}

	private:
		FILE* pInFile;
		bool use_simd;
	public:
		explicit InFileReserver(FILE* inFile, bool use_simd)
		{
			pInFile = inFile;
			this->use_simd = use_simd;
		}
	public:
		bool operator() (int thr_num, char*& buffer, size_t* buffer_len, int64_t*& token_arr, size_t* token_arr_len)
		{
			bool x = Scan(pInFile, thr_num, buffer, buffer_len, token_arr, token_arr_len, use_simd).second > 0;

			return x;
		}
	};

	class Type {
	protected:
		std::string name;

	public:
		explicit Type() { }
		explicit Type(const char* str, size_t len) : name(std::string(str, len)) { }
		explicit Type(const std::string& name, const bool valid = true) : name(name) { }//chk();  }
		explicit Type(std::string&& name, const bool valid = true) : name(move(name)) { }//chk(); }
		Type(const Type& type)
			: name(type.name)
		{
			//chk();
		}
		virtual ~Type() { }
		bool IsFail() const { // change body
			return "" == name;
		}
		std::string& GetName() {
			return name;
		}
		const std::string& GetName() const {
			return name;
		}
		void SetName(const std::string& name)
		{
			this->name = name;

			//chk();
		}
		void SetName(std::string&& name)
		{
			this->name = name;

			//chk();
		}
		bool operator==(const Type& t) const {
			return name == t.name;
		}
		bool operator<(const Type& t) const {
			return name < t.name;
		}
		Type& operator=(const Type& type)
		{
			name = type.name;
			return *this;
		}
		void operator=(Type&& type) noexcept
		{
			name = std::move(type.name);
		}
	};

	template < class T >
	class ItemType : public Type {
	public:
		typedef T item_type; //
	private:
		T data;
		bool inited;
	public:
		ItemType(const ItemType<T>& ta) : Type(ta), data(ta.data), inited(ta.inited)
		{

		}
		ItemType(ItemType<T>&& ta) noexcept : Type(std::move(ta))
		{
			data = std::move(ta.data);
			inited = ta.inited;
		}
	public:
		explicit ItemType()
			: Type("", 0), inited(false) { }
		explicit ItemType(const std::string& name, const T& value, const bool valid = true)
			:Type(name, valid), data(value), inited(true)
		{
			//
		}
		explicit ItemType(std::string&& name, T&& value, const bool valid = true)
			:Type(std::move(name), valid), data(move(value)), inited(true)
		{
			//
		}
		explicit ItemType(const char* name, size_t name_len, const char* value, size_t value_len)
			:Type(name, name_len), data(value, value_len), inited(true)
		{
			//
		}
		virtual ~ItemType() { }
	public:
		void Remove(const size_t  idx = 0)
		{
			data = T();
			inited = false;
		}
		bool Push(const T& val) { /// do not change..!!
			if (inited) { throw "ItemType already inited"; }
			data = val;
			inited = true;

			return true;
		}
		bool Push(T&& val) {
			if (inited) { throw "ItemType already inited"; }
			data = std::move(val);
			inited = true;

			return true;
		}
		T& Get(const size_t index = 0) {
			if (!inited) {
				throw "ItemType, not inited";
			}
			return data;
		}
		const T& Get(const size_t index = 0) const {
			if (!inited) {
				throw "ItemType, not inited";
			}
			return data;
		}
		void Set(const size_t index, const T& val) {
			if (!inited) {
				throw "ItemType, not inited";
			} // removal?
			data = val;
		}
		void Set(const size_t index, T&& val) {
			if (!inited) {
				throw "ItemType, not inited";
			} // removal?
			data = std::move(val);
		}
		size_t size()const {
			return inited ? 1 : 0;
		}
		bool empty()const { return !inited; }
		std::string ToString()const
		{
			if (Type::GetName().empty()) {
				return this->Get(0);
			}
			return Type::GetName() + " = " + this->Get(0);
		}
	public:
		ItemType<T>& operator=(const ItemType<T>& ta)
		{
			Type::operator=(ta);
			ItemType<T> temp = ta;

			data = std::move(temp.data);
			inited = temp.inited;
			return *this;
		}
		void operator=(ItemType<T>&& ta) noexcept
		{
			Type::operator=(ta);
			if (data == ta.data) { return; }

			data = std::move(ta.data);
			inited = ta.inited;
			return;
		}

		Type* ToType() {
			return this;
		}
		const Type* ToType()const {
			return this;
		}

	};

	template <class T>
	class WrapType {
	public:
		T ptr;
		size_t idx;
	public:
		WrapType(T ptr, size_t idx) : ptr(ptr), idx(idx) { }
	};

	class UserType : public Type {
	private:
		class UserTypeCompare
		{
		public:
			bool operator() (const WrapType<UserType*> x, const WrapType<UserType*> y) const {
				return x.ptr->GetName() < y.ptr->GetName();
			}
		};
		class ItemTypeStringPtrCompare {
		public:
			bool operator() (const WrapType<ItemType<std::string>*> x, const WrapType<ItemType<std::string>*> y) const {
				return x.ptr->GetName() < y.ptr->GetName();
			}
		};

		int Dif(const std::string& x, const std::string& y) const {
			return x.compare(y); // strcmp
		}
		size_t binary_find_ut(const std::vector<WrapType<UserType*>>& arr, const UserType& x, bool& err) const
		{
			err = false;

			if (arr.empty()) { err = true;  return -1; }

			size_t left = 0, right = arr.size() - 1;
			size_t middle = (left + right) / 2;

			while (left <= right) {
				const int dif = Dif(arr[middle].ptr->GetName(), x.GetName());

				if (dif == 0) { //arr[middle]->GetName() == x.GetName()) {
					return middle;
				}
				else if (dif < 0) { //arr[middle]->GetName() < x.GetName()) {
					left = middle + 1;
				}
				else {
					if (0 == middle) {
						break;
					}
					right = middle - 1;
				}

				middle = (left + right) / 2;
			}
			err = true;
			return -1;
		}

		size_t binary_find_it(const std::vector<WrapType<ItemType<std::string>*>>& arr, const ItemType<std::string>& x, bool& err) const {
			err = false;
			if (arr.empty()) { err = true;  return -1; }

			size_t left = 0, right = arr.size() - 1;
			size_t middle = (left + right) / 2;

			while (left <= right) {
				const int dif = Dif(arr[middle].ptr->GetName(), x.GetName());

				if (dif == 0) { //arr[middle]->GetName() == x.GetName()) {
					return middle;
				}
				else if (dif < 0) { //arr[middle]->GetName() < x.GetName()) {
					left = middle + 1;
				}
				else {
					if (0 == middle) {
						break;
					}
					right = middle - 1;
				}

				middle = (left + right) / 2;
			}

			err = true;
			return -1;
		}

	public:
		size_t GetIListSize()const { return ilist.size(); }
		size_t GetItemListSize()const { return itemList.size(); }
		size_t GetUserTypeListSize()const { return userTypeList.size(); }
		ItemType<std::string>& GetItemList(const size_t idx) { return itemList[idx]; }
		const ItemType<std::string>& GetItemList(const size_t idx) const { return itemList[idx]; }
		UserType*& GetUserTypeList(const size_t idx) { return userTypeList[idx]; }
		const UserType*& GetUserTypeList(const size_t idx) const { return const_cast<const UserType*&>(userTypeList[idx]); }

		Type* ToType() {
			return this;
		}
		const Type* ToType()const {
			return this;
		}

		bool IsItemList(const size_t idx) const
		{
			return ilist[idx] == 1;
		}
		bool IsUserTypeList(const size_t idx) const
		{
			return ilist[idx] == 2;
		}
		/*
		void AddItemList(const ItemType<std::string>& strTa)
		{
			for (int i = 0; i < strTa.size(); ++i) {
				this->AddItem(strTa.GetName(), strTa.Get(i));
			}
		}
		void AddItemList(ItemType<std::string>&& strTa)
		{
			for (int i = 0; i < strTa.size(); ++i) {
				this->AddItem(std::move(strTa.GetName()), std::move(strTa.Get(i)));
			}
		}
		*/
	public:
		void Clear() {
			itemList.clear();
			ilist.clear();
			userTypeList.clear();

			sortedItemList.clear();
			sortedUserTypeList.clear();
		}

		void SetParent(UserType* other)
		{
			parent = other;
		}
		UserType* GetParent() { return parent; }
		const UserType* GetParent()const { return parent; }

		void LinkUserType(UserType* ut) // friend?
		{
			userTypeList.push_back(ut);
			ilist.push_back(2);
			ut->parent = this;

			useSortedUserTypeList = false;
		}
	private:
		UserType* parent = nullptr;
		std::vector<int> ilist;
		std::vector< ItemType<std::string> > itemList;
		std::vector< UserType* > userTypeList;
		mutable std::vector< WrapType<ItemType<std::string>*> > sortedItemList;
		mutable std::vector< WrapType<UserType*> > sortedUserTypeList;
		mutable bool useSortedItemList = false;
		mutable bool useSortedUserTypeList = false;
	public:
		explicit UserType(const char* str, size_t len) : Type(std::string(str, len)), parent(nullptr) { }
		explicit UserType(std::string&& name) : Type(std::move(name)), parent(nullptr) { }
		explicit UserType(const std::string& name = "") : Type(name), parent(nullptr) { }
		UserType(const UserType& ut) : Type(ut.GetName()) {
			Reset(ut);  // Initial
		}
		UserType(UserType&& ut) noexcept : Type(std::move(ut.GetName())) {
			Reset2(std::move(ut));
		}
		virtual ~UserType() {
			_Remove();
		}
		UserType& operator=(const UserType& ut) {
			if (this == &ut) { return *this; }
			Type::operator=(ut);

			RemoveUserTypeList();
			Reset(ut);
			return *this;
		}
		void operator=(UserType&& ut) noexcept {
			if (this == &ut) { return; }

			Type::operator=(ut);
			RemoveUserTypeList();
			Reset2(std::move(ut));
			return;
		}
		class Wrap {
		public:
			UserType* ut = nullptr;
			size_t idx = 0;
			size_t max = 0;
		public:
			Wrap() {
				//
			}
			Wrap(UserType* ut) :
				ut(ut)
			{
				max = ut->GetUserTypeListSize();
			}
		};
		class Wrap2 {
		public:
			const UserType* ut = nullptr;
			size_t idx = 0;
			size_t max = 0;
		public:
			Wrap2() {
				//
			}
			Wrap2(const UserType* ut) :
				ut(ut)
			{
				max = ut->GetUserTypeListSize();
			}
		};

		static void Delete(void* ptr) {
			std::vector<Wrap> _stack;
			_stack.push_back(Wrap((UserType*)ptr));

			while (!_stack.empty()) {
				if (_stack.back().idx >= _stack.back().max) {
					_stack.back().ut->userTypeList.clear();
					delete _stack.back().ut;

					_stack.pop_back();
					if (_stack.empty()) {
						break;
					}
					_stack.back().idx++;
					continue;
				}

				_stack.push_back(Wrap(_stack.back().ut->GetUserTypeList(_stack.back().idx)));
			}
		}

	private:
		void Reset(const UserType& ut) {
			std::vector<UserType*> _stack;
			std::vector<Wrap2> _stack2;

			_stack.push_back(this);
			_stack2.push_back(Wrap2((const UserType*)&ut));

			while (!_stack2.empty()) {
				if (_stack2.back().idx >= _stack2.back().max) {
					{
						_stack.back()->sortedItemList.clear();
						_stack.back()->sortedUserTypeList.clear();
						_stack.back()->name = _stack2.back().ut->name;
						_stack.back()->ilist = _stack2.back().ut->ilist;
						_stack.back()->itemList = _stack2.back().ut->itemList;
						_stack.back()->useSortedItemList = false; // ut.useSortedItemList;
						_stack.back()->useSortedUserTypeList = false; //ut.useSortedUserTypeList;
					}

					{
						UserType* child = _stack.back();

						_stack.pop_back();

						if (!_stack.empty()) {
							_stack.back()->LinkUserType(child);
						}
					}

					_stack2.pop_back();
					if (_stack2.empty()) {
						break;
					}
					_stack2.back().idx++;
					continue;
				}

				{
					UserType* child = new UserType(_stack2.back().ut->GetName());
					_stack.push_back(child);
				}
				_stack2.push_back(Wrap2(_stack2.back().ut->GetUserTypeList(_stack2.back().idx)));
				_stack.back()->ReserveUserTypeList(_stack2.back().ut->GetUserTypeListSize());
			}
		}
		void Reset2(UserType&& ut) {
			ilist = std::move(ut.ilist);
			itemList = std::move(ut.itemList);

			//sortedItemList = std::move(ut.sortedItemList);
			//sortedUserTypeList = std::move(ut.sortedUserTypeList);

			useSortedItemList = false; // ut.useSortedItemList;
			useSortedUserTypeList = false; // ut.useSortedUserTypeList;

			userTypeList.reserve(ut.userTypeList.size());

			for (size_t i = 0; i < ut.userTypeList.size(); ++i) {
				userTypeList.push_back(std::move(ut.userTypeList[i]));
				ut.userTypeList[i] = nullptr;
				userTypeList.back()->parent = this;
			}
			ut.userTypeList.clear();


			sortedItemList.clear();
			sortedUserTypeList.clear();

			ut.sortedItemList.clear();
			ut.sortedUserTypeList.clear();
		}

		void _Remove()
		{
			//parent = nullptr;
			ilist = std::vector<int>();
			itemList = std::vector< ItemType<std::string> >();

			sortedItemList.clear();
			sortedUserTypeList.clear();

			useSortedItemList = false;
			useSortedUserTypeList = false;
			RemoveUserTypeList();
		}
	public:
		size_t  GetIlistIndex(const size_t  index, const int type, bool& err)
		{
			return _GetIlistIndex(ilist, index, type, err);
		}
		size_t GetUserTypeIndexFromIlistIndex(const size_t ilist_idx, bool& err)
		{
			return _GetUserTypeIndexFromIlistIndex(ilist, ilist_idx, err);
		}
		size_t GetItemIndexFromIlistIndex(const size_t  ilist_idx, bool& err)
		{
			return _GetItemIndexFromIlistIndex(ilist, ilist_idx, err);
		}
	private:
		/// val : 1 or 2
		size_t  _GetIndex(const std::vector<int>& ilist, const int val, bool& err, const size_t  start = 0) {
			err = false;
			for (size_t i = start; i < ilist.size(); ++i) {
				if (ilist[i] == val) { return i; }
			}
			err = true;
			return -1;
		}
		// test? - need more thinking!
		size_t  _GetItemIndexFromIlistIndex(const std::vector<int>& ilist, const size_t  ilist_idx, bool& _err) {
			if (ilist.size() == ilist_idx) { return ilist.size(); }
			bool err = false;
			_err = false;
			size_t idx = _GetIndex(ilist, 1, err, 0);
			size_t item_idx = -1;

			while (!err) {
				item_idx++;
				if (ilist_idx == idx) { return item_idx; }
				idx = _GetIndex(ilist, 1, err, idx + 1);
			}
			_err = true;
			return -1;
		}
		size_t  _GetUserTypeIndexFromIlistIndex(const std::vector<int>& ilist, const size_t  ilist_idx, bool& _err) {
			_err = false;
			bool err = false;
			if (ilist.size() == ilist_idx) { return ilist.size(); }
			size_t  idx = _GetIndex(ilist, 2, err, 0);
			size_t  usertype_idx = -1;

			while (!err) {
				usertype_idx++;
				if (ilist_idx == idx) { return usertype_idx; }
				idx = _GetIndex(ilist, 2, err, idx + 1);
			}
			_err = true;
			return -1;
		}
		/// type : 1 or 2
		size_t _GetIlistIndex(const std::vector<int>& ilist, const size_t  index, const int type, bool& err) {
			size_t  count = -1;
			err = false;
			for (size_t i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == type) {
					count++;
					if (index == count) {
						return i;
					}
				}
			}
			err = true;
			return -1;
		}
	public:
		void RemoveItemList(const size_t  idx)
		{
			// left shift start idx, to end, at itemList. and resize!
			for (size_t i = idx + 1; i < GetItemListSize(); ++i) {
				itemList[i - 1] = std::move(itemList[i]);
			}
			itemList.resize(itemList.size() - 1);
			//  ilist left shift and resize - count itemType!
			size_t count = 0;
			for (size_t i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == 1) { count++; }
				if (count == idx + 1) {
					// left shift!and resize!
					for (size_t k = i + 1; k < ilist.size(); ++k) {
						ilist[k - 1] = std::move(ilist[k]);
					}
					ilist.resize(ilist.size() - 1);
					break;
				}
			}

			useSortedItemList = false;
		}
		void RemoveUserTypeList(const size_t idx, const bool chk = true)
		{
			if (chk && userTypeList[idx]) {
				delete userTypeList[idx];
			}

			// left shift start idx, to end, at itemList. and resize!
			for (size_t i = idx + 1; i < GetUserTypeListSize(); ++i) {
				userTypeList[i - 1] = std::move(userTypeList[i]);
			}
			userTypeList.resize(userTypeList.size() - 1);
			//  ilist left shift and resize - count itemType!
			size_t  count = 0;
			for (size_t i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == 2) { count++; }
				if (count == idx + 1) {
					// left shift! and resize!
					for (size_t k = i + 1; k < ilist.size(); ++k) {
						ilist[k - 1] = std::move(ilist[k]);
					}
					ilist.resize(ilist.size() - 1);
					break;
				}
			}

			useSortedUserTypeList = false;
		}
		void RemoveItemList(const std::string& varName)
		{
			bool err = false;
			size_t k = _GetIndex(ilist, 1, err, 0);

			std::vector<ItemType<std::string>> tempDic;
			for (size_t i = 0; i < itemList.size(); ++i) {
				if (varName != itemList[i].GetName()) {
					tempDic.push_back(itemList[i]);
					k = _GetIndex(ilist, 1, err, k + 1);
				}
				else {
					// remove item, ilist left shift 1.
					for (size_t j = k + 1; j < ilist.size(); ++j) {
						ilist[j - 1] = ilist[j];
					}
					ilist.resize(ilist.size() - 1);
					k = _GetIndex(ilist, 1, err, k);
				}
			}
			itemList = std::move(tempDic);

			useSortedItemList = false;
		}
		void RemoveItemList() /// ALL
		{
			itemList = std::vector<ItemType<std::string>>();
			//
			std::vector<int> temp;
			for (size_t i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == 2)
				{
					temp.push_back(2);
				}
			}
			ilist = move(temp);

			useSortedItemList = false;
		}
		void RemoveEmptyItem()
		{
			bool err = false;
			size_t k = _GetIndex(ilist, 1, err, 0);
			std::vector<ItemType<std::string>> tempDic;
			for (size_t i = 0; i < itemList.size(); ++i) {
				if (itemList[i].size() > 0) {
					tempDic.push_back(itemList[i]);
					k = _GetIndex(ilist, 1, err, k + 1);
				}
				else {
					// remove item, ilist left shift 1.
					for (size_t j = k + 1; j < ilist.size(); ++j) {
						ilist[j - 1] = ilist[j];
					}
					ilist.resize(ilist.size() - 1);
					k = _GetIndex(ilist, 1, err, k);
				}
			}
			itemList = move(tempDic);

			useSortedItemList = false;
		}
		void Remove()
		{
			ilist = std::vector<int>();
			itemList = std::vector< ItemType<std::string> >();

			RemoveUserTypeList();

			sortedItemList.clear();
			sortedUserTypeList.clear();

			useSortedItemList = false;
			useSortedUserTypeList = false;
		}
		void RemoveUserTypeList() {
			for (size_t i = 0; i < userTypeList.size(); i++) {
				if (nullptr != userTypeList[i]) {
					Delete(userTypeList[i]); //
					userTypeList[i] = nullptr;
				}
			}

			userTypeList.clear();

			std::vector<int> temp;
			for (size_t i = 0; i < ilist.size(); ++i) {
				if (ilist[i] == 1)
				{
					temp.push_back(1);
				}
			}
			ilist = move(temp);

			useSortedUserTypeList = false;
		}
		void RemoveUserTypeList(const std::string& varName, const bool chk = true)
		{
			bool err = false;
			size_t k = _GetIndex(ilist, 2, err, 0);
			std::vector<UserType*> tempDic;
			for (size_t i = 0; i < userTypeList.size(); ++i) {
				if (userTypeList[i] && varName != userTypeList[i]->GetName()) {
					tempDic.push_back(userTypeList[i]);
					k = _GetIndex(ilist, 2, err, k + 1);
				}
				else {
					if (chk && userTypeList[i]) {
						delete userTypeList[i];
					}
					// remove usertypeitem, ilist left shift 1.
					for (size_t j = k + 1; j < ilist.size(); ++j) {
						ilist[j - 1] = ilist[j];
					}
					ilist.resize(ilist.size() - 1);
					k = _GetIndex(ilist, 2, err, k);
				}
			}
			userTypeList = move(tempDic);

			useSortedUserTypeList = false;
		}
		//			
		void RemoveList(const size_t idx) // ilist_idx!
		{
			// chk whether item or usertype.
			// find item_idx or usertype_idx.
			// remove item or remove usertype.
			if (ilist[idx] == 1) {
				size_t  item_idx = 0; // -1

				for (size_t i = 0; i < ilist.size() && i <= idx; ++i) {
					if (ilist[i] == 1) { item_idx++; }
				}

				RemoveItemList(item_idx - 1); // item_idx
			}
			else {
				size_t  usertype_idx = 0;

				for (size_t i = 0; i < ilist.size() && i <= idx; ++i) {
					if (ilist[i] == 2) { usertype_idx++; }
				}

				RemoveUserTypeList(usertype_idx - 1);
			}
		}
	public:
		bool empty()const { return ilist.empty(); }

		void InsertItemByIlist(const size_t ilist_idx, const std::string& name, const std::string& item) {
			ilist.push_back(1);
			for (size_t i = ilist.size(); i > ilist_idx + 1; --i) {
				ilist[i - 1] = ilist[i - 2];
			}
			ilist[ilist_idx] = 1;

			bool err = false;
			size_t  itemIndex = _GetItemIndexFromIlistIndex(ilist, ilist_idx, err);

			itemList.emplace_back("", std::string(""));

			if (!err) {
				for (size_t i = itemList.size(); i > itemIndex + 1; --i) {
					itemList[i - 1] = std::move(itemList[i - 2]);
				}
				itemList[itemIndex] = ItemType<std::string>(name, item);
			}
			else {
				itemList[0] = ItemType<std::string>(name, item);
			}

			useSortedItemList = false;
		}
		void InsertItemByIlist(const size_t  ilist_idx, std::string&& name, std::string&& item) {
			ilist.push_back(1);

			bool err = false;

			for (size_t i = ilist.size(); i > ilist_idx + 1; --i) {
				ilist[i - 1] = ilist[i - 2];
			}
			ilist[ilist_idx] = 1;

			size_t  itemIndex = _GetItemIndexFromIlistIndex(ilist, ilist_idx, err);

			itemList.emplace_back("", std::string(""));
			if (!err) {
				for (size_t i = itemList.size(); i > itemIndex + 1; --i) {
					itemList[i - 1] = std::move(itemList[i - 2]);
				}
				itemList[itemIndex] = ItemType<std::string>(std::move(name), std::move(item));
			}
			else {
				itemList[0] = ItemType<std::string>(std::move(name), std::move(item));
			}


			useSortedItemList = false;
		}

		void InsertUserTypeByIlist(const size_t ilist_idx, UserType&& item) {
			ilist.push_back(2);
			UserType* temp = new UserType(std::move(item));

			temp->parent = this;

			for (size_t i = ilist.size(); i > ilist_idx + 1; --i) {
				ilist[i - 1] = ilist[i - 2];
			}
			ilist[ilist_idx] = 2;

			bool err = false;
			size_t userTypeIndex = _GetUserTypeIndexFromIlistIndex(ilist, ilist_idx, err);
			userTypeList.push_back(nullptr);
			if (!err) {
				for (size_t i = userTypeList.size(); i > userTypeIndex + 1; --i) {
					userTypeList[i - 1] = std::move(userTypeList[i - 2]);
				}
				userTypeList[userTypeIndex] = temp;
			}
			else {
				userTypeList[0] = temp;
			}


			useSortedUserTypeList = false;
		}
		void InsertUserTypeByIlist(const size_t ilist_idx, const UserType& item) {
			ilist.push_back(2);
			UserType* temp = new UserType(item);

			temp->parent = this;


			for (size_t i = ilist.size(); i > ilist_idx + 1; --i) {
				ilist[i - 1] = ilist[i - 2];
			}
			ilist[ilist_idx] = 2;

			bool err = false;
			size_t  userTypeIndex = _GetUserTypeIndexFromIlistIndex(ilist, ilist_idx, err);
			userTypeList.push_back(nullptr);
			if (!err) {
				for (size_t i = userTypeList.size(); i > userTypeIndex + 1; --i) {
					userTypeList[i - 1] = std::move(userTypeList[i - 2]);
				}
				userTypeList[userTypeIndex] = temp;
			}
			else {
				userTypeList[0] = temp;
			}

			useSortedUserTypeList = false;
		}

		void ReserveIList(size_t offset)
		{
			if (offset > 0) {
				ilist.reserve(offset);
			}
		}
		void ReserveItemList(size_t offset)
		{
			if (offset > 0) {
				itemList.reserve(offset);
			}
		}
		void ReserveUserTypeList(size_t offset)
		{
			if (offset > 0) {
				userTypeList.reserve(offset);
			}
		}
		void AddItem(const char* str1, size_t len1, const char* str2, size_t len2) {
			itemList.emplace_back(str1, len1, str2, len2);
			ilist.push_back(1);

			useSortedItemList = false;
		}

		void AddItem(std::string&& name, std::string&& item) {
			itemList.emplace_back(std::move(name), std::move(item));
			ilist.push_back(1);

			useSortedItemList = false;
		}
		void AddItem(const std::string& name, const std::string& item) {
			itemList.emplace_back(name, item);
			ilist.push_back(1);

			useSortedItemList = false;
		}
		void AddItemType(clau_parser11::ItemType<std::string>&& item) {
			itemList.push_back(std::move(item));
			ilist.push_back(1);

			useSortedItemList = false;
		}

		void AddItemType(const clau_parser11::ItemType<std::string>& item) {
			itemList.push_back(item);
			ilist.push_back(1);

			useSortedItemList = false;
		}
		void AddUserTypeItem(UserType&& item) {
			UserType* temp = new UserType(std::move(item.GetName()));

			temp->parent = this;

			ilist.push_back(2);

			userTypeList.push_back(temp);

			useSortedUserTypeList = false;
			useSortedItemList = false;


			{
				temp->itemList = std::move(item.itemList);
				temp->ilist = std::move(item.ilist);
				temp->userTypeList = std::move(item.userTypeList);

				for (size_t i = 0; i < item.GetUserTypeListSize(); ++i) {
					item.userTypeList[i] = nullptr;
				}
				for (size_t i = 0; i < temp->GetUserTypeListSize(); ++i) {
					temp->userTypeList[i]->parent = temp;
				}
			}
		}
		void AddUserTypeItem(const UserType& item) {
			UserType* temp = new UserType(item);
			temp->parent = this;

			ilist.push_back(2);

			userTypeList.push_back(temp);

			useSortedUserTypeList = false;
		}
		void AddItemAtFront(std::string&& name, std::string&& item) {
			itemList.emplace(itemList.begin(), name, item);

			ilist.insert(ilist.begin(), 1);

			useSortedItemList = false;
		}
		void AddItemAtFront(const std::string& name, const std::string& item) {
			itemList.emplace(itemList.begin(), name, item);

			ilist.insert(ilist.begin(), 1);

			useSortedItemList = false;
		}
		void AddUserTypeItemAtFront(const UserType& item) {
			UserType* temp = new UserType(item);
			temp->parent = this;

			ilist.insert(ilist.begin(), 2);

			userTypeList.insert(userTypeList.begin(), temp);

			useSortedUserTypeList = false;
		}
		void AddUserTypeItemAtFront(UserType&& item) {
			UserType* temp = new UserType(item);
			temp->parent = this;

			ilist.insert(ilist.begin(), 2);

			userTypeList.insert(userTypeList.begin(), temp);

			useSortedUserTypeList = false;
		}


		std::vector<int> GetUserTypeIdx(const std::string& name) const {
			std::vector<int> temp;

			if (name == "*") {
				std::cout << userTypeList.size() << "|\n";
				for (size_t i = 0; i < userTypeList.size(); ++i) {
					temp.push_back(i);
				}
				return temp;
			}

			if (false == useSortedUserTypeList) {
				// make sortedUserTypeList.
				sortedUserTypeList.clear();
				for (size_t i = 0; i < userTypeList.size(); ++i) {
					sortedUserTypeList.emplace_back(userTypeList[i], i);

				}
				//	sortedUserTypeList = userTypeList;

				std::stable_sort(sortedUserTypeList.begin(), sortedUserTypeList.end(), UserTypeCompare());

				useSortedUserTypeList = true;
			}
			// binary search
			{
				UserType x = UserType(name);
				bool err = false;
				size_t idx = binary_find_ut(sortedUserTypeList, x, err);
				if (!err) {
					size_t start = idx;
					size_t  last = idx;

					for (size_t i = idx; i > 0; --i) {
						if (name == sortedUserTypeList[i - 1].ptr->GetName()) {
							start--;
						}
						else {
							break;
						}
					}
					for (size_t i = idx + 1; i < sortedUserTypeList.size(); ++i) {
						if (name == sortedUserTypeList[i].ptr->GetName()) {
							last++;
						}
						else {
							break;
						}
					}

					for (size_t i = start; i < last + 1; ++i) {
						temp.push_back(sortedUserTypeList[i].idx);
					}
				}
				else {
					//std::cout << "no found" << "\n";
				}
			}

			return temp;
		}

		std::vector<ItemType<std::string>> GetItem(const std::string& name) const {
			std::vector<ItemType<std::string>> temp;
			/*if (String::startsWith(name, "$.") && name.size() >= 5) {
				// later, change to binary search?
				std::string str = name.substr(3, name.size() - 4);
				std::regex rgx(str);
				for (int i = 0; i < itemList.size(); ++i) {
					if (regex_match(itemList[i].GetName(), rgx)) {
						temp.push_back(itemList[i]);
					}
				}
			}
			else*/
			{
				if (false == useSortedItemList) {
					sortedItemList.clear();
					for (size_t i = 0; i < itemList.size(); ++i) {
						sortedItemList.push_back(WrapType<ItemType<std::string>*>((ItemType<std::string>*) & itemList[i], i));
					}

					std::sort(sortedItemList.begin(), sortedItemList.end(), ItemTypeStringPtrCompare());

					useSortedItemList = true;
				}
				// binary search
				{
					bool err = false;
					ItemType<std::string> x = ItemType<std::string>(name, "");
					size_t  idx = binary_find_it(sortedItemList, x, err);
					if (!err) {
						size_t  start = idx;
						size_t  last = idx;

						for (size_t i = idx; i > 0; --i) {
							if (name == sortedItemList[i - 1].ptr->GetName()) {
								start--;
							}
							else {
								break;
							}
						}
						for (size_t i = idx + 1; i < sortedItemList.size(); ++i) {
							if (name == sortedItemList[i].ptr->GetName()) {
								last++;
							}
							else {
								break;
							}
						}

						for (size_t i = start; i < last + 1; ++i) {
							temp.push_back(*sortedItemList[i].ptr);
						}
					}
					else {
						//std::cout << "no found" << "\n";
					}
				}
			}
			return temp;
		}
		// regex to SetItem?
		bool SetItem(const std::string& name, const std::string& value) {
			size_t index = -1;
			bool err = true;

			for (size_t i = 0; i < itemList.size(); ++i) {
				if (itemList[i].GetName() == name)
				{
					itemList[i].Set(0, value);
					index = i;
					err = false;
				}
			}

			return err;
		}
		/// add set Data
		bool SetItem(const size_t  var_idx, const std::string& value) {
			itemList[var_idx].Set(0, value);
			return true;
		}

		//// O(N) -> O(logN)?
		std::vector<UserType*> GetUserTypeItem(const std::string& name) const { /// chk...
			std::vector<UserType*> temp;

			if (false == useSortedUserTypeList) {
				// make sortedUserTypeList.
				for (size_t i = 0; i < userTypeList.size(); ++i) {
					sortedUserTypeList.emplace_back(userTypeList[i], i);
				}

				std::sort(sortedUserTypeList.begin(), sortedUserTypeList.end(), UserTypeCompare());

				useSortedUserTypeList = true;
			}
			// binary search
			{
				UserType x = UserType(name);
				bool err = false;
				size_t idx = binary_find_ut(sortedUserTypeList, x, err);
				if (!err) {
					size_t start = idx;
					size_t  last = idx;

					for (size_t i = idx; i > 0; --i) {
						if (name == sortedUserTypeList[i - 1].ptr->GetName()) {
							start--;
						}
						else {
							break;
						}
					}
					for (size_t i = idx + 1; i < sortedUserTypeList.size(); ++i) {
						if (name == sortedUserTypeList[i].ptr->GetName()) {
							last++;
						}
						else {
							break;
						}
					}

					for (size_t i = start; i < last + 1; ++i) {
						temp.push_back(sortedUserTypeList[i].ptr);
					}
				}
				else {
					//std::cout << "no found" << "\n";
				}
			}

			return temp;
		}

		// deep copy.
		std::vector<UserType*> GetCopyUserTypeItem(const std::string& name) const { /// chk...
			std::vector<UserType*> temp;

			if (false == useSortedUserTypeList) {
				// make sortedUserTypeList.
				for (size_t i = 0; i < userTypeList.size(); ++i) {
					sortedUserTypeList.emplace_back(userTypeList[i], i);
				}

				std::sort(sortedUserTypeList.begin(), sortedUserTypeList.end(), UserTypeCompare());

				useSortedUserTypeList = true;
			}
			// binary search
			{
				UserType x = UserType(name);
				bool err = false;
				size_t  idx = binary_find_ut(sortedUserTypeList, x, err);
				if (!err) {
					size_t start = idx;
					size_t last = idx;

					for (size_t i = idx; i > 0; --i) {
						if (name == sortedUserTypeList[i - 1].ptr->GetName()) {
							start--;
						}
						else {
							break;
						}
					}
					for (size_t i = idx + 1; i < sortedUserTypeList.size(); ++i) {
						if (name == sortedUserTypeList[i].ptr->GetName()) {
							last++;
						}
						else {
							break;
						}
					}

					for (size_t i = start; i < last + 1; ++i) {
						temp.push_back(new UserType(*sortedUserTypeList[i].ptr));
					}
				}
				else {
					//std::cout << "no found" << "\n";
				}
			}

			return temp;
		}

		std::vector<int> GetItemIdx(const std::string& name) const {
			std::vector<int> temp;
			/*if (String::startsWith(name, "$.") && name.size() >= 5) {
				// later, change to binary search?
				std::string str = name.substr(3, name.size() - 4);
				std::regex rgx(str);
				for (int i = 0; i < itemList.size(); ++i) {
					if (regex_match(itemList[i].GetName(), rgx)) {
						temp.push_back(itemList[i]);
					}
				}
			}
			else*/
			{
				if (false == useSortedItemList) {
					sortedItemList.clear();
					for (size_t i = 0; i < itemList.size(); ++i) {
						sortedItemList.push_back(WrapType<ItemType<std::string>*>((ItemType<std::string>*) & itemList[i], i));
					}

					std::stable_sort(sortedItemList.begin(), sortedItemList.end(), ItemTypeStringPtrCompare());

					useSortedItemList = true;
				}
				// binary search
				{
					bool err = false;
					ItemType<std::string> x = ItemType<std::string>(name, "");
					size_t  idx = binary_find_it(sortedItemList, x, err);

					if (!err) {
						size_t  start = idx;
						size_t  last = idx;

						for (size_t i = idx; i > 0; --i) {
							if (name == sortedItemList[i - 1].ptr->GetName()) {
								start--;
							}
							else {
								break;
							}
						}
						for (size_t i = idx + 1; i < sortedItemList.size(); ++i) {
							if (name == sortedItemList[i].ptr->GetName()) {
								last++;
							}
							else {
								break;
							}
						}

						for (size_t i = start; i < last + 1; ++i) {
							temp.push_back(sortedItemList[i].idx);
						}
					}
					else {
						//std::cout << "no found" << "\n";
					}
				}
			}
			return temp;
		}



	public:
		bool GetUserTypeItemRef(const size_t  idx, UserType*& ref)
		{
			ref = userTypeList[idx];
			return true;
		}
		bool GetLastUserTypeItemRef(UserType*& ref) {
			if (userTypeList.empty() == false) {
				ref = userTypeList.back();
				return true;
			}
			return false;
		}
	private:
		/// save1 - like EU4 savefiles.
		void Save1(std::ostream& stream, const UserType* ut, const int depth = 0) const {
			size_t  itemListCount = 0;
			size_t userTypeListCount = 0;

			const bool existUserType = ut->GetUserTypeListSize() > 0;

			for (size_t i = 0; i < ut->ilist.size(); ++i) {
				//std::cout << "ItemList" << endl;
				if (ut->ilist[i] == 1) {
					for (size_t j = 0; j < ut->itemList[itemListCount].size(); j++) {
						std::string temp;
						if (existUserType) {
							for (int k = 0; k < depth; ++k) {
								temp += "\t";
							}
						}
						if (ut->itemList[itemListCount].GetName() != "") {
							temp += ut->itemList[itemListCount].GetName();
							temp += " = ";
						}
						temp += ut->itemList[itemListCount].Get(j);
						if (j != ut->itemList[itemListCount].size() - 1) {
							temp += " ";
						}
						stream << temp;
					}
					if (i != ut->ilist.size() - 1) {
						if (existUserType) {
							stream << "\n";
						}
						else {
							stream << " "; // \n
						}
					}
					itemListCount++;
				}
				else if (ut->ilist[i] == 2) {
					// std::cout << "UserTypeList" << endl;
					for (int k = 0; k < depth; ++k) {
						stream << "\t";
					}

					if (ut->userTypeList[userTypeListCount]->GetName() != "") {
						stream << ut->userTypeList[userTypeListCount]->GetName() << " = ";
					}

					stream << "{\n";

					Save1(stream, ut->userTypeList[userTypeListCount], depth + 1);
					stream << "\n";

					for (int k = 0; k < depth; ++k) {
						stream << "\t";
					}
					stream << "}";
					if (i != ut->ilist.size() - 1) {
						stream << "\n";
					}

					userTypeListCount++;
				}
			}
		}
		/// save2 - test form.
		void Save2(std::ostream& stream, const UserType* ut, const int depth = 0) const {
			size_t  itemListCount = 0;
			size_t  userTypeListCount = 0;


			for (size_t i = 0; i < ut->ilist.size(); ++i) {
				//std::cout << "ItemList" << endl;
				if (ut->ilist[i] == 1) {
					for (size_t j = 0; j < ut->itemList[itemListCount].size(); j++) {
						//for (int k = 0; k < depth; ++k) {
						//	stream << "\t";
						//}
						if (ut->itemList[itemListCount].GetName() != "") {

							stream << ut->itemList[itemListCount].GetName() << " : ";
						}
						stream << ut->itemList[itemListCount].Get(j);
						if (j != ut->itemList[itemListCount].size() - 1) {
							stream << " ";
						}
					}

					if (i != ut->ilist.size() - 1) {
						stream << " ";//"\n";
					}
					itemListCount++;
				}
				else if (ut->ilist[i] == 2) {
					// std::cout << "UserTypeList" << endl;
					if (ut->userTypeList[userTypeListCount]->GetName() != "") {
						stream << ut->userTypeList[userTypeListCount]->GetName() << " ";
					}
					stream << "{\n";

					Save2(stream, ut->userTypeList[userTypeListCount], depth + 1);
					stream << "\n";

					for (int k = 0; k < depth; ++k) {
						stream << "\t";
					}
					stream << "}";
					if (i != ut->ilist.size() - 1) {
						stream << "\n";
					}
					userTypeListCount++;
				}
			}
		}

	public:
		void Save1(std::ostream& stream, int depth = 0) const {
			Save1(stream, this, depth);
		}

		void Save2(std::ostream& stream, int depth = 0) const {
			Save2(stream, this, depth);
		}

		std::string ItemListToString()const
		{
			std::string temp;
			size_t itemListCount = 0;

			for (size_t i = 0; i < itemList.size(); ++i) {
				for (size_t j = 0; j < itemList[itemListCount].size(); j++) {
					if (itemList[itemListCount].GetName() != "")
						temp = temp + itemList[itemListCount].GetName() + " = ";
					temp = temp + itemList[itemListCount].Get(j);
					if (j != itemList[itemListCount].size() - 1) {
						temp = temp + "/";
					}
				}
				if (i != itemList.size() - 1)
				{
					temp = temp + "/";
				}
				itemListCount++;
			}
			return temp;
		}
		std::string ItemListNamesToString()const
		{
			std::string temp;
			size_t  itemListCount = 0;

			for (size_t i = 0; i < itemList.size(); ++i) {
				for (size_t j = 0; j < itemList[itemListCount].size(); j++) {
					if (itemList[itemListCount].GetName() != "")
						temp = temp + itemList[itemListCount].GetName();
					else
						temp = temp + " ";

					if (j != itemList[itemListCount].size() - 1) {
						temp = temp + "/";
					}
				}
				if (i != itemList.size() - 1)
				{
					temp = temp + "/";
				}
				itemListCount++;
			}
			return temp;
		}
		std::vector<std::string> userTypeListNamesToStringArray()const
		{
			std::vector<std::string> temp;
			size_t userTypeListCount = 0;

			for (size_t i = 0; i < userTypeList.size(); ++i) {
				if (userTypeList[userTypeListCount]->GetName() != "") {
					temp.push_back(userTypeList[userTypeListCount]->GetName());
				}
				else {
					temp.push_back(" ");
				}
				userTypeListCount++;
			}
			return temp;
		}
		std::string UserTypeListNamesToString()const
		{
			std::string temp;
			size_t  userTypeListCount = 0;

			for (size_t i = 0; i < userTypeList.size(); ++i) {
				if (userTypeList[userTypeListCount]->GetName() != "") {
					temp = temp + userTypeList[userTypeListCount]->GetName();
				}
				else {
					temp = temp + " ";
				}

				if (i != itemList.size() - 1)
				{
					temp = temp + "/";
				}
				userTypeListCount++;
			}
			return temp;
		}
		std::string ToString()const
		{
			std::string temp;
			size_t itemListCount = 0;
			size_t  userTypeListCount = 0;

			for (size_t i = 0; i < ilist.size(); ++i) {
				//std::cout << "ItemList" << endl;
				if (ilist[i] == 1) {
					for (size_t j = 0; j < itemList[itemListCount].size(); j++) {
						if (itemList[itemListCount].GetName() != "") {
							temp.append(itemList[itemListCount].GetName());
							temp.append(" = ");
						}
						temp.append(itemList[itemListCount].Get(j));
						if (j != itemList[itemListCount].size() - 1)
						{
							temp.append(" ");
						}
					}
					if (i != ilist.size() - 1) {
						temp.append(" ");
					}
					itemListCount++;
				}
				else if (ilist[i] == 2) {
					// std::cout << "UserTypeList" << endl;
					if (userTypeList[userTypeListCount]->GetName() != "") {
						temp.append(userTypeList[userTypeListCount]->GetName());
						temp.append(" = ");
					}
					temp.append(" { ");
					temp.append(userTypeList[userTypeListCount]->ToString());
					temp.append(" ");
					temp.append(" }");
					if (i != ilist.size() - 1) {
						temp.append(" ");
					}

					userTypeListCount++;
				}
			}
			return temp;
		}
	public:
	};

	class LoadData
	{

		enum {
			TYPE_LEFT = 1, // 01
			TYPE_RIGHT = 2, // 10
			TYPE_ASSIGN = 3 // 11
		};
	private:
		static int64_t check_syntax_error1(int64_t str, int* err) {
			int64_t len = Utility::GetLength(str);
			int64_t type = Utility::GetType(str);

			if (1 == len && (type == TYPE_LEFT || type == TYPE_RIGHT ||
				type == TYPE_ASSIGN)) {
				*err = -4;
				std::cout << "err " << type << "\n";
			}
			return str;
		}
	public:
		static int Merge(UserType* next, UserType* ut, UserType** ut_next)
		{
			//check!!
			while (ut->GetIListSize() >= 1 && ut->GetUserTypeListSize() >= 1
				&& (ut->GetUserTypeList(0)->GetName() == "#"))
			{
				ut = ut->GetUserTypeList(0);
			}

			bool chk_ut_next = false;

			while (true) {
				int itCount = 0;
				int utCount = 0;

				UserType* _ut = ut;
				UserType* _next = next;


				if (ut_next && _ut == *ut_next) {
					*ut_next = _next;
					chk_ut_next = true;
				}

				_next->ReserveItemList(_ut->GetItemListSize());

				for (size_t i = 0; i < _ut->GetIListSize(); ++i) {
					if (_ut->IsUserTypeList(i)) {
						if (_ut->GetUserTypeList(utCount)->GetName() == "#") {
							_ut->GetUserTypeList(utCount)->SetName("");
						}
						else {
							{
								_next->LinkUserType(_ut->GetUserTypeList(utCount));
								_ut->GetUserTypeList(utCount) = nullptr;
							}
						}
						utCount++;
					}
					else if (_ut->IsItemList(i)) {
						_next->AddItemType(std::move(_ut->GetItemList(itCount)));
						itCount++;
					}
				}
				_ut->Remove();

				ut = ut->GetParent();
				next = next->GetParent();


				if (next && ut) {
					//
				}
				else {
					// right_depth > left_depth
					if (!next && ut) {
						return -1;
					}
					else if (next && !ut) {
						return 1;
					}

					return 0;
				}
			}
		}

	private:
		static bool __LoadData(const char* buffer, const int64_t* token_arr, int64_t token_arr_len, UserType* _global,
			int start_state, int last_state, UserType** next, int* err)
		{

			std::vector<int64_t> varVec;
			std::vector<int64_t> valVec;


			if (token_arr_len <= 0) {
				return false;
			}

			UserType& global = *_global;

			int state = start_state;
			size_t braceNum = 0;
			std::vector< UserType* > nestedUT(1);
			int64_t var = 0, val = 0;

			nestedUT.reserve(10);
			nestedUT[0] = &global;


			int64_t count = 0;
			const int64_t* x = token_arr;
			const int64_t* x_next = x;

			for (int64_t i = 0; i < token_arr_len; ++i) {
				x = x_next;
				{
					x_next = x + 1;
				}
				if (count > 0) {
					count--;
					continue;
				}
				int64_t len = Utility::GetLength(token_arr[i]);

				switch (state)
				{
				case 0:
				{
					// Left 1
					if (len == 1 && (-1 != Utility::Equal(TYPE_LEFT, Utility::GetType(token_arr[i])))) {
						if (!varVec.empty()) {
							nestedUT[braceNum]->ReserveIList(nestedUT[braceNum]->GetIListSize() + varVec.size());
							nestedUT[braceNum]->ReserveItemList(nestedUT[braceNum]->GetItemListSize() + varVec.size());

							for (size_t x = 0; x < varVec.size(); ++x) {
								nestedUT[braceNum]->AddItem(buffer + Utility::GetIdx(varVec[x]), Utility::GetLength(varVec[x]),
									buffer + Utility::GetIdx(valVec[x]), Utility::GetLength(valVec[x]));
							}

							varVec.clear();
							valVec.clear();
						}

						UserType temp("");

						nestedUT[braceNum]->AddUserTypeItem(temp);
						UserType* pTemp = nullptr;
						nestedUT[braceNum]->GetLastUserTypeItemRef(pTemp);

						braceNum++;

						/// new nestedUT
						if (nestedUT.size() == braceNum) {
							nestedUT.push_back(nullptr);
						}

						/// initial new nestedUT.
						nestedUT[braceNum] = pTemp;
						///

						state = 0;
					}
					// Right 2
					else if (len == 1 && (-1 != Utility::Equal(TYPE_RIGHT, Utility::GetType(token_arr[i])))) {
						state = 0;

						if (!varVec.empty()) {

							{
								nestedUT[braceNum]->ReserveIList(nestedUT[braceNum]->GetIListSize() + varVec.size());
								nestedUT[braceNum]->ReserveItemList(nestedUT[braceNum]->GetItemListSize() + varVec.size());

								for (size_t x = 0; x < varVec.size(); ++x) {
									nestedUT[braceNum]->AddItem(buffer + Utility::GetIdx(varVec[x]), Utility::GetLength(varVec[x]),
										buffer + Utility::GetIdx(valVec[x]), Utility::GetLength(valVec[x]));
								}
							}

							varVec.clear();
							valVec.clear();
						}

						if (braceNum == 0) {
							UserType ut;
							ut.AddUserTypeItem(UserType("#", 1)); // json -> "var_name" = val  // clautext, # is line comment delimiter.
							UserType* pTemp = nullptr;
							ut.GetLastUserTypeItemRef(pTemp);
							int utCount = 0;
							int itCount = 0;
							auto max = nestedUT[braceNum]->GetIListSize();
							for (size_t i = 0; i < max; ++i) {
								if (nestedUT[braceNum]->IsUserTypeList(i)) {
									ut.GetUserTypeList(0)->AddUserTypeItem(std::move(*(nestedUT[braceNum]->GetUserTypeList(utCount))));
									utCount++;
								}
								else {
									ut.GetUserTypeList(0)->AddItemType(std::move(nestedUT[braceNum]->GetItemList(itCount)));
									itCount++;
								}
							}

							nestedUT[braceNum]->Remove();
							nestedUT[braceNum]->AddUserTypeItem(std::move(*(ut.GetUserTypeList(0))));

							braceNum++;
						}

						{
							if (braceNum < nestedUT.size()) {
								nestedUT[braceNum] = nullptr;
							}
							braceNum--;
						}
					}
					else {
						if (x < token_arr + token_arr_len - 1) {
							int64_t _len = Utility::GetLength(token_arr[i + 1]);
							// EQ 3
							if (_len == 1 && -1 != Utility::Equal(TYPE_ASSIGN, Utility::GetType(token_arr[i + 1]))) {
								var = token_arr[i];

								state = 1;

								{
									count = 1;
								}
							}
							else {
								// var1
								if (x <= token_arr + token_arr_len - 1) {

									val = token_arr[i];

									varVec.push_back(check_syntax_error1(var, err));
									valVec.push_back(check_syntax_error1(val, err));

									val = 0;

									state = 0;

								}
							}
						}
						else
						{
							// var1
							if (x <= token_arr + token_arr_len - 1)
							{
								val = token_arr[i];
								varVec.push_back(check_syntax_error1(var, err));
								valVec.push_back(check_syntax_error1(val, err));
								val = 0;

								state = 0;
							}
						}
					}
				}
				break;
				case 1:
				{
					// LEFT 1
					if (len == 1 && (-1 != Utility::Equal(TYPE_LEFT, Utility::GetType(token_arr[i])))) {
						nestedUT[braceNum]->ReserveIList(nestedUT[braceNum]->GetIListSize() + varVec.size());
						nestedUT[braceNum]->ReserveItemList(nestedUT[braceNum]->GetItemListSize() + varVec.size());

						for (size_t x = 0; x < varVec.size(); ++x) {
							nestedUT[braceNum]->AddItem(buffer + Utility::GetIdx(varVec[x]), Utility::GetLength(varVec[x]),
								buffer + Utility::GetIdx(valVec[x]), Utility::GetLength(valVec[x]));
						}


						varVec.clear();
						valVec.clear();

						///
						{
							nestedUT[braceNum]->AddUserTypeItem(UserType(buffer + Utility::GetIdx(var), Utility::GetLength(var)));
							UserType* pTemp = nullptr;
							nestedUT[braceNum]->GetLastUserTypeItemRef(pTemp);
							var = 0;
							braceNum++;

							/// new nestedUT
							if (nestedUT.size() == braceNum) {
								nestedUT.push_back(nullptr);
							}

							/// initial new nestedUT.
							nestedUT[braceNum] = pTemp;
						}
						///
						state = 0;
					}
					else {
						if (x <= token_arr + token_arr_len - 1) {
							val = token_arr[i];

							varVec.push_back(check_syntax_error1(var, err));
							valVec.push_back(check_syntax_error1(val, err));
							var = 0; val = 0;

							state = 0;
						}
					}
				}
				break;
				default:
					// syntax err!!
					*err = -1;
					return false; // throw "syntax error ";
					break;
				}
			}

			if (next) {
				*next = nestedUT[braceNum];
			}

			if (varVec.empty() == false) {
				nestedUT[braceNum]->ReserveIList(nestedUT[braceNum]->GetIListSize() + varVec.size());
				nestedUT[braceNum]->ReserveItemList(nestedUT[braceNum]->GetItemListSize() + varVec.size());

				for (size_t x = 0; x < varVec.size(); ++x) {
					nestedUT[braceNum]->AddItem(buffer + Utility::GetIdx(varVec[x]), Utility::GetLength(varVec[x]),
						buffer + Utility::GetIdx(valVec[x]), Utility::GetLength(valVec[x]));
				}


				varVec.clear();
				valVec.clear();
			}

			if (state != last_state) {
				*err = -2;
				return false;
				// throw std::string("error final state is not last_state!  : ") + toStr(state);
			}
			if (x > token_arr + token_arr_len) {
				*err = -3;
				return false;
				//throw std::string("error x > buffer + buffer_len: ");
			}

			return true;
		}


		static int64_t FindDivisionPlace(const char* buffer, const int64_t* token_arr, int64_t start, int64_t last)
		{
			for (int64_t a = last; a >= start; --a) {
				int64_t len = Utility::GetLength(token_arr[a]);
				int64_t val = Utility::GetType(token_arr[a]);


				if (len == 1 && (-1 != Utility::Equal(TYPE_RIGHT, val))) { // right
					return a;
				}

				bool pass = false;
				if (len == 1 && (-1 != Utility::Equal(TYPE_LEFT, val))) { // left
					return a;
				}
				else if (len == 1 && -1 != Utility::Equal(TYPE_ASSIGN, val)) { // assignment
					//
					pass = true;
				}

				if (a < last && pass == false) {
					int64_t len = Utility::GetLength(token_arr[a + 1]);
					int64_t val = Utility::GetType(token_arr[a + 1]);

					if (!(len == 1 && -1 != Utility::Equal(TYPE_ASSIGN, val))) // assignment
					{ // NOT
						return a;
					}
				}
			}
			return -1;
		}

		static bool _LoadData(InFileReserver& reserver, UserType& global, const int lex_thr_num, const int parse_num) // first, strVec.empty() must be true!!
		{
			const int pivot_num = parse_num - 1;
			char* buffer = nullptr;
			int64_t* token_arr = nullptr;
			size_t buffer_total_len;
			size_t token_arr_len = 0;

			{

				auto a = std::chrono::steady_clock::now();

				bool success = reserver(lex_thr_num, buffer, &buffer_total_len, token_arr, &token_arr_len);


				auto b = std::chrono::steady_clock::now();
				auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(b - a);
				//	std::cout << "scan " << dur.count() << "ms\n";

					//	{
					//		for (int64_t i = 0; i < token_arr_len; ++i) {
					//			std::string(buffer + Utility::GetIdx(token_arr[i]), Utility::GetLength(token_arr[i]));
				//				if (0 == Utility::GetIdx(token_arr[i])) {
					//				std::cout << "chk";
					//			}
					//		}
					//	}

				if (!success) {
					return false;
				}
				if (token_arr_len <= 0) {
					if (buffer) {
						delete[] buffer;
					}
					if (token_arr) {
						delete[] token_arr;
					}
					return true;
				}
			}

			UserType* before_next = nullptr;
			UserType _global;

			bool first = true;
			int64_t sum = 0;

			{
				std::set<int64_t> _pivots;
				std::vector<int64_t> pivots;
				const int64_t num = token_arr_len; //

				if (pivot_num > 0) {
					std::vector<int64_t> pivot;
					pivots.reserve(pivot_num);
					pivot.reserve(pivot_num);

					for (int i = 0; i < pivot_num; ++i) {
						pivot.push_back(FindDivisionPlace(buffer, token_arr, (num / (pivot_num + 1)) * (i), (num / (pivot_num + 1)) * (i + 1) - 1));
					}

					for (size_t i = 0; i < pivot.size(); ++i) {
						if (pivot[i] != -1) {
							_pivots.insert(pivot[i]);
						}
					}

					for (auto& x : _pivots) {
						pivots.push_back(x);
					}
				}

				std::vector<UserType*> next(pivots.size() + 1, nullptr);

				{
					std::vector<UserType> __global(pivots.size() + 1);

					std::vector<std::thread> thr(pivots.size() + 1);
					std::vector<int> err(pivots.size() + 1, 0);
					{
						int64_t idx = pivots.empty() ? num - 1 : pivots[0];
						int64_t _token_arr_len = idx - 0 + 1;

						thr[0] = std::thread(__LoadData, buffer, token_arr, _token_arr_len, &__global[0], 0, 0, &next[0], &err[0]);
					}

					for (size_t i = 1; i < pivots.size(); ++i) {
						int64_t _token_arr_len = pivots[i] - (pivots[i - 1] + 1) + 1;

						thr[i] = std::thread(__LoadData, buffer, token_arr + pivots[i - 1] + 1, _token_arr_len, &__global[i], 0, 0, &next[i], &err[i]);

					}

					if (pivots.size() >= 1) {
						int64_t _token_arr_len = num - 1 - (pivots.back() + 1) + 1;

						thr[pivots.size()] = std::thread(__LoadData, buffer, token_arr + pivots.back() + 1, _token_arr_len, &__global[pivots.size()],
							0, 0, &next[pivots.size()], &err[pivots.size()]);
					}


					auto a = std::chrono::steady_clock::now();

					// wait
					for (size_t i = 0; i < thr.size(); ++i) {
						thr[i].join();
					}


					auto b = std::chrono::steady_clock::now();
					auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(b - a);
					//std::cout << "parse1 " << dur.count() << "ms\n";

					for (size_t i = 0; i < err.size(); ++i) {
						switch (err[i]) {
						case 0:
							break;
						case -1:
						case -4:
							std::cout << "Syntax Error\n";
							break;
						case -2:
							std::cout << "error final state is not last_state!\n";
							break;
						case -3:
							std::cout << "error x > buffer + buffer_len:\n";
							break;
						default:
							std::cout << "unknown parser error\n";
							break;
						}
					}

					// Merge
					try {
						if (__global[0].GetUserTypeListSize() > 0 && __global[0].GetUserTypeList(0)->GetName() == "#") {
							std::cout << "not valid file1\n";
							throw 1;
						}
						if (next.back()->GetParent() != nullptr) {
							std::cout << "not valid file2\n";
							throw 2;
						}

						int err = Merge(&_global, &__global[0], &next[0]);
						if (-1 == err || (pivots.size() == 0 && 1 == err)) {
							std::cout << "not valid file3\n";
							throw 3;
						}

						for (size_t i = 1; i < pivots.size() + 1; ++i) {
							// linearly merge and error check...
							int err = Merge(next[i - 1], &__global[i], &next[i]);
							if (-1 == err) {
								std::cout << "not valid file4\n";
								throw 4;
							}
							else if (i == pivots.size() && 1 == err) {
								std::cout << "not valid file5\n";
								throw 5;
							}
						}
					}
					catch (...) {
						delete[] buffer;
						delete[] token_arr;
						buffer = nullptr;
						throw "in Merge, error";
					}

					before_next = next.back();

					auto c = std::chrono::steady_clock::now();
					auto dur2 = std::chrono::duration_cast<std::chrono::nanoseconds>(c - b);
					//std::cout << "parse2 " << dur2.count() << "ns\n";
				}
			}

			delete[] buffer;
			delete[] token_arr;

			global = std::move(_global);

			return true;
		}
	public:
		static bool LoadDataFromFile(const std::string& fileName, UserType& global, int lex_thr_num = 1, int parse_thr_num = 1, bool use_simd = false) /// global should be empty
		{
			if (lex_thr_num <= 0) {
				lex_thr_num = std::thread::hardware_concurrency();
			}
			if (lex_thr_num <= 0) {
				lex_thr_num = 1;
			}

			if (parse_thr_num <= 0) {
				parse_thr_num = std::thread::hardware_concurrency();
			}
			if (parse_thr_num <= 0) {
				parse_thr_num = 1;
			}

			bool success = true;
			FILE* inFile;

#ifdef _WIN32 
			fopen_s(&inFile, fileName.c_str(), "rb");
#else
			inFile = fopen(fileName.c_str(), "rb");
#endif

			if (!inFile)
			{
				return false;
			}

			UserType globalTemp;

			try {

				InFileReserver ifReserver(inFile, use_simd);
				char* buffer = nullptr;

				//	strVec.reserve(ifReserver.Num);
				// cf) empty file..
				if (false == _LoadData(ifReserver, globalTemp, lex_thr_num, parse_thr_num))
				{
					fclose(inFile);
					return false; // return true?
				}

				fclose(inFile);
			}
			catch (const char* err) { std::cout << err << "\n"; fclose(inFile); return false; }
			catch (const std::string& e) { std::cout << e << "\n"; fclose(inFile); return false; }
			catch (const std::exception& e) { std::cout << e.what() << "\n"; fclose(inFile); return false; }
			catch (...) { std::cout << "not expected error" << "\n"; fclose(inFile); return false; }


			global = std::move(globalTemp);

			return true;
		}

		static bool Loadclau_parser11DB(UserType& global, const std::string& fileName, const int thr_num) {
			UserType globalTemp = UserType("global");

			// Scan + Parse 
			if (false == LoadDataFromFile(fileName, globalTemp, thr_num, thr_num)) { return false; }
			//std::cout << "LoadData End" << "\n";

			global = std::move(globalTemp);
			return true;
		}

		static bool Saveclau_parser11DB(const UserType& global, const std::string& fileName, const bool append = false) {
			std::ofstream outFile;
			if (fileName.empty()) { return false; }
			if (false == append) {
				outFile.open(fileName);
				if (outFile.fail()) { return false; }
			}
			else {
				outFile.open(fileName, std::ios::app);
				if (outFile.fail()) { return false; }

				outFile << "\n";
			}

			/// saveFile
			global.Save1(outFile); // cf) friend

			outFile.close();

			return true;
		}

		static bool Saveclau_parser11DB2(const UserType& global, const std::string& fileName, const bool append = false) {
			std::ofstream outFile;
			if (fileName.empty()) { return false; }
			if (false == append) {
				outFile.open(fileName);
				if (outFile.fail()) { return false; }
			}
			else {
				outFile.open(fileName, std::ios::app);
				if (outFile.fail()) { return false; }

				outFile << "\n";
			}

			/// saveFile
			global.Save2(outFile); // cf) friend

			outFile.close();

			return true;
		}
	};



	enum class ValueType {
		end_of_container,
		end_of_document,
		container, // array or object or mixed
		key,
		value
	};

	class ClauParserTraverser {

		class Iterator {
		private:
			ValueType state;
			std::string value;
			clau_parser11::UserType* ut;
			std::stack<clau_parser11::UserType*> _stack; // 
			std::stack<int> _ut_stack; // usertype
			std::stack<int> _it_stack; // itemtype
			std::stack<int> _idx_stack; // ilist
			int chk_name; // key or data?
		public:
			Iterator(clau_parser11::UserType* ut) : ut(ut) {
				_stack.push(ut);
				_ut_stack.push(0);
				_it_stack.push(0);
				_idx_stack.push(0);
				chk_name = 0;

				next();
			}

			void up() {
				_stack.pop();
				_ut_stack.pop();
				_it_stack.pop();
				_idx_stack.pop();
			}

			bool down() {
				_stack.push(_stack.top()->GetUserTypeList(_ut_stack.top() - 1));
				_ut_stack.push(0);
				_it_stack.push(0);
				_idx_stack.push(0);

				return true;
			}

			bool next() {
				if (_idx_stack.top() >= _stack.top()->GetIListSize()) {
					if (_stack.size() > 1) {
						state = ValueType::end_of_container;

						value = "}";
					}
					else {
						state = ValueType::end_of_document;
					}
					return false;
				}

				if (_stack.top()->IsUserTypeList(_idx_stack.top())) {
					bool empty_key = false; // key : 
					static bool first = true;

					if (_stack.top()->GetUserTypeList(_ut_stack.top())->GetName().empty()) {
						empty_key = true;
					}

					if (empty_key || !first) {
						state = ValueType::container;
						value = "{";

						first = true;

						_idx_stack.top()++;
						_ut_stack.top()++;
					}
					else {
						first = false;
						state = ValueType::key;
						value = _stack.top()->GetUserTypeList(_ut_stack.top())->GetName();
					}
				}
				else {
					if (chk_name == 0) { // name or value
						auto item = (_stack.top()->GetItemList(_it_stack.top()));

						if (item.GetName().empty()) { // array data
							chk_name = 0;
							state = ValueType::value;
							value = item.Get();

							_idx_stack.top()++;
							_it_stack.top()++;
						}
						else {
							chk_name = 1;
							state = ValueType::key;
							value = item.GetName();
						}
					}
					else if (chk_name == 1) { // value
						chk_name = 0;

						auto item = (_stack.top()->GetItemList(_it_stack.top()));

						state = ValueType::value;
						value = item.Get();

						_idx_stack.top()++;
						_it_stack.top()++;
					}
				}

				return true;
			}

			ValueType get_type() {
				return state;
			}

			std::string get_data() {
				return value;
			}
		};
	public:

		ClauParserTraverser(clau_parser11::UserType* parsed_json) :
			parsed_json(parsed_json)
			, iterator(parsed_json)
		{
			//
		}

		ValueType next() {
			{

				ValueType before_type = iterator.get_type();

				if (before_type == ValueType::end_of_document) {
					return ValueType::end_of_document;
				}


				if (before_type == ValueType::end_of_container) {
					iterator.up();
					iterator.next();
					return iterator.get_type();
				}


				if (before_type == ValueType::container) {
					bool end_of_container = !iterator.down();

					if (end_of_container) {
						return ValueType::end_of_container;
					}
					else {
						end_of_container = !iterator.next();
					}

					return iterator.get_type();
				}

				iterator.next();
				return iterator.get_type();
			}

		}

		ValueType get_type() { return iterator.get_type(); }

		std::string get_string() {
			return iterator.get_data();
		}
		int64_t get_integer() {
			return std::stoll(get_string());
		}
		double get_floating() {
			return std::stod(get_string());
		}

	private:
		clau_parser11::UserType* parsed_json;
		Iterator iterator;
	};

	class TraverserUtility {
	public:
		static std::ostream& Save(std::ostream& stream, ClauParserTraverser& traverser) {
			while (traverser.get_type() != ValueType::end_of_document) {

				stream << traverser.get_string();

				if (traverser.get_type() == ValueType::key) {
					stream << " = ";
				}
				else {
					stream << " ";
				}

				if (traverser.get_type() == ValueType::end_of_container) {
					stream << "\n";
				}

				traverser.next();
			}

			return stream;
		}
	};


	class Maker {
	private:
		Maker(const Maker&) = delete;
		Maker& operator=(const Maker&) = delete;
	private:
		UserType* ut = nullptr;
		std::vector<UserType*> _stack;
	public:
		Maker(const std::string name = "") : ut(new UserType(name)) { _stack.push_back(ut); }
		virtual ~Maker() {
			if (ut) {
				delete ut;
			}
		}

	public:
		Maker& NewItem(const std::string& name, const std::string& value) {
			_stack.back()->AddItem(name, value);
			return *this;
		}
		Maker& NewGroup(const std::string& name) {
			_stack.back()->AddUserTypeItem(UserType(name));
			_stack.push_back(_stack.back()->GetUserTypeList(_stack.back()->GetUserTypeListSize() - 1));
			return *this;
		}
		Maker& NewGroup(Maker& other) {
			if (this->ut == other.ut) {
				throw "New Group this == other";
			}

			_stack.back()->LinkUserType(other.ut);
			other.ut = nullptr;
			return *this;
		}
		Maker& EndGroup() {
			_stack.pop_back();
			return *this;
		}
		UserType* Get() {
			UserType* result = ut;
			this->ut = nullptr;
			return result;
		}
	};
}

#endif



