#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include "engine/core/io/stream/DataStream.h"

namespace Echo
{
	class XmlBinaryReader
	{
	public:
		// data
		struct Data
		{
			String		m_name;
			String		m_type;
			i32			m_offset = 0;
			i32			m_size = 0;
			ByteArray	m_data;

			bool isEmpty() { return m_data.empty(); }
		};

	public:
		XmlBinaryReader();
		~XmlBinaryReader();

		// root node
		pugi::xml_node getRoot();

		// get data
		bool getData(const char* name, Data& binaryData);

		// load
		bool load(const char* path);

	private:
		pugi::xml_document  m_doc;
		DataStream*			m_stream = nullptr;
	};

	class XmlBinaryWriter
	{
	public:
		XmlBinaryWriter();
		~XmlBinaryWriter();

		// root node
		pugi::xml_node getRoot();

		// add binary data
		void addData(const char* name, const char* type, void* data, i32 bytes);

		// save
		void save(const char* path);

	private:
		pugi::xml_document m_doc;
		vector<Byte>::type m_binary;
	};
}