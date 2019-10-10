#include "decoder_mpeg1.h"
#include <assert.h>

static int CLAMP(int v, int m, int ma) 
{ 
	if(v<m) v=m; 
	if(v>ma) v=ma; 
	
	return v;
}

#define SAFLE_DELETE_ARRAY(arr) if(arr) {delete[] arr;} arr=NULL

namespace cmpeg 
{
	namespace MPEG1 {

		static double PICTURE_RATE[] = {
			0.000, 23.976, 24.000, 25.000, 29.970, 30.000, 50.000, 59.940,
			60.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000
		};

		static uint8_t PREMULTIPLIER_MATRIX[] = {
			32, 44, 42, 38, 32, 25, 17, 9,
				44, 62, 58, 52, 44, 35, 24, 12,
				42, 58, 55, 49, 42, 33, 23, 12,
				38, 52, 49, 44, 38, 30, 20, 10,
				32, 44, 42, 38, 32, 25, 17, 9,
				25, 35, 33, 30, 25, 20, 14, 7,
				17, 24, 23, 20, 17, 14, 9, 5,
				9, 12, 12, 10, 9, 7, 5, 2
		};

		static uint8_t ZIG_ZAG[] = {
			0, 1, 8, 16, 9, 2, 3, 10,
			17, 24, 32, 25, 18, 11, 4, 5,
			12, 19, 26, 33, 40, 48, 41, 34,
			27, 20, 13, 6, 7, 14, 21, 28,
			35, 42, 49, 56, 57, 50, 43, 36,
			29, 22, 15, 23, 30, 37, 44, 51,
			58, 59, 52, 45, 38, 31, 39, 46,
			53, 60, 61, 54, 47, 55, 62, 63
		};

		static int DEFAULT_INTRA_QUANT_MATRIX[] = {
			8, 16, 19, 22, 26, 27, 29, 34,
			16, 16, 22, 24, 27, 29, 34, 37,
			19, 22, 26, 27, 29, 34, 34, 38,
			22, 22, 26, 27, 29, 34, 37, 40,
			22, 26, 27, 29, 32, 35, 40, 48,
			26, 27, 29, 32, 35, 40, 48, 58,
			26, 27, 29, 34, 38, 46, 56, 69,
			27, 29, 35, 38, 46, 56, 69, 83
		};

		static int DEFAULT_NON_INTRA_QUANT_MATRIX[] = {
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16
		};

		static int16_t MACROBLOCK_ADDRESS_INCREMENT[] = {
			1 * 3, 2 * 3, 0, //   0
			3 * 3, 4 * 3, 0, //   1  0
			0, 0, 1, //   2  1.
			5 * 3, 6 * 3, 0, //   3  00
			7 * 3, 8 * 3, 0, //   4  01
			9 * 3, 10 * 3, 0, //   5  000
			11 * 3, 12 * 3, 0, //   6  001
			0, 0, 3, //   7  010.
			0, 0, 2, //   8  011.
			13 * 3, 14 * 3, 0, //   9  0000
			15 * 3, 16 * 3, 0, //  10  0001
			0, 0, 5, //  11  0010.
			0, 0, 4, //  12  0011.
			17 * 3, 18 * 3, 0, //  13  0000 0
			19 * 3, 20 * 3, 0, //  14  0000 1
			0, 0, 7, //  15  0001 0.
			0, 0, 6, //  16  0001 1.
			21 * 3, 22 * 3, 0, //  17  0000 00
			23 * 3, 24 * 3, 0, //  18  0000 01
			25 * 3, 26 * 3, 0, //  19  0000 10
			27 * 3, 28 * 3, 0, //  20  0000 11
			-1, 29 * 3, 0, //  21  0000 000
			-1, 30 * 3, 0, //  22  0000 001
			31 * 3, 32 * 3, 0, //  23  0000 010
			33 * 3, 34 * 3, 0, //  24  0000 011
			35 * 3, 36 * 3, 0, //  25  0000 100
			37 * 3, 38 * 3, 0, //  26  0000 101
			0, 0, 9, //  27  0000 110.
			0, 0, 8, //  28  0000 111.
			39 * 3, 40 * 3, 0, //  29  0000 0001
			41 * 3, 42 * 3, 0, //  30  0000 0011
			43 * 3, 44 * 3, 0, //  31  0000 0100
			45 * 3, 46 * 3, 0, //  32  0000 0101
			0, 0, 15, //  33  0000 0110.
			0, 0, 14, //  34  0000 0111.
			0, 0, 13, //  35  0000 1000.
			0, 0, 12, //  36  0000 1001.
			0, 0, 11, //  37  0000 1010.
			0, 0, 10, //  38  0000 1011.
			47 * 3, -1, 0, //  39  0000 0001 0
			-1, 48 * 3, 0, //  40  0000 0001 1
			49 * 3, 50 * 3, 0, //  41  0000 0011 0
			51 * 3, 52 * 3, 0, //  42  0000 0011 1
			53 * 3, 54 * 3, 0, //  43  0000 0100 0
			55 * 3, 56 * 3, 0, //  44  0000 0100 1
			57 * 3, 58 * 3, 0, //  45  0000 0101 0
			59 * 3, 60 * 3, 0, //  46  0000 0101 1
			61 * 3, -1, 0, //  47  0000 0001 00
			-1, 62 * 3, 0, //  48  0000 0001 11
			63 * 3, 64 * 3, 0, //  49  0000 0011 00
			65 * 3, 66 * 3, 0, //  50  0000 0011 01
			67 * 3, 68 * 3, 0, //  51  0000 0011 10
			69 * 3, 70 * 3, 0, //  52  0000 0011 11
			71 * 3, 72 * 3, 0, //  53  0000 0100 00
			73 * 3, 74 * 3, 0, //  54  0000 0100 01
			0, 0, 21, //  55  0000 0100 10.
			0, 0, 20, //  56  0000 0100 11.
			0, 0, 19, //  57  0000 0101 00.
			0, 0, 18, //  58  0000 0101 01.
			0, 0, 17, //  59  0000 0101 10.
			0, 0, 16, //  60  0000 0101 11.
			0, 0, 35, //  61  0000 0001 000. -- macroblock_escape
			0, 0, 34, //  62  0000 0001 111. -- macroblock_stuffing
			0, 0, 33, //  63  0000 0011 000.
			0, 0, 32, //  64  0000 0011 001.
			0, 0, 31, //  65  0000 0011 010.
			0, 0, 30, //  66  0000 0011 011.
			0, 0, 29, //  67  0000 0011 100.
			0, 0, 28, //  68  0000 0011 101.
			0, 0, 27, //  69  0000 0011 110.
			0, 0, 26, //  70  0000 0011 111.
			0, 0, 25, //  71  0000 0100 000.
			0, 0, 24, //  72  0000 0100 001.
			0, 0, 23, //  73  0000 0100 010.
			0, 0, 22  //  74  0000 0100 011.
		};

