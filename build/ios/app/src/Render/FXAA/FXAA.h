//
//
//
//
//
//
//

#pragma once

#include "Frame/Example.h"

namespace LORD
{
	class PostProcessRenderStage;
}

namespace Examples
{
	class FXAA : public Example
	{
		FXAA(const FXAA& rhs); 
		FXAA& operator=(const FXAA& rhs); 

	public: 
		FXAA(); 
		virtual ~FXAA(); 

	public: 
		// 帮助信息
		virtual const LORD::String getHelp();

		// 初始化
		virtual bool initialize();

		// 更新
		virtual void tick(LORD::ui32 elapsedTime);

		// 销毁
		virtual void destroy();

		// 键盘事件（处理FXAA的开启&&关闭）
		virtual void keyboardProc(LORD::ui32 keyChar, bool isKeyDown) override;

		// 鼠标事件
		virtual void mouseLBProc(LORD::i32 xpos, LORD::i32 ypos) override;
		virtual void mouseMoveProc(LORD::i32 xpos, LORD::i32 ypos) override;
		virtual void mouseWhellProc(LORD::i32 parma) override; 

	private: 
		// 
		bool initMainCameraSettings(); 

		//
		bool initSphereWireFrame(); 

		//
		bool initGeometry(); 

		//
		bool rotateCamera(float deltaX, float deltaY); 

	private:
		LORD::Camera* m_camera;        // 主摄像机

		LORD::Vector3	 m_lastCameraPosition; 
		LORD::Vector3	 m_lastCameraDirection; 
		float			 m_lastNearClip; 
		float			 m_lastFarClip; 
		float			 m_lastFov; 

		LORD::Vector3 m_cameraPosition; 
		LORD::Vector3 m_cameraLookAt; 

		float		  m_horizonAngle;  // 水平旋转角度(顺时针)
		float		  m_verticleAngle; // 垂直旋转角度(顺时针)

		LORD::i32 m_xPos; 

		LORD::PostProcessRenderStage* m_postRenderStage; 

		LORD::DebugDisplayItem* m_Sphere; 
		LORD::SceneNode*		m_SphereNode; 

		bool m_autoSpin; 
	};
}