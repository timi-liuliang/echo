#pragma once

#include "engine/core/Math/MathLib.h"
#include "engine/core/render/render/Color.h"

namespace Echo
{

	//forward decaleration
	class EffectSystem;						//a system has some layers
	class EffectLayer;						//a layer has some keyframes,special layers like particleLayer has some ParticleControllerSets and a material
	class EffectKeyFrame;					//a keyframe has some controllers
	class EffectParticleControllerSet;		//
	class EffectController;					//
	class EffectMaterial;					//
	class EffectParticle;
	class EffectRenderalbe;
	class DataStream;

	enum EFFECT_RENDER_LEVEL
	{
		ERL_Low	=0 ,
		ERL_Normal,
		ERL_High,
		ERL_Super,
	};

	enum EFFECT_RENDER_GROUP
	{
		ERG_GROUPUP = 0,
		ERG_GROUPDOWN,
	};

	enum EFFECT_BLEND_MODE
	{
		AlphaLightMode = 0,
		HighLightMode,
		DistortionMode
	};

	enum EFFECT_DISTORTION_TYPE
	{
		DISTORTION_LINEAR = 0,
		DISTORTION_CENTER
	};

	enum EFFECT_LAYER_TYPE
	{
		ELT_2DBillboard = 1,
		ELT_3DBillboard,
		ELT_TRAIL,
		ELT_Column,
		ELT_Paraboloidal,
		ELT_ParticlesPoint,
		ELT_ParticlesCube,
		ELT_ParticlesSphere,
		ELT_ParticlesColumn,
		ELT_ParticlesCruve,
		ELT_Model,
		ELT_Lighting,
		ELT_2DEditableSurface,
		ELT_3DEditableSurface,
		ELT_Skin,
		ELT_SpriteAnimation,
		ELT_Child,
		ELT_MT_Model,
		ELT_Unknow = 99,
	};

	enum EFFECT_CONTROL_TYPE
	{
		ECT_Translation = 1,
		ECT_RotateSelf,
		ECT_RotateAxis,
		ECT_RotateRevolution,
		ECT_Offcenter,
		ECT_ColorChange,
		ECT_ScaleChange,
		ECT_ScaleNoise,
		ECT_ColorNoise,
		ECT_ColorSet,
		ECT_ScaleSet,
		ECT_UVSpeedSet,
		ECT_Unknow = 99,
	};

	struct EffectVertexFmt
	{
		Vector3 pos;
		ui32 color;
		float tx, ty;
		float tx1, ty1;
	};

	struct EffectVertexFmt2
	{
		Vector3 pos;
		float tx, ty;
	};

	typedef map<String, String>::type NameValuePairList;
}