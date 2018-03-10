#include "Engine/modules/Effect/EffectParticle.h"

namespace Echo
{
	const size_t EffectParticle::sCopyAttributeSize = (sizeof(Vector3)+sizeof(Quaternion)+sizeof(Vector3)+sizeof(float)+sizeof(Color)+sizeof(float))*2+sizeof(Vector3)+sizeof(float);
	const size_t EffectParticle::sReStateSize = sizeof(Vector3)+sizeof(Quaternion)+sizeof(float)+sizeof(Color)+sizeof(float);

	void EffectParticle::copyAttributesTo(EffectParticle* particle)
	{
		memcpy(&(particle->position), &(position), sCopyAttributeSize);
	}

	// 构造函数
	EffectParticle::EffectParticle()
		:parentParticle(NULL)
		,position(Vector3::ZERO)
		,oritation(Quaternion::IDENTITY)
		,selfRotation(0.f)
		,color(Color::WHITE)
		,scale(1.f)
		,direction(Vector3::UNIT_Z)
		,originalPosition(Vector3::ZERO)
		,originalOritation(Quaternion::IDENTITY)
		,originalSelfRotation(0.f)
		,originalColor(Color::WHITE)
		,originalScale(1.f)
		,originalSpeed(0.0f)
		,originalDirection(Vector3::UNIT_Z)
		,movementDirection(Vector3::ZERO)
		,mReDirection(2)
		,startPlayTime(0)
		,mRandomAnimIndex(0)
	{
	}

	// 析构函数
	EffectParticle::~EffectParticle()
	{
	}

	void EffectParticle::reParticleState()
	{
		memcpy(&position, &originalPosition, sReStateSize);
	}

	void EffectParticle::forwardParticleState()
	{	
		memcpy(&originalPosition, &position, sReStateSize);
	}

	void EffectParticle::forwardParticleState2()
	{
		originalScale = scale;
		originalColor = color;
	}

	// 页构造函数
	EffectParticlePool::Page::Page(ui32 capacity)
	{
		m_capacity = capacity;
		m_pool = EchoNew( ObjectPool<EffectParticle>(capacity));
		m_capacityRemain = capacity;
		m_idleTime = 0;
	}

	//页析构函数
	EffectParticlePool::Page::~Page()
	{
		EchoSafeDelete(m_pool, ObjectPool);
	}

	// 构造函数
	EffectParticlePool::EffectParticlePool(ui32 pageSize)
		: m_pageSize( pageSize)
	{

	}

	EffectParticlePool::~EffectParticlePool()
	{
		m_pages.clear();
	}

	// 获取描述
	void EffectParticlePool::buildDesc(String& desc)
	{
		ui32 capacity = 0;
		ui32 chunkCapacity = 0;
		ui32 size = 0;
		for (Page& page : m_pages)
		{
			capacity += page.m_pool->getCapacity();
			chunkCapacity += capacity - page.m_capacityRemain;
			size += page.m_pool->getSize();
		}

		desc = StringUtil::Format("%d/%d/%d", size, chunkCapacity, capacity);
	}

	// 新建块
	EffectParticlePool::Chunk* EffectParticlePool::NewChunk(ui32 chunkCapacity)
	{
		// 从现有池中分配块
		for ( Page& page : m_pages)
		{
			if (chunkCapacity <= page.m_capacityRemain)
			{
				page.m_capacityRemain -= chunkCapacity;
				EffectParticlePool::Chunk* chunk = EchoNew( EffectParticlePool::Chunk(chunkCapacity, page.m_pool));
#ifdef ECHO_DEBUG
				m_chunks.push_back(chunk);
#endif
				return chunk;
			}
		}

		// 新建池
		ui32 pageSize = std::max<ui32>(chunkCapacity, m_pageSize);
		m_pages.push_back(Page(pageSize));

		return NewChunk(chunkCapacity);
	}

	// 释放块
	void EffectParticlePool::ReleaseChunk(EffectParticlePool::Chunk* chunk)
	{
		if (chunk)
		{
			for (Page& page : m_pages)
			{
				if (page.m_pool == chunk->m_pool)
				{
					page.m_capacityRemain += chunk->m_capacity;
					EchoSafeDelete(chunk, Chunk);
#ifdef ECHO_DEBUG
#ifndef ECHO_PLATFORM_HTML5
					if (!m_chunks.empty())
					{
						m_chunks.erase(std::remove_if(m_chunks.begin(), m_chunks.end(), [chunk](Chunk* c) {
							return c == chunk;
						}),m_chunks.end());
					}
#else
					EchoAssert(false);
#endif
#endif
					return;
				}
			}

			EchoAssert(false && "Can't find the chunk");
		}
	}

	// 更新
	void EffectParticlePool::tick(i32 elapsedTime)
	{
		// 删除空闲页
		for (Page& page : m_pages)
		{
			if (page.m_capacityRemain == page.m_pool->getCapacity())
			{
				EchoAssert(!page.m_pool->getSize());

				page.m_idleTime += elapsedTime;
			}
			else
			{
				page.m_idleTime = 0;
			}
		}
		if (!m_pages.empty())
		{
#ifndef ECHO_PLATFORM_HTML5
			m_pages.erase(std::remove_if(m_pages.begin(), m_pages.end(), [](const Page& p) {
				return p.m_idleTime > 30000;
			}),m_pages.end());
#else
			EchoAssert(false);
#endif
		}
	}
}