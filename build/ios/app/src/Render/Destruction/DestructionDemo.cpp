#include "DestructionDemo.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Physics/QueryObject.h"
#include "Engine/Render/RenderStage/RenderStage.h"
#include "Engine/Render/RenderStage/ShadowMapRenderStage.h"
#include "Engine/Render/RenderStage/RenderStageManager.h"
// #include "Extension/Actor/ActorObject.h"
// #include "Extension/Actor/ActorManager.h"

namespace Examples
{
	static const float kCameraRotationYScalar = 0.1f;
	static const float kCameraRotationZScalar = 0.3f;
	static const float kCameraRadiusScalar = 0.005f;

	DestructionDemo::DestructionDemo( )
		: Example()
		, m_cameraRadius( 0.15f )
		, m_cameraMoveDir( LORD::Vector3::ZERO )
		, m_xOffset( 0.f )
		, m_yOffset( 0.f )
		, m_box( NULL )
		, m_boxNode( NULL )
	{

		m_cameraPositon = LORD::Vector3( 0.f, 18.f, 18.f );
		m_cameraLookAt = LORD::Vector3::ZERO;
		m_cameraForward = m_cameraLookAt - m_cameraPositon;
		m_cameraForward.normalize( );
		m_cameraRadius = 0.15f;
		m_cameraLookAt = m_cameraPositon + m_cameraForward * m_cameraRadius;

		m_cameraForward.toHVAngle(m_horizonAngle, m_verticleAngle);
#if defined(_MSC_VER)
		ShowCursor( false );
#endif
		// 场景数据
 		m_scene = LordSceneManager->createScene( "DestructionDemo", false, 8, 8, 2, 2, true );
 		m_scene->setIsRenderTerrain( false );
 		m_scene->setActorAmbientColor( LORD::Color::GRAY );
 		m_scene->setActorLightColor( LORD::Color::WHITE );
 		m_scene->setLightColor( LORD::Color::WHITE );
 		LORD::Vector3 lightDir( 1.f, 1.f, 1.f );
 		lightDir.normalize( );
 		m_scene->setLightDirection( lightDir );
//		m_scene = LordSceneManager->loadScene( "tpsart_aijifuben03" );
		// 默认场景数据

		m_camera = LordSceneManager->getMainCamera( );

// 		LORD::Box box;
// 		m_scene->buildAABB( box );
// 		cameraZoom( box, 1.f );

		m_ground = LordDebugDisplayManager->createQueryObject( "Ground" );
		m_ground->setVisible(true);
		LORD::Color color( LORD::Color::GRAY );
		LORD::real32 r = 30.f;
		LORD::real32 h = 0.2f;
		{
			m_ground->setTestDepth( true );
			m_ground->setWriteDepth( true );
			m_ground->setUseNormal( true );
			m_ground->beginPolygon( LORD::QueryObject::QO_SOLIDQUAD );
			m_ground->setPosition( -r, -h, -r );
			m_ground->setColor( color.r, color.g, color.b, color.a );
			m_ground->setPosition( r, -h, -r );
			m_ground->setColor( color.r, color.g, color.b, color.a );
			m_ground->setPosition( r, -h, r );
			m_ground->setColor( color.r, color.g, color.b, color.a );
			m_ground->setPosition( -r, -h, r );
			m_ground->setColor( color.r, color.g, color.b, color.a );
			m_ground->setPosition( -r, h, -r );
			m_ground->setColor( color.r, color.g, color.b, color.a );
			m_ground->setPosition( r, h, -r );
			m_ground->setColor( color.r, color.g, color.b, color.a );
			m_ground->setPosition( r, h, r );
			m_ground->setColor( color.r, color.g, color.b, color.a );
			m_ground->setPosition( -r, h, r );
			m_ground->setColor( color.r, color.g, color.b, color.a );

			m_ground->setQuad( 0, 1, 2, 3 );
			m_ground->setQuad( 4, 5, 6, 7 );
			m_ground->setQuad( 1, 2, 6, 5 );
			m_ground->setQuad( 0, 3, 7, 4 );
			m_ground->setQuad( 4, 5, 1, 0 );
			m_ground->setQuad( 2, 3, 7, 6 );

			m_ground->endPolygon( );
		}

		m_groundNode = LordSceneManager->getRootNode( )->createChild( );
		m_ground->attachTo( m_groundNode );

		m_groundNode->setLocalPosition( LORD::Vector3( 0.f, -0.4f, 0.f) );
	}

	const LORD::String DestructionDemo::getHelp( )
	{
		return "Destruction Demo";
	}

	bool DestructionDemo::initialize( )
	{
		LORD::ShadowMapRenderStage* pStage = (LORD::ShadowMapRenderStage*)LORD::RenderStageManager::instance()->getRenderStageByID(LORD::RSI_ShadowMap);
		pStage->setShadowShade( 0.8f );
		return true;
	}

	void DestructionDemo::tick( LORD::ui32 elapsedTime )
	{
		updateCamera( elapsedTime );
	}

