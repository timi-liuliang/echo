#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class Variant;
	class VariantArray
	{
	public:
		VariantArray(){}

		// add variant
		void add(const String& name, const Variant& variant);

		// get variants
		const map<String, Variant>::type& getVariants() const { return m_variants; }

	private:
		map<String, Variant>::type	m_variants;
	};
}