#include "VRModeAUX.h"
#include "engine/core/Math/EchoMath.h"

namespace Echo
{

	//To allow custom distortion to be introduced to CatMulSpline.
	float(*CustomDistortion)(float) = NULL;
	float(*CustomDistortionInv)(float) = NULL;

#define TPH_SPLINE_STATISTICS 0


	FovPort CalculateFovFromEyePosition(float eyeReliefInMeters,
		float offsetToRightInMeters,
		float offsetDownwardsInMeters,
		float lensDiameterInMeters,
		float extraEyeRotationInRadians /*= 0.0f*/) 
	{
		// 2D view of things:
		//       |-|            <--- offsetToRightInMeters (in this case, it is negative)
		// |=======C=======|    <--- lens surface (C=center)
		//  \    |       _/
		//   \   R     _/
		//    \  |   _/
		//     \ | _/
		//      \|/
		//       O  <--- center of pupil

		// (technically the lens is round rather than square, so it's not correct to
		// separate vertical and horizontal like this, but it's close enough)
		float halfLensDiameter = lensDiameterInMeters * 0.5f;
		FovPort fovPort;
		fovPort.UpTan = (halfLensDiameter + offsetDownwardsInMeters) / eyeReliefInMeters;
		fovPort.DownTan = (halfLensDiameter - offsetDownwardsInMeters) / eyeReliefInMeters;
		fovPort.LeftTan = (halfLensDiameter + offsetToRightInMeters) / eyeReliefInMeters;
		fovPort.RightTan = (halfLensDiameter - offsetToRightInMeters) / eyeReliefInMeters;

		if (extraEyeRotationInRadians > 0.0f)
		{
			// That's the basic looking-straight-ahead eye position relative to the lens.
			// But if you look left, the pupil moves left as the eyeball rotates, which
			// means you can see more to the right than this geometry suggests.
			// So add in the bounds for the extra movement of the pupil.

			// Beyond 30 degrees does not increase FOV because the pupil starts moving backwards more than sideways.
			extraEyeRotationInRadians = Echo::Math::Min(30.0f*Echo::Math::DEG2RAD, Echo::Math::Max(0.0f, extraEyeRotationInRadians));

			// The rotation of the eye is a bit more complex than a simple circle.  The center of rotation
			// at 13.5mm from cornea is slightly further back than the actual center of the eye.
			// Additionally the rotation contains a small lateral component as the muscles pull the eye
			const float eyeballCenterToPupil = 0.0135f;  // center of eye rotation
			const float eyeballLateralPull = 0.001f * (extraEyeRotationInRadians / (30.0f*Echo::Math::DEG2RAD));  // lateral motion as linear function 
			float extraTranslation = eyeballCenterToPupil * sinf(extraEyeRotationInRadians) + eyeballLateralPull;
			float extraRelief = eyeballCenterToPupil * (1.0f - cosf(extraEyeRotationInRadians));

			fovPort.UpTan = Echo::Math::Max(fovPort.UpTan, (halfLensDiameter + offsetDownwardsInMeters + extraTranslation) / (eyeReliefInMeters + extraRelief));
			fovPort.DownTan = Echo::Math::Max(fovPort.DownTan, (halfLensDiameter - offsetDownwardsInMeters + extraTranslation) / (eyeReliefInMeters + extraRelief));
			fovPort.LeftTan = Echo::Math::Max(fovPort.LeftTan, (halfLensDiameter + offsetToRightInMeters + extraTranslation) / (eyeReliefInMeters + extraRelief));
			fovPort.RightTan = Echo::Math::Max(fovPort.RightTan, (halfLensDiameter - offsetToRightInMeters + extraTranslation) / (eyeReliefInMeters + extraRelief));
		}

		return fovPort;
	}




	static float EvalCatmullRom10Spline(float const *K, float scaledVal)
	{
		int const NumSegments = LensConfig::NumCoefficients;

#if TPH_SPLINE_STATISTICS
		//Value should be in range of 0 to (NumSegments-1) (typically 10) if spline is valid. Right?
		if (scaledVal > (NumSegments - 1))
		{
			num_out_of_range++;
			average_total_out_of_range += scaledVal;
			average_out_of_range = average_total_out_of_range / ((float)num_out_of_range);
			percent_out_of_range = 100.0f*(num_out_of_range) / num_total;
		}
		if (scaledVal > (NumSegments - 1 + 1)) num_out_of_range_over_1++;
		if (scaledVal > (NumSegments - 1 + 2)) num_out_of_range_over_2++;
		if (scaledVal > (NumSegments - 1 + 3)) num_out_of_range_over_3++;
		num_total++;
		if (scaledVal > max_scaledVal)
		{
			max_scaledVal = scaledVal;
			max_scaledVal = scaledVal;
		}
#endif

		float scaledValFloor = floorf(scaledVal);
		scaledValFloor = Echo::Math::Max(0.0f, Echo::Math::Min((float)(NumSegments - 1), scaledValFloor));
		float t = scaledVal - scaledValFloor;
		int k = (int)scaledValFloor;

		float p0, p1;
		float m0, m1;
		switch (k)
		{
		case 0:
			// Curve starts at 1.0 with gradient K[1]-K[0]
			p0 = 1.0f;
			m0 = (K[1] - K[0]);    // general case would have been (K[1]-K[-1])/2
			p1 = K[1];
			m1 = 0.5f * (K[2] - K[0]);
			break;
		default:
			// General case
			p0 = K[k];
			m0 = 0.5f * (K[k + 1] - K[k - 1]);
			p1 = K[k + 1];
			m1 = 0.5f * (K[k + 2] - K[k]);
			break;
		case NumSegments - 2:
			// Last tangent is just the slope of the last two points.
			p0 = K[NumSegments - 2];
			m0 = 0.5f * (K[NumSegments - 1] - K[NumSegments - 2]);
			p1 = K[NumSegments - 1];
			m1 = K[NumSegments - 1] - K[NumSegments - 2];
			break;
		case NumSegments - 1:
			// Beyond the last segment it's just a straight line
			p0 = K[NumSegments - 1];
			m0 = K[NumSegments - 1] - K[NumSegments - 2];
			p1 = p0 + m0;
			m1 = m0;
			break;
		}

		float omt = 1.0f - t;
		float res = (p0 * (1.0f + 2.0f *   t) + m0 *   t) * omt * omt
			+ (p1 * (1.0f + 2.0f * omt) - m1 * omt) *   t *   t;

		return res;
	}

