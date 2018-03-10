#include "Animation.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Math/Quaternion.h"
#include "AnimKeyFrame.h"


namespace Echo
{
	AnimKeyFrame::AnimKeyFrame(Animation* pAnim, ui32 time)
		: m_pAnim(pAnim)
		, m_KeyOrientation(NULL)
		, m_KeyPosition(NULL)
	{
	}

	AnimKeyFrame::~AnimKeyFrame()
	{
		m_KeyOrientation.clear();
		m_KeyPosition.clear();
	}

	void AnimKeyFrame::createAnimData(Dword boneCount)
	{
		assert(m_KeyOrientation.empty() && m_KeyPosition.empty());
		m_KeyOrientation.resize(boneCount);
		m_KeyPosition.resize(boneCount);
	}

	void AnimKeyFrame::setAnimData(Dword handle, const Quaternion& ori, const Vector4& pos)
	{
		m_KeyOrientation[handle] = ori;
		m_KeyPosition[handle] = pos;
	}

	// 设置动画数据
	void AnimKeyFrame::setAnimDatammap(ui32 boundCount, const char* dataPtr)
	{
#ifdef LIULIANG_ANIMATION_MMAP
		m_boneCount = boundCount;
		m_data = (const KeyData*)dataPtr;
#endif
	}

	Animation* AnimKeyFrame::getAnim() const
	{
		return m_pAnim;
	}

	const Quaternion& AnimKeyFrame::getAnimQuaternion(ui32 idx) const
	{
#ifdef LIULIANG_ANIMATION_MMAP
		return m_data[idx].orientation;
#else

		return m_KeyOrientation[idx];
#endif
	}

	const Vector4& AnimKeyFrame::getAnimTranslate(ui32 idx) const
	{
#ifdef LIULIANG_ANIMATION_MMAP
		return m_data[idx].pos;
#else
		return m_KeyPosition[idx];
#endif
	}

}