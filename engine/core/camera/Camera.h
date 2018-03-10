#pragma once

#include "engine/core/Math/Quaternion.h"
#include "engine/core/geom/Frustum.h"
#include "engine/core/Util/Array.hpp"

#define ECHO_CAMERA_SUPPORT_SCREENOFFSET

namespace Echo
{
	static const size_t MAX_RENDER_LAYER = 20;
	/**
	 * 摄像机接口
	 */
	class Ray;
	class Camera
 	{
	public:
		enum ProjMode
		{
			PM_PERSPECTIVE,
			PM_ORTHO,
			PM_UI,				// UI专用
		};

	public:	
		Camera(ProjMode mode = PM_PERSPECTIVE, bool isFixedYaw = true );
		virtual ~Camera();

		// vr模式下，判断是左眼还是右眼
		void setVRModeForEye(bool isleftEye){ m_bNeedUpdateProj = true; m_bNeedUpdateView = true; m_vrModeForEye = isleftEye; update(); }

		// 更新
		virtual void frameMove( float elapsedTime){}

		virtual void setPosition(const Vector3& pos);
		virtual void setDirection(const Vector3& dir);
		virtual void setUp(const Vector3& vUp);

		virtual void yaw(Real rad);
		virtual void pitch(Real rad);
		virtual void roll(Real rad);
		virtual void rotate(const Vector3& vAxis, Real rad);
		virtual void move(const Vector3& offset);

		// 获取摄像机位置
		const Vector3& getPosition() const { return m_position; }

		// 获取摄像机位置指针
		Vector3* getPositionPtr() { return &m_position; }

		// 获取朝向
		const Quaternion& getOritation() const { return m_oritation; }

		// 获取朝向
		const Vector3& getDirection() const { return m_dir; }

		// 获取右方向 
		const Vector3& getRight() const { return m_right; }

		// 获取上方向
		const Vector3& getUp() const { return m_up; }

		// 获取观察矩阵
		const Matrix4& getViewMatrix() const { return m_matView; }
		
		// 获取投影矩阵
		const Matrix4& getProjMatrix() const { return m_matProj; }

		// 获取观察投影矩阵
		const Matrix4& getViewProjMatrix() const { return m_matVP; }

		//获得镜面反射观察矩阵
		const Matrix4& getMirrViewMatric() const { return m_matMirrView; }

		// 获取天空盒观察投影矩阵
		const Matrix4& getSkyViewProj() const { return m_matSkyViewProj; }

		//变到mirrcamera
		void toMirrCamera(const Vector4& clipPlane);

		//回到original
		void toOriginalCamera();

		void calculateObliquematrixOrtho(Matrix4& projection, const Vector4& clipPlane);


		void setWaterHeight(float water_height) { m_water_height = water_height; }

		void			setProjectionMode(ProjMode mode);
		ProjMode		getProjectionMode() const;

		void			setFixedYawAxis(bool fixed){ m_bFixedYawAxis = fixed; }
		void			setFov(Real fov);
		void			setWidth(Real width);
		void			setHeight(Real height);
		void			setNearClip(Real nearClip);
		void			setFarClip(Real farClip);
		Real			getFov() const;
		Real			getWidth() const;
		Real			getHeight() const;
		Real			getNearClip() const;
		Real			getFarClip() const;
		const Frustum&	getFrustum() const;
		Real			getNearClipWidth() const;
		Real			getNearClipHeight() const;

		void			getCameraRay(Ray& ray, const Vector2& screenPos);

		void			unProjectionMousePos( Vector3& from, Vector3& to, const Vector2& screenPos );

		void			setRenderLayers(const array<float, MAX_RENDER_LAYER>& layers);
		void			setIsRenderLayers(const array<bool, MAX_RENDER_LAYER>& layers);
		bool			checkNeedRender(const Vector3& pos, const ui32& layer, const ui32& renderLayer);

		virtual void	update();

		virtual void    needUpdate();

#ifdef ECHO_CAMERA_SUPPORT_SCREENOFFSET
		void			setScreenOffset( float xOffset, float yOffset);
#endif
		void			setOrthoZRadian(float rad){ m_orthoZRadian = rad; }
		float			getOrthoZRadian(){ return m_orthoZRadian; }

	public:
		// 克隆
		virtual void clone(Camera* other);

		// 加载
		void load(const String& name);

		// 保存
		void save(const String& pathName);

	protected:
		Vector3			m_position;
		Vector3			m_dir;
		Quaternion		m_oritation;
		Vector3			m_up;
		Vector3			m_right;
		bool			m_bFixedYawAxis;
		Vector3			m_fixedYawAxis;
		Matrix4			m_matView;
		Matrix4			m_matMirrView;
		Matrix4			m_matSkyView;
		Matrix4			m_matSkyProj;
		Matrix4			m_matSkyViewProj;
		bool			m_bNeedUpdateView;
		ProjMode		m_projMode;
		Real			m_fov;
		Real			m_width;
		Real			m_height;
		Real			m_aspect;
		Real			m_nearClip;
		Real			m_farClip;
		Matrix4			m_matProj;
		bool			m_bNeedUpdateProj;
		Matrix4			m_matVP;
		Frustum			m_frustum;
		Matrix4			m_originalViewProjMatrix;

#ifdef ECHO_CAMERA_SUPPORT_SCREENOFFSET
		Matrix4			m_matScreenOffset;		// 屏幕空间偏移矩阵
#endif

		bool			m_vrModeForEye;				// vr模式相机需要偏移

		float			m_water_height;			//水面高度，根据这个值来计算，水面反射相机的位置
		array<float, MAX_RENDER_LAYER> m_renderLayers;      // 渲染层级 按距离计算
		array<bool, MAX_RENDER_LAYER> m_isRenderLayers;      // 渲染层级 按标记

		float			m_orthoZRadian;			//ortho print screen z radian
	};

}