	// Inputs are 4 points (pFitX[0],pFitY[0]) through (pFitX[3],pFitY[3])
	// Result is four coefficients in pResults[0] through pResults[3] such that
	//      y = pResult[0] + x * ( pResult[1] + x * ( pResult[2] + x * ( pResult[3] ) ) );
	// passes through all four input points.
	// Return is true if it succeeded, false if it failed (because two control points
	// have the same pFitX value).
	bool FitCubicPolynomial(float *pResult, const float *pFitX, const float *pFitY)
	{
		float d0 = ((pFitX[0] - pFitX[1]) * (pFitX[0] - pFitX[2]) * (pFitX[0] - pFitX[3]));
		float d1 = ((pFitX[1] - pFitX[2]) * (pFitX[1] - pFitX[3]) * (pFitX[1] - pFitX[0]));
		float d2 = ((pFitX[2] - pFitX[3]) * (pFitX[2] - pFitX[0]) * (pFitX[2] - pFitX[1]));
		float d3 = ((pFitX[3] - pFitX[0]) * (pFitX[3] - pFitX[1]) * (pFitX[3] - pFitX[2]));

		if ((d0 == 0.0f) || (d1 == 0.0f) || (d2 == 0.0f) || (d3 == 0.0f))
		{
			return false;
		}

		float f0 = pFitY[0] / d0;
		float f1 = pFitY[1] / d1;
		float f2 = pFitY[2] / d2;
		float f3 = pFitY[3] / d3;

		pResult[0] = -(f0*pFitX[1] * pFitX[2] * pFitX[3]
			+ f1*pFitX[0] * pFitX[2] * pFitX[3]
			+ f2*pFitX[0] * pFitX[1] * pFitX[3]
			+ f3*pFitX[0] * pFitX[1] * pFitX[2]);
		pResult[1] = f0*(pFitX[1] * pFitX[2] + pFitX[2] * pFitX[3] + pFitX[3] * pFitX[1])
			+ f1*(pFitX[0] * pFitX[2] + pFitX[2] * pFitX[3] + pFitX[3] * pFitX[0])
			+ f2*(pFitX[0] * pFitX[1] + pFitX[1] * pFitX[3] + pFitX[3] * pFitX[0])
			+ f3*(pFitX[0] * pFitX[1] + pFitX[1] * pFitX[2] + pFitX[2] * pFitX[0]);
		pResult[2] = -(f0*(pFitX[1] + pFitX[2] + pFitX[3])
			+ f1*(pFitX[0] + pFitX[2] + pFitX[3])
			+ f2*(pFitX[0] + pFitX[1] + pFitX[3])
			+ f3*(pFitX[0] + pFitX[1] + pFitX[2]));
		pResult[3] = f0 + f1 + f2 + f3;

		return true;
	}

	// The result is a scaling applied to the distance.
	float LensConfig::DistortionFnScaleRadiusSquared(float rsq) const
	{
		float scale = 1.0f;
		switch (Eqn)
		{
		case Distortion_Poly4:
			// This version is deprecated! Prefer one of the other two.
			scale = (K[0] + rsq * (K[1] + rsq * (K[2] + rsq * K[3])));
			break;
		case Distortion_RecipPoly4:
			scale = 1.0f / (K[0] + rsq * (K[1] + rsq * (K[2] + rsq * K[3])));
			break;
		case Distortion_CatmullRom10:{
			// A Catmull-Rom spline through the values 1.0, K[1], K[2] ... K[10]
			// evenly spaced in R^2 from 0.0 to MaxR^2
			// K[0] controls the slope at radius=0.0, rather than the actual value.
			const int NumSegments = LensConfig::NumCoefficients;
			assert(NumSegments <= NumCoefficients);
			float scaledRsq = (float)(NumSegments - 1) * rsq / (MaxR * MaxR);
			scale = EvalCatmullRom10Spline(K, scaledRsq);


			//Intercept, and overrule if needed
			if (CustomDistortion)
			{
				scale = CustomDistortion(rsq);
			}

		}break;
		default:
			assert(false);
			break;
		}
		return scale;
	}

	// x,y,z components map to r,g,b
	Echo::Vector3 LensConfig::DistortionFnScaleRadiusSquaredChroma(float rsq) const
	{
		float scale = DistortionFnScaleRadiusSquared(rsq);
		Echo::Vector3 scaleRGB;
		scaleRGB.x = scale * (1.0f + ChromaticAberration[0] + rsq * ChromaticAberration[1]);     // Red
		scaleRGB.y = scale;                                                                        // Green
		scaleRGB.z = scale * (1.0f + ChromaticAberration[2] + rsq * ChromaticAberration[3]);     // Blue
		return scaleRGB;
	}

