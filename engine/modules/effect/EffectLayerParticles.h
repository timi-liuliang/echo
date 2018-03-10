#ifndef __EFFECT_LAYER_PARTICLES_H__
#define __EFFECT_LAYER_PARTICLES_H__

#include "EffectLayer.h"
#include "EffectParticleControllerSet.h"
#include <engine/core/Util/ObjectPool.h>

namespace Echo
{
	/**
	 * 特效层级(粒子系统)
	 */
	class EffectLayerParticles : public EffectLayer
	{
	public:
		static const i32	DEFAULT_PT_QUOTA;			
		static const bool	DEFAULT_PT_3DBILLBOARD;		
		static const bool	DEFAULT_PT_NOTRANSSCALE;	
		static const i32    DEFAULT_PT_EMITTERRATE;    
		static const float  DEFAULT_PT_EMITTERANGLE;   
		static const float  DEFAULT_PT_PTSPEED;	
		static const float  DEFAULT_PT_MAXPTSPEED;
		static const float  DEFAULT_PT_PTADDSPEED;
		static const i32    DEFAULT_PT_PTLEAVETIME;
		static const i32    DEFAULT_PT_MAXPTLEAVETIME;
		static const Color	DEFAULT_PT_MINCOLOR;
		static const Color	DEFAULT_PT_MAXCOLOR;
		static const float  DEFAULT_PT_MINSCALE;
		static const float  DEFAULT_PT_MAXSCALE;
		static const float  DEFAULT_PT_MINANGLE;
		static const float  DEFAULT_PT_MAXANGLE;
		static const bool	DEFAULT_PT_FALLOWPARENT;
		static const bool	DEFAULT_PT_EMITTERINSURFACE;
		static const i32	DEFAULT_PT_ANIMROWNUM;
		static const i32	DEFAULT_PT_ANIMCOLUMNNUM ;
		static const i32	DEFAULT_PT_ANIMTIMEGAP;
		static const i32    DEFAULT_PT_ANIMSTARTINDEX;
		static const i32	DEFAULT_PT_ANIMENDINDEX;
		static const bool	DEFAULT_PT_ANIMTILED;

	public:
		virtual void getPropertyList(PropertyList& list);
		virtual bool getPropertyType(const String& name, PropertyType& type);
		virtual bool getPropertyValue(const String& name, String& value);
		virtual bool setPropertyValue(const String& name,const String& value);

	public:
		EffectLayerParticles();
		virtual ~EffectLayerParticles();

		virtual void prepare();

		virtual void unprepare();

		virtual void emitterParticle(EffectParticle* particle);

		virtual void _notifyStart(void);
		virtual void _notifyReStart(void);

		virtual void _notifyShowVisualHelp(bool show);

		virtual int _update(i32 time);

		virtual void updateRenderData();

		// 提交到渲染队列
		virtual void submitToRenderQueue(std::vector<EffectRenderable*>& renderables);

		void renderBillboard(const EffectParticle* billboard, EffectVertexFmt* pVertexPtr);

		virtual void copyAttributesTo(EffectParticle* layer);
		virtual void exportData(DataStream* pStream, int version);
		virtual void importData(DataStream* pStream, int version);
		virtual void mergeAligenBox(Box& aabb);

		// 清空粒子池
		void clearPool();
	
		// 创建粒子池
		void createPool(size_t size);

		EffectParticleControllerSet* createParticleControllerSet();
		void   setParticleControllerSet(EffectParticleControllerSet* particleControllerSet);
		EffectParticleControllerSet* getParticleControllerSet() const;
		void destroyParticleControllerSet();

		//子特效相关操作
		void addChildEffect(EffectParticle* particle);
		void removeChildEffect(EffectParticle* particle);
		void updateChildEffect(ui32 time);
		void childEffectsSubmitToRQ();
		void clearChildEffect();

	protected:
		i32				mParticleQuota;
		float			mWidth;
		float			mHeight;
		bool			mb3dBillboard;
		bool			mbPerpendicular;
		bool			mb3DBillboardTrans;
		bool			mbNoTransScale;
		bool			mbNoScaleWidth;
		bool			mbNoScaleHeight;
		float			mCenterOffsetW;
		float			mCenterOffsetH;
		i32				mEmitterRate;
		float			mEmitterAngle;
		float			mParticleSpeed;
		float			mParticleMaxSpeed;
		float			mParticleAddSpeed;
		i32				mParticleLeaveTime;
		i32				mParticleMaxLeaveTime;
		Color			mMinColor;
		Color			mMaxColor;
		float			mMinScale;
		float			mMaxScale;
		float			mMin2dAngle;
		float			mMax2dAngle;
		bool			mFallowParent;  
		bool			mEmitterInSurface;

		i32				mAnimRownum;		//序列帧动画行数
		i32				mAnimColumnnum;		//序列帧动画列数
		i32             mAnimTimeGap;		//序列帧动画间隔
		i32				mAnimStartIndex;	//序列帧动画开始序号
		i32				mAnimEndIndex;		//序列帧动画结束序号(由于美术可能把多个动画的序列帧放在一个贴图上不规则排布,所以提供这两个参数)
		bool			mbAnimTiled;		//是否平铺


		typedef list<EffectParticle* >::type ParticlePool;
		typedef list<EffectParticle* >::type::iterator ParticlePoolItor;
		ParticlePool		mParticlePoolInUse;
		//ParticlePool		mParticlePoolFree;
		EffectParticlePool::Chunk*	m_particlePoolChunk;

		EffectParticleControllerSet* mParticleControllerSet;

		i32				mEmitterTimeStep;		
		i32				mEmitterTime;			
		i32				mInnerParticleQuota; 
		size_t			particleUsed;

		String				mChildEffectName;	// 子特效名
		bool				mChildEffectLoop;	// 子特效是否循环
		float				mChildEffectSpeed;  // 子特效播放速度
		struct ChildEffect
		{
			EffectSystemID		effectID;
			Node*				sceneNode;
			EffectParticle*     particle;
		};
		
		list<ChildEffect*>::type  mChildEffects;		// 子特效队列

		bool            mPrewarm;				//预更新（一个mParticleMaxLeaveTime时间长度）
	};
}

#endif