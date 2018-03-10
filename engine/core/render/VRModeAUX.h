#pragma once

#include "engine/core/Math/EchoMathFunction.h"
#include "engine/core/Math/EchoMathDef.h"
#include "engine/core/Math/Vector2.h"
#include "engine/core/Math/Vector3.h"
#include "engine/core/Math/Matrix4.h"
#include "engine/core/render/render/Color.h"

namespace Echo
{


// VRMode 辅助结构体和函数

struct ScaleAndOffset2D
{
	Echo::Vector2 Scale;
	Echo::Vector2 Offset;

	ScaleAndOffset2D(float sx = 0.0f, float sy = 0.0f, float ox = 0.0f, float oy = 0.0f)
		: Scale(sx, sy), Offset(ox, oy)
	{ }
};


// ***** FovPort

// FovPort describes Field Of View (FOV) of a viewport.
// This class has values for up, down, left and right, stored in 
// tangent of the angle units to simplify calculations.
//
// As an example, for a standard 90 degree vertical FOV, we would 
// have: { UpTan = tan(90 degrees / 2), DownTan = tan(90 degrees / 2) }.
//
// CreateFromRadians/Degrees helper functions can be used to
// access FOV in different units.

struct FovPort
{
	float UpTan;
	float DownTan;
	float LeftTan;
	float RightTan;

	FovPort(float sideTan = 0.0f) :
		UpTan(sideTan), DownTan(sideTan), LeftTan(sideTan), RightTan(sideTan) { }
	FovPort(float u, float d, float l, float r) :
		UpTan(u), DownTan(d), LeftTan(l), RightTan(r) { }

	static FovPort CreateFromRadians(float horizontalFov, float verticalFov)
	{
		FovPort result;
		result.UpTan = Echo::Math::Tan(verticalFov * 0.5f);
		result.DownTan = Echo::Math::Tan(verticalFov * 0.5f);
		result.LeftTan = Echo::Math::Tan(horizontalFov * 0.5f);
		result.RightTan = Echo::Math::Tan(horizontalFov * 0.5f);
		return result;
	}

	static FovPort CreateFromDegrees(float horizontalFovDegrees,
		float verticalFovDegrees)
	{
		return CreateFromRadians(horizontalFovDegrees*Echo::Math::DEG2RAD,
			verticalFovDegrees*Echo::Math::DEG2RAD);
	}

	//  Get Horizontal/Vertical components of Fov in radians.
	float GetVerticalFovRadians() const     { return Echo::Math::ATan(UpTan) + Echo::Math::ATan(DownTan); }
	float GetHorizontalFovRadians() const   { return Echo::Math::ATan(LeftTan) + Echo::Math::ATan(RightTan); }
	//  Get Horizontal/Vertical components of Fov in degrees.
	float GetVerticalFovDegrees() const     { return Echo::Math::RAD2DEG*GetVerticalFovRadians(); }
	float GetHorizontalFovDegrees() const   { return Echo::Math::RAD2DEG*GetHorizontalFovRadians(); }

	// Compute maximum tangent value among all four sides.
	float GetMaxSideTan() const
	{
		return Echo::Math::Max(Echo::Math::Max(UpTan, DownTan), Echo::Math::Max(LeftTan, RightTan));
	}

	static ScaleAndOffset2D CreateNDCScaleAndOffsetFromFov(FovPort tanHalfFov)
	{
		float projXScale = 2.0f / (tanHalfFov.LeftTan + tanHalfFov.RightTan);
		float projXOffset = (tanHalfFov.LeftTan - tanHalfFov.RightTan) * projXScale * 0.5f;
		float projYScale = 2.0f / (tanHalfFov.UpTan + tanHalfFov.DownTan);
		float projYOffset = (tanHalfFov.UpTan - tanHalfFov.DownTan) * projYScale * 0.5f;

		ScaleAndOffset2D result;
		result.Scale = Echo::Vector2(projXScale, projYScale);
		result.Offset = Echo::Vector2(projXOffset, projYOffset);
		// Hey - why is that Y.Offset negated?
		// It's because a projection matrix transforms from world coords with Y=up,
		// whereas this is from NDC which is Y=down.

		return result;
	}

	// Converts Fov Tan angle units to [-1,1] render target NDC space
	Echo::Vector2 TanAngleToRendertargetNDC(Echo::Vector2 const &tanEyeAngle)
	{
		ScaleAndOffset2D eyeToSourceNDC = CreateNDCScaleAndOffsetFromFov(*this);
		return tanEyeAngle * eyeToSourceNDC.Scale + eyeToSourceNDC.Offset;
	}


