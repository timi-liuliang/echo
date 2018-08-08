#include "VenusPentagramExample.h"
#include <Foundation/Util/Array.hpp>
#include <Engine/Camera/CameraMain.h>
#include <Engine/Physics/QueryObject.h>
#include <Engine/Object/Root.h>

namespace Examples
{
	// 构造函数
	VenusPentagram::VenusPentagram()
		: Example()
		, m_sun( NULL)
		, m_mercury( NULL)
		, m_venus(NULL)
		, m_earth(NULL)
		, m_mars(NULL)
		, m_jupiter(NULL)
		, m_saturn(NULL)
		, m_uranus(NULL)
		, m_neptune(NULL)
	{

	}

	// 帮助信息
	const LORD::String VenusPentagram::getHelp()
	{
		return "VenusPentagram";
	}

	#define PLANET_SCALE 20.f

	// 初始化
	bool VenusPentagram::initialize()
	{
		// 1.修改背景色为黑色(星空背景)
		LORD::Renderer::BGCOLOR = LORD::Color(0.f, 0.f, 0.f);

		// 2.新建行星
		m_sun = LordNew( Planet("Sun", 1392000.f, LORD::Color::YELLOW));
		m_mercury = LordNew(Planet("mercury", 4850.f * PLANET_SCALE, LORD::Color(1.f, 1.f, 1.f)));
		m_venus = LordNew(Planet("Venus", 12000.f * PLANET_SCALE, LORD::Color(1.f, 0.9f, 0.7f)));
		m_earth = LordNew(Planet("Earth", 12500.f * PLANET_SCALE, LORD::Color(0.2f, 0.2f, 0.7f)));
		m_mars = LordNew(Planet("mars", 6790.f * PLANET_SCALE, LORD::Color(0.9f, 0.7f, 0.6f)));
		m_jupiter = LordNew(Planet("jupiter", 142980.f * PLANET_SCALE, LORD::Color(1.f, 0.9f, 0.8f)));
		m_saturn = LordNew(Planet("saturn", 120536.f * PLANET_SCALE, LORD::Color(1.f, 1.f, 1.f)));
		m_uranus = LordNew(Planet("uranus", 51118.f * PLANET_SCALE, LORD::Color(1.f, 1.f, 1.f)));
		m_neptune = LordNew(Planet("neptune", 49500.f * PLANET_SCALE, LORD::Color(1.f, 1.f, 1.f)));

		// 3.设置父星及相对父星的运行轨迹
		m_mercury->setRevolution(m_sun, 3.f, -LORD::Math::PI / 88.f);
		m_venus->setRevolution(m_sun, 7.f, -LORD::Math::PI / 224.f);
		m_earth->setRevolution(m_sun, 10.f, -LORD::Math::PI / 364.f);
		m_mars->setRevolution(m_sun, 15.f, -LORD::Math::PI / 687.f);
		m_jupiter->setRevolution(m_sun, 20.f, -LORD::Math::PI / 12.f / 365.f);
		m_saturn->setRevolution(m_sun, 25.f, -LORD::Math::PI / 12.f / 365.f);
		m_uranus->setRevolution(m_sun, 30.f, -LORD::Math::PI / 84.f / 365.f);
		m_neptune->setRevolution(m_sun, 35.f, -LORD::Math::PI / 165.f / 365.f);

		// 4.设置金星显示轨迹
		//m_mercury->setShowTrack(true);
		m_venus->setShowTrack(true);
		//m_earth->setShowTrack(true);
		//m_mars->setShowTrack(true);
		//m_jupiter->setShowTrack(true);
		//m_saturn->setShowTrack(true);
		//m_uranus->setShowTrack(true);
		//m_neptune->setShowTrack(true);

		return false;
	}

	// 更新
	void VenusPentagram::tick(LORD::ui32 elapsedTime)
	{
		// 固定单步更新时长，减少浮点计算误差
		LORD::ui32 updateDuration = 10;
		static LORD::ui32 leftTime = 10; leftTime += elapsedTime;
		if (leftTime >= updateDuration)
		{
			leftTime -= updateDuration;

			// 1.更新地球，金星位置
			m_sun->update(updateDuration);
			m_mercury->update(updateDuration);
			m_venus->update(updateDuration);
			m_earth->update(updateDuration);
			m_mars->update(updateDuration);
			m_jupiter->update(updateDuration);
			m_saturn->update(updateDuration);
			m_uranus->update(updateDuration);
			m_neptune->update(updateDuration);

			// 2.摄像机对准地球
			LORD::Vector3 position = m_earth->getSceneNode()->getWorldPosition();
			LORD::Camera* mainCamera = LordSceneManager->getMainCamera();
			mainCamera->setPosition(LORD::Vector3(0.f, 0.f, 60.f) + position);
			mainCamera->setDirection(LORD::Vector3(0.f, 0.f, -1.f));
		}

		// 3.更新行星轨迹显示
		m_sun->updateTrackDisplay();
		m_mercury->updateTrackDisplay();
		m_venus->updateTrackDisplay();
		m_earth->updateTrackDisplay();
		m_mars->updateTrackDisplay();
		m_jupiter->updateTrackDisplay();
		m_saturn->updateTrackDisplay();
		m_uranus->updateTrackDisplay();
		m_neptune->updateTrackDisplay();
	}