		int16_t CODE_BLOCK_PATTERN[] = {
			2 * 3, 1 * 3, 0,  //   0
			3 * 3, 6 * 3, 0,  //   1  1
			4 * 3, 5 * 3, 0,  //   2  0
			8 * 3, 11 * 3, 0,  //   3  10
			12 * 3, 13 * 3, 0,  //   4  00
			9 * 3, 7 * 3, 0,  //   5  01
			10 * 3, 14 * 3, 0,  //   6  11
			20 * 3, 19 * 3, 0,  //   7  011
			18 * 3, 16 * 3, 0,  //   8  100
			23 * 3, 17 * 3, 0,  //   9  010
			27 * 3, 25 * 3, 0,  //  10  110
			21 * 3, 28 * 3, 0,  //  11  101
			15 * 3, 22 * 3, 0,  //  12  000
			24 * 3, 26 * 3, 0,  //  13  001
			0, 0, 60,  //  14  111.
			35 * 3, 40 * 3, 0,  //  15  0000
			44 * 3, 48 * 3, 0,  //  16  1001
			38 * 3, 36 * 3, 0,  //  17  0101
			42 * 3, 47 * 3, 0,  //  18  1000
			29 * 3, 31 * 3, 0,  //  19  0111
			39 * 3, 32 * 3, 0,  //  20  0110
			0, 0, 32,  //  21  1010.
			45 * 3, 46 * 3, 0,  //  22  0001
			33 * 3, 41 * 3, 0,  //  23  0100
			43 * 3, 34 * 3, 0,  //  24  0010
			0, 0, 4,  //  25  1101.
			30 * 3, 37 * 3, 0,  //  26  0011
			0, 0, 8,  //  27  1100.
			0, 0, 16,  //  28  1011.
			0, 0, 44,  //  29  0111 0.
			50 * 3, 56 * 3, 0,  //  30  0011 0
			0, 0, 28,  //  31  0111 1.
			0, 0, 52,  //  32  0110 1.
			0, 0, 62,  //  33  0100 0.
			61 * 3, 59 * 3, 0,  //  34  0010 1
			52 * 3, 60 * 3, 0,  //  35  0000 0
			0, 0, 1,  //  36  0101 1.
			55 * 3, 54 * 3, 0,  //  37  0011 1
			0, 0, 61,  //  38  0101 0.
			0, 0, 56,  //  39  0110 0.
			57 * 3, 58 * 3, 0,  //  40  0000 1
			0, 0, 2,  //  41  0100 1.
			0, 0, 40,  //  42  1000 0.
			51 * 3, 62 * 3, 0,  //  43  0010 0
			0, 0, 48,  //  44  1001 0.
			64 * 3, 63 * 3, 0,  //  45  0001 0
			49 * 3, 53 * 3, 0,  //  46  0001 1
			0, 0, 20,  //  47  1000 1.
			0, 0, 12,  //  48  1001 1.
			80 * 3, 83 * 3, 0,  //  49  0001 10
			0, 0, 63,  //  50  0011 00.
			77 * 3, 75 * 3, 0,  //  51  0010 00
			65 * 3, 73 * 3, 0,  //  52  0000 00
			84 * 3, 66 * 3, 0,  //  53  0001 11
			0, 0, 24,  //  54  0011 11.
			0, 0, 36,  //  55  0011 10.
			0, 0, 3,  //  56  0011 01.
			69 * 3, 87 * 3, 0,  //  57  0000 10
			81 * 3, 79 * 3, 0,  //  58  0000 11
			68 * 3, 71 * 3, 0,  //  59  0010 11
			70 * 3, 78 * 3, 0,  //  60  0000 01
			67 * 3, 76 * 3, 0,  //  61  0010 10
			72 * 3, 74 * 3, 0,  //  62  0010 01
			86 * 3, 85 * 3, 0,  //  63  0001 01
			88 * 3, 82 * 3, 0,  //  64  0001 00
			-1, 94 * 3, 0,  //  65  0000 000
			95 * 3, 97 * 3, 0,  //  66  0001 111
			0, 0, 33,  //  67  0010 100.
			0, 0, 9,  //  68  0010 110.
			106 * 3, 110 * 3, 0,  //  69  0000 100
			102 * 3, 116 * 3, 0,  //  70  0000 010
			0, 0, 5,  //  71  0010 111.
			0, 0, 10,  //  72  0010 010.
			93 * 3, 89 * 3, 0,  //  73  0000 001
			0, 0, 6,  //  74  0010 011.
			0, 0, 18,  //  75  0010 001.
			0, 0, 17,  //  76  0010 101.
			0, 0, 34,  //  77  0010 000.
			113 * 3, 119 * 3, 0,  //  78  0000 011
			103 * 3, 104 * 3, 0,  //  79  0000 111
			90 * 3, 92 * 3, 0,  //  80  0001 100
			109 * 3, 107 * 3, 0,  //  81  0000 110
			117 * 3, 118 * 3, 0,  //  82  0001 001
			101 * 3, 99 * 3, 0,  //  83  0001 101
			98 * 3, 96 * 3, 0,  //  84  0001 110
			100 * 3, 91 * 3, 0,  //  85  0001 011
			114 * 3, 115 * 3, 0,  //  86  0001 010
			105 * 3, 108 * 3, 0,  //  87  0000 101
			112 * 3, 111 * 3, 0,  //  88  0001 000
			121 * 3, 125 * 3, 0,  //  89  0000 0011
			0, 0, 41,  //  90  0001 1000.
			0, 0, 14,  //  91  0001 0111.
			0, 0, 21,  //  92  0001 1001.
			124 * 3, 122 * 3, 0,  //  93  0000 0010
			120 * 3, 123 * 3, 0,  //  94  0000 0001
			0, 0, 11,  //  95  0001 1110.
			0, 0, 19,  //  96  0001 1101.
			0, 0, 7,  //  97  0001 1111.
			0, 0, 35,  //  98  0001 1100.
			0, 0, 13,  //  99  0001 1011.
			0, 0, 50,  // 100  0001 0110.
			0, 0, 49,  // 101  0001 1010.
			0, 0, 58,  // 102  0000 0100.
			0, 0, 37,  // 103  0000 1110.
			0, 0, 25,  // 104  0000 1111.
			0, 0, 45,  // 105  0000 1010.
			0, 0, 57,  // 106  0000 1000.
			0, 0, 26,  // 107  0000 1101.
			0, 0, 29,  // 108  0000 1011.
			0, 0, 38,  // 109  0000 1100.
			0, 0, 53,  // 110  0000 1001.
			0, 0, 23,  // 111  0001 0001.
			0, 0, 43,  // 112  0001 0000.
			0, 0, 46,  // 113  0000 0110.
			0, 0, 42,  // 114  0001 0100.
			0, 0, 22,  // 115  0001 0101.
			0, 0, 54,  // 116  0000 0101.
			0, 0, 51,  // 117  0001 0010.
			0, 0, 15,  // 118  0001 0011.
			0, 0, 30,  // 119  0000 0111.
			0, 0, 39,  // 120  0000 0001 0.
			0, 0, 47,  // 121  0000 0011 0.
			0, 0, 55,  // 122  0000 0010 1.
			0, 0, 27,  // 123  0000 0001 1.
			0, 0, 59,  // 124  0000 0010 0.
			0, 0, 31   // 125  0000 0011 1.
		};

		int MOTION[] = {
			1 * 3, 2 * 3, 0,  //   0
			4 * 3, 3 * 3, 0,  //   1  0
			0, 0, 0,  //   2  1.
			6 * 3, 5 * 3, 0,  //   3  01
			8 * 3, 7 * 3, 0,  //   4  00
			0, 0, -1,  //   5  011.
			0, 0, 1,  //   6  010.
			9 * 3, 10 * 3, 0,  //   7  001
			12 * 3, 11 * 3, 0,  //   8  000
			0, 0, 2,  //   9  0010.
			0, 0, -2,  //  10  0011.
			14 * 3, 15 * 3, 0,  //  11  0001
			16 * 3, 13 * 3, 0,  //  12  0000
			20 * 3, 18 * 3, 0,  //  13  0000 1
			0, 0, 3,  //  14  0001 0.
			0, 0, -3,  //  15  0001 1.
			17 * 3, 19 * 3, 0,  //  16  0000 0
			-1, 23 * 3, 0,  //  17  0000 00
			27 * 3, 25 * 3, 0,  //  18  0000 11
			26 * 3, 21 * 3, 0,  //  19  0000 01
			24 * 3, 22 * 3, 0,  //  20  0000 10
			32 * 3, 28 * 3, 0,  //  21  0000 011
			29 * 3, 31 * 3, 0,  //  22  0000 101
			-1, 33 * 3, 0,  //  23  0000 001
			36 * 3, 35 * 3, 0,  //  24  0000 100
			0, 0, -4,  //  25  0000 111.
			30 * 3, 34 * 3, 0,  //  26  0000 010
			0, 0, 4,  //  27  0000 110.
			0, 0, -7,  //  28  0000 0111.
			0, 0, 5,  //  29  0000 1010.
			37 * 3, 41 * 3, 0,  //  30  0000 0100
			0, 0, -5,  //  31  0000 1011.
			0, 0, 7,  //  32  0000 0110.
			38 * 3, 40 * 3, 0,  //  33  0000 0011
			42 * 3, 39 * 3, 0,  //  34  0000 0101
			0, 0, -6,  //  35  0000 1001.
			0, 0, 6,  //  36  0000 1000.
			51 * 3, 54 * 3, 0,  //  37  0000 0100 0
			50 * 3, 49 * 3, 0,  //  38  0000 0011 0
			45 * 3, 46 * 3, 0,  //  39  0000 0101 1
			52 * 3, 47 * 3, 0,  //  40  0000 0011 1
			43 * 3, 53 * 3, 0,  //  41  0000 0100 1
			44 * 3, 48 * 3, 0,  //  42  0000 0101 0
			0, 0, 10,  //  43  0000 0100 10.
			0, 0, 9,  //  44  0000 0101 00.
			0, 0, 8,  //  45  0000 0101 10.
			0, 0, -8,  //  46  0000 0101 11.
			57 * 3, 66 * 3, 0,  //  47  0000 0011 11
			0, 0, -9,  //  48  0000 0101 01.
			60 * 3, 64 * 3, 0,  //  49  0000 0011 01
			56 * 3, 61 * 3, 0,  //  50  0000 0011 00
			55 * 3, 62 * 3, 0,  //  51  0000 0100 00
			58 * 3, 63 * 3, 0,  //  52  0000 0011 10
			0, 0, -10,  //  53  0000 0100 11.
			59 * 3, 65 * 3, 0,  //  54  0000 0100 01
			0, 0, 12,  //  55  0000 0100 000.
			0, 0, 16,  //  56  0000 0011 000.
			0, 0, 13,  //  57  0000 0011 110.
			0, 0, 14,  //  58  0000 0011 100.
			0, 0, 11,  //  59  0000 0100 010.
			0, 0, 15,  //  60  0000 0011 010.
			0, 0, -16,  //  61  0000 0011 001.
			0, 0, -12,  //  62  0000 0100 001.
			0, 0, -14,  //  63  0000 0011 101.
			0, 0, -15,  //  64  0000 0011 011.
			0, 0, -11,  //  65  0000 0100 011.
			0, 0, -13   //  66  0000 0011 111.
		};

