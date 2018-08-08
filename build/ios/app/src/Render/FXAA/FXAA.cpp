#include "FXAA.h"

#include <array>

#include "Frame/Helper.h"

#include "Engine/Object/Root.h"
#include "Engine/Physics/QueryObject.h"
#include "Engine/Render/RenderStage/RenderStageManager.h"
#include "Engine/Render/RenderStage/PostProcessRenderStage.h"


namespace Examples
{
	FXAA::FXAA()
		: m_camera(NULL)
		, m_lastCameraPosition(LORD::Vector3::ZERO)
		, m_lastCameraDirection(LORD::Vector3::ZERO)
		, m_lastNearClip(1.f)
		, m_lastFarClip(2000.f)
		, m_lastFov(15.f)
		, m_xPos(0)
		, m_postRenderStage(NULL)
		, m_Sphere(NULL)
		, m_SphereNode(NULL)
		, m_horizonAngle(0.f)
		, m_verticleAngle(0.f)
		, m_autoSpin(true)
	{
		m_Sphere = LordDebugDisplayManager->createQueryObject("WireframeSphere");
		m_Sphere->setVisible(true);
		m_Sphere->setQueryFlag(0x00000001);
		m_Sphere->setTestDepth(true);
		m_Sphere->setWriteDepth(true);

		m_SphereNode = LordSceneManager->getRootNode()->createChild();
	}

	FXAA::~FXAA()
	{
		if (m_Sphere)
		{
			LordDebugDisplayManager->destroyQueryObject(m_Sphere);
			m_Sphere = NULL;
		}

		if (m_SphereNode)
		{
			LordSceneManager->destroySceneNodes(&m_SphereNode, 1); 
			m_SphereNode = NULL; 
		}
	}

	const LORD::String FXAA::getHelp()
	{
		return LORD::String("LORD::FXAA"); 
	}

	bool FXAA::initialize()
	{
#ifdef LORD_DEBUG
		m_postRenderStage =
			LORD_DOWN_CAST<LORD::PostProcessRenderStage*>(LORD::RenderStageManager::instance()->getRenderStageByID(LORD::RSI_PostProcess));
#else
		m_postRenderStage =
			static_cast<LORD::PostProcessRenderStage*>(LORD::RenderStageManager::instance()->getRenderStageByID(LORD::RSI_PostProcess));
#endif

		LordAssertX(m_postRenderStage, "[%s][%s]postRenderStage empty.", __FILE__, __LINE__);

		m_postRenderStage->setImageEffectEnable("FXAA", true);

		m_camera = LordSceneManager->getMainCamera();

		// save global camera config.
		m_lastCameraPosition  = m_camera->getPosition(); 
		m_lastCameraDirection = m_camera->getDirection(); 
		m_lastNearClip        = m_camera->getNearClip(); 
		m_lastFarClip         = m_camera->getFarClip(); 
		m_lastFov             = m_camera->getFov(); 

		initMainCameraSettings(); 

		initSphereWireFrame(); 

		initGeometry(); 

		return true; 
	}

	bool FXAA::initMainCameraSettings()
	{
		m_cameraPosition = LORD::Vector3(0.f, 28.f, 28.f);
		m_cameraLookAt   = LORD::Vector3::ZERO; 

		m_camera->setPosition(m_cameraPosition); 
		m_camera->setDirection(m_cameraLookAt-m_cameraPosition); 
		m_camera->setNearClip(1.f);
		m_camera->setFarClip(2000.f);
		m_camera->setFov(sin(45.f / 180.f * LORD::Math::PI)); 

		m_camera->getDirection().toHVAngle(m_horizonAngle, m_verticleAngle);

		return true; 
	}

