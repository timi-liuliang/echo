#ifndef __ECHO_EFFECT_LAYER_3DBILLBOARD_H__
#define __ECHO_EFFECT_LAYER_3DBILLBOARD_H__

#include "EffectLayer.h"

namespace Echo
{
	/**
	 * 3D公告板
	 */
	class EffectLayer3DBillBoard : public EffectLayer
	{
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		EffectLayer3DBillBoard();
		virtual ~EffectLayer3DBillBoard();

		virtual void prepare();
		virtual void updateRenderData();
		
		// 提交到渲染队列
		virtual void submitToRenderQueue(std::vector<EffectRenderable*>& renderables);

		// 用于特效的伸缩（只要设置了，原来的数据只有宽度有用）
		virtual void _notifyExtenalParam(const Vector3& p1, const Vector3& p2);

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);
		virtual void mergeAligenBox(Box& aabb);

		virtual void copyAttributesTo(EffectParticle* particle);

		virtual int	 _update(i32 time);


	protected:
		float mWidth;
		float mHeight;
		float mCenterOffestW;
		float mCenterOffestH;
		Vector3  mPosCached[4];
		Vector3  mStartPoint;
		Vector3  mEndPoint;
		bool	 mbStretch;		//是否设置可拉伸
		float	 mAutoUV[4];
	};
}

#endif