		int8_t DCT_DC_SIZE_LUMINANCE[] = {
			2 * 3, 1 * 3, 0,  //   0
			6 * 3, 5 * 3, 0,  //   1  1
			3 * 3, 4 * 3, 0,  //   2  0
			0, 0, 1,  //   3  00.
			0, 0, 2,  //   4  01.
			9 * 3, 8 * 3, 0,  //   5  11
			7 * 3, 10 * 3, 0,  //   6  10
			0, 0, 0,  //   7  100.
			12 * 3, 11 * 3, 0,  //   8  111
			0, 0, 4,  //   9  110.
			0, 0, 3,  //  10  101.
			13 * 3, 14 * 3, 0,  //  11  1111
			0, 0, 5,  //  12  1110.
			0, 0, 6,  //  13  1111 0.
			16 * 3, 15 * 3, 0,  //  14  1111 1
			17 * 3, -1, 0,  //  15  1111 11
			0, 0, 7,  //  16  1111 10.
			0, 0, 8   //  17  1111 110.
		};

		int8_t DCT_DC_SIZE_CHROMINANCE[] = {
			2 * 3, 1 * 3, 0,  //   0
			4 * 3, 3 * 3, 0,  //   1  1
			6 * 3, 5 * 3, 0,  //   2  0
			8 * 3, 7 * 3, 0,  //   3  11
			0, 0, 2,  //   4  10.
			0, 0, 1,  //   5  01.
			0, 0, 0,  //   6  00.
			10 * 3, 9 * 3, 0,  //   7  111
			0, 0, 3,  //   8  110.
			12 * 3, 11 * 3, 0,  //   9  1111
			0, 0, 4,  //  10  1110.
			14 * 3, 13 * 3, 0,  //  11  1111 1
			0, 0, 5,  //  12  1111 0.
			16 * 3, 15 * 3, 0,  //  13  1111 11
			0, 0, 6,  //  14  1111 10.
			17 * 3, -1, 0,  //  15  1111 111
			0, 0, 7,  //  16  1111 110.
			0, 0, 8   //  17  1111 1110.
		};

		//  dct_coeff bitmap:
		//    0xff00  run
		//    0x00ff  level

		//  Decoded values are unsigned. Sign bit follows in the stream.

		//  Interpretation of the value 0x0001
		//    for dc_coeff_first:  run=0, level=1
		//    for dc_coeff_next:   If the next bit is 1: run=0, level=1
		//                         If the next bit is 0: end_of_block

		//  escape decodes as 0xffff.

