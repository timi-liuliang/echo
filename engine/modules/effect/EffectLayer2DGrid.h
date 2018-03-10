#ifndef __ECHO_EFFECT_LAYER_2DGRID_H__
#define __ECHO_EFFECT_LAYER_2DGRID_H__

#include "EffectLayer3DGrid.h"

namespace Echo
{
	class EffectLayer2DGrid : public EffectLayer3DGrid
	{
	public:
		EffectLayer2DGrid();
		virtual ~EffectLayer2DGrid();

		virtual void getMatrixWorld(Matrix4& mtxWorld);

		virtual const Quaternion& getSurfaceOritation()const;
	protected:
		Quaternion mCurrentCamOritation;
	};
}

#endif