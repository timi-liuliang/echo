#pragma once
#include "ShaderProgram.h"

namespace Echo
{
	class UniformCache
	{
	public:
		struct UniformBindInfo
		{
			ShaderParamType	type;
			int location;
			int count;
			Byte* value;
		};

		UniformCache();
		~UniformCache();

		Byte* AppendUniform(const void* value, ShaderParamType uniformType, size_t size);
		void Reset();
		
		size_t BeginAppendUniformInfo();
		size_t EndAppendUniformInfo();
		void AppendUniformInfo(ShaderParamType type, int location, int count, Byte* value);
		const UniformBindInfo& GetUniformBindInfo(size_t offset);
		void ResetInfoCache();

	private:
		const size_t CHUNK_CAPACITY = 16 * 1024;
		std::vector<Byte*> m_chunk_list;
		size_t m_chunk_index;
		size_t m_chunk_offset;

		std::vector<UniformBindInfo*> m_info_cache;
		size_t m_append_info_count;
		size_t m_info_count;
	};

	extern UniformCache* g_uniform_cache;
}