		int32_t DCT_COEFF[] = {
			1 * 3, 2 * 3, 0,  //   0
			4 * 3, 3 * 3, 0,  //   1  0
			0, 0, 0x0001,  //   2  1.
			7 * 3, 8 * 3, 0,  //   3  01
			6 * 3, 5 * 3, 0,  //   4  00
			13 * 3, 9 * 3, 0,  //   5  001
			11 * 3, 10 * 3, 0,  //   6  000
			14 * 3, 12 * 3, 0,  //   7  010
			0, 0, 0x0101,  //   8  011.
			20 * 3, 22 * 3, 0,  //   9  0011
			18 * 3, 21 * 3, 0,  //  10  0001
			16 * 3, 19 * 3, 0,  //  11  0000
			0, 0, 0x0201,  //  12  0101.
			17 * 3, 15 * 3, 0,  //  13  0010
			0, 0, 0x0002,  //  14  0100.
			0, 0, 0x0003,  //  15  0010 1.
			27 * 3, 25 * 3, 0,  //  16  0000 0
			29 * 3, 31 * 3, 0,  //  17  0010 0
			24 * 3, 26 * 3, 0,  //  18  0001 0
			32 * 3, 30 * 3, 0,  //  19  0000 1
			0, 0, 0x0401,  //  20  0011 0.
			23 * 3, 28 * 3, 0,  //  21  0001 1
			0, 0, 0x0301,  //  22  0011 1.
			0, 0, 0x0102,  //  23  0001 10.
			0, 0, 0x0701,  //  24  0001 00.
			0, 0, 0xffff,  //  25  0000 01. -- escape
			0, 0, 0x0601,  //  26  0001 01.
			37 * 3, 36 * 3, 0,  //  27  0000 00
			0, 0, 0x0501,  //  28  0001 11.
			35 * 3, 34 * 3, 0,  //  29  0010 00
			39 * 3, 38 * 3, 0,  //  30  0000 11
			33 * 3, 42 * 3, 0,  //  31  0010 01
			40 * 3, 41 * 3, 0,  //  32  0000 10
			52 * 3, 50 * 3, 0,  //  33  0010 010
			54 * 3, 53 * 3, 0,  //  34  0010 001
			48 * 3, 49 * 3, 0,  //  35  0010 000
			43 * 3, 45 * 3, 0,  //  36  0000 001
			46 * 3, 44 * 3, 0,  //  37  0000 000
			0, 0, 0x0801,  //  38  0000 111.
			0, 0, 0x0004,  //  39  0000 110.
			0, 0, 0x0202,  //  40  0000 100.
			0, 0, 0x0901,  //  41  0000 101.
			51 * 3, 47 * 3, 0,  //  42  0010 011
			55 * 3, 57 * 3, 0,  //  43  0000 0010
			60 * 3, 56 * 3, 0,  //  44  0000 0001
			59 * 3, 58 * 3, 0,  //  45  0000 0011
			61 * 3, 62 * 3, 0,  //  46  0000 0000
			0, 0, 0x0a01,  //  47  0010 0111.
			0, 0, 0x0d01,  //  48  0010 0000.
			0, 0, 0x0006,  //  49  0010 0001.
			0, 0, 0x0103,  //  50  0010 0101.
			0, 0, 0x0005,  //  51  0010 0110.
			0, 0, 0x0302,  //  52  0010 0100.
			0, 0, 0x0b01,  //  53  0010 0011.
			0, 0, 0x0c01,  //  54  0010 0010.
			76 * 3, 75 * 3, 0,  //  55  0000 0010 0
			67 * 3, 70 * 3, 0,  //  56  0000 0001 1
			73 * 3, 71 * 3, 0,  //  57  0000 0010 1
			78 * 3, 74 * 3, 0,  //  58  0000 0011 1
			72 * 3, 77 * 3, 0,  //  59  0000 0011 0
			69 * 3, 64 * 3, 0,  //  60  0000 0001 0
			68 * 3, 63 * 3, 0,  //  61  0000 0000 0
			66 * 3, 65 * 3, 0,  //  62  0000 0000 1
			81 * 3, 87 * 3, 0,  //  63  0000 0000 01
			91 * 3, 80 * 3, 0,  //  64  0000 0001 01
			82 * 3, 79 * 3, 0,  //  65  0000 0000 11
			83 * 3, 86 * 3, 0,  //  66  0000 0000 10
			93 * 3, 92 * 3, 0,  //  67  0000 0001 10
			84 * 3, 85 * 3, 0,  //  68  0000 0000 00
			90 * 3, 94 * 3, 0,  //  69  0000 0001 00
			88 * 3, 89 * 3, 0,  //  70  0000 0001 11
			0, 0, 0x0203,  //  71  0000 0010 11.
			0, 0, 0x0104,  //  72  0000 0011 00.
			0, 0, 0x0007,  //  73  0000 0010 10.
			0, 0, 0x0402,  //  74  0000 0011 11.
			0, 0, 0x0502,  //  75  0000 0010 01.
			0, 0, 0x1001,  //  76  0000 0010 00.
			0, 0, 0x0f01,  //  77  0000 0011 01.
			0, 0, 0x0e01,  //  78  0000 0011 10.
			105 * 3, 107 * 3, 0,  //  79  0000 0000 111
			111 * 3, 114 * 3, 0,  //  80  0000 0001 011
			104 * 3, 97 * 3, 0,  //  81  0000 0000 010
			125 * 3, 119 * 3, 0,  //  82  0000 0000 110
			96 * 3, 98 * 3, 0,  //  83  0000 0000 100
			-1, 123 * 3, 0,  //  84  0000 0000 000
			95 * 3, 101 * 3, 0,  //  85  0000 0000 001
			106 * 3, 121 * 3, 0,  //  86  0000 0000 101
			99 * 3, 102 * 3, 0,  //  87  0000 0000 011
			113 * 3, 103 * 3, 0,  //  88  0000 0001 110
			112 * 3, 116 * 3, 0,  //  89  0000 0001 111
			110 * 3, 100 * 3, 0,  //  90  0000 0001 000
			124 * 3, 115 * 3, 0,  //  91  0000 0001 010
			117 * 3, 122 * 3, 0,  //  92  0000 0001 101
			109 * 3, 118 * 3, 0,  //  93  0000 0001 100
			120 * 3, 108 * 3, 0,  //  94  0000 0001 001
			127 * 3, 136 * 3, 0,  //  95  0000 0000 0010
			139 * 3, 140 * 3, 0,  //  96  0000 0000 1000
			130 * 3, 126 * 3, 0,  //  97  0000 0000 0101
			145 * 3, 146 * 3, 0,  //  98  0000 0000 1001
			128 * 3, 129 * 3, 0,  //  99  0000 0000 0110
			0, 0, 0x0802,  // 100  0000 0001 0001.
			132 * 3, 134 * 3, 0,  // 101  0000 0000 0011
			155 * 3, 154 * 3, 0,  // 102  0000 0000 0111
			0, 0, 0x0008,  // 103  0000 0001 1101.
			137 * 3, 133 * 3, 0,  // 104  0000 0000 0100
			143 * 3, 144 * 3, 0,  // 105  0000 0000 1110
			151 * 3, 138 * 3, 0,  // 106  0000 0000 1010
			142 * 3, 141 * 3, 0,  // 107  0000 0000 1111
			0, 0, 0x000a,  // 108  0000 0001 0011.
			0, 0, 0x0009,  // 109  0000 0001 1000.
			0, 0, 0x000b,  // 110  0000 0001 0000.
			0, 0, 0x1501,  // 111  0000 0001 0110.
			0, 0, 0x0602,  // 112  0000 0001 1110.
			0, 0, 0x0303,  // 113  0000 0001 1100.
			0, 0, 0x1401,  // 114  0000 0001 0111.
			0, 0, 0x0702,  // 115  0000 0001 0101.
			0, 0, 0x1101,  // 116  0000 0001 1111.
			0, 0, 0x1201,  // 117  0000 0001 1010.
			0, 0, 0x1301,  // 118  0000 0001 1001.
			148 * 3, 152 * 3, 0,  // 119  0000 0000 1101
			0, 0, 0x0403,  // 120  0000 0001 0010.
			153 * 3, 150 * 3, 0,  // 121  0000 0000 1011
			0, 0, 0x0105,  // 122  0000 0001 1011.
			131 * 3, 135 * 3, 0,  // 123  0000 0000 0001
			0, 0, 0x0204,  // 124  0000 0001 0100.
			149 * 3, 147 * 3, 0,  // 125  0000 0000 1100
			172 * 3, 173 * 3, 0,  // 126  0000 0000 0101 1
			162 * 3, 158 * 3, 0,  // 127  0000 0000 0010 0
			170 * 3, 161 * 3, 0,  // 128  0000 0000 0110 0
			168 * 3, 166 * 3, 0,  // 129  0000 0000 0110 1
			157 * 3, 179 * 3, 0,  // 130  0000 0000 0101 0
			169 * 3, 167 * 3, 0,  // 131  0000 0000 0001 0
			174 * 3, 171 * 3, 0,  // 132  0000 0000 0011 0
			178 * 3, 177 * 3, 0,  // 133  0000 0000 0100 1
			156 * 3, 159 * 3, 0,  // 134  0000 0000 0011 1
			164 * 3, 165 * 3, 0,  // 135  0000 0000 0001 1
			183 * 3, 182 * 3, 0,  // 136  0000 0000 0010 1
			175 * 3, 176 * 3, 0,  // 137  0000 0000 0100 0
			0, 0, 0x0107,  // 138  0000 0000 1010 1.
			0, 0, 0x0a02,  // 139  0000 0000 1000 0.
			0, 0, 0x0902,  // 140  0000 0000 1000 1.
			0, 0, 0x1601,  // 141  0000 0000 1111 1.
			0, 0, 0x1701,  // 142  0000 0000 1111 0.
			0, 0, 0x1901,  // 143  0000 0000 1110 0.
			0, 0, 0x1801,  // 144  0000 0000 1110 1.
			0, 0, 0x0503,  // 145  0000 0000 1001 0.
			0, 0, 0x0304,  // 146  0000 0000 1001 1.
			0, 0, 0x000d,  // 147  0000 0000 1100 1.
			0, 0, 0x000c,  // 148  0000 0000 1101 0.
			0, 0, 0x000e,  // 149  0000 0000 1100 0.
			0, 0, 0x000f,  // 150  0000 0000 1011 1.
			0, 0, 0x0205,  // 151  0000 0000 1010 0.
			0, 0, 0x1a01,  // 152  0000 0000 1101 1.
			0, 0, 0x0106,  // 153  0000 0000 1011 0.
			180 * 3, 181 * 3, 0,  // 154  0000 0000 0111 1
			160 * 3, 163 * 3, 0,  // 155  0000 0000 0111 0
			196 * 3, 199 * 3, 0,  // 156  0000 0000 0011 10
			0, 0, 0x001b,  // 157  0000 0000 0101 00.
			203 * 3, 185 * 3, 0,  // 158  0000 0000 0010 01
			202 * 3, 201 * 3, 0,  // 159  0000 0000 0011 11
			0, 0, 0x0013,  // 160  0000 0000 0111 00.
			0, 0, 0x0016,  // 161  0000 0000 0110 01.
			197 * 3, 207 * 3, 0,  // 162  0000 0000 0010 00
			0, 0, 0x0012,  // 163  0000 0000 0111 01.
			191 * 3, 192 * 3, 0,  // 164  0000 0000 0001 10
			188 * 3, 190 * 3, 0,  // 165  0000 0000 0001 11
			0, 0, 0x0014,  // 166  0000 0000 0110 11.
			184 * 3, 194 * 3, 0,  // 167  0000 0000 0001 01
			0, 0, 0x0015,  // 168  0000 0000 0110 10.
			186 * 3, 193 * 3, 0,  // 169  0000 0000 0001 00
			0, 0, 0x0017,  // 170  0000 0000 0110 00.
			204 * 3, 198 * 3, 0,  // 171  0000 0000 0011 01
			0, 0, 0x0019,  // 172  0000 0000 0101 10.
			0, 0, 0x0018,  // 173  0000 0000 0101 11.
			200 * 3, 205 * 3, 0,  // 174  0000 0000 0011 00
			0, 0, 0x001f,  // 175  0000 0000 0100 00.
			0, 0, 0x001e,  // 176  0000 0000 0100 01.
			0, 0, 0x001c,  // 177  0000 0000 0100 11.
			0, 0, 0x001d,  // 178  0000 0000 0100 10.
			0, 0, 0x001a,  // 179  0000 0000 0101 01.
			0, 0, 0x0011,  // 180  0000 0000 0111 10.
			0, 0, 0x0010,  // 181  0000 0000 0111 11.
			189 * 3, 206 * 3, 0,  // 182  0000 0000 0010 11
			187 * 3, 195 * 3, 0,  // 183  0000 0000 0010 10
			218 * 3, 211 * 3, 0,  // 184  0000 0000 0001 010
			0, 0, 0x0025,  // 185  0000 0000 0010 011.
			215 * 3, 216 * 3, 0,  // 186  0000 0000 0001 000
			0, 0, 0x0024,  // 187  0000 0000 0010 100.
			210 * 3, 212 * 3, 0,  // 188  0000 0000 0001 110
			0, 0, 0x0022,  // 189  0000 0000 0010 110.
			213 * 3, 209 * 3, 0,  // 190  0000 0000 0001 111
			221 * 3, 222 * 3, 0,  // 191  0000 0000 0001 100
			219 * 3, 208 * 3, 0,  // 192  0000 0000 0001 101
			217 * 3, 214 * 3, 0,  // 193  0000 0000 0001 001
			223 * 3, 220 * 3, 0,  // 194  0000 0000 0001 011
			0, 0, 0x0023,  // 195  0000 0000 0010 101.
			0, 0, 0x010b,  // 196  0000 0000 0011 100.
			0, 0, 0x0028,  // 197  0000 0000 0010 000.
			0, 0, 0x010c,  // 198  0000 0000 0011 011.
			0, 0, 0x010a,  // 199  0000 0000 0011 101.
			0, 0, 0x0020,  // 200  0000 0000 0011 000.
			0, 0, 0x0108,  // 201  0000 0000 0011 111.
			0, 0, 0x0109,  // 202  0000 0000 0011 110.
			0, 0, 0x0026,  // 203  0000 0000 0010 010.
			0, 0, 0x010d,  // 204  0000 0000 0011 010.
			0, 0, 0x010e,  // 205  0000 0000 0011 001.
			0, 0, 0x0021,  // 206  0000 0000 0010 111.
			0, 0, 0x0027,  // 207  0000 0000 0010 001.
			0, 0, 0x1f01,  // 208  0000 0000 0001 1011.
			0, 0, 0x1b01,  // 209  0000 0000 0001 1111.
			0, 0, 0x1e01,  // 210  0000 0000 0001 1100.
			0, 0, 0x1002,  // 211  0000 0000 0001 0101.
			0, 0, 0x1d01,  // 212  0000 0000 0001 1101.
			0, 0, 0x1c01,  // 213  0000 0000 0001 1110.
			0, 0, 0x010f,  // 214  0000 0000 0001 0011.
			0, 0, 0x0112,  // 215  0000 0000 0001 0000.
			0, 0, 0x0111,  // 216  0000 0000 0001 0001.
			0, 0, 0x0110,  // 217  0000 0000 0001 0010.
			0, 0, 0x0603,  // 218  0000 0000 0001 0100.
			0, 0, 0x0b02,  // 219  0000 0000 0001 1010.
			0, 0, 0x0e02,  // 220  0000 0000 0001 0111.
			0, 0, 0x0d02,  // 221  0000 0000 0001 1000.
			0, 0, 0x0c02,  // 222  0000 0000 0001 1001.
			0, 0, 0x0f02   // 223  0000 0000 0001 0110.
		};


