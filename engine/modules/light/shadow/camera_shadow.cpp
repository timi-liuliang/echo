#include "camera_shadow.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	CameraShadow::CameraShadow()
		: m_enable(true)
		, m_Box(AABB::ZERO)
		, m_CalcBox(AABB::ZERO)
	{
		m_viewProj = (Matrix4::IDENTITY);
		m_dir = Vector3::ZERO;
	}

	CameraShadow::~CameraShadow()
	{

	}

	void CameraShadow::update(const AABB* visibleActorsAABB)
	{
		m_enable = visibleActorsAABB->isValid();
		m_Box = *visibleActorsAABB;

		if (visibleActorsAABB && visibleActorsAABB->isValid())
		{
			const Vector3 cameraDir = /*m_dir == Vector3::ZERO ? -EchoSceneManager->getMainLightDir() :*/ m_dir;

			float	halfLen = visibleActorsAABB->getDY() * 0.5f / Echo::Math::Abs(cameraDir.y) + 0.5f;
			Vector3 eyePos = visibleActorsAABB->getCenter() - cameraDir * halfLen;

			Matrix4 viewMat;
			Matrix4::LookAtRH(viewMat, eyePos, eyePos + cameraDir, Vector3::UNIT_Y);

			Matrix4 orthMat;
			calcOrthoRH(orthMat, *visibleActorsAABB, viewMat);

			m_viewProj = viewMat * orthMat;
			m_view = viewMat;

			Vector3 zAxis = -cameraDir;
			zAxis.normalize();
			Vector3 _right;
			Vector3 vUp = Vector3::UNIT_Y;
			Vector3::Cross(_right, vUp, zAxis);
			_right.normalize();
			Vector3 vNearCenter = eyePos;
			Vector3 vFarCenter = eyePos - zAxis * (visibleActorsAABB->getDZ() + 40.f);
			float halfW = visibleActorsAABB->getDX() * 0.5f;
			float halfH = visibleActorsAABB->getDY() * 0.5f;
			Vector3 corner[8];
			corner[0] = vNearCenter - _right * halfW + vUp * halfH;
			corner[1] = vNearCenter + _right * halfW - vUp * halfH;
			corner[2] = vNearCenter - _right * halfW - vUp * halfH;
			corner[3] = vNearCenter + _right * halfW + vUp * halfH;
			corner[4] = vFarCenter - _right * halfW + vUp * halfH;
			corner[5] = vFarCenter + _right * halfW - vUp * halfH;
			corner[6] = vFarCenter - _right * halfW - vUp * halfH;
			corner[7] = vFarCenter + _right * halfW + vUp * halfH;

			m_CalcBox.reset();
			for (ui32 i = 0; i < 8; ++i)
			{
				m_CalcBox.addPoint(corner[i]);
			}
		}
	}

	void CameraShadow::calcOrthoRH(Matrix4& oOrth, const AABB& box, const Matrix4& viewMat)
	{
		AABB orthAABB;
		for (int i = AABB::CORNER_NLT; i < AABB::CORNER_FRT; i++)
		{
			orthAABB.addPoint(viewMat.transform(box.getCorner(AABB::Corner(i))));
		}

		orthAABB.vMax.z += 40.f;

		float dx = orthAABB.getDX();
		float dy = orthAABB.getDY();

		Matrix4::OrthoRH(oOrth, dx, dy, orthAABB.vMin.z, orthAABB.vMax.z);
	}

	void CameraShadow::setLightDir(const Vector3& dir)
	{
		m_dir = dir;
	}
}