	// Compute per-channel minimum and maximum of Fov.
	static FovPort Min(const FovPort& a, const FovPort& b)
	{
		FovPort fov(Echo::Math::Min(a.UpTan, b.UpTan),
			Echo::Math::Min(a.DownTan, b.DownTan),
			Echo::Math::Min(a.LeftTan, b.LeftTan),
			Echo::Math::Min(a.RightTan, b.RightTan));
		return fov;
	}

	static FovPort Max(const FovPort& a, const FovPort& b)
	{
		FovPort fov(Echo::Math::Max(a.UpTan, b.UpTan),
			Echo::Math::Max(a.DownTan, b.DownTan),
			Echo::Math::Max(a.LeftTan, b.LeftTan),
			Echo::Math::Max(a.RightTan, b.RightTan));
		return fov;
	}
};


//-------------------------------------------------------------------------------------
// Different lens distortion types supported by devices.
// 
enum DistortionEqnType
{
	Distortion_No_Override = -1,
	// These two are leagcy and deprecated.
	Distortion_Poly4 = 0,    // scale = (K0 + K1*r^2 + K2*r^4 + K3*r^6)
	Distortion_RecipPoly4 = 1,    // scale = 1/(K0 + K1*r^2 + K2*r^4 + K3*r^6)

	// CatmullRom10 is the preferred distortion format.
	Distortion_CatmullRom10 = 2,    // scale = Catmull-Rom spline through points (1.0, K[1]...K[9])

	Distortion_LAST                 // For ease of enumeration.
};
float DistortionFnScaleRadiusSquared(float rsq);
//-----------------------------------------------------------------------------------
// ***** LensConfig

// LensConfig describes the configuration of a single lens in an HMD.
// - Eqn and K[] describe a distortion function.
// - MetersPerTanAngleAtCenter is the relationship between distance on a
//   screen (at the center of the lens), and the angle variance of the light after it
//   has passed through the lens.
// - ChromaticAberration is an array of parameters for controlling
//   additional Red and Blue scaling in order to reduce chromatic aberration
//   caused by the Rift lenses.
struct LensConfig
{
	LensConfig()
		: Eqn(Distortion_CatmullRom10)
		//K()
		, MaxR(0.0f)
		, MetersPerTanAngleAtCenter(0.0f)
		//ChromaticAberration()
		//InvK()
		, MaxInvR(0.0f)
	{
		memset(&K, 0, sizeof(K));
		memset(&ChromaticAberration, 0, sizeof(ChromaticAberration));
		memset(&InvK, 0, sizeof(InvK));
	}

	// The result is a scaling applied to the distance from the center of the lens.
	float    DistortionFnScaleRadiusSquared(float rsq) const;
	// x,y,z components map to r,g,b scales.
	Echo::Vector3 DistortionFnScaleRadiusSquaredChroma(float rsq) const;

	// DistortionFn applies distortion to the argument.
	// Input: the distance in TanAngle/NIC space from the optical center to the input pixel.
	// Output: the resulting distance after distortion.
	static float DistortionFn(float r)
	{
		return r * Echo::DistortionFnScaleRadiusSquared(r * r);
	}

	// DistortionFnInverse computes the inverse of the distortion function on an argument.
	static float DistortionFnInverse(float r);

	// Also computes the inverse, but using a polynomial approximation. Warning - it's just an approximation!
	float DistortionFnInverseApprox(float r) const;
	// Sets up InvK[].
	void SetUpInverseApprox();

	// Sets a bunch of sensible defaults.
	void SetToIdentity();



	enum { NumCoefficients = 11 };

	DistortionEqnType   Eqn;
	float               K[NumCoefficients];
	float               MaxR;       // The highest R you're going to query for - the curve is unpredictable beyond it.

	float               MetersPerTanAngleAtCenter;

	// Additional per-channel scaling is applied after distortion:
	//  Index [0] - Red channel constant coefficient.
	//  Index [1] - Red channel r^2 coefficient.
	//  Index [2] - Blue channel constant coefficient.
	//  Index [3] - Blue channel r^2 coefficient.
	float               ChromaticAberration[4];

	float               InvK[NumCoefficients];
	float               MaxInvR;
};


//-----------------------------------------------------------------------------------
// ***** DistortionRenderDesc

// This describes distortion for a single eye in an HMD with a display, not just the lens by itself.
struct DistortionRenderDesc
{
	// The raw lens values.
	LensConfig          Lens;

