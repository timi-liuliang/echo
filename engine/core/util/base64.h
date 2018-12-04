#pragma once

#include "StringUtil.h"

namespace Echo
{
	class Base64Encode
	{
	public:
		Base64Encode(const String& orig);

		// size
		ui32 getSize() { return static_cast<ui32>(m_encoded.size()); }

		// data
		const char* getData() { return m_encoded.data(); }

	private:
		String		m_encoded;
	};

	class Base64Decode
	{
	public:
		Base64Decode(const String& data);

		// size
		ui32 getSize() { return static_cast<ui32>(m_decoded.size()); }

		// data
		const char* getData() { return m_decoded.data(); }

	private:
		vector<char>::type		m_decoded;
	};

	// a string saved in base64 format, 
	// that means it can be decoded into a string.
	struct Base64String
	{
		String		m_data;			// data

		Base64String() {}
		Base64String(const char* data) : m_data(data) {}

		// encode
		void encode(const char* originStr);

		// decode
		String decode() const;

		// data
		const String& getData() const { return m_data; }
	};
}
