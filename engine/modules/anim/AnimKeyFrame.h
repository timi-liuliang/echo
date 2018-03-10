#ifndef __ECHO_ANIMKEYFRAME_H__
#define __ECHO_ANIMKEYFRAME_H__

namespace Echo
{
	class Animation;


	class AnimKeyFrame
	{
	public:
		AnimKeyFrame(Animation* pAnim, ui32 time);
		virtual ~AnimKeyFrame();

		virtual void createAnimData(Dword boneCount);
		virtual void setAnimData(Dword handle, const Quaternion& ori, const Vector4& pos);

		// 设置动画数据
		virtual void setAnimDatammap(ui32 boundCount, const char* dataPtr);

		virtual Animation* getAnim() const;

		virtual const Quaternion& getAnimQuaternion(ui32 idx) const;
		virtual const Vector4& getAnimTranslate(ui32 idx) const;

	protected:
		Animation*					m_pAnim;			// 对应动画数据
		vector<Quaternion>::type	m_KeyOrientation;	// 旋转数据	
		vector<Vector4>::type		m_KeyPosition;		// 位移数据

#ifdef LIULIANG_ANIMATION_MMAP
		struct KeyData
		{
			Vector4	 pos;
			Quaternion	orientation;
		};

		ui32						m_boneCount;		// 骨骼数量
		const KeyData*				m_data;
#endif
	};
}

#endif