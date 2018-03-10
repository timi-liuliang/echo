#pragma once

#include "Engine/core/Geom/GeomLib.h"
#include "Engine/modules/Anim/AnimKeyFrame.h"

namespace Echo
{
	class AABBKeyFrame: public AnimKeyFrame
	{	
	public:
		AABBKeyFrame(Animation* pAnim, ui32 time);
		virtual ~AABBKeyFrame();

	public:

		virtual void		setBox(const Box &box);

		virtual void setBoxmmap(const char* dataPtr);

		virtual const Box&	getBox() const;

	protected:
		Box		m_box;
	};
}
