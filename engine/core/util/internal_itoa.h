//
// internal_itoa.h
//
// 2016Äê12ÔÂ27ÈÕ 16:31:13
//
// moved from milo's benchmark
//

#pragma once
namespace Echo
{
	namespace internal
	{
		// use branchlut: 
		// Use branchiing to divide-and-conquer the range of value.
		// make computation more parallel.

		// look up table.
		const char gDigitsLut[200] = {
			'0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
			'1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
			'2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
			'3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
			'4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
			'5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
			'6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
			'7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
			'8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
			'9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
		};

		void u32toa_branchlut(uint32_t value, char* buffer) {
			if (value < 10000) {
				const uint32_t d1 = (value / 100) << 1;
				const uint32_t d2 = (value % 100) << 1;

				if (value >= 1000)
					*buffer++ = gDigitsLut[d1];
				if (value >= 100)
					*buffer++ = gDigitsLut[d1 + 1];
				if (value >= 10)
					*buffer++ = gDigitsLut[d2];
				*buffer++ = gDigitsLut[d2 + 1];
			}
			else if (value < 100000000) {
				// value = bbbbcccc
				const uint32_t b = value / 10000;
				const uint32_t c = value % 10000;

				const uint32_t d1 = (b / 100) << 1;
				const uint32_t d2 = (b % 100) << 1;

				const uint32_t d3 = (c / 100) << 1;
				const uint32_t d4 = (c % 100) << 1;

				if (value >= 10000000)
					*buffer++ = gDigitsLut[d1];
				if (value >= 1000000)
					*buffer++ = gDigitsLut[d1 + 1];
				if (value >= 100000)
					*buffer++ = gDigitsLut[d2];
				*buffer++ = gDigitsLut[d2 + 1];

				*buffer++ = gDigitsLut[d3];
				*buffer++ = gDigitsLut[d3 + 1];
				*buffer++ = gDigitsLut[d4];
				*buffer++ = gDigitsLut[d4 + 1];
			}
			else {
				// value = aabbbbcccc in decimal

				const uint32_t a = value / 100000000; // 1 to 42
				value %= 100000000;

				if (a >= 10) {
					const unsigned i = a << 1;
					*buffer++ = gDigitsLut[i];
					*buffer++ = gDigitsLut[i + 1];
				}
				else
					*buffer++ = '0' + static_cast<char>(a);

				const uint32_t b = value / 10000; // 0 to 9999
				const uint32_t c = value % 10000; // 0 to 9999

				const uint32_t d1 = (b / 100) << 1;
				const uint32_t d2 = (b % 100) << 1;

				const uint32_t d3 = (c / 100) << 1;
				const uint32_t d4 = (c % 100) << 1;

				*buffer++ = gDigitsLut[d1];
				*buffer++ = gDigitsLut[d1 + 1];
				*buffer++ = gDigitsLut[d2];
				*buffer++ = gDigitsLut[d2 + 1];
				*buffer++ = gDigitsLut[d3];
				*buffer++ = gDigitsLut[d3 + 1];
				*buffer++ = gDigitsLut[d4];
				*buffer++ = gDigitsLut[d4 + 1];
			}
			*buffer++ = '\0';
		}

		void i32toa_branchlut(int32_t value, char* buffer) {
			uint32_t u = static_cast<uint32_t>(value);
			if (value < 0) {
				*buffer++ = '-';
				u = ~u + 1;
			}

			u32toa_branchlut(u, buffer);
		}