		namespace START
		{
			static const uint8_t SEQUENCE = 0xB3;
			static const uint8_t SLICE_FIRST = 0x01;
			static const uint8_t SLICE_LAST = 0xAF;
			static const uint8_t PICTURE = 0x00;
			static const uint8_t EXTENSION = 0xB5;
			static const uint8_t USER_DATA = 0xB2;
		}

		enum PICTURE_TYPE {
			INTRA = 1,
			PREDICTIVE = 2,
			B = 3
		};

		int MACROBLOCK_TYPE_INTRA[] = {
			1 * 3, 2 * 3, 0, //   0
			-1, 3 * 3, 0, //   1  0
			0, 0, 0x01, //   2  1.
			0, 0, 0x11  //   3  01.
		};

		int MACROBLOCK_TYPE_PREDICTIVE[] = {
			1 * 3, 2 * 3, 0, //  0
			3 * 3, 4 * 3, 0, //  1  0
			0, 0, 0x0a, //  2  1.
			5 * 3, 6 * 3, 0, //  3  00
			0, 0, 0x02, //  4  01.
			7 * 3, 8 * 3, 0, //  5  000
			0, 0, 0x08, //  6  001.
			9 * 3, 10 * 3, 0, //  7  0000
			11 * 3, 12 * 3, 0, //  8  0001
			-1, 13 * 3, 0, //  9  00000
			0, 0, 0x12, // 10  00001.
			0, 0, 0x1a, // 11  00010.
			0, 0, 0x01, // 12  00011.
			0, 0, 0x11  // 13  000001.
		};

		int MACROBLOCK_TYPE_B[] = {
			1 * 3, 2 * 3, 0,  //  0
			3 * 3, 5 * 3, 0,  //  1  0
			4 * 3, 6 * 3, 0,  //  2  1
			8 * 3, 7 * 3, 0,  //  3  00
			0, 0, 0x0c,  //  4  10.
			9 * 3, 10 * 3, 0,  //  5  01
			0, 0, 0x0e,  //  6  11.
			13 * 3, 14 * 3, 0,  //  7  001
			12 * 3, 11 * 3, 0,  //  8  000
			0, 0, 0x04,  //  9  010.
			0, 0, 0x06,  // 10  011.
			18 * 3, 16 * 3, 0,  // 11  0001
			15 * 3, 17 * 3, 0,  // 12  0000
			0, 0, 0x08,  // 13  0010.
			0, 0, 0x0a,  // 14  0011.
			-1, 19 * 3, 0,  // 15  00000
			0, 0, 0x01,  // 16  00011.
			20 * 3, 21 * 3, 0,  // 17  00001
			0, 0, 0x1e,  // 18  00010.
			0, 0, 0x11,  // 19  000001.
			0, 0, 0x16,  // 20  000010.
			0, 0, 0x1a   // 21  000011.
		};

		int* MACROBLOCK_TYPE[] = {
			NULL,
			MACROBLOCK_TYPE_INTRA,
			MACROBLOCK_TYPE_PREDICTIVE,
			MACROBLOCK_TYPE_B
		};
	}

	decoder_mpeg1::decoder_mpeg1()
		: decoder_base()
		, m_has_sequence_header(false)
		, m_frame_rate(30.0)
		, m_decode_first_frame(true)
		, m_width(0)
		, m_height(0)
		, m_mb_width(0)
		, m_mb_height(0)
		, m_coded_width(0)
		, m_coded_height(0)
		, m_coded_size(0)
		, m_half_width(0)
		, m_half_height(0)
		, m_picture_type(-1)
		, m_full_pel_foward(false)
		, m_forward_f_code(0)
		, m_forward_r_size(0)
		, m_forward_f(0)
		, m_slice_begin(false)
		, m_macro_block_address(0)
		, m_macro_block_type(0)
		, m_macro_block_intra(0)
		, m_macro_block_motFw(0)
		, m_mb_row(0)
		, m_mb_col(0)
		, m_motion_FwH(0)
		, m_motion_FwH_prev(0)
		, m_motion_FwV(0)
		, m_motion_FwV_prev(0)
		, m_intra_quant_matrix(NULL)
		, m_non_intra_quant_matrix(NULL)
		, m_current_Y(NULL)
		, m_current_Cr(NULL)
		, m_current_Cb(NULL)
		, m_forward_Y(NULL)
		, m_forward_Cr(NULL)
		, m_forward_Cb(NULL)
		, m_current_frame(0)
	{
		m_custom_intra_quant_matrix = new int[64];
		m_custom_non_intra_quant_matrix = new int[64];

		m_block_data = new int32_t[64];
		fill(m_block_data, 0);
	}

	decoder_mpeg1::~decoder_mpeg1()
	{
		SAFLE_DELETE_ARRAY(m_custom_intra_quant_matrix);
		SAFLE_DELETE_ARRAY(m_custom_non_intra_quant_matrix);
		SAFLE_DELETE_ARRAY(m_block_data);

		SAFLE_DELETE_ARRAY(m_current_Y);
		SAFLE_DELETE_ARRAY(m_current_Cr);
		SAFLE_DELETE_ARRAY(m_current_Cb);

		SAFLE_DELETE_ARRAY(m_forward_Y);
		SAFLE_DELETE_ARRAY(m_forward_Cr);
		SAFLE_DELETE_ARRAY(m_forward_Cb);
	}

	bool decoder_mpeg1::write(int pts, const std::vector<uint8_t>& buffer)
	{
		decoder_base::write(pts, buffer);

		if (!m_has_sequence_header) {
			if (m_bits->find_start_code(MPEG1::START::SEQUENCE) == -1)
				return false;

			decode_sequence_header();

			//if (m_decode_first_frame)
			//{
			//	decode();
			//}
		}

		decode();

		return true;
	}

	void decoder_mpeg1::decode_sequence_header()
	{
		int new_width = m_bits->read(12);
		int new_height = m_bits->read(12);

		// skip pixel aspect ratio
		m_bits->skip(4);

		m_frame_rate = MPEG1::PICTURE_RATE[m_bits->read(4)];

		// skip bitrate, marker, buffersize, and constrained bit
		m_bits->skip(18+1+10+1);

		if (new_width != m_width || new_height != m_height) {
			m_width = new_width;
			m_height = new_height;

			init_buffers();

			if (m_destination) {
				m_destination->resize( m_width, m_height);
			}
		}

		if (m_bits->read(1)) {	// load custom intra quant matrix
			for (int i = 0; i < 64; i++) {
				m_custom_intra_quant_matrix[MPEG1::ZIG_ZAG[i]] = m_bits->read(8);
			}

			m_intra_quant_matrix = m_custom_intra_quant_matrix;
		}

		if (m_bits->read(1)) {
			for (int i = 0; i < 64; i++)
			{
				int idx = MPEG1::ZIG_ZAG[i];
				m_custom_non_intra_quant_matrix[idx] = m_bits->read(8);
			}

			m_non_intra_quant_matrix = m_custom_non_intra_quant_matrix;
		}

		m_has_sequence_header = true;
	}

	bool decoder_mpeg1::decode()
	{
		if(!m_has_sequence_header)
		{
			return false;
		}

		if (m_bits->find_start_code(MPEG1::START::PICTURE) == -1)
		{
			int buffered_bytes = m_bits->byte_size() - (m_bits->read_index() >> 3);
			return false;
		}

		decode_picture();
		advance_decoded_time(1.0 / m_frame_rate);

		return true;
	}

