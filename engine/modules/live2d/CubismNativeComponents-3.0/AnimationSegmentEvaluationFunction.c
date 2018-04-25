/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Live2DCubismFrameworkINTERNAL.h"


// ------- //
// HELPERS //
// ------- //

/// Lerps between two points.
///
/// @param  a  First point.
/// @param  b  Second point.
/// @param  t  Lerp factor.
///
/// @return  Interpolation result.
static inline csmAnimationPoint LerpPoints(const csmAnimationPoint a, const csmAnimationPoint b, const float t)
{
  csmAnimationPoint result;


  result.Time = a.Time + ((b.Time - a.Time) * t);
  result.Value = a.Value + ((b.Value - a.Value) * t);


  return result;
}


// -------------- //
// IMPLEMENTATION //
// -------------- //

float csmLinearAnimationSegmentEvaluationFunction(const csmAnimationPoint* points, const float time)
{
  float t = (time - points[0].Time) / (points[1].Time - points[0].Time);


  return points[0].Value + ((points[1].Value - points[0].Value) * t);
}

//　Simple De Casteljau's algorithm implementation.
float csmBezierAnimationSegmentEvaluationFunction(const csmAnimationPoint* points, const float time)
{
  csmAnimationPoint p01, p12, p23, p012, p123;
  float t;
  
  
  t = (time - points[0].Time) / (points[3].Time - points[0].Time);


  p01 = LerpPoints(points[0], points[1], t);
  p12 = LerpPoints(points[1], points[2], t);
  p23 = LerpPoints(points[2], points[3], t);

  p012 = LerpPoints(p01, p12, t);
  p123 = LerpPoints(p12, p23, t);


  return LerpPoints(p012, p123, t).Value;
}

float csmSteppedAnimationSegmentEvaluationFunction(const csmAnimationPoint* points, const float time)
{
  return points[0].Value;
}

float csmInverseSteppedAnimationSegmentEvaluationFunction(const csmAnimationPoint* points, const float time)
{
  return points[1].Value;
}
