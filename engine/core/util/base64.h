#pragma once

#include "StringUtil.h"

namespace Echo
{
	class Base64Encode
	{
	public:
		Base64Encode();

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

	};
}
