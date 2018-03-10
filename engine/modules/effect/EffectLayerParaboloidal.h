#ifndef __ECHO_EFFECT_LAYER_PARABOLOIDAL_H__
#define __ECHO_EFFECT_LAYER_PARABOLOIDAL_H__

#include "EffectLayer.h"

namespace Echo
{
	class EffectLayerParaboloidal : public EffectLayer
	{
	public:
		virtual void getPropertyList(PropertyList& list);
		virtual bool getPropertyType(const String& name, PropertyType& type);
		virtual bool getPropertyValue(const String& name, String& value);
		virtual bool setPropertyValue(const String& name,const String& value);

	public:
		EffectLayerParaboloidal();
		virtual ~EffectLayerParaboloidal();

		// 
		virtual void updateRenderData();
		
		// 提交到渲染队列
		virtual void submitToRenderQueue(std::vector<EffectRenderable*>& renderables);

		virtual void prepare();
		virtual void unprepare();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);
		virtual void mergeAligenBox(Box& aabb);

		virtual void copyAttributesTo(EffectParticle* particle);

	protected:
		static const i32	sParaboloidalLatitude;
		static const i32    sParaboloidalLongitude;
		float				mParam;
		float				mHeight;
		EffectVertexFmt*	mVertices;
	};
}

#endif