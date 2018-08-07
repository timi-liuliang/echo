#include "engine/core/camera/CameraShadow.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	// 构造函数
	CameraShadow::CameraShadow()
		: m_enable(true)
		, m_Box(AABB::ZERO)
		, m_CalcBox(AABB::ZERO)
	{
		m_viewProj = (Matrix4::IDENTITY);
		m_dir = Vector3::ZERO;
	}

	// 析构函数
	CameraShadow::~CameraShadow()
	{

	}

	// 更新
	void CameraShadow::update(const AABB* visibleActorsAABB)
	{
		m_enable = visibleActorsAABB->isValid();
		m_Box = *visibleActorsAABB;

		// 由于在计算物体是否需要接受阴影时会使用包围盒相交
		// 但是如果出现单个角色投射阴影时会因为包围盒较小但是阴影投射到2D平面之后会和其他物体相交
		// 这里扩大一下包围盒保证投射之后的物体可以正确接收阴影
		// 一个比较蛋疼的Trick
		

		if (visibleActorsAABB && visibleActorsAABB->isValid())
		{
			const Vector3 cameraDir = /*m_dir == Vector3::ZERO ? -EchoSceneManager->getMainLightDir() :*/ m_dir;

			// 计算在摄像机朝向上的投影长度
			float	halfLen = visibleActorsAABB->getDY() * 0.5f / Echo::Math::Abs(cameraDir.y) + 0.5f;
			Vector3 eyePos = visibleActorsAABB->getCenter() - cameraDir * halfLen;

			// 计算观察矩阵
			Matrix4 viewMat;
			Matrix4::LookAtRH(viewMat, eyePos, eyePos + cameraDir, Vector3::UNIT_Y);

			// 计算正交矩阵
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

	// 根据包围盒与观察矩阵计算正交矩阵
	void CameraShadow::calcOrthoRH(Matrix4& oOrth, const AABB& box, const Matrix4& viewMat)
	{
		// 计算观察空间AABB
		AABB orthAABB;
		for (int i = AABB::CORNER_NLT; i < AABB::CORNER_FRT; i++)
		{
			orthAABB.addPoint(viewMat.transform(box.getCorner(AABB::Corner(i))));
		}

		// 纠正远近截面
		orthAABB.vMax.z += 40.f;

		float dx = orthAABB.getDX();
		float dy = orthAABB.getDY();

		// 执行计算
		Matrix4::OrthoRH(oOrth, dx, dy, orthAABB.vMin.z, orthAABB.vMax.z);
	}

	void CameraShadow::setLightDir(const Vector3& dir)
	{
		m_dir = dir;
	}
}
