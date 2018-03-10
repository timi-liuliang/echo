#ifndef __ECHO_EFFECT_LAYER_2DBILLBOARD_H__
#define __ECHO_EFFECT_LAYER_2DBILLBOARD_H__

#include "EffectLayer.h"

namespace Echo
{
	/**
	 * 2D公告板
	 */
	class EffectLayer2DBillBoard : public EffectLayer
	{
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		EffectLayer2DBillBoard();
		virtual ~EffectLayer2DBillBoard();

		virtual void updateRenderData();

		// 提交到渲染队列
		virtual void submitToRenderQueue(std::vector<EffectRenderable*>& renderables);

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);
		virtual void mergeAligenBox(Box& aabb);

		virtual void copyAttributesTo(EffectParticle* particle);

	protected:
		float mWidth;
		float mHeight;
		float mCenterOffestW;
		float mCenterOffestH;
		float mOffsetViewport;
		bool  mbPerpendicular;
	};
}

#endif