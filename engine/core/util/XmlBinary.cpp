#include "XmlBinary.h"
#include "engine/core/io/IO.h"

namespace Echo
{
	XmlBinaryReader::XmlBinaryReader()
	{

	}

	XmlBinaryReader::~XmlBinaryReader()
	{

	}

	XmlBinaryWriter::XmlBinaryWriter()
	{
		pugi::xml_node root = m_doc.append_child("header");
		root.append_child("_binarys_");
	}

	XmlBinaryWriter::~XmlBinaryWriter()
	{

	}

	pugi::xml_node XmlBinaryWriter::getRoot()
	{
		return m_doc.child("header");
	}

	void XmlBinaryWriter::addData(const char* name, const char* type, void* data, i32 bytes)
	{
		i32 offset = int(m_binary.size());
		pugi::xml_node binarys = getRoot().child("_binarys_");
		pugi::xml_node binary = binarys.append_child("_binary_");
		binary.append_attribute("name").set_value(name);
		binary.append_attribute("type").set_value(type);
		binary.append_attribute("offset").set_value(m_binary.size());
		binary.append_attribute("size").set_value(bytes);

		m_binary.resize(m_binary.size() + bytes);
		std::memcpy(&m_binary[offset], data, bytes);
	}

	void XmlBinaryWriter::save(const char* path)
	{
		DataStream* stream = IO::instance()->open(path, DataStream::WRITE);
		if (stream && stream->isWriteable())
		{
			// binary data
			stream->write(m_binary.data(), m_binary.size());

			// write header
			std::ostringstream xmlstream;
			m_doc.save(xmlstream);
			Base64String header64;
			header64.encode(xmlstream.str().data());
			i32 headerSize = header64.getData().size();
			stream->write(header64.getData().data(), headerSize);

			// header size
			stream->write(&headerSize, sizeof(headerSize));

			// close
			stream->close();
			EchoSafeDelete(stream, DataStream);
		}
	}
}