	void decoder_mpeg1::init_buffers()
	{
		m_intra_quant_matrix = MPEG1::DEFAULT_INTRA_QUANT_MATRIX;
		m_non_intra_quant_matrix = MPEG1::DEFAULT_NON_INTRA_QUANT_MATRIX;

		m_mb_width = (m_width + 15) >> 4;
		m_mb_height = (m_height + 15) >> 4;
		m_mb_size = m_mb_width * m_mb_height;

		m_coded_width = m_mb_width << 4;
		m_coded_height = m_mb_height << 4;
		m_coded_size = m_coded_width * m_coded_height;

		m_half_width = m_mb_width << 3;
		m_half_height = m_mb_height << 3;

		// Allocated buffers and resize the canvas
		m_current_Y = new uint8_t[m_coded_size];
		m_current_Cr = new uint8_t[m_coded_size >> 2];
		m_current_Cb = new uint8_t[m_coded_size >> 2];

		m_forward_Y = new uint8_t[m_coded_size];
		m_forward_Cr = new uint8_t[m_coded_size >> 2];
		m_forward_Cb = new uint8_t[m_coded_size >> 2];
	}

	void decoder_mpeg1::decode_picture()
	{
		m_current_frame++;

		m_bits->skip(10);	// skip temporal reference
		m_picture_type = m_bits->read(3);
		m_bits->skip(16);	// skip vbv_delay

		// skip b and d frames or unknown coding type
		if (m_picture_type <= 0 || m_picture_type >= MPEG1::PICTURE_TYPE::B) {
			return;
		}

		// full_pel_forward, forward_f_code
		if (m_picture_type == MPEG1::PICTURE_TYPE::PREDICTIVE) {
			m_full_pel_foward = m_bits->read(1);
			m_forward_f_code = m_bits->read(3);
			if (m_forward_f_code == 0)
			{
				// ignore picture with zero forward_f_code
				return;
			}

			m_forward_r_size = m_forward_f_code - 1;
			m_forward_f = 1 << m_forward_r_size;
		}

		int code = 0;
		do 
		{
			code = m_bits->find_next_start_code();
		} while (code==MPEG1::START::EXTENSION || code==MPEG1::START::USER_DATA);

		while (code >= MPEG1::START::SLICE_FIRST && code <= MPEG1::START::SLICE_LAST) {
			decode_slice(code & 0x000000FF);
			code = m_bits->find_next_start_code();
		}

		if (code != -1)
		{
			m_bits->rewind(32);
		}

		// invoke decode callbacks
		if (m_destination)
		{
			m_destination->render(m_current_Y, m_current_Cr, m_current_Cb);
		}

		 // if this is a reference picture then rotate the prediction pointers
		if (m_picture_type == MPEG1::PICTURE_TYPE::INTRA || m_picture_type == MPEG1::PICTURE_TYPE::PREDICTIVE)
		{
			std::swap( m_forward_Y, m_current_Y);
			std::swap( m_forward_Cr, m_current_Cr);
			std::swap( m_forward_Cb, m_current_Cb);
		}
	}

	void decoder_mpeg1::decode_slice(int slice)
	{
		m_slice_begin = true;
		m_macro_block_address = (slice - 1) * m_mb_width - 1;

		// reset motion vectors are DC predictors
		m_motion_FwH = m_motion_FwH_prev = 0;
		m_motion_FwV = m_motion_FwV_prev = 0;
		m_dc_predictor_Y = 128;
		m_dc_predictor_Cr = 128;
		m_dc_predictor_Cb = 128;

		m_quantizer_scale = m_bits->read(5);

		while (m_bits->read(1)) {
			m_bits->skip(8);
		}

		do {
			decode_macro_block();
		} while (!m_bits->next_bytes_are_start_code());
	}

	void decoder_mpeg1::decode_macro_block()
	{
		int increment = 0;
		int t = read_huffman(MPEG1::MACROBLOCK_ADDRESS_INCREMENT);

		while (t == 34) {
			// macroblock stuffing
			t = read_huffman(MPEG1::MACROBLOCK_ADDRESS_INCREMENT);
		}

		while (t == 35) {
			// macroblock escape
			increment += 33;
			t = read_huffman(MPEG1::MACROBLOCK_ADDRESS_INCREMENT);
		}

		increment += t;

		// process any skipped macroblocks
		if (m_slice_begin) {
			// the first macroblock_address_increment of each slice is relative
			// to beginning of the preverious row, not the preverious macroblock
			m_slice_begin = false;
			m_macro_block_address += increment;
		}
		else{
			if (m_macro_block_address + increment >= m_mb_size) {
				return;
			}

			if (increment > 1) {
				// skipped macroblock reset DC predictors
				m_dc_predictor_Y = 128;
				m_dc_predictor_Cr = 128;
				m_dc_predictor_Cb = 128;

				// skipped macroblocks in P-pictures reset motion vectors
				if (m_picture_type == MPEG1::PICTURE_TYPE::PREDICTIVE) {
					m_motion_FwH = m_motion_FwH_prev = 0;
					m_motion_FwV = m_motion_FwV_prev = 0;
				}
			}

			// predict skipped macroblocks
			while (increment > 1) {
				m_macro_block_address++;
				m_mb_row = (m_macro_block_address / m_mb_width) | 0;
				m_mb_col = (m_macro_block_address % m_mb_width);

				copy_macro_block( m_motion_FwH, m_motion_FwV, m_forward_Y, m_forward_Cr, m_forward_Cb);

				increment--;
			}

			m_macro_block_address++;
		}

		m_mb_row = (m_macro_block_address / m_mb_width) | 0;
		m_mb_col = (m_macro_block_address % m_mb_width);

		// process the current macroblock
		int* mb_table = MPEG1::MACROBLOCK_TYPE[m_picture_type];
		m_macro_block_type = read_huffman(mb_table);
		m_macro_block_intra = (m_macro_block_type & 0x01);
		m_macro_block_motFw = (m_macro_block_type & 0x08);

		if ((m_macro_block_type & 0x10) != 0) {
			m_quantizer_scale = m_bits->read(5);
		}

		if (m_macro_block_intra) {
			// intra-code macroblocks reset motion vectors
			m_motion_FwH = m_motion_FwH_prev = 0;
			m_motion_FwV = m_motion_FwV_prev = 0;
		}
		else {
			m_dc_predictor_Y = 128;
			m_dc_predictor_Cr = 128;
			m_dc_predictor_Cb = 128;

			decode_motion_vectors();
			copy_macro_block( m_motion_FwH, m_motion_FwV, m_forward_Y, m_forward_Cr, m_forward_Cb);
		}

		// decode blocks
		int16_t cbp = ((m_macro_block_type & 0x02) != 0) ? read_huffman(MPEG1::CODE_BLOCK_PATTERN) : (m_macro_block_intra ? 0x3f : 0);

		for (int block = 0, mask = 0x20; block < 6; block++) {
			if ((cbp & mask) != 0) {
				decode_block(block);
			}

			mask >>= 1;
		}
	}

