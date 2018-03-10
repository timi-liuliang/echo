#include "engine/core/Memory/MemManager.h"
#include "UniformCache.h"

namespace Echo
{
	const size_t UNIFORM_INFO_TRUNK_COUNT = 1024;

	UniformCache* g_uniform_cache = nullptr;


	UniformCache::UniformCache()
	{
		m_chunk_list.push_back((Byte*)EchoMalloc(CHUNK_CAPACITY));
		m_chunk_index = 0;
		m_chunk_offset = 0;

		m_append_info_count = 0;

		m_info_cache.reserve(1024);
		m_info_cache.push_back(EchoNewArray(UniformBindInfo, UNIFORM_INFO_TRUNK_COUNT));
		m_info_count = 0;
	}

	UniformCache::~UniformCache()
	{
		for (size_t i = 0; i < m_chunk_list.size(); ++i)
		{
			EchoSafeFree(m_chunk_list[i]);
			m_chunk_list[i] = nullptr;
		}

		for (size_t i = 0; i < m_info_cache.size(); ++i)
		{
			EchoSafeDeleteArray(m_info_cache[i], UniformBindInfo, UNIFORM_INFO_TRUNK_COUNT);
		}
	}

	Byte* UniformCache::AppendUniform(const void* value, ShaderParamType uniformType, size_t size)
	{
		EchoAssert(size < CHUNK_CAPACITY);

		if (m_chunk_offset + size > CHUNK_CAPACITY)
		{
			++m_chunk_index;
			m_chunk_offset = 0;

			if (m_chunk_index >= m_chunk_list.size())
			{
				m_chunk_list.push_back((Byte*)EchoMalloc(CHUNK_CAPACITY));
			}
		}

		Byte* dest_address = &m_chunk_list[m_chunk_index][m_chunk_offset];
		memcpy(dest_address, value, size);
		m_chunk_offset += size;

		return dest_address;
	}

	size_t UniformCache::BeginAppendUniformInfo()
	{
		m_append_info_count = 0;
		return m_info_count;
	}

	size_t UniformCache::EndAppendUniformInfo()
	{
		return m_append_info_count;
	}

	void UniformCache::AppendUniformInfo(ShaderParamType type, int location, int count, Byte* value)
	{
		size_t chunk_index = m_info_count / UNIFORM_INFO_TRUNK_COUNT;
		size_t chunk_offset = m_info_count % UNIFORM_INFO_TRUNK_COUNT;

		if (chunk_index >= m_info_cache.size())
		{
			m_info_cache.push_back(EchoNewArray(UniformBindInfo, UNIFORM_INFO_TRUNK_COUNT));
		}

		m_info_cache[chunk_index][chunk_offset].type = type;
		m_info_cache[chunk_index][chunk_offset].location = location;
		m_info_cache[chunk_index][chunk_offset].count = count;
		m_info_cache[chunk_index][chunk_offset].value = value;

		++m_info_count;
		++m_append_info_count;
	}

	const UniformCache::UniformBindInfo& UniformCache::GetUniformBindInfo(size_t offset)
	{
		size_t chunk_index = offset / UNIFORM_INFO_TRUNK_COUNT;
		size_t chunk_offset = offset % UNIFORM_INFO_TRUNK_COUNT;
		return m_info_cache[chunk_index][chunk_offset];
	}

	void UniformCache::ResetInfoCache()
	{
		m_info_count = 0;
	}

	void UniformCache::Reset()
	{
		m_chunk_index = 0;
		m_chunk_offset = 0;
	}

}