	// These map from [-1,1] across the eye being rendered into TanEyeAngle space (but still distorted)
	Echo::Vector2            LensCenter;
	Echo::Vector2            TanEyeAngleScale;
	// Computed from device characteristics, IPD and eye-relief.
	// (not directly used for rendering, but very useful)
	Echo::Vector2            PixelsPerTanAngleAtCenter;
};

struct DistortionVertex
{
	Echo::Vector2 ScreenPosNDC;
	Echo::Vector2 TanEyeAnglesR;
	Echo::Vector2 TanEyeAnglesG;
	Echo::Vector2 TanEyeAnglesB;
	unsigned int Col;
};

struct DistortionVertex1
{
// 	float ScreenPosNDC_x;
// 	float ScreenPosNDC_y;
	Echo::Vector2 ScreenPosNDC;

};

/// Describes a vertex used by the distortion mesh. This is intended to be converted into
/// the engine-specific format. Some fields may be unused based on the ovrDistortionCaps
/// flags selected. TexG and TexB, for example, are not used if chromatic correction is
/// not requested.
typedef struct ovrDistortionVertex_
{
	Echo::Vector2 ScreenPosNDC;    ///< [-1,+1],[-1,+1] over the entire framebuffer.
	float       TimeWarpFactor;  ///< Lerp factor between time-warp matrices. Can be encoded in Pos.z.
	float       VignetteFactor;  ///< Vignette fade factor. Can be encoded in Pos.w.
	Echo::Vector2 TanEyeAnglesR;   ///< The tangents of the horizontal and vertical eye angles for the red channel.
	Echo::Vector2 TanEyeAnglesG;   ///< The tangents of the horizontal and vertical eye angles for the green channel.
	Echo::Vector2 TanEyeAnglesB;   ///< The tangents of the horizontal and vertical eye angles for the blue channel.
} ovrDistortionVertex;

/// Describes a full set of distortion mesh data, filled in by ovrHmd_CreateDistortionMesh.
/// Contents of this data structure, if not null, should be freed by ovrHmd_DestroyDistortionMesh.
typedef struct ovrDistortionMesh_
{
	ovrDistortionVertex* pVertexData; ///< The distortion vertices representing each point in the mesh.
	unsigned short*      pIndexData;  ///< Indices for connecting the mesh vertices into polygons.
	unsigned int         VertexCount; ///< The number of vertices in the mesh.
	unsigned int         IndexCount;  ///< The number of indices in the mesh.
} ovrDistortionMesh;

// Stores both texture UV coords, or tan(angle) values.
// Use whichever set of data the specific distortion algorithm requires.
// This struct *must* be binary compatible with CAPI ovrDistortionVertex.
struct DistortionMeshVertexData
{
	// [-1,+1],[-1,+1] over the entire framebuffer.
	Echo::Vector2    ScreenPosNDC;
	// [0.0-1.0] interpolation value for timewarping - see documentation for details.
	float       TimewarpLerp;
	// [0.0-1.0] fade-to-black at the edges to reduce peripheral vision noise.
	float       Shade;
	// The red, green, and blue vectors in tan(angle) space.
	// Scale and offset by the values in StereoEyeParams.EyeToSourceUV.Scale
	// and StereoParams.EyeToSourceUV.Offset to get to real texture UV coords.
	Echo::Vector2    TanEyeAnglesR;
	Echo::Vector2    TanEyeAnglesG;
	Echo::Vector2    TanEyeAnglesB;
};



// 对外开放函数接口
FovPort CalculateFovFromEyePosition(float eyeReliefInMeters,
	float offsetToRightInMeters,
	float offsetDownwardsInMeters,
	float lensDiameterInMeters,
	float extraEyeRotationInRadians /*= 0.0f*/);
FovPort ClampToPhysicalScreenFov(int eyeType, FovPort inputFovPort);
Echo::Vector2 CalculateIdealPixelSize(FovPort tanHalfFov, float pixelsPerDisplayPixel);
Echo::Matrix4 CreateProjection(FovPort tanHalfFov,
	float zNear = 0.01f, float zFar = 10000.0f);

bool CreateDistortionMeshInternal(
	int eyeType, FovPort fov,
	ovrDistortionMesh *meshData,
	float overrideEyeReliefIfNonZero = 0.0f);
void DestroyDistortionMesh(ovrDistortionMesh* meshData);



} // END namespace