	// 销毁
	void VenusPentagram::destroy()
	{
		LordSafeDelete( m_mercury, Planet);
		LordSafeDelete( m_venus, Planet);
		LordSafeDelete( m_earth, Planet);
		LordSafeDelete( m_mars, Planet);
		LordSafeDelete( m_jupiter, Planet);
		LordSafeDelete( m_saturn, Planet);
		LordSafeDelete( m_uranus, Planet);
		LordSafeDelete( m_neptune, Planet);
		LordSafeDelete( m_sun, Planet);
	}

	// 构造函数
	Planet::Planet(const LORD::String& name, float radius, const LORD::Color& color)
		: m_name( name)
		, m_revolutionSpeed(0.f)
		, m_revolutionAngle(0.f)
		, m_revolutionRadius(0.f)
		, m_color(color)
		, m_isShowTrack(false)
		, m_track( NULL)
		, m_trackNode( NULL)
		, m_links( NULL)
	{
		// 2.渲染几何体
		m_planet = LordDebugDisplayManager->createQueryObject( name + "Planet");
		m_planet->setVisible(true);
		m_planet->setQueryFlag(0x00400000);
		m_planet->setTestDepth(true);
		m_planet->setWriteDepth(true);

		m_planet->createShpere(radius / 1392000.f, 15, 15, m_color);

		m_sceneNode = LordSceneManager->getRootNode()->createChild();
		m_sceneNode->setWorldPosition(LORD::Vector3::ZERO);
		m_planet->attachTo( m_sceneNode);
	}

	// 析构函数
	Planet::~Planet()
	{
		// 删除DebugDisplayItem
		if (m_planet)
		{
			m_planet->attachTo(NULL);
			LordDebugDisplayManager->destroyQueryObject(m_planet);
		}

		// 删除结点
		if ( m_sceneNode)
		{
			m_sceneNode->getParent()->destroyChild(m_sceneNode);
		}

		// 删除轨迹
		if (m_track)
		{
			m_track->attachTo(NULL);
			LordDebugDisplayManager->destroyQueryObject( m_track);
		}

		// 删除轨迹节点
		if (m_trackNode)
		{
			m_trackNode->getParent()->destroyChild(m_trackNode);
		}
	}

	// 更新
	void Planet::update( LORD::ui32 elapsedTime)
	{
		m_revolutionAngle += m_revolutionSpeed * elapsedTime * 0.001f;
		
		LORD::Vector3 localDir(sin(m_revolutionAngle), cos(m_revolutionAngle), 0.f); localDir.normalize();
		m_sceneNode->setLocalPosition( localDir * m_revolutionRadius);
		m_sceneNode->update();
	}

	// 更新显示轨迹
	void Planet::updateTrackDisplay()
	{
		// 记录轨迹
		if (m_isShowTrack)
		{
			LORD::Vector3 posRelativeToView = m_sceneNode->getWorldPosition() - LordSceneManager->getMainCamera()->getPosition();
			m_trackPositions.push_back( posRelativeToView);

			// 最大顶点数限制为15000
			if (m_trackPositions.size() > 15000)
			{
				while (m_trackPositions.size() > 10000)
				{
					m_trackPositions.pop_front();
				}
			}
		}

		// 更新Track显示
		if (m_trackPositions.size() >= 2)
		{
 			createTrack();
			m_track->beginPolygon(LORD::QueryObject::QO_WIREFRAME);
 			{
 				// 保证渲染时为偶数个顶点
 				Vector3List::iterator endIt = m_trackPositions.size() % 2 == 0 ? m_trackPositions.end() : --m_trackPositions.end();
 				for (Vector3List::iterator it = m_trackPositions.begin(); it != endIt; it++)
				{
 					m_track->setPosition( (*it) + LordSceneManager->getMainCamera()->getPosition());
 					m_track->setColor(m_color);
 				}
 			}
 			m_track->endPolygon();
		}
	}

	// 设置父星
	void Planet::setRevolution(Planet* parent, float revolutionRadius, float revolutionSpeed)
	{
		m_revolutionRadius = revolutionRadius;
		m_revolutionSpeed  = revolutionSpeed * 100.f;
		m_parent           = parent;

		m_sceneNode->setParent(parent->getSceneNode());
		LORD::Vector3 localDir(sin(m_revolutionAngle), cos(m_revolutionAngle), 0.f); localDir.normalize();
		m_sceneNode->setLocalPosition(localDir * m_revolutionRadius);
		m_sceneNode->update();
	}

	// 设置是否显示轨迹
	bool Planet::setShowTrack(bool isShowTrack)
	{
		m_trackPositions.clear();
		m_isShowTrack = true;

		return true;
	}

	// 创建轨迹显示项
	void Planet::createTrack()
	{
		// 创建Track
		if (!m_track)
		{
			m_track = LordDebugDisplayManager->createQueryObject(m_name + "_Track");
			m_track->setVisible(true);
			m_track->setQueryFlag(0x00400000);
			m_track->setTestDepth(true);
			m_track->setWriteDepth(true);

			if (!m_trackNode)
			{
				m_trackNode = LordSceneManager->getRootNode()->createChild();
			}
			m_trackNode->setWorldPosition(LORD::Vector3::ZERO);
			m_track->attachTo(m_trackNode);
		}
	}
}