	void decoder_mpeg1::copy_macro_block(int motionH, int motionV, uint8_t* sY, uint8_t* sCr, uint8_t* sCb) 
	{
		int width, scan,
			H, V, oddH, oddV,
			src, dest, last;

		// We use 32bit writes here
		uint32_t* dY = (uint32_t*)m_current_Y;
		uint32_t* dCb = (uint32_t*)m_current_Cb;
		uint32_t* dCr = (uint32_t*)m_current_Cr;

		// Luminance
		width = m_coded_width;
		scan = width - 16;

		H = motionH >> 1;
		V = motionV >> 1;
		oddH = (motionH & 1) == 1;
		oddV = (motionV & 1) == 1;

		src = ((m_mb_row << 4) + V) * width + (m_mb_col << 4) + H;
		dest = (m_mb_row * width + m_mb_col) << 2;
		last = dest + (width << 2);

		int x, y1, y2, y;
		if (oddH) {
			if (oddV) {
				while (dest < last) {
					y1 = sY[src] + sY[src + width]; src++;
					for (x = 0; x < 4; x++) {
						y2 = sY[src] + sY[src + width]; src++;
						y = (((y1 + y2 + 2) >> 2) & 0xff);

						y1 = sY[src] + sY[src + width]; src++;
						y |= (((y1 + y2 + 2) << 6) & 0xff00);

						y2 = sY[src] + sY[src + width]; src++;
						y |= (((y1 + y2 + 2) << 14) & 0xff0000);

						y1 = sY[src] + sY[src + width]; src++;
						y |= (((y1 + y2 + 2) << 22) & 0xff000000);

						dY[dest++] = y;
					}
					dest += scan >> 2; src += scan - 1;
				}
			}
			else {
				while (dest < last) {
					y1 = sY[src++];
					for (x = 0; x < 4; x++) {
						y2 = sY[src++];
						y = (((y1 + y2 + 1) >> 1) & 0xff);

						y1 = sY[src++];
						y |= (((y1 + y2 + 1) << 7) & 0xff00);

						y2 = sY[src++];
						y |= (((y1 + y2 + 1) << 15) & 0xff0000);

						y1 = sY[src++];
						y |= (((y1 + y2 + 1) << 23) & 0xff000000);

						dY[dest++] = y;
					}
					dest += scan >> 2; src += scan - 1;
				}
			}
		}
		else {
			if (oddV) {
				while (dest < last) {
					for (x = 0; x < 4; x++) {
						y = (((sY[src] + sY[src + width] + 1) >> 1) & 0xff); src++;
						y |= (((sY[src] + sY[src + width] + 1) << 7) & 0xff00); src++;
						y |= (((sY[src] + sY[src + width] + 1) << 15) & 0xff0000); src++;
						y |= (((sY[src] + sY[src + width] + 1) << 23) & 0xff000000); src++;

						dY[dest++] = y;
					}
					dest += scan >> 2; src += scan;
				}
			}
			else {
				while (dest < last) {
					for (x = 0; x < 4; x++) {
						y = sY[src]; src++;
						y |= sY[src] << 8; src++;
						y |= sY[src] << 16; src++;
						y |= sY[src] << 24; src++;

						dY[dest++] = y;
					}
					dest += scan >> 2; src += scan;
				}
			}
		}

		// Chrominance

		width = m_half_width;
		scan = width - 8;

		H = (motionH / 2) >> 1;
		V = (motionV / 2) >> 1;
		oddH = ((motionH / 2) & 1) == 1;
		oddV = ((motionV / 2) & 1) == 1;

		src = ((m_mb_row << 3) + V) * width + (m_mb_col << 3) + H;
		dest = (m_mb_row * width + m_mb_col) << 1;
		last = dest + (width << 1);

		int cr1, cr2, cr,
			cb1, cb2, cb;
		if (oddH) {
			if (oddV) {
				while (dest < last) {
					cr1 = sCr[src] + sCr[src + width];
					cb1 = sCb[src] + sCb[src + width];
					src++;
					for (x = 0; x < 2; x++) {
						cr2 = sCr[src] + sCr[src + width];
						cb2 = sCb[src] + sCb[src + width]; src++;
						cr = (((cr1 + cr2 + 2) >> 2) & 0xff);
						cb = (((cb1 + cb2 + 2) >> 2) & 0xff);

						cr1 = sCr[src] + sCr[src + width];
						cb1 = sCb[src] + sCb[src + width]; src++;
						cr |= (((cr1 + cr2 + 2) << 6) & 0xff00);
						cb |= (((cb1 + cb2 + 2) << 6) & 0xff00);

						cr2 = sCr[src] + sCr[src + width];
						cb2 = sCb[src] + sCb[src + width]; src++;
						cr |= (((cr1 + cr2 + 2) << 14) & 0xff0000);
						cb |= (((cb1 + cb2 + 2) << 14) & 0xff0000);

						cr1 = sCr[src] + sCr[src + width];
						cb1 = sCb[src] + sCb[src + width]; src++;
						cr |= (((cr1 + cr2 + 2) << 22) & 0xff000000);
						cb |= (((cb1 + cb2 + 2) << 22) & 0xff000000);

						dCr[dest] = cr;
						dCb[dest] = cb;
						dest++;
					}
					dest += scan >> 2; src += scan - 1;
				}
			}
			else {
				while (dest < last) {
					cr1 = sCr[src];
					cb1 = sCb[src];
					src++;
					for (x = 0; x < 2; x++) {
						cr2 = sCr[src];
						cb2 = sCb[src++];
						cr = (((cr1 + cr2 + 1) >> 1) & 0xff);
						cb = (((cb1 + cb2 + 1) >> 1) & 0xff);

						cr1 = sCr[src];
						cb1 = sCb[src++];
						cr |= (((cr1 + cr2 + 1) << 7) & 0xff00);
						cb |= (((cb1 + cb2 + 1) << 7) & 0xff00);

						cr2 = sCr[src];
						cb2 = sCb[src++];
						cr |= (((cr1 + cr2 + 1) << 15) & 0xff0000);
						cb |= (((cb1 + cb2 + 1) << 15) & 0xff0000);

						cr1 = sCr[src];
						cb1 = sCb[src++];
						cr |= (((cr1 + cr2 + 1) << 23) & 0xff000000);
						cb |= (((cb1 + cb2 + 1) << 23) & 0xff000000);

						dCr[dest] = cr;
						dCb[dest] = cb;
						dest++;
					}
					dest += scan >> 2; src += scan - 1;
				}
			}
		}
		else {
			if (oddV) {
				while (dest < last) {
					for (x = 0; x < 2; x++) {
						cr = (((sCr[src] + sCr[src + width] + 1) >> 1) & 0xff);
						cb = (((sCb[src] + sCb[src + width] + 1) >> 1) & 0xff); src++;

						cr |= (((sCr[src] + sCr[src + width] + 1) << 7) & 0xff00);
						cb |= (((sCb[src] + sCb[src + width] + 1) << 7) & 0xff00); src++;

						cr |= (((sCr[src] + sCr[src + width] + 1) << 15) & 0xff0000);
						cb |= (((sCb[src] + sCb[src + width] + 1) << 15) & 0xff0000); src++;

						cr |= (((sCr[src] + sCr[src + width] + 1) << 23) & 0xff000000);
						cb |= (((sCb[src] + sCb[src + width] + 1) << 23) & 0xff000000); src++;

						dCr[dest] = cr;
						dCb[dest] = cb;
						dest++;
					}
					dest += scan >> 2; src += scan;
				}
			}
			else {
				while (dest < last) {
					for (x = 0; x < 2; x++) {
						cr = sCr[src];
						cb = sCb[src]; src++;

						cr |= sCr[src] << 8;
						cb |= sCb[src] << 8; src++;

						cr |= sCr[src] << 16;
						cb |= sCb[src] << 16; src++;

						cr |= sCr[src] << 24;
						cb |= sCb[src] << 24; src++;

						dCr[dest] = cr;
						dCb[dest] = cb;
						dest++;
					}
					dest += scan >> 2; src += scan;
				}
			}
		}
	};

	void decoder_mpeg1::decode_motion_vectors() {

		int code, d, r = 0;

		// Forward
		if ( m_macro_block_motFw) {
			// Horizontal forward
			code = read_huffman(MPEG1::MOTION);
			if ((code != 0) && (m_forward_f !=1)) {
				r = m_bits->read( m_forward_r_size);
				d = ((abs(code) - 1) << m_forward_r_size) + r + 1;
				if (code < 0) {
					d = -d;
				}
			}
			else {
				d = code;
			}
	
			m_motion_FwH_prev += d;
			if (m_motion_FwH_prev > (m_forward_f << 4) - 1) {
				m_motion_FwH_prev -= m_forward_f << 5;
			}
			else if ( m_motion_FwH_prev < ((-m_forward_f) << 4)) {
				m_motion_FwH_prev += m_forward_f << 5;
			}

			m_motion_FwH = m_motion_FwH_prev;
			if ( m_full_pel_foward) {
				m_motion_FwH <<= 1;
			}

			// Vertical forward
			code = read_huffman(MPEG1::MOTION);
			if ((code != 0) && (m_forward_f != 1)) {
				r = m_bits->read(m_forward_r_size);
				d = ((abs(code) - 1) << m_forward_r_size) + r + 1;
				if (code < 0) {
					d = -d;
				}
			}
			else {
				d = code;
			}

			m_motion_FwV_prev += d;
			if (m_motion_FwV_prev > (m_forward_f << 4) - 1) {
				m_motion_FwV_prev -= m_forward_f << 5;
			}
			else if (m_motion_FwV_prev < ((-m_forward_f) << 4)) {
				m_motion_FwV_prev += m_forward_f << 5;
			}

			m_motion_FwV = m_motion_FwV_prev;
			if (m_full_pel_foward) {
				m_motion_FwV <<= 1;
			}
		}
		else if (m_picture_type == MPEG1::PICTURE_TYPE::PREDICTIVE) {
			// No motion information in P-picture, reset vectors
			m_motion_FwH = m_motion_FwH_prev = 0;
			m_motion_FwV = m_motion_FwV_prev = 0;
		}
	};