	// DistortionFnInverse computes the inverse of the distortion function on an argument.
	float LensConfig::DistortionFnInverse(float r)
	{
		assert((r <= 20.0f));

		float s, d;
		float delta = r * 0.25f;

		// Better to start guessing too low & take longer to converge than too high
		// and hit singularities. Empirically, r * 0.5f is too high in some cases.
		s = r * 0.25f;
		d = fabs(r - DistortionFn(s));

		for (int i = 0; i < 20; i++)
		{
			float sUp = s + delta;
			float sDown = s - delta;
			float dUp = fabs(r - DistortionFn(sUp));
			float dDown = fabs(r - DistortionFn(sDown));

			if (dUp < d)
			{
				s = sUp;
				d = dUp;
			}
			else if (dDown < d)
			{
				s = sDown;
				d = dDown;
			}
			else
			{
				delta *= 0.5f;
			}
		}

		return s;
	}



	float LensConfig::DistortionFnInverseApprox(float r) const
	{
		float rsq = r * r;
		float scale = 1.0f;
		switch (Eqn)
		{
		case Distortion_Poly4:
			// Deprecated
			assert(false);
			break;
		case Distortion_RecipPoly4:
			scale = 1.0f / (InvK[0] + rsq * (InvK[1] + rsq * (InvK[2] + rsq * InvK[3])));
			break;
		case Distortion_CatmullRom10:{
			// A Catmull-Rom spline through the values 1.0, K[1], K[2] ... K[9]
			// evenly spaced in R^2 from 0.0 to MaxR^2
			// K[0] controls the slope at radius=0.0, rather than the actual value.
			const int NumSegments = LensConfig::NumCoefficients;
			assert(NumSegments <= NumCoefficients);
			float scaledRsq = (float)(NumSegments - 1) * rsq / (MaxInvR * MaxInvR);
			scale = EvalCatmullRom10Spline(InvK, scaledRsq);

			//Intercept, and overrule if needed
			if (CustomDistortionInv)
			{
				scale = CustomDistortionInv(rsq);
			}

		}break;
		default:
			assert(false);
			break;
		}
		return r * scale;
	}

	void LensConfig::SetUpInverseApprox()
	{
		float maxR = MaxInvR;

		switch (Eqn)
		{
		case Distortion_Poly4:
			// Deprecated
			assert(false);
			break;
		case Distortion_RecipPoly4:{

			float sampleR[4];
			float sampleRSq[4];
			float sampleInv[4];
			float sampleFit[4];

			// Found heuristically...
			sampleR[0] = 0.0f;
			sampleR[1] = maxR * 0.4f;
			sampleR[2] = maxR * 0.8f;
			sampleR[3] = maxR * 1.5f;
			for (int i = 0; i < 4; i++)
			{
				sampleRSq[i] = sampleR[i] * sampleR[i];
				sampleInv[i] = DistortionFnInverse(sampleR[i]);
				sampleFit[i] = sampleR[i] / sampleInv[i];
			}
			sampleFit[0] = 1.0f;
			FitCubicPolynomial(InvK, sampleRSq, sampleFit);

#if 0
			// Should be a nearly exact match on the chosen points.
			OVR_ASSERT(fabs(DistortionFnInverse(sampleR[0]) - DistortionFnInverseApprox(sampleR[0])) / maxR < 0.0001f);
			OVR_ASSERT(fabs(DistortionFnInverse(sampleR[1]) - DistortionFnInverseApprox(sampleR[1])) / maxR < 0.0001f);
			OVR_ASSERT(fabs(DistortionFnInverse(sampleR[2]) - DistortionFnInverseApprox(sampleR[2])) / maxR < 0.0001f);
			OVR_ASSERT(fabs(DistortionFnInverse(sampleR[3]) - DistortionFnInverseApprox(sampleR[3])) / maxR < 0.0001f);
			// Should be a decent match on the rest of the range.
			const int maxCheck = 20;
			for (int i = 0; i < maxCheck; i++)
			{
				float checkR = (float)i * maxR / (float)maxCheck;
				float realInv = DistortionFnInverse(checkR);
				float testInv = DistortionFnInverseApprox(checkR);
				float error = fabsf(realInv - testInv) / maxR;
				OVR_ASSERT(error < 0.1f);
			}
#endif

		}break;
		case Distortion_CatmullRom10:{

			const int NumSegments = LensConfig::NumCoefficients;
			assert(NumSegments <= NumCoefficients);
			for (int i = 1; i < NumSegments; i++)
			{
				float scaledRsq = (float)i;
				float rsq = scaledRsq * MaxInvR * MaxInvR / (float)(NumSegments - 1);
				float r = sqrtf(rsq);
				float inv = DistortionFnInverse(r);
				InvK[i] = inv / r;
				InvK[0] = 1.0f;     // TODO: fix this.
			}

#if 0
			const int maxCheck = 20;
			for (int i = 0; i <= maxCheck; i++)
			{
				float checkR = (float)i * MaxInvR / (float)maxCheck;
				float realInv = DistortionFnInverse(checkR);
				float testInv = DistortionFnInverseApprox(checkR);
				float error = fabsf(realInv - testInv) / MaxR;
				OVR_ASSERT(error < 0.01f);
			}
#endif

		}break;

		default:
			break;
		}
	}


