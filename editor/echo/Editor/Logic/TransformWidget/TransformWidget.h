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
		enum EditType
		{
			EM_EDIT_TRANSLATE,		// 移动
			EM_EDIT_ROTATE,			// 旋转
			EM_EDIT_SCALE,			// 缩放
		};

		enum EMoveType
		{
			EM_MOVE_NULL = -1,  // 默认类型,不移动
			EM_MOVE_X,			// 沿X轴正向移动
			EM_MOVE_Y,			// 沿Y轴正向移动
			EM_MOVE_Z,			// 沿Z轴正向移动
			EM_MOVE_XYPLANE,	// 在xy平面内移动
			EM_MOVE_YZPLANE,	// 在YZ平面内移动
			EM_MOVE_XZPLANE,	// 在XZ平面内移动
		};

		enum ERotateType
		{
			EM_ROTATE_NULL,
			EM_ROTATE_X,	// 绕X轴旋转
			EM_ROTATE_Y,	// 绕Y轴旋转
			EM_ROTATE_Z,	// 绕Z轴旋转
		};

	public:
		TransformWidget();

		// 关连接点
		void CatchEntity(Echo::Node* enity);

		// 清除附着的场景结点
		void Clear() { /*m_entityList.clear(); m_transforms.clear();*/ }

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

		// 是否工作中
		bool IsWorking();

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
		EMoveType				m_moveType;			// 移动类型
		ERotateType				m_rotateType;		// 旋转类型
		bool					m_bVisible;			// 是否可见
		float					m_fScale;
		//boost::array<Box3, 6>	m_moveBoxs;			// 移动时盒子
		std::vector<Echo::Node*>m_entityList;		// attached node list
	};
}