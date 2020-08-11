#pragma once

#include "engine/core/resource/Res.h"
#include "engine/core/render/base/mesh/Mesh.h"

namespace Echo
{
	class MeshRes : public Res
	{
		ECHO_RES(MeshRes, Res, ".mesh", Res::create<MeshRes>, MeshRes::load);

	public:
		MeshRes() {}

	protected:
		// load|save
		static Res* load(const ResourcePath& path);
		virtual void save() override;

	private:
		MeshRes(const ResourcePath& path);
		~MeshRes();

	private:
		Mesh*		m_mesh = nullptr;
	};
	typedef Echo::ResRef<Echo::MeshRes> MeshResPtr;
}