	void LensConfig::SetToIdentity()
	{
		for (int i = 0; i < NumCoefficients; i++)
		{
			K[i] = 0.0f;
			InvK[i] = 0.0f;
		}
		Eqn = Distortion_RecipPoly4;
		K[0] = 1.0f;
		InvK[0] = 1.0f;
		MaxR = 1.0f;
		MaxInvR = 1.0f;
		ChromaticAberration[0] = 0.0f;
		ChromaticAberration[1] = 0.0f;
		ChromaticAberration[2] = 0.0f;
		ChromaticAberration[3] = 0.0f;
		MetersPerTanAngleAtCenter = 0.05f;
	}

	static Echo::Vector2 TransformScreenNDCToTanFovSpace(DistortionRenderDesc const &distortion,
		const Echo::Vector2 &framebufferNDC)
	{
		// Scale to TanHalfFov space, but still distorted.
		Echo::Vector2 tanEyeAngleDistorted;
		tanEyeAngleDistorted.x = (framebufferNDC.x - distortion.LensCenter.x) * distortion.TanEyeAngleScale.x;
		tanEyeAngleDistorted.y = (framebufferNDC.y - distortion.LensCenter.y) * distortion.TanEyeAngleScale.y;
		// Distort.
		float radiusSquared = (tanEyeAngleDistorted.x * tanEyeAngleDistorted.x)
			+ (tanEyeAngleDistorted.y * tanEyeAngleDistorted.y);
		float distortionScale = distortion.Lens.DistortionFnScaleRadiusSquared(radiusSquared);
		Echo::Vector2 tanEyeAngle;
		tanEyeAngle.x = tanEyeAngleDistorted.x * distortionScale;
		tanEyeAngle.y = tanEyeAngleDistorted.y * distortionScale;

		return tanEyeAngle;
	}

	static FovPort GetPhysicalScreenFov(DistortionRenderDesc const &distortion)
	{


		FovPort resultFovPort;

		// Figure out the boundaries of the screen. We take the middle pixel of the screen,
		// move to each of the four screen edges, and transform those back into TanAngle space.
		Echo::Vector2 dmiddle = distortion.LensCenter;

		// The gotcha is that for some distortion functions, the map will "wrap around"
		// for screen pixels that are not actually visible to the user (especially on DK1,
		// which has a lot of invisible pixels), and map to pixels that are close to the middle.
		// This means the edges of the screen will actually be
		// "closer" than the visible bounds, so we'll clip too aggressively.

		// Solution - step gradually towards the boundary, noting the maximum distance.
		struct FunctionHider
		{
			static FovPort FindRange(Echo::Vector2 from, Echo::Vector2 to, int numSteps,
				DistortionRenderDesc const &distortionL)
			{
				FovPort result;
				result.UpTan = 0.0f;
				result.DownTan = 0.0f;
				result.LeftTan = 0.0f;
				result.RightTan = 0.0f;

				float stepScale = 1.0f / (numSteps - 1);
				for (int step = 0; step < numSteps; step++)
				{
					float    lerpFactor = stepScale * (float)step;
					Echo::Vector2 sample = from + (to - from) * lerpFactor;
					Echo::Vector2 tanEyeAngle = TransformScreenNDCToTanFovSpace(distortionL, sample);

					result.LeftTan = Echo::Math::Max(result.LeftTan, -tanEyeAngle.x);
					result.RightTan = Echo::Math::Max(result.RightTan, tanEyeAngle.x);
					result.UpTan = Echo::Math::Max(result.UpTan, -tanEyeAngle.y);
					result.DownTan = Echo::Math::Max(result.DownTan, tanEyeAngle.y);
				}
				return result;
			}
		};

		FovPort leftFovPort = FunctionHider::FindRange(dmiddle, Echo::Vector2(-1.0f, dmiddle.y), 10, distortion);
		FovPort rightFovPort = FunctionHider::FindRange(dmiddle, Echo::Vector2(1.0f, dmiddle.y), 10, distortion);
		FovPort upFovPort = FunctionHider::FindRange(dmiddle, Echo::Vector2(dmiddle.x, -1.0f), 10, distortion);
		FovPort downFovPort = FunctionHider::FindRange(dmiddle, Echo::Vector2(dmiddle.x, 1.0f), 10, distortion);

		resultFovPort.LeftTan = leftFovPort.LeftTan;
		resultFovPort.RightTan = rightFovPort.RightTan;
		resultFovPort.UpTan = upFovPort.UpTan;
		resultFovPort.DownTan = downFovPort.DownTan;

		return resultFovPort;
	}


	FovPort ClampToPhysicalScreenFov(int eyeType, FovPort inputFovPort)
	{
		FovPort resultFovPort;
		FovPort phsyicalFovPort;
		if (eyeType == 0)
			phsyicalFovPort = FovPort(3.72385716f, 3.72385716f, 2.49200320f, 0.970739245f);

		else
		{
			phsyicalFovPort = FovPort(3.72385716f, 3.72385716f, 0.970739245f, 2.49200320f);
		}
		//	FovPort phsyicalFovPort = GetPhysicalScreenFov(distortion);
		resultFovPort.LeftTan = Echo::Math::Min(inputFovPort.LeftTan, phsyicalFovPort.LeftTan);
		resultFovPort.RightTan = Echo::Math::Min(inputFovPort.RightTan, phsyicalFovPort.RightTan);
		resultFovPort.UpTan = Echo::Math::Min(inputFovPort.UpTan, phsyicalFovPort.UpTan);
		resultFovPort.DownTan = Echo::Math::Min(inputFovPort.DownTan, phsyicalFovPort.DownTan);

		return resultFovPort;
	}

