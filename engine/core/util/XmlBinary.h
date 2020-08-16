#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	class XmlBinaryReader
	{
	public:
		XmlBinaryReader();
		~XmlBinaryReader();
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