#pragma once

#include "Frame/Example.h"

namespace Examples
{
	/**
	 * 金星轨迹(五角星形)
	 * 学习目标:
	 *     1.使用引擎DebugDisplay渲染基本几何体
	 *	   2.了解并使用SceneNode控制物体空间关系
	 */
	class Planet;
	class VenusPentagram : public Example
	{
	public:
		VenusPentagram();
		virtual ~VenusPentagram() {}

		// 帮助信息
		virtual const LORD::String getHelp();

		// 初始化
		virtual bool initialize();

		// 更新
		virtual void tick(LORD::ui32 elapsedTime);

		// 销毁
		virtual void destroy();

	private:
		Planet*			m_sun;			// 太阳
		Planet*			m_mercury;		// 水星
		Planet*			m_venus;		// 金星
		Planet*			m_earth;		// 地球
		Planet*			m_mars;			// 火星
		Planet*			m_jupiter;		// 木星
		Planet*			m_saturn;		// 土星
		Planet*			m_uranus;		// 天王星
		Planet*			m_neptune;		// 海王星
	};

	/**
	 * 行星定义
	 */
	class Planet
	{
		typedef LORD::list<LORD::Vector3>::type Vector3List;
	public:
		Planet( const LORD::String& name, float radius, const LORD::Color& color);
		~Planet();

		// 设置公转参数
		void setRevolution(Planet* parent, float revolutionRadius, float revolutionSpeed);

		// 设置是否显示轨迹
		bool setShowTrack( bool isShowTrack);

		// 获取场景节点
		LORD::SceneNode* getSceneNode() { return m_sceneNode;  }

		// 更新
		void update( LORD::ui32 elapsedTime);

		// 更新显示轨迹
		void updateTrackDisplay();

	private:
		// 创建轨迹显示项
		void createTrack();

	private:
		LORD::String			m_name;				// 行星名称(唯一)
		float					m_radius;			// 半径
		LORD::SceneNode*		m_sceneNode;		// 场景结点
		LORD::Color				m_color;			// 颜色
		LORD::DebugDisplayItem*	m_planet;			// 行星自已
		bool					m_isShowTrack;		// 是否显示轨迹
		Vector3List				m_trackPositions;	// 轨迹(世界坐标系)	
		LORD::DebugDisplayItem* m_track;			// 轨迹(世界坐标系)
		LORD::SceneNode*		m_trackNode;		// 轨迹结点
		LORD::DebugDisplayItem*	m_links;			// 与其它行星连线
		Planet*					m_parent;			// 父星
		float					m_revolutionRadius;	// 公转半径
		float					m_revolutionSpeed;	// 公转速度
		float					m_revolutionAngle;	// 公转当前角度
	};
}