	Echo::Vector2 CalculateIdealPixelSize(FovPort tanHalfFov, float pixelsPerDisplayPixel)
	{

		Echo::Vector2 result;

		//
		Echo::Vector2 pixelsPerMeter(8544.0f, 8544.0f);
		float MetersPerTanAngleAtCenter = 0.0425f;
		Echo::Vector2 PixelsPerTanAngleAtCenter = pixelsPerMeter * MetersPerTanAngleAtCenter;


		// TODO: if the app passes in a FOV that doesn't cover the centre, use the distortion values for the nearest edge/corner to match pixel size.
		result.x = (0.5f + pixelsPerDisplayPixel * PixelsPerTanAngleAtCenter.x * (tanHalfFov.LeftTan + tanHalfFov.RightTan));
		result.y = (0.5f + pixelsPerDisplayPixel * PixelsPerTanAngleAtCenter.y * (tanHalfFov.UpTan + tanHalfFov.DownTan));
		return result;
	}

	static Echo::Vector2 TransformTanFovSpaceToRendertargetNDC(ScaleAndOffset2D const &eyeToSourceNDC,
		Echo::Vector2 const &tanEyeAngle)
	{
		Echo::Vector2 textureNDC;
		textureNDC.x = tanEyeAngle.x * eyeToSourceNDC.Scale.x + eyeToSourceNDC.Offset.x;
		textureNDC.y = tanEyeAngle.y * eyeToSourceNDC.Scale.y + eyeToSourceNDC.Offset.y;
		return textureNDC;
	}

	static float DistortionFnInverseApprox(float r)
	{
		float rsq = r * r;
		float scale = 1.0f;

		// A Catmull-Rom spline through the values 1.0, K[1], K[2] ... K[9]
		// evenly spaced in R^2 from 0.0 to MaxR^2
		// K[0] controls the slope at radius=0.0, rather than the actual value.
		const int NumSegments = 11;

		float scaledRsq = (float)(NumSegments - 1) * rsq / (5.55045557f * 5.55045557f);

		float InvK[11];
		InvK[0] = 1.0f;
		InvK[1] = 0.534606934f;
		InvK[2] = 0.408691406f;
		InvK[3] = 0.347900450f;
		InvK[4] = 0.310073912f;
		InvK[5] = 0.283462554f;
		InvK[6] = 0.263793916f;
		InvK[7] = 0.248050719f;
		InvK[8] = 0.234710723f;
		InvK[9] = 0.223541260f;
		InvK[10] = 0.214111343f;

		scale = EvalCatmullRom10Spline(InvK, scaledRsq);

		//Intercept, and overrule if needed
		if (CustomDistortionInv)
		{
			scale = CustomDistortionInv(rsq);
		}

		return r * scale;
	}

	//-----------------------------------------------------------------------------------
	// A set of "reverse-mapping" functions, mapping from real-world and/or texture space back to the framebuffer.

	static Echo::Vector2 TransformTanFovSpaceToScreenNDC(bool rightEye,const Echo::Vector2 &tanEyeAngle)
	{
		float tanEyeAngleRadius = tanEyeAngle.len();
		float tanEyeAngleDistortedRadius = DistortionFnInverseApprox(tanEyeAngleRadius);
		tanEyeAngleDistortedRadius = LensConfig::DistortionFnInverse(tanEyeAngleRadius);
		Echo::Vector2 tanEyeAngleDistorted = tanEyeAngle;
		if (tanEyeAngleRadius > 0.0f)
		{
			tanEyeAngleDistorted = tanEyeAngle * (tanEyeAngleDistortedRadius / tanEyeAngleRadius);
		}

		Echo::Vector2 TanEyeAngleScale;
		Echo::Vector2 LensCenter;
		if (!rightEye)
		{
			TanEyeAngleScale = Echo::Vector2(0.881176412f, 1.10117638f);
			LensCenter = Echo::Vector2(0.152202964f,0);
		}
		else
		{
			TanEyeAngleScale = Echo::Vector2(0.881176412f, 1.10117638f);
			LensCenter = Echo::Vector2(-0.152202964f, 0);
		}

		Echo::Vector2 framebufferNDC;
		framebufferNDC.x = (tanEyeAngleDistorted.x / TanEyeAngleScale.x) + LensCenter.x;
		framebufferNDC.y = (tanEyeAngleDistorted.y / TanEyeAngleScale.y) + LensCenter.y;

		return framebufferNDC;
	}

	Echo::Matrix4 CreateProjection(FovPort tanHalfFov,
		float zNear /*= 0.01f*/, float zFar /*= 10000.0f*/)
	{
		// A projection matrix is very like a scaling from NDC, so we can start with that.
		ScaleAndOffset2D scaleAndOffset = FovPort::CreateNDCScaleAndOffsetFromFov(tanHalfFov);

		float handednessScale = -1.0f;

		Echo::Matrix4 projection;
		// Produces X result, mapping clip edges to [-w,+w]
		projection.m00 = scaleAndOffset.Scale.x;
		projection.m01 = 0.0f;
		projection.m02 = handednessScale * scaleAndOffset.Offset.x;
		projection.m03 = 0.0f;

		// Produces Y result, mapping clip edges to [-w,+w]
		// Hey - why is that YOffset negated?
		// It's because a projection matrix transforms from world coords with Y=up,
		// whereas this is derived from an NDC scaling, which is Y=down.
		projection.m10 = 0.0f;
		projection.m11 = scaleAndOffset.Scale.y;
		projection.m12 = handednessScale * -scaleAndOffset.Offset.y;
		projection.m13 = 0.0f;

		// Produces Z-buffer result - app needs to fill this in with whatever Z range it wants.
		// We'll just use some defaults for now.
		projection.m20 = 0.0f;
		projection.m21 = 0.0f;
		projection.m22 = -handednessScale * zFar / (zNear - zFar);
		projection.m23 = (zFar * zNear) / (zNear - zFar);

		// Produces W result (= Z in)
		projection.m30 = 0.0f;
		projection.m31 = 0.0f;
		projection.m32 = handednessScale;
		projection.m33 = 0.0f;

		return projection;
	}