		void u64toa_branchlut(uint64_t value, char* buffer) {
			if (value < 100000000) {
				uint32_t v = static_cast<uint32_t>(value);
				if (v < 10000) {
					const uint32_t d1 = (v / 100) << 1;
					const uint32_t d2 = (v % 100) << 1;

					if (v >= 1000)
						*buffer++ = gDigitsLut[d1];
					if (v >= 100)
						*buffer++ = gDigitsLut[d1 + 1];
					if (v >= 10)
						*buffer++ = gDigitsLut[d2];
					*buffer++ = gDigitsLut[d2 + 1];
				}
				else {
					// value = bbbbcccc
					const uint32_t b = v / 10000;
					const uint32_t c = v % 10000;

					const uint32_t d1 = (b / 100) << 1;
					const uint32_t d2 = (b % 100) << 1;

					const uint32_t d3 = (c / 100) << 1;
					const uint32_t d4 = (c % 100) << 1;

					if (value >= 10000000)
						*buffer++ = gDigitsLut[d1];
					if (value >= 1000000)
						*buffer++ = gDigitsLut[d1 + 1];
					if (value >= 100000)
						*buffer++ = gDigitsLut[d2];
					*buffer++ = gDigitsLut[d2 + 1];

					*buffer++ = gDigitsLut[d3];
					*buffer++ = gDigitsLut[d3 + 1];
					*buffer++ = gDigitsLut[d4];
					*buffer++ = gDigitsLut[d4 + 1];
				}
			}
			else if (value < 10000000000000000) {
				const uint32_t v0 = static_cast<uint32_t>(value / 100000000);
				const uint32_t v1 = static_cast<uint32_t>(value % 100000000);

				const uint32_t b0 = v0 / 10000;
				const uint32_t c0 = v0 % 10000;

				const uint32_t d1 = (b0 / 100) << 1;
				const uint32_t d2 = (b0 % 100) << 1;

				const uint32_t d3 = (c0 / 100) << 1;
				const uint32_t d4 = (c0 % 100) << 1;

				const uint32_t b1 = v1 / 10000;
				const uint32_t c1 = v1 % 10000;

				const uint32_t d5 = (b1 / 100) << 1;
				const uint32_t d6 = (b1 % 100) << 1;

				const uint32_t d7 = (c1 / 100) << 1;
				const uint32_t d8 = (c1 % 100) << 1;

				if (value >= 1000000000000000)
					*buffer++ = gDigitsLut[d1];
				if (value >= 100000000000000)
					*buffer++ = gDigitsLut[d1 + 1];
				if (value >= 10000000000000)
					*buffer++ = gDigitsLut[d2];
				if (value >= 1000000000000)
					*buffer++ = gDigitsLut[d2 + 1];
				if (value >= 100000000000)
					*buffer++ = gDigitsLut[d3];
				if (value >= 10000000000)
					*buffer++ = gDigitsLut[d3 + 1];
				if (value >= 1000000000)
					*buffer++ = gDigitsLut[d4];
				if (value >= 100000000)
					*buffer++ = gDigitsLut[d4 + 1];

				*buffer++ = gDigitsLut[d5];
				*buffer++ = gDigitsLut[d5 + 1];
				*buffer++ = gDigitsLut[d6];
				*buffer++ = gDigitsLut[d6 + 1];
				*buffer++ = gDigitsLut[d7];
				*buffer++ = gDigitsLut[d7 + 1];
				*buffer++ = gDigitsLut[d8];
				*buffer++ = gDigitsLut[d8 + 1];
			}
			else {
				const uint32_t a = static_cast<uint32_t>(value / 10000000000000000); // 1 to 1844
				value %= 10000000000000000;

				if (a < 10)
					*buffer++ = '0' + static_cast<char>(a);
				else if (a < 100) {
					const uint32_t i = a << 1;
					*buffer++ = gDigitsLut[i];
					*buffer++ = gDigitsLut[i + 1];
				}
				else if (a < 1000) {
					*buffer++ = '0' + static_cast<char>(a / 100);

					const uint32_t i = (a % 100) << 1;
					*buffer++ = gDigitsLut[i];
					*buffer++ = gDigitsLut[i + 1];
				}
				else {
					const uint32_t i = (a / 100) << 1;
					const uint32_t j = (a % 100) << 1;
					*buffer++ = gDigitsLut[i];
					*buffer++ = gDigitsLut[i + 1];
					*buffer++ = gDigitsLut[j];
					*buffer++ = gDigitsLut[j + 1];
				}

				const uint32_t v0 = static_cast<uint32_t>(value / 100000000);
				const uint32_t v1 = static_cast<uint32_t>(value % 100000000);

				const uint32_t b0 = v0 / 10000;
				const uint32_t c0 = v0 % 10000;

				const uint32_t d1 = (b0 / 100) << 1;
				const uint32_t d2 = (b0 % 100) << 1;

				const uint32_t d3 = (c0 / 100) << 1;
				const uint32_t d4 = (c0 % 100) << 1;

				const uint32_t b1 = v1 / 10000;
				const uint32_t c1 = v1 % 10000;

				const uint32_t d5 = (b1 / 100) << 1;
				const uint32_t d6 = (b1 % 100) << 1;

				const uint32_t d7 = (c1 / 100) << 1;
				const uint32_t d8 = (c1 % 100) << 1;

				*buffer++ = gDigitsLut[d1];
				*buffer++ = gDigitsLut[d1 + 1];
				*buffer++ = gDigitsLut[d2];
				*buffer++ = gDigitsLut[d2 + 1];
				*buffer++ = gDigitsLut[d3];
				*buffer++ = gDigitsLut[d3 + 1];
				*buffer++ = gDigitsLut[d4];
				*buffer++ = gDigitsLut[d4 + 1];
				*buffer++ = gDigitsLut[d5];
				*buffer++ = gDigitsLut[d5 + 1];
				*buffer++ = gDigitsLut[d6];
				*buffer++ = gDigitsLut[d6 + 1];
				*buffer++ = gDigitsLut[d7];
				*buffer++ = gDigitsLut[d7 + 1];
				*buffer++ = gDigitsLut[d8];
				*buffer++ = gDigitsLut[d8 + 1];
			}

			*buffer = '\0';
		}

		void i64toa_branchlut(int64_t value, char* buffer) {
			uint64_t u = static_cast<uint64_t>(value);
			if (value < 0) {
				*buffer++ = '-';
				u = ~u + 1;
			}

			u64toa_branchlut(u, buffer);
		}
	}
}