#pragma once

#include "Engine/modules/Model/Mesh.h"

namespace Echo
{
	/**
	 * 程序控制动态Mesh
	 */
	class DynamicMesh : public Mesh
	{
	public:
		DynamicMesh(const String& name);
		virtual ~DynamicMesh();

		// 是否需要更新
		virtual bool isNeedUpdate(){ return true; }

		// 更新
		virtual void update(ui32 delta, const Echo::Vector3& vpos, const Echo::Quaternion& qrotate);

	protected:
		virtual bool prepareImpl(DataStream* stream);
		virtual void unprepareImpl();
	};
}