	static Echo::Vector2 TransformRendertargetNDCToTanFovSpace(const ScaleAndOffset2D &eyeToSourceNDC,
		const Echo::Vector2 &textureNDC)
	{
		Echo::Vector2 tanEyeAngle = (textureNDC - eyeToSourceNDC.Offset) / eyeToSourceNDC.Scale;
		return tanEyeAngle;
	}



	// The result is a scaling applied to the distance.
	float DistortionFnScaleRadiusSquared(float rsq)
	{
		float scale = 1.0f;

		// A Catmull-Rom spline through the values 1.0, K[1], K[2] ... K[10]
		// evenly spaced in R^2 from 0.0 to MaxR^2
		// K[0] controls the slope at radius=0.0, rather than the actual value.
		const int NumSegments = 11;
		float scaledRsq = (float)(NumSegments - 1) * rsq / (1.18861759f * 1.18861759f);

		float K[11];
		K[0] = 1.0f;
		K[1] = 1.04890203f;
		K[2] = 1.10926867f;
		K[3] = 1.19040751f;
		K[4] = 1.30657387f;
		K[5] = 1.48237169f;
		K[6] = 1.77181065f;
		K[7] = 2.27528763f;
		K[8] = 2.93304133f;
		K[9] = 3.73202395f;
		K[10] = 4.66967297f;

		scale = EvalCatmullRom10Spline(K, scaledRsq);

		//Intercept, and overrule if needed
		if (CustomDistortion)
		{
			scale = CustomDistortion(rsq);
		}


		return scale;
	}

	// Same, with chromatic aberration correction.
	static void TransformScreenNDCToTanFovSpaceChroma(bool rightEye, Echo::Vector2 *resultR, Echo::Vector2 *resultG, Echo::Vector2 *resultB,
		const Echo::Vector2 &framebufferNDC)
	{
		Echo::Vector2 TanEyeAngleScale;
		Echo::Vector2 LensCenter;
		if (!rightEye)
		{
			TanEyeAngleScale = Echo::Vector2(0.881176412f, 1.10117638f);
			LensCenter = Echo::Vector2(0.152202964f, 0);
		}
		else
		{
			TanEyeAngleScale = Echo::Vector2(0.881176412f, 1.10117638f);
			LensCenter = Echo::Vector2(-0.152202964f, 0);
		}

		// Scale to TanHalfFov space, but still distorted.
		Echo::Vector2 tanEyeAngleDistorted;
		tanEyeAngleDistorted.x = (framebufferNDC.x - LensCenter.x) * TanEyeAngleScale.x;
		tanEyeAngleDistorted.y = (framebufferNDC.y - LensCenter.y) * TanEyeAngleScale.y;
		// Distort.
		float radiusSquared = (tanEyeAngleDistorted.x * tanEyeAngleDistorted.x)
			+ (tanEyeAngleDistorted.y * tanEyeAngleDistorted.y);
		
		float scale;
		if (!rightEye)
		{
			scale = DistortionFnScaleRadiusSquared(radiusSquared);
		}
		else
		{
			scale = DistortionFnScaleRadiusSquared(radiusSquared);

		}
		Echo::Vector3 scaleRGB;
		scaleRGB.x = scale * (1.0f + -0.006f);     // Red
		scaleRGB.y = scale;                       // Green
		scaleRGB.z = scale * (1.0f + 0.014f);     // Blue
		Echo::Vector3 distortionScales = scaleRGB;
		*resultR = tanEyeAngleDistorted * distortionScales.x;
		*resultG = tanEyeAngleDistorted * distortionScales.y;
		*resultB = tanEyeAngleDistorted * distortionScales.z;
	}

