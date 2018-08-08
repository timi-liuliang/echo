#pragma once

#include "Frame/Example.h"

namespace Examples
{
	/**
	 * 多线程示例
	 */
	class MultiThread : public Example
	{
		MultiThread(const MultiThread& rhs);
		MultiThread& operator=(const MultiThread& rhs);

	public: 
		MultiThread();
		virtual ~MultiThread();

		// 帮助信息
		virtual const LORD::String getHelp() override;

		// 初始化
		virtual bool initialize() override;

		// 更新
		virtual void tick(LORD::ui32 elapsedTime) override;

		// 销毁
		virtual void destroy() override;

		// 键盘事件（处理FXAA的开启&&关闭）
		virtual void keyboardProc(LORD::ui32 keyChar, bool isKeyDown) override;

		// 鼠标事件
		virtual void mouseLBProc(LORD::i32 xpos, LORD::i32 ypos) override;

		// 鼠标事件
		virtual void mouseMoveProc(LORD::i32 xpos, LORD::i32 ypos) override;

		// 滚轮事件
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
		LORD::Vector3 m_cameraPosition; 
		LORD::Vector3 m_cameraLookAt; 
		LORD::Vector3 m_cameraForward; 
		float		  m_cameraRadius; 
		float		  m_horizonAngle;  // 水平旋转角度(顺时针)
		float		  m_verticleAngle; // 垂直旋转角度(顺时针)

		LORD::i32 m_xPos; 

		LORD::DebugDisplayItem* m_Sphere; 
		LORD::SceneNode*		m_SphereNode; 

		bool m_autoSpin; 
	};
}