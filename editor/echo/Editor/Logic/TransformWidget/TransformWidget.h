#pragma once

#include <vector>
#include <engine/core/math/Math.h>
#include <engine/core/scene/node.h>
#include <engine/core/gizmos/Gizmos.h>
#include <engine/core/main/Engine.h>

namespace Studio
{
	class TransformWidget
	{
	public:
		enum class EditType
		{
			Translate,
			Rotate,
			Scale,
		};

		enum class MoveType
		{
			None = -1, 
			XAxis,
			YAxis,
			ZAxis,
			XYPlane,
			YZPlane,
			XZPlane,
		};

		enum class RotateType
		{
			None,
			XAxis,
			YAxis,
			ZAxis,
		};

		enum class ScaleType
		{
			None,
			X,
			Y,
			Z,
			All,
		};

	public:
		TransformWidget();

		// 鼠标移动
		//void OnMouseMove(const Echo::Vector3& rayOrig0, const Echo::Vector3& rayDir0, const Echo::Vector3& rayOrig1, const Echo::Vector3& rayDir1, POINT* ptPre = NULL, POINT* ptCurr = NULL);

		// 被选取
		bool OnMouseDown(const Echo::Vector3& rayOrig, const Echo::Vector3& rayDir);

		// 取消选择
		void OnMouseUp();

		// position
		void SetPosition(float _posX, float _posY, float _posZ);
		void SetPosition(const Echo::Vector3& pos);

		// 设置是否可见
		void SetVisible(bool visible);

		// 渲染
		void SetEditType(EditType type);

		// 移动
		void  Translate(const Echo::Vector3& trans);

		// 设置缩放
		void  SetScale(float fScale);

	private:
		// 更新碰撞检测盒子位置
		void UpdateTranslateCollBox();

		// draw
		void draw();
		void drawCone(float radius, float height, const Echo::Transform& transform, const Echo::Color& color);

	private:
		Echo::Gizmos*			m_axis;			// x, y, z 轴线
		//VisualSegment3* m_pPlaneLine[6];	// 显示面的6根线
		//VisualCone3*	m_pCone[3];			// 圆锥体
		Echo::Vector3			m_vPosition;		// 3D轴位置
		//VisualCycle3*			m_pCycle[3];
		//VisualShape*			m_pScale;			// 缩放
		EditType				m_editType;			// 编辑类型
		MoveType				m_moveType;			// 移动类型
		RotateType				m_rotateType;		// 旋转类型
		bool					m_bVisible;			// 是否可见
		float					m_fScale;
		//Echo::array<Box3, 6>	m_moveBoxs;			// 移动时盒子
	};
}