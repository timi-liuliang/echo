#include <stdint.h>
#include <stdlib.h>
#include "base64.h"
#include "Array.hpp"

namespace Echo
{
	static char encoding_table[] = 
	{ 
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/' 
	};

	static int   mod_table[] = { 0, 2, 1 };

	Base64Encode::Base64Encode(const String& data)
	{
		int input_length = static_cast<int>(data.length());
		int output_length = 4 * (int(data.length() + 2) / 3);
		m_encoded.resize(output_length);


		for (int i = 0, j = 0; i < input_length;)
		{
			uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
			uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
			uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

			uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

			m_encoded[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
			m_encoded[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
			m_encoded[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
			m_encoded[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
		}

		for (int i = 0; i < mod_table[input_length % 3]; i++)
		{
			m_encoded[output_length - 1 - i] = '=';
		}
	}

	Base64Decode::Base64Decode(const String& data)
	{
		// build decoding table
		array<char, 256> decoding_table;
		for (int i = 0; i < 64; i++)
		{
			ui8 idx = (unsigned char)encoding_table[i];
			decoding_table[idx] = i;
		}
			
		size_t input_length = data.size();
		if (input_length % 4 != 0) return;

		size_t output_length = input_length / 4 * 3;
		if (data[input_length - 1] == '=') output_length--;
		if (data[input_length - 2] == '=') output_length--;

		m_decoded.resize(output_length);
		for (size_t i = 0, j = 0; i<input_length;)
		{
			uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
			uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
			uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
			uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

			uint32_t triple = (sextet_a << 3 * 6)
							+ (sextet_b << 2 * 6)
							+ (sextet_c << 1 * 6)
							+ (sextet_d << 0 * 6);

			if (j < output_length) m_decoded[j++] = (triple >> 2 * 8) & 0xFF;
			if (j < output_length) m_decoded[j++] = (triple >> 1 * 8) & 0xFF;
			if (j < output_length) m_decoded[j++] = (triple >> 0 * 8) & 0xFF;
		}
	}

	void Base64String::encode(const char* originStr)
	{
		Base64Encode encoder(originStr);
		m_data = encoder.getData();
	}

	String Base64String::decode() const
	{
		Base64Decode decoder(m_data);
		return decoder.getData();
	}
}
