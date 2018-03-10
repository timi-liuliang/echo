#ifndef __EFFECT_PARTICLE_H__
#define __EFFECT_PARTICLE_H__

#include "Engine/Core.h"
#include <engine/core/Util/ObjectPool.h>

namespace Echo
{
	/**
	 * 粒子定义
	 */
	class EffectParticle
	{
	public:
		EffectParticle(void);
		virtual ~EffectParticle(void);

		virtual void copyAttributesTo(EffectParticle* particle);

		void reParticleState();
		void forwardParticleState();
		void forwardParticleState2();

		EffectParticle* parentParticle;

		//// copy attribute begin
		Vector3		position;			// 位置
		Quaternion	oritation;
		float		selfRotation;
		Color		color;				// 颜色
		float		scale;				// 缩放
		Vector3		direction;			// 朝向

		Vector3		originalPosition;
		Quaternion	originalOritation;
		float		originalSelfRotation;
		Color		originalColor;
		float		originalScale;
		float       originalSpeed;
		Vector3		originalDirection;

		Vector3		movementDirection;
		i32			mTime;
		i32         mLeaveTime;
		// copy attribute end

		Vector3		scenePosition;
		Quaternion	sceneOritation;

		Vector3		mlastPosition;
		int			mReDirection;
		bool		mInController;

		unsigned long startPlayTime;	//开始播放粒子的时间,用于确定播放到了第几帧
		int			  mRandomAnimIndex; //平铺模式下要播放动画哪一帧

		static const size_t sCopyAttributeSize;
		static const size_t sReStateSize;
	};

	/**
	 * 粒子池
	 * 所有粒子建议页数不要超过10页
	 */
	class EffectParticlePool
	{
	public:
		// 结点
		struct Chunk
		{
			friend class EffectParticlePool;

			ui32						m_size;
			ui32						m_capacity;		// 大小
			ObjectPool<EffectParticle>*	m_pool;			// 对应的对象池

			// 新建
			EffectParticle* New() 
			{
				if (m_size < m_capacity)
				{
					m_size++;

					EffectParticle* particle = m_pool->New();
					EchoAssert(particle);
					return particle;
				}

				return NULL;
			}

			// 析构
			void Delete(EffectParticle* particle)
			{
				m_size--;
				m_pool->Delete(particle);
			}

		private:
			Chunk(ui32 capacity, ObjectPool<EffectParticle>* pool)
				: m_size(0), m_capacity(capacity), m_pool(pool)
			{}

			~Chunk()
			{
				EchoAssert(!m_size);
			}
		};

	private:
		// 页
		struct Page
		{
			ObjectPool<EffectParticle>*	m_pool;				// 对象池
			ui32						m_capacityRemain;	// 剩余对象池
			ui32						m_idleTime;			// 
			ui32						m_capacity;			// 容量

			Page(ui32 capacity);
			Page(const Page&) = delete;
			Page(Page&& other) :
				m_pool(other.m_pool),
				m_capacityRemain(other.m_capacityRemain),
				m_idleTime(other.m_idleTime)
			{
				other.m_pool = nullptr;
			}
			Page& operator=(const Page&) = delete;
			Page& operator=(Page&& other)
			{
				std::swap(m_pool, other.m_pool);
				m_capacityRemain = other.m_capacityRemain;
				m_idleTime = other.m_idleTime;
				other.m_pool = nullptr;
				return *this;
			}
			~Page();
		};

	public:
		EffectParticlePool(ui32 pageSize);
		~EffectParticlePool();

		// 获取描述
		void buildDesc( String& desc);

		// 新建块
		Chunk* NewChunk(ui32 chunkCapacity);

		// 释放块
		void ReleaseChunk(Chunk* chunk);

		// 更新
		void tick(i32 elapsedTime);

	private:
		vector<Page>::type		m_pages;	// 所有对象池
		ui32					m_pageSize;	// 默认页大小
#ifdef ECHO_DEBUG
		vector<Chunk*>::type	m_chunks;	// 块
#endif
	};
}

#endif