	void DestructionDemo::destroy( )
	{
		LordDebugDisplayManager->destroyAllQueryObject( );
		LORD::SceneNode* rootNode = LordSceneManager->getRootNode( );
		if ( m_boxNode )
		{
			rootNode->destroyChild( m_boxNode );
			m_box = NULL;
			m_boxNode = NULL;
		}
		rootNode->destroyChild( m_groundNode );

		m_ground = NULL;
		m_groundNode = NULL;

		LordSceneManager->closeScene( );
		m_scene = NULL;
		
#if defined(_MSC_VER)
		ShowCursor( true );
#endif
//		LORD::ActorManager::Instance()->FreeActors();
	}

	void DestructionDemo::keyboardProc( LORD::ui32 keyChar, bool isKeyDown )
	{
		if (isKeyDown)
		{
			LORD::Vector3 cameraMoveDir = LORD::Vector3::ZERO;
			switch (keyChar)
			{
				// W
			case 87:
				cameraMoveDir.z += 10.f;
				break;
				// A
			case 65:
				cameraMoveDir.x += 10.f;
				break;
				// S
			case 83:
				cameraMoveDir.z -= 10.f;
				break;
				// D
			case 68:
				cameraMoveDir.x -= 10.f;
				break;
			}
			setCameraMoveDir( cameraMoveDir );
		}
		else
		{
			setCameraMoveDir( LORD::Vector3::ZERO );
		}
	}

	void DestructionDemo::mouseWhellProc( LORD::i32 parma )
	{
		if ( parma > 0 )
		{
			cameraZoom( -1.f );
		}
		else
		{
			cameraZoom( 1.f );
		}
	}

	void DestructionDemo::mouseProc( LORD::i32 xpos, LORD::i32 ypos )
	{
		m_mousePos = LORD::Vector2( (LORD::Real)xpos, (LORD::Real)ypos );
	}

	void DestructionDemo::mouseLBProc( LORD::i32 xpos, LORD::i32 ypos )
	{
		throwBox( );
	}

	void DestructionDemo::mouseMoveProc( LORD::i32 xpos, LORD::i32 ypos )
	{
		LORD::Vector2 curMousePos( (LORD::Real)xpos, (LORD::Real)ypos );
		LORD::Vector2 deltaPos = curMousePos - m_mousePos;
		m_mousePos = curMousePos;
		rotationCamera( deltaPos.x * kCameraRotationYScalar * kCameraRotationYScalar, deltaPos.y * kCameraRadiusScalar );
	}

	void DestructionDemo::updateCamera( const LORD::ui32 elapse )
	{
		m_cameraLookAt += m_cameraMoveDir * (LORD::Real)elapse * 0.001f;

		m_cameraForward.normalize( );
		m_cameraPositon = m_cameraLookAt - m_cameraForward * m_cameraRadius;

		m_camera->setPosition( m_cameraPositon );
		m_camera->setDirection( m_cameraLookAt-m_cameraPositon );

		LordSceneManager->setMainPosition( m_cameraPositon );
	}

	void DestructionDemo::cameraZoom( const LORD::Box& box, float scale )
	{
		float radius = (box.getSize( ).len( ) * 0.5f);
		LORD::Vector3 center = ((box.vMin + box.vMax) * 0.5f);

		float fovW = m_camera->getFov( ) * m_camera->getWidth( ) / m_camera->getHeight( );
		float fov = std::max<float>( fovW, m_camera->getFov( ) );

		float cameraRadius = radius / std::sin( fov / 2.f ) / scale;

		m_cameraLookAt = center - LORD::Vector3( m_cameraForward.x, 0.f, m_cameraForward.z ) * radius * 0.3f;

		m_cameraPositon = m_cameraLookAt - m_cameraForward * cameraRadius;
		m_cameraLookAt = m_cameraPositon + m_cameraForward * m_cameraRadius;

		addScreenOffset( -m_xOffset, -m_yOffset );
	}

	void DestructionDemo::cameraZoom( float zValue )
	{
		m_cameraLookAt -= m_cameraForward * zValue;
	}

	void DestructionDemo::setCameraMoveDir( const LORD::Vector3& dir )
	{
		LORD::Vector3 forward = m_cameraForward; forward.y = 0.f;
		forward.normalize( );

		LORD::Vector3 right = forward.cross( LORD::Vector3::UNIT_Y );
		right.normalize( );

		m_cameraMoveDir = m_cameraForward * dir.z - right * dir.x;
		m_cameraMoveDir.normalize( );
		m_cameraMoveDir *= 10.f;
	}

	void DestructionDemo::rotationCamera( float xValue, float yValue )
	{
		if (!xValue && !yValue)
			return;

		m_horizonAngle += xValue;
		m_verticleAngle += yValue;

		m_verticleAngle = (std::min)( m_verticleAngle, LORD::Math::PI - 0.01f );
		m_verticleAngle = (std::max)( m_verticleAngle, 0.01f );

		m_cameraForward.fromHVAngle(m_horizonAngle, m_verticleAngle);
	}

	void DestructionDemo::addScreenOffset( float xOffset, float yOffset )
	{
		m_xOffset += xOffset;
		m_yOffset += yOffset;

		m_camera->setScreenOffset( m_xOffset * 2.f / m_camera->getWidth( ), m_yOffset * -2.f / m_camera->getHeight( ) );
	}

	void DestructionDemo::throwBox( )
	{

	}

//	Test Code
// 	void DestructionDemo::OnCollideHit( const hknpEventHandlerInput& input, const hknpEvent& event )
// 	{
// 		int a = 0;
// 	}
}