	void decoder_mpeg1::decode_block(int block) {

		int n = 0;
		int* quantMatrix = NULL;

		// Decode DC coefficient of intra-coded blocks
		if (m_macro_block_intra) {
			int predictor;
			int	dctSize;

			// DC prediction

			if (block < 4) {
				predictor = m_dc_predictor_Y;
				dctSize = read_huffman(MPEG1::DCT_DC_SIZE_LUMINANCE);
			}
			else {
				predictor = (block == 4 ? m_dc_predictor_Cr : m_dc_predictor_Cb);
				dctSize = read_huffman(MPEG1::DCT_DC_SIZE_CHROMINANCE);
			}

			// Read DC coeff
			if (dctSize > 0) {
				int differential = m_bits->read(dctSize);
				if ((differential & (1 << (dctSize - 1))) != 0) {
					m_block_data[0] = predictor + differential;
				}
				else {
					m_block_data[0] = predictor + ((-1 << dctSize) | (differential + 1));
				}
			}
			else {
				m_block_data[0] = predictor;
			}

			// Save predictor value
			if (block < 4) {
				m_dc_predictor_Y = m_block_data[0];
			}
			else if (block == 4) {
				m_dc_predictor_Cr = m_block_data[0];
			}
			else {
				m_dc_predictor_Cb = m_block_data[0];
			}

			// Dequantize + premultiply
			m_block_data[0] <<= (3 + 5);

			quantMatrix = m_intra_quant_matrix;
			n = 1;
		}
		else {
			quantMatrix = m_non_intra_quant_matrix;
		}

		// Decode AC coefficients (+DC for non-intra)
		int level = 0;
		while (true) {
			int run = 0;
			int32_t coeff = read_huffman(MPEG1::DCT_COEFF);

			if ((coeff == 0x0001) && (n > 0) && (m_bits->read(1) == 0)) {
				// end_of_block
				break;
			}
			if (coeff == 0xffff) {
				// escape
				run = m_bits->read(6);
				level = m_bits->read(8);
				if (level == 0) {
					level = m_bits->read(8);
				}
				else if (level == 128) {
					level = m_bits->read(8) - 256;
				}
				else if (level > 128) {
					level = level - 256;
				}
			}
			else {
				run = coeff >> 8;
				level = coeff & 0xff;
				if (m_bits->read(1)) {
					level = -level;
				}
			}

			n += run;
			int dezigZagged = MPEG1::ZIG_ZAG[n];
			n++;

			// Dequantize, oddify, clip
			level <<= 1;
			if (!m_macro_block_intra) {
				level += (level < 0 ? -1 : 1);
			}
			level = (level * m_quantizer_scale * quantMatrix[dezigZagged]) >> 4;
			if ((level & 1) == 0) {
				level -= level > 0 ? 1 : -1;
			}
			if (level > 2047) {
				level = 2047;
			}
			else if (level < -2048) {
				level = -2048;
			}

			// Save premultiplied coefficient
			m_block_data[dezigZagged] = level * MPEG1::PREMULTIPLIER_MATRIX[dezigZagged];
		}

		// Move block to its place
		uint8_t* destArray;
		int	destIndex;
		int	scan;

		if (block < 4) {
			destArray = m_current_Y;
			scan = m_coded_width - 8;
			destIndex = (m_mb_row * m_coded_width + m_mb_col) << 4;
			if ((block & 1) != 0) {
				destIndex += 8;
			}
			if ((block & 2) != 0) {
				destIndex += m_coded_width << 3;
			}
		}
		else {
			destArray = (block == 4) ? m_current_Cb : m_current_Cr;
			scan = (m_coded_width >> 1) - 8;
			destIndex = ((m_mb_row * m_coded_width) << 2) + (m_mb_col << 3);
		}

		if (m_macro_block_intra) {
			// Overwrite (no prediction)
			if (n == 1) {
				copy_value_to_destination((m_block_data[0] + 128) >> 8, destArray, destIndex, scan);
				m_block_data[0] = 0;
			}
			else {
				IDCT(m_block_data);
				copy_block_to_destination(m_block_data, destArray, destIndex, scan);
				fill(m_block_data, 0);
			}
		}
		else {
			// Add data to the predicted macroblock
			if (n == 1) {
				add_value_to_destination((m_block_data[0] + 128) >> 8, destArray, destIndex, scan);
				m_block_data[0] = 0;
			}
			else {
				IDCT( m_block_data);
				add_block_to_destination(m_block_data, destArray, destIndex, scan);
				fill(m_block_data, 0);
			}
		}

		n = 0;
	};

	void decoder_mpeg1::fill( int* block_data, int value) {
		for (int i = 0; i < 64; i++)
		{
			block_data[i] = value;
		}
	}

	void decoder_mpeg1::copy_value_to_destination(int value,  uint8_t* dest, int index, int scan) {
		for (int n = 0; n < 64; n += 8, index += scan + 8) {

			for (int i = 0; i < 8; i++)
			{
				dest[index + i] = CLAMP(value, 0, 255);
			}
		}
	};

	void decoder_mpeg1::add_value_to_destination(int value, uint8_t* dest, int index, int scan) {
		for (int n = 0; n < 64; n += 8, index += scan + 8) {	
			for (int i = 0; i < 8; i++)
			{
				int result = dest[index + i] + value;
				dest[index + i] = CLAMP( result, 0, 255);
			}
		}
	}

	void decoder_mpeg1::copy_block_to_destination(int* block, uint8_t* dest, int index, int scan) {
		for (int n = 0; n < 64; n += 8, index += scan + 8) {

			for (int i = 0; i < 8; i++)
			{
				int result = block[n+i];
				dest[index + i] = CLAMP(result, 0, 255);
			}
		}
	};

	void decoder_mpeg1::add_block_to_destination(int* block, uint8_t* dest, int index, int scan) {
		for (int n = 0; n < 64; n += 8, index += scan + 8) {
			
			for (int i = 0; i < 8; i++)
			{
				int result = dest[index + i] + block[n + i];
				dest[index + i] = CLAMP(result, 0, 255);
			}
		}
	};

	void decoder_mpeg1::IDCT( int* block) {
		// See http://vsr.informatik.tu-chemnitz.de/~jan/MPEG/HTML/IDCT.html
		// for more info.

		int
			b1, b3, b4, b6, b7, tmp1, tmp2, m0,
			x0, x1, x2, x3, x4, y3, y4, y5, y6, y7;

		// Transform columns
		for (int i = 0; i < 8; ++i) {
			b1 = block[4 * 8 + i];
			b3 = block[2 * 8 + i] + block[6 * 8 + i];
			b4 = block[5 * 8 + i] - block[3 * 8 + i];
			tmp1 = block[1 * 8 + i] + block[7 * 8 + i];
			tmp2 = block[3 * 8 + i] + block[5 * 8 + i];
			b6 = block[1 * 8 + i] - block[7 * 8 + i];
			b7 = tmp1 + tmp2;
			m0 = block[0 * 8 + i];
			x4 = ((b6 * 473 - b4 * 196 + 128) >> 8) - b7;
			x0 = x4 - (((tmp1 - tmp2) * 362 + 128) >> 8);
			x1 = m0 - b1;
			x2 = (((block[2 * 8 + i] - block[6 * 8 + i]) * 362 + 128) >> 8) - b3;
			x3 = m0 + b1;
			y3 = x1 + x2;
			y4 = x3 + b3;
			y5 = x1 - x2;
			y6 = x3 - b3;
			y7 = -x0 - ((b4 * 473 + b6 * 196 + 128) >> 8);
			block[0 * 8 + i] = b7 + y4;
			block[1 * 8 + i] = x4 + y3;
			block[2 * 8 + i] = y5 - x0;
			block[3 * 8 + i] = y6 - y7;
			block[4 * 8 + i] = y6 + y7;
			block[5 * 8 + i] = x0 + y5;
			block[6 * 8 + i] = y3 - x4;
			block[7 * 8 + i] = y4 - b7;
		}

		// Transform rows
		for (int i = 0; i < 64; i += 8) {
			b1 = block[4 + i];
			b3 = block[2 + i] + block[6 + i];
			b4 = block[5 + i] - block[3 + i];
			tmp1 = block[1 + i] + block[7 + i];
			tmp2 = block[3 + i] + block[5 + i];
			b6 = block[1 + i] - block[7 + i];
			b7 = tmp1 + tmp2;
			m0 = block[0 + i];
			x4 = ((b6 * 473 - b4 * 196 + 128) >> 8) - b7;
			x0 = x4 - (((tmp1 - tmp2) * 362 + 128) >> 8);
			x1 = m0 - b1;
			x2 = (((block[2 + i] - block[6 + i]) * 362 + 128) >> 8) - b3;
			x3 = m0 + b1;
			y3 = x1 + x2;
			y4 = x3 + b3;
			y5 = x1 - x2;
			y6 = x3 - b3;
			y7 = -x0 - ((b4 * 473 + b6 * 196 + 128) >> 8);
			block[0 + i] = (b7 + y4 + 128) >> 8;
			block[1 + i] = (x4 + y3 + 128) >> 8;
			block[2 + i] = (y5 - x0 + 128) >> 8;
			block[3 + i] = (y6 - y7 + 128) >> 8;
			block[4 + i] = (y6 + y7 + 128) >> 8;
			block[5 + i] = (x0 + y5 + 128) >> 8;
			block[6 + i] = (y3 - x4 + 128) >> 8;
			block[7 + i] = (y4 - b7 + 128) >> 8;
		}
	};

}