#include "pugixml.hpp"

namespace pugi
{
	struct xml_string_writer : pugi::xml_writer
	{
		std::string m_result;

		virtual void write(const void* data, size_t size)
		{
			m_result.append(static_cast<const char*>(data), size);
		}
	};

	// save doc to string
	inline std::string get_doc_string(xml_document& doc)
	{
		xml_string_writer writer;
		doc.save(writer);

		return writer.m_result;
	}
}