	bool FXAA::initSphereWireFrame()
	{
		static const LORD::i32 lineNumber = 40; 
		static const LORD::i32 radius	  = 10;

		float drho   = LORD::Math::PI / lineNumber; 
		float dtheta = 2.0f * LORD::Math::PI / lineNumber; 

		m_Sphere->beginPolygon(LORD::QueryObject::QO_WIREFRAME); 

		std::array<LORD::Vector3, lineNumber * lineNumber * 2> vertexData; 

		// whole sphere
		for (int i = 0; i < lineNumber; i++)
		{
			float rho = i * drho; 

			// single line
			for (int j = 0; j < lineNumber; j+= 2)
			{
				float theta = j == lineNumber ? 0.f : j * dtheta; 
				vertexData[i * lineNumber + j] = LORD::Vector3( -sin(theta) * sin(rho) * radius, 
																-cos(rho) * radius, 
																cos(theta) * sin(rho) * radius); 
				vertexData[i * lineNumber + j + 1] = LORD::Vector3(-sin(theta) * sin(rho + drho)*radius,
																	-cos(rho + drho)*radius, 
																	cos(theta) * sin(rho + drho)*radius);
			}
		}


		for (size_t i = 0; i < vertexData.size(); i++)
		{
			m_Sphere->setPosition(vertexData[i]);
			m_Sphere->setColor(LORD::Color(0.f, 4.f / 255.f, 96.f / 255.f));
		}

		m_Sphere->endPolygon();

		m_SphereNode->setLocalScaling(LORD::Vector3(1,1,1));
		m_SphereNode->setWorldPosition(LORD::Vector3::ZERO);
		m_Sphere->attachTo(m_SphereNode);

		// ±³¾°É«
		LORD::Renderer::BGCOLOR = LORD::Color::GRAY;
		return true; 
	}

	bool FXAA::initGeometry()
	{
		return true; 
	}

	void FXAA::destroy()
	{
		m_postRenderStage->setImageEffectEnable("FXAA", false);

		m_camera->setPosition(m_lastCameraPosition); 
		m_camera->setDirection(m_lastCameraDirection); 
		m_camera->setNearClip(m_lastNearClip); 
		m_camera->setFarClip(m_lastFarClip); 
		m_camera->setFov(m_lastFov); 
		m_camera = NULL;
	}

	void FXAA::tick(LORD::ui32 elapsedTime)
	{
		if (m_autoSpin)
		{
			rotateCamera(elapsedTime * 0.0005f, 0.f);
		}
	}

	void FXAA::keyboardProc(LORD::ui32 keyChar, bool isKeyDown)
	{
		// 65688783
		LORD::Vector3 position = m_camera->getPosition(); 
		LORD::Vector3 direction = m_camera->getDirection(); 
		LORD::Real far_Clip = m_camera->getFarClip(); 
		LORD::Real near_Clip = m_camera->getNearClip(); 

		switch (keyChar)
		{
		case 65: 
			position += LORD::Vector3(-1.0f, 0.0f, -1.0f);
			break; 
		case 68:
			position += LORD::Vector3(1.0f, 0.0f, 1.0f);
			break; 
		case 87: 
			position += LORD::Vector3(1.0f, 0.0f, -1.0f);
			break; 
		case 83:
			position += LORD::Vector3(-1.0f, 0.0f, 1.0f);
			break; 
		case 69:
			position += LORD::Vector3(0.0f, 1.0f, 0.0f); 
			break; 
		case 81:
			position += LORD::Vector3(0.0f, -1.0f, 0.0f); 
			break; 
		case 67: 
			direction += LORD::Vector3(-0.010f, 0.0f, -0.01f);
			break; 
		case 90:
			direction += LORD::Vector3(0.010f, 0.0f, 0.01f);
			break; 
		case 70:
			m_postRenderStage->setImageEffectEnable("FXAA", true);
			break; 
		case 71:
			m_postRenderStage->setImageEffectEnable("FXAA", false);
			break; 
		default:
			break;
		}

		m_camera->setPosition(position); 
		m_camera->setDirection(direction);
		m_camera->setFarClip(far_Clip);
		m_camera->setNearClip(near_Clip); 
		m_camera->update();
	}

	void FXAA::mouseLBProc(LORD::i32 xpos, LORD::i32 ypos)
	{
		m_autoSpin = !m_autoSpin;
		m_xPos = xpos; 
	}

	void FXAA::mouseMoveProc(LORD::i32 xpos, LORD::i32 ypos)
	{
		LORD::i32 delta = xpos - m_xPos; 
		m_xPos = xpos;

		rotateCamera(delta * 0.01f, 0.f);
	}

	void FXAA::mouseWhellProc(LORD::i32 param)
	{
		// m_cameraLookAt -= m_cameraForward * param;
	}

	bool FXAA::rotateCamera(float xValue, float yValue)
	{
		if (!xValue && !yValue)
			return false;

		m_horizonAngle += xValue;
		m_verticleAngle += yValue;

		m_verticleAngle = (std::min)(m_verticleAngle, LORD::Math::PI - 0.01f);
		m_verticleAngle = (std::max)(m_verticleAngle, 0.01f);

		m_SphereNode->setLocalOrientation(LORD::Quaternion(LORD::Vector3::UNIT_Y, m_horizonAngle)); 
		return true; 
	}
}