#include "engine/core/Math/Quaternion.h"
#include "engine/modules/Anim/Skeleton.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Render/render/Renderer.h"
#include "engine/modules/Anim/Bone.h"

namespace Echo
{
	Bone::Bone(Skeleton* pSkel, int id, int parentId, const String& name, const Quaternion& rot, const Vector4& pos, bool bHelper)
		:m_pSkel(pSkel)
		,m_pParent(NULL)
		,m_id(id)
		,m_parentId(parentId)
		,m_name(name)
		,m_bDummy(bHelper)
		,m_rotation(rot)
		,m_position(pos.x, pos.y, pos.z)
		,m_scale(pos.w)
	{
		if(!pSkel)
			EchoException("Skeleton is NULL.");

		if(pSkel->getBoneById(id))
			EchoException("The Bone [%d] is already exist.", id);

		float invScale = 1.0f / m_scale;
		m_invMatrix.fromQuan(rot);
		m_invMatrix.scale(invScale, invScale, invScale);
		m_invMatrix.transpose();

		Vector4 invPos = m_invMatrix.transform(pos);
		m_invMatrix.translate(-invPos.x, -invPos.y, -invPos.z);
	}

	Bone::~Bone()
	{
	}

	void Bone::setParent(Bone* pBone)
	{
		EchoAssert(pBone);
		m_pParent = pBone;
	}

	void Bone::addChildren(Bone* pBone)
	{
		EchoAssert(pBone);
		m_children.push_back(pBone);
	}

	Skeleton* Bone::getSkeleton() const
	{
		return m_pSkel;
	}

	int Bone::getId() const
	{
		return m_id;
	}

	int Bone::getParentId() const
	{
		return m_parentId;
	}

	const String& Bone::getName() const
	{
		return m_name;
	}

	bool Bone::isDummy() const
	{
		return m_bDummy;
	}

	Bone* Bone::getParent() const
	{
		return m_pParent;
	}

	const Bone::BoneList& Bone::getChildren() const
	{
		return m_children;
	}

	const Quaternion& Bone::getRotation() const
	{
		return m_rotation;
	}

	const Vector3& Bone::getPosition() const
	{
		return m_position;
	}

	const Matrix4& Bone::getInverseMatrix() const
	{
		return m_invMatrix;
	}
}