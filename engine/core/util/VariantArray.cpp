#include "VariantArray.h"
#include "engine/core/base/variant.h"

namespace Echo
{
	// add variant
	void VariantArray::add(const String& name, const Variant& variant)
	{
		if (m_variants.find(name) == m_variants.end())
		{
			m_variants[name] = variant;
		}
	}
}