	static DistortionMeshVertexData DistortionMeshMakeVertex(Echo::Vector2 screenNDC,
		bool rightEye, const ScaleAndOffset2D &eyeToSourceNDC)
	{
		DistortionMeshVertexData result;

		float xOffset = 0.0f;
		if (rightEye)
		{
			xOffset = 1.0f;
		}

		Echo::Vector2 tanEyeAnglesR, tanEyeAnglesG, tanEyeAnglesB;
		TransformScreenNDCToTanFovSpaceChroma(rightEye,&tanEyeAnglesR, &tanEyeAnglesG, &tanEyeAnglesB,
			 screenNDC);

		result.TanEyeAnglesR = tanEyeAnglesR;
		result.TanEyeAnglesG = tanEyeAnglesG;
		result.TanEyeAnglesB = tanEyeAnglesB;


		// Retrace is top to bottom on both eyes at the same time.
		result.TimewarpLerp = screenNDC.y * 0.5f + 0.5f;


		// When does the fade-to-black edge start? Chosen heuristically.
		float fadeOutBorderFractionTexture = 0.1f;
		float fadeOutBorderFractionTextureInnerEdge = 0.1f;
		float fadeOutBorderFractionScreen = 0.1f;
		float fadeOutFloor = 0.6f;        // the floor controls how much black is in the fade region

		fadeOutBorderFractionTexture = 0.3f;
		fadeOutBorderFractionTextureInnerEdge = 0.075f;
		fadeOutBorderFractionScreen = 0.075f;
		fadeOutFloor = 0.25f;

		// Fade out at texture edges.
		// The furthest out will be the blue channel, because of chromatic aberration (true of any standard lens)
		Echo::Vector2 sourceTexCoordBlueNDC = TransformTanFovSpaceToRendertargetNDC(eyeToSourceNDC, tanEyeAnglesB);
		if (rightEye)
		{
			// The inner edge of the eye texture is usually much more magnified, because it's right against the middle of the screen, not the FOV edge.
			// So we want a different scaling factor for that. This code flips the texture NDC so that +1.0 is the inner edge
			sourceTexCoordBlueNDC.x = -sourceTexCoordBlueNDC.x;
		}
		float edgeFadeIn = (1.0f / fadeOutBorderFractionTextureInnerEdge) * (1.0f - sourceTexCoordBlueNDC.x);   // Inner
		edgeFadeIn = Echo::Math::Min(edgeFadeIn, (1.0f / fadeOutBorderFractionTexture) * (1.0f + sourceTexCoordBlueNDC.x));   // Outer
		edgeFadeIn = Echo::Math::Min(edgeFadeIn, (1.0f / fadeOutBorderFractionTexture) * (1.0f - sourceTexCoordBlueNDC.y));   // Upper
		edgeFadeIn = Echo::Math::Min(edgeFadeIn, (1.0f / fadeOutBorderFractionTexture) * (1.0f + sourceTexCoordBlueNDC.y));   // Lower

		// Also fade out at screen edges. Since this is in pixel space, no need to do inner specially.
		float edgeFadeInScreen = (1.0f / fadeOutBorderFractionScreen) *
			(1.0f - Echo::Math::Max(Echo::Math::Abs(screenNDC.x), Echo::Math::Abs(screenNDC.y)));
		edgeFadeIn = Echo::Math::Min(edgeFadeInScreen, edgeFadeIn) + fadeOutFloor;

		// Note - this is NOT clamped negatively.
		// For rendering methods that interpolate over a coarse grid, we need the values to go negative for correct intersection with zero.
		result.Shade = Echo::Math::Min(edgeFadeIn, 1.0f);
		result.ScreenPosNDC.x = 0.5f * screenNDC.x - 0.5f + xOffset;
		result.ScreenPosNDC.y = -screenNDC.y;

		return result;
	}

	// Pow2 for the Morton order to work!
	// 4 is too low - it is easy to see the "wobbles" in the HMD.
	// 5 is realllly close but you can see pixel differences with even/odd frame checking.
	// 6 is indistinguishable on a monitor on even/odd frames.
	static const int DMA_GridSizeLog2 = 5;
	static const int DMA_GridSize = 1 << DMA_GridSizeLog2;
	static const int DMA_NumVertsPerEye = (DMA_GridSize + 1)*(DMA_GridSize + 1);
	static const int DMA_NumTrisPerEye = (DMA_GridSize)*(DMA_GridSize)* 2;

