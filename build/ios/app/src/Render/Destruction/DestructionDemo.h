#ifndef __DESTRUCTION_DEMO_H__
#define __DESTRUCTION_DEMO_H__

#include "Frame/Example.h"

namespace Examples
{
	class DestructionDemo : public Example
	{
	public:
		DestructionDemo( );
		virtual ~DestructionDemo( ) {}

		// 帮助信息
		virtual const LORD::String getHelp( );

		// 初始化
		virtual bool initialize( );

		// 更新
		virtual void tick( LORD::ui32 elapsedTime );

		// 销毁
		virtual void destroy( );

		virtual void keyboardProc( LORD::ui32 keyChar, bool isKeyDown );
		virtual	void mouseWhellProc( LORD::i32 parma );
		virtual void mouseProc( LORD::i32 xpos, LORD::i32 ypos );
		virtual void mouseLBProc( LORD::i32 xpos, LORD::i32 ypos );
		virtual void mouseMoveProc( LORD::i32 xpos, LORD::i32 ypos );

//		void OnCollideHit( const hknpEventHandlerInput& input, const hknpEvent& event );

	private:
		void	updateCamera( const LORD::ui32 elapse );
		void	cameraZoom( const LORD::Box& box, float scale );
		void	cameraZoom( float zValue );
		void	setCameraMoveDir( const LORD::Vector3& dir );
		void	rotationCamera( float xValue, float yValue );
		void	addScreenOffset( float xOffset, float yOffset );

		void	throwBox( );
	private:
		LORD::Scene*	m_scene;

		LORD::Camera*		m_camera;			// 主摄像机
		float				m_cameraRadius;		// 摄像机观察半径
		LORD::Vector3		m_cameraLookAt;		// 摄像机观察点
		LORD::Vector3		m_cameraMoveDir;	// 摄像机移动
		LORD::Vector3		m_cameraForward;	// 朝向
		LORD::Vector3		m_cameraPositon;	// 摄像机位置
		float				m_horizonAngle;		// 水平旋转角度(顺时针)
		float				m_verticleAngle;	// 垂直旋转角度(顺时针)
		float				m_xOffset;			// X方向偏移
		float				m_yOffset;			// Y方向偏移
		LORD::Vector2		m_mousePos;

		LORD::QueryObject*	m_box;
		LORD::SceneNode*	m_boxNode;
		LORD::QueryObject*	m_ground;
		LORD::SceneNode*	m_groundNode;

	};
}

#endif
