#include "engine/core/Math/Quaternion.h"
#include "engine/core/Math/Vector4.h"
#include "Engine/modules/Anim/Animation.h"
#include "engine/core/Util/LogManager.h"
#include "Engine/modules/Anim/AABBKeyFrame.h"


namespace Echo
{
	AABBKeyFrame::AABBKeyFrame(Animation* pAnim, ui32 time)
		:AnimKeyFrame(pAnim, time)
	{
	}

	AABBKeyFrame::~AABBKeyFrame()
	{
	}

	void AABBKeyFrame::setBox(const Box &box)
	{
		m_box = box;
	}

	void AABBKeyFrame::setBoxmmap(const char* dataPtr)
	{
		m_box.vMin = (Vector3&)dataPtr; dataPtr += sizeof(Vector3);
		m_box.vMax = (Vector3&)dataPtr;
	}

	const Box& AABBKeyFrame::getBox() const
	{
		return m_box;
	}
}