	// Generate distortion mesh for a eye.
	static void DistortionMeshCreate(DistortionMeshVertexData **ppVertices, uint16_t **ppTriangleListIndices,
		int *pNumVertices, int *pNumTriangles,
		bool rightEye,
		const ScaleAndOffset2D &eyeToSourceNDC)
	{
		*pNumVertices = DMA_NumVertsPerEye;
		*pNumTriangles = DMA_NumTrisPerEye;

		*ppVertices = (DistortionMeshVertexData*)
			malloc(sizeof(DistortionMeshVertexData) * (*pNumVertices));
		*ppTriangleListIndices = (uint16_t*)malloc(sizeof(uint16_t) * (*pNumTriangles) * 3);

		if (!*ppVertices || !*ppTriangleListIndices)
		{
			if (*ppVertices)
			{
				free(*ppVertices);
			}
			if (*ppTriangleListIndices)
			{
				free(*ppTriangleListIndices);
			}
			*ppVertices = NULL;
			*ppTriangleListIndices = NULL;
			*pNumTriangles = 0;
			*pNumVertices = 0;
			return;
		}



		// Populate vertex buffer info

		// First pass - build up raw vertex data.
		DistortionMeshVertexData* pcurVert = *ppVertices;

		for (int y = 0; y <= DMA_GridSize; y++)
		{
			for (int x = 0; x <= DMA_GridSize; x++)
			{

				Echo::Vector2   sourceCoordNDC;
				// NDC texture coords [-1,+1]
				sourceCoordNDC.x = 2.0f * ((float)x / (float)DMA_GridSize) - 1.0f;
				sourceCoordNDC.y = 2.0f * ((float)y / (float)DMA_GridSize) - 1.0f;
				Echo::Vector2 tanEyeAngle = TransformRendertargetNDCToTanFovSpace(eyeToSourceNDC, sourceCoordNDC);

				// Find a corresponding screen position.
				// Note - this function does not have to be precise - we're just trying to match the mesh tessellation
				// with the shape of the distortion to minimise the number of trianlges needed.
				Echo::Vector2 screenNDC = TransformTanFovSpaceToScreenNDC(rightEye,tanEyeAngle);
				// ...but don't let verts overlap to the other eye.
				screenNDC.x = Echo::Math::Max(-1.0f, Echo::Math::Min(screenNDC.x, 1.0f));
				screenNDC.y = Echo::Math::Max(-1.0f, Echo::Math::Min(screenNDC.y, 1.0f));

				// From those screen positions, generate the vertex.
				*pcurVert = DistortionMeshMakeVertex(screenNDC, rightEye, eyeToSourceNDC);
				pcurVert++;
			}
		}


		// Populate index buffer info
		uint16_t *pcurIndex = *ppTriangleListIndices;

		for (int triNum = 0; triNum < DMA_GridSize * DMA_GridSize; triNum++)
		{
			// Use a Morton order to help locality of FB, texture and vertex cache.
			// (0.325ms raster order -> 0.257ms Morton order)
			assert(DMA_GridSize <= 256);
			int x = ((triNum & 0x0001) >> 0) |
				((triNum & 0x0004) >> 1) |
				((triNum & 0x0010) >> 2) |
				((triNum & 0x0040) >> 3) |
				((triNum & 0x0100) >> 4) |
				((triNum & 0x0400) >> 5) |
				((triNum & 0x1000) >> 6) |
				((triNum & 0x4000) >> 7);
			int y = ((triNum & 0x0002) >> 1) |
				((triNum & 0x0008) >> 2) |
				((triNum & 0x0020) >> 3) |
				((triNum & 0x0080) >> 4) |
				((triNum & 0x0200) >> 5) |
				((triNum & 0x0800) >> 6) |
				((triNum & 0x2000) >> 7) |
				((triNum & 0x8000) >> 8);
			int FirstVertex = x * (DMA_GridSize + 1) + y;
			// Another twist - we want the top-left and bottom-right quadrants to
			// have the triangles split one way, the other two split the other.
			// +---+---+---+---+
			// |  /|  /|\  |\  |
			// | / | / | \ | \ |
			// |/  |/  |  \|  \|
			// +---+---+---+---+
			// |  /|  /|\  |\  |
			// | / | / | \ | \ |
			// |/  |/  |  \|  \|
			// +---+---+---+---+
			// |\  |\  |  /|  /|
			// | \ | \ | / | / |
			// |  \|  \|/  |/  |
			// +---+---+---+---+
			// |\  |\  |  /|  /|
			// | \ | \ | / | / |
			// |  \|  \|/  |/  |
			// +---+---+---+---+
			// This way triangle edges don't span long distances over the distortion function,
			// so linear interpolation works better & we can use fewer tris.
			if ((x < DMA_GridSize / 2) != (y < DMA_GridSize / 2))       // != is logical XOR
			{
				*pcurIndex++ = (uint16_t)FirstVertex;
				*pcurIndex++ = (uint16_t)FirstVertex + 1;
				*pcurIndex++ = (uint16_t)FirstVertex + (DMA_GridSize + 1) + 1;

				*pcurIndex++ = (uint16_t)FirstVertex + (DMA_GridSize + 1) + 1;
				*pcurIndex++ = (uint16_t)FirstVertex + (DMA_GridSize + 1);
				*pcurIndex++ = (uint16_t)FirstVertex;
			}
			else
			{
				*pcurIndex++ = (uint16_t)FirstVertex;
				*pcurIndex++ = (uint16_t)FirstVertex + 1;
				*pcurIndex++ = (uint16_t)FirstVertex + (DMA_GridSize + 1);

				*pcurIndex++ = (uint16_t)FirstVertex + 1;
				*pcurIndex++ = (uint16_t)FirstVertex + (DMA_GridSize + 1) + 1;
				*pcurIndex++ = (uint16_t)FirstVertex + (DMA_GridSize + 1);
			}
		}
	}


	bool CreateDistortionMeshInternal(
		int eyeType, FovPort fov,
		ovrDistortionMesh *meshData,
		float overrideEyeReliefIfNonZero)
	{
		if (!meshData)
			return 0;

		// *** Calculate a part of "StereoParams" needed for mesh generation

		// Note that mesh distortion generation is invariant of RenderTarget UVs, allowing
		// render target size and location to be changed after the fact dynamically. 
		// eyeToSourceUV is computed here for convenience, so that users don't need
		// to call ovrHmd_GetRenderScaleAndOffset unless changing RT dynamically.

		// Find the mapping from TanAngle space to target NDC space.
		ScaleAndOffset2D      eyeToSourceNDC = FovPort::CreateNDCScaleAndOffsetFromFov(fov);

		int triangleCount = 0;
		int vertexCount = 0;

		DistortionMeshCreate((DistortionMeshVertexData**)&meshData->pVertexData,
			(uint16_t**)&meshData->pIndexData,
			&vertexCount, &triangleCount,
			(eyeType == 1),
			eyeToSourceNDC);

		if (meshData->pVertexData)
		{
			// Convert to index
			meshData->IndexCount = triangleCount * 3;
			meshData->VertexCount = vertexCount;
			return 1;
		}

		return 0;
	}

	// Frees distortion mesh allocated by ovrHmd_GenerateDistortionMesh. meshData elements
	// are set to null and 0s after the call.
	void DestroyDistortionMesh(ovrDistortionMesh* meshData)
	{
		if (meshData->pVertexData)
		{
			free((DistortionMeshVertexData*)meshData->pVertexData);
			free(meshData->pIndexData);
		}
			
		meshData->pVertexData = 0;
		meshData->pIndexData = 0;
		meshData->VertexCount = 0;
		meshData->IndexCount = 0;
	}




}









