#include "MultiThread.h"
#include <array>
#include "Frame/Helper.h"
#include <Foundation/Thread/Pool/CpuThreadPool.h>
#include "Engine/Object/Root.h"
#include "Engine/Physics/QueryObject.h"
#include "Engine/Render/RenderStage/RenderStageManager.h"
#include "Engine/Render/RenderStage/PostProcessRenderStage.h"

namespace Examples
{
	// 构造函数
	MultiThread::MultiThread() : m_camera(NULL), m_xPos(0), m_Sphere(NULL)
        , m_horizonAngle(0.f)
        , m_verticleAngle(0.f)
        , m_autoSpin(true)
	{

	}

	// 析构函数
	MultiThread::~MultiThread()
	{

	}

	// 获取帮助信息
	const LORD::String MultiThread::getHelp()
	{
		return LORD::String("LORD::MultiThread"); 
	}

	class JobTest : public LORD::CpuThreadPool::Job
	{
		// 处理任务
		virtual bool process()
		{
			float result;
			for (int i = 0; i < 1000; i++)
			{
				for (int j = 0; j < 1000; j++)
				{
					for (int k = 0; k < 100; k++)
					{
						result = 5.786f * 2.987f * 223.1f * i * j * k;
					}
				}
			}

			return true;
		}

		// 获取任务类型
		virtual int getType()
		{
			return 3;
		}

		// 任务完成响应
		virtual bool finished(){ return true; }
	};

	// 初始化
	bool MultiThread::initialize()
	{
#ifdef LORD_PLATFORM_WINDOWS
		DWORD _time0 = GetTickCount();

		scl::semaphore testSemaphore;
		testSemaphore.create();

		for (int i = 0; i < 100000; i++)
			testSemaphore.post();

		for (int i = 0; i < 100000; i++)
			testSemaphore.wait();

		DWORD _time1 = GetTickCount();

		DWORD x111 = _time1 - _time0;

		/**
		 * 多线程测试
		 */
		//Sleep(10000);

		std::vector<LORD::CpuThreadPool::Job*>	m_jobs1;
		std::vector<LORD::CpuThreadPool::Job*>	m_jobs2;
		for (int i = 0; i < 16; i++)
		{
			m_jobs1.push_back(new JobTest());
			m_jobs2.push_back(new JobTest());
		}

		// 线程池配置
		LORD::CpuThreadPool::Cinfo info;
		info.m_numThreads = 8;
		info.m_isBlocking = true;

		DWORD time0 = GetTickCount();

		// 创建线程池
		LORD::CpuThreadPool* threadPool = LordNew( LORD::CpuThreadPool( info));
		threadPool->processJobs( m_jobs1.data(), m_jobs1.size());

		DWORD time1 = GetTickCount();

 		for (auto job : m_jobs2)
 		{
			job->process();
 		}

		DWORD time2 = GetTickCount();

		DWORD x1 = time1 - time0;
		DWORD x2 = time2 - time1;

		float xr = (float)x2 / (float)x1;

		LordLogError( "----------------------------------------------------------%f", xr);

		Sleep( 10000);
#endif
		m_camera = LordSceneManager->getMainCamera();
		m_camera->setNearClip(1.f);
		m_camera->setFarClip(2000.f);

		initMainCameraSettings(); 

		initSphereWireFrame(); 

		initGeometry(); 

		return true; 
	}

	bool MultiThread::initMainCameraSettings()
	{
		m_cameraPosition = LORD::Vector3(0.f, 28.f, 28.f);
		m_cameraLookAt = LORD::Vector3::ZERO;
		m_cameraForward = m_cameraLookAt - m_cameraPosition;
		m_cameraForward.normalize();
		m_cameraPosition += m_cameraForward * 0.0f;
		m_cameraRadius = 0.15f;
		m_cameraLookAt = m_cameraPosition + m_cameraForward * m_cameraRadius;
		m_cameraForward.toHVAngle(m_horizonAngle, m_verticleAngle);

		return true; 
	}

	bool MultiThread::initSphereWireFrame()
	{
		static const LORD::i32 lineNumber = 40; 
		static const LORD::i32 radius	  = 10;

		float drho   = LORD::Math::PI / lineNumber; 
		float dtheta = 2.0f * LORD::Math::PI / lineNumber; 

		if (m_Sphere)
		{
			LordDebugDisplayManager->destroyQueryObject(m_Sphere); 
			m_Sphere = NULL; 
		}

		m_Sphere = LordDebugDisplayManager->createQueryObject("WireframeSphere"); 
		m_Sphere->setVisible(true);
		m_Sphere->setQueryFlag(0x00000001); 
		m_Sphere->setTestDepth(true); 
		m_Sphere->setWriteDepth(true); 

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
			m_Sphere->setColor(LORD::Color::BLACK);
		}

		m_Sphere->endPolygon();

		if (!m_SphereNode)
		{
			m_SphereNode = LordSceneManager->getRootNode()->createChild();
		}

		m_SphereNode->setLocalScaling(LORD::Vector3(5,5,5)); 
		m_SphereNode->setWorldPosition(LORD::Vector3::ZERO);
		m_Sphere->attachTo(m_SphereNode);

		// 背景色
		LORD::Renderer::BGCOLOR = LORD::Color::WHITE;
		return true; 
	}

	bool MultiThread::initGeometry()
	{
		return true; 
	}

	void MultiThread::destroy()
	{
		LordSceneManager->getMainCamera()->setFov(LORD::Math::PI_DIV4);
		m_camera = NULL;
	}

	void MultiThread::tick(LORD::ui32 elapsedTime)
	{
		if (m_autoSpin)
		{
			rotateCamera(elapsedTime * 0.0005f, 0.f);
		}
	}

	void MultiThread::keyboardProc(LORD::ui32 keyChar, bool isKeyDown)
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
		default:
			break;
		}

		m_camera->setPosition(position); 
		m_camera->setDirection(direction);
		m_camera->setFarClip(far_Clip);
		m_camera->setNearClip(near_Clip); 
		m_camera->update();
	}

	void MultiThread::mouseLBProc(LORD::i32 xpos, LORD::i32 ypos)
	{
		m_autoSpin = !m_autoSpin;
		m_xPos = xpos; 
	}

	void MultiThread::mouseMoveProc(LORD::i32 xpos, LORD::i32 ypos)
	{
		LORD::i32 delta = xpos - m_xPos; 
		m_xPos = xpos;

		rotateCamera(delta * 0.01f, 0.f);
	}

	void MultiThread::mouseWhellProc(LORD::i32 param)
	{
		// m_cameraLookAt -= m_cameraForward * param;
	}

	bool MultiThread::rotateCamera(float xValue, float yValue)
	{
		if (!xValue && !yValue)
			return false;

		m_horizonAngle += xValue;
		m_verticleAngle += yValue;

		m_verticleAngle = (std::min)(m_verticleAngle, LORD::Math::PI - 0.01f);
		m_verticleAngle = (std::max)(m_verticleAngle, 0.01f);

		m_cameraForward.fromHVAngle(m_horizonAngle, m_verticleAngle);

		m_SphereNode->setLocalOrientation(LORD::Quaternion(LORD::Vector3::UNIT_Y, m_horizonAngle)); 
		return true; 
	}
}