#ifndef __ECHO_EFFECT_LAYER_TRAIL_H__
#define __ECHO_EFFECT_LAYER_TRAIL_H__

#include "EffectLayer.h"

namespace Echo
{
	class EffectLayerTrail : public EffectLayer
	{
		struct TrailElement
		{
			TrailElement(const Vector3& pos1, const Vector3& pos2, const Quaternion& orient, const Vector3& translate, const Vector3& worldScale, const Quaternion& worldOrient, const Vector3& worldTranslate, i32 t)
				: top(pos1)
				, bottom(pos2)
				, orient(orient)
				, translate(translate)
				, worldScale(worldScale)
				, worldOrient(worldOrient)
				, worldTranslate(worldTranslate)
				, currentTime(t)
			{
			}

			TrailElement()
			{
			}

			Vector3 top;
			Vector3 bottom;
			Quaternion orient;
			Vector3 translate;
			Vector3 worldScale;
			Quaternion worldOrient;
			Vector3 worldTranslate;
			i32  currentTime;
		};

	public:
		virtual void getPropertyList(PropertyList& list);
		virtual bool getPropertyType(const String& name, PropertyType& type);
		virtual bool getPropertyValue(const String& name, String& value);
		virtual bool setPropertyValue(const String& name,const String& value);

	public:
		EffectLayerTrail();
		virtual ~EffectLayerTrail();

		virtual void prepare();
		virtual void unprepare();

		virtual int _update(i32 time);
		virtual void updateRenderData();
		
		// 提交到渲染队列
		virtual void submitToRenderQueue(std::vector<EffectRenderable*>& renderables);

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);
		virtual void mergeAligenBox(Box& aabb);

		virtual void copyAttributesTo(EffectParticle* particle);

		virtual void _notifyReStart();
		virtual void _notifyStart();

	protected:
		void resetElements();

	private:
		// 根据最小步长更新
		int _updateMinStep(i32 time);

	protected:
		Vector3 mOriginalPosition1;					
		Vector3 mOriginalPosition2;					
		int	 mDurationTime;						
		bool mFallowParent;

		static const float minDistance;         
		typedef list<TrailElement>::type TrailElementList;
		typedef list<TrailElement>::type::iterator TrailElementIter;

		TrailElementList mTrailElementList;			
		size_t			mElementNum;						
		Vector3			mLastPosition;

		bool			mEndWithEffectTime;
		bool			mUseAnimTrail;       //是否使用动画轨迹
		ui32            mAnimMinStep;		 //细分时间间隔
		ui32			mAnimBeginTime;
		String			mSkeltonName;
		String			mAnimationName;
		String			mAttachBoneName;
		Vector3			mAttachPosOffset;
		Vector3         mAttachOriOffset;

		Skeleton*		mSkeleton;
		Animation*		mAnimation;
		AnimState*		mAnimationState;
		AnimBlender*	mAnimationBlender;
		i32				mInnerAnimationTime;
		Node*      mAttachNode;
		Node*		mAttachBoneNode;	
		Node*		mAttachDummyNode;

	private:
		ui32			mCurrentFrameTime;
		i32				m_elapsedTimecache;	// 更新时使用
	};
}

#endif