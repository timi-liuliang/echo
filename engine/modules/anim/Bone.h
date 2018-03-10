#ifndef __ECHO_BONE_H__
#define __ECHO_BONE_H__

#include "engine/core/Math/Matrix4.h"
#include "engine/core/Math/Quaternion.h"

namespace Echo
{
	class Skeleton;


	class Bone
	{
	public:
		typedef vector<Bone*>::type BoneList;
	public:
		Bone( Skeleton* pSkel, int id, int parentId, const String& name, const Quaternion& rot, const Vector4& pos, bool bHelper = false);
		virtual ~Bone();

	public:
		void			setParent(Bone* pBone);
		void			addChildren(Bone* pBone);

		Skeleton*		getSkeleton() const;
		int				getId() const;
		int				getParentId() const;
		const String&	getName() const;
		bool			isDummy() const;
		Bone*			getParent() const;
		const BoneList&	getChildren() const;
		const Quaternion& getRotation() const;
		const Vector3& getPosition() const;
		const Matrix4& getInverseMatrix() const;

	protected:
		Skeleton*	m_pSkel;
		Bone*		m_pParent;			// ¸¸¹Ç÷À
		int			m_id;
		int			m_parentId;
		ui32		m_shaderIdx;
		String		m_name;
		bool		m_bDummy;
		BoneList	m_children;			// ×Ó¹Ç÷À
		Quaternion	m_rotation;			// ¹Ç÷À³õÊ¼Ðý×ª
		Vector3		m_position;			// ¹Ç÷À³õÊ¼Î»ÖÃ
		float		m_scale;
		Matrix4		m_invMatrix;
	};
}

#endif