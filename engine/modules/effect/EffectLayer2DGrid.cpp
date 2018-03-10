#include "Engine/Core.h"
#include "EffectLayer2DGrid.h"
#include "EffectSystemManager.h"
#include "Engine/core/Scene/Node.h"

namespace Echo
{
	EffectLayer2DGrid::EffectLayer2DGrid()
		: EffectLayer3DGrid() 
	{
		mType = ELT_2DEditableSurface;
	}

	EffectLayer2DGrid::~EffectLayer2DGrid()
	{

	}

	void EffectLayer2DGrid::getMatrixWorld(Matrix4& mtxWorld)
	{
		Node* parentNode = mParentSystem->getEffectSceneNode();

		Matrix4 mtxLocal;
		mtxLocal.makeScaling(scale, scale, scale);
		mCurrentCamOritation = mParentSystem->getCurrentCamera()->getOritation();
		Matrix4 mtxRot;
		Quaternion quanRot;
		quanRot.fromAxisAngle(originalDirection, selfRotation*Math::DEG2RAD);
		quanRot = quanRot*mCurrentCamOritation;
		quanRot.toMat4(mtxRot);
		mtxLocal = mtxLocal * mtxRot;
		mtxLocal.translate(position);

		const Vector3& scale = parentNode->getWorldScaling();
		const Vector3& pos = parentNode->getWorldPosition();
		mtxWorld.makeScaling(scale);
		mtxWorld.translate(pos);

		mtxWorld = mtxLocal * mtxWorld;
	}

	const Quaternion& EffectLayer2DGrid::getSurfaceOritation()const
	{
		return mCurrentCamOritation;
	}
}
