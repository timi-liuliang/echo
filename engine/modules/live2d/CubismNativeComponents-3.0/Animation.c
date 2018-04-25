/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Live2DCubismFramework.h"
#include "Live2DCubismFrameworkINTERNAL.h"


// -------- //
// REQUIRES //
// -------- //

#include "Local.h"

#include <Live2DCubismCore.h>


// ------- //
// HELPERS //
// ------- //

/// Evaluates curve.
///
/// @param  animation  Animation containing curve.
/// @param  index      Curve index.
/// @param  time       Time to evaluate at.
///
/// @return  Value at time.
static float EvaluateCurve(const csmAnimation* animation, const int index, float time)
{
  csmAnimationSegment* segment, * lastSegment;
  csmAnimationPoint* nextBasePoint;
  csmAnimationCurve* curve;


  curve = animation->Curves + index;


  // Find segment to evaluate.
  segment = animation->Segments + curve->BaseSegmentIndex;
  lastSegment = segment + curve->SegmentCount - 1;


  for (; segment < lastSegment; ++segment)
  {
    // Get first point of next segment.
    nextBasePoint = animation->Points + (segment + 1)->BasePointIndex;


    // Break if time lies within current segment.
    if (nextBasePoint->Time > time)
    {
      break;
    }
  }


  return segment->Evaluate(animation->Points + segment->BasePointIndex, time);
}


// -------------- //
// IMPLEMENTATION //
// -------------- //

unsigned int csmGetDeserializedSizeofAnimation(const char* motionJson)
{
  MotionJsonMeta meta;


  // Validate argument.
  Ensure(motionJson, "\"motionJson\" is invalid.", return 0);


  ReadMotionJsonMeta(motionJson, &meta);


  return (unsigned int)(sizeof(csmAnimation)
    + (sizeof(csmAnimationCurve) * meta.CurveCount)
    + (sizeof(csmAnimationSegment) * meta.TotalSegmentCount)
    + (sizeof(csmAnimationPoint) * meta.TotalPointCount));
}


csmAnimation* csmDeserializeAnimationInPlace(const char *motionJson, void* address, const unsigned int size)
{
  csmAnimation* animation;


  // 'Patch' pointer.
  animation = (csmAnimation*)address;


  // Deserialize animation.
  ReadMotionJson(motionJson, animation);


  return animation;
}

void csmInitializeAnimation(csmAnimation* animation,
                            float duration,
                            short loop,
                            csmAnimationCurve* curves, const short curveCount,
                            csmAnimationSegment* segments,
                            csmAnimationPoint* points)
{
  // Validate arguments.
  Ensure(animation, "\"animation\" is invalid.", return);
  Ensure((duration > 0.0f), "\"duration\" is invalid.", return);
  Ensure(curves, "\"curves\" are invalid.", return);
  Ensure((curveCount > 0), "\"curveCount\" is invalid.", return);
  Ensure(segments, "\"segments\" is invalid.", return);
  Ensure(points, "\"points\" is invalid.", return);


  // Store   
  animation->Duration = duration;
  animation->Loop = loop;

  animation->CurveCount = curveCount;

  animation->Curves = curves;
  animation->Segments = segments;
  animation->Points = points;
}


void csmEvaluateAnimationFAST(const csmAnimation *animation,
                              const csmAnimationState *state,
                              const csmFloatBlendFunction blend,
                              const float weight,
                              csmModel* model,
                              const csmModelHashTable* table,
                              csmModelAnimationCurveHandler handleModelCurve,
                              void* userData)
{
  float* parameterValues, * partOpacities;
  csmAnimationCurve* curves;
  float time, value;
  int c, p;


  // Validate arguments.
  Ensure(animation, "\"animation\" is invalid.", return);
  Ensure(state, "\"state\" is invalid.", return);
  Ensure(blend, "\"blend\" are invalid.", return);
  Ensure(model, "\"model\" is invalid.", return);
  Ensure(table, "\"table\" is invalid.", return);;


  // 'Repeat' time as necessary.
  time = state->Time;


  if (animation->Loop)
  {
    while (time > animation->Duration)
    {
      time -= animation->Duration;
    }
  }


  curves = animation->Curves;


  // Evaluate model curves.
  for (c = 0; c < animation->CurveCount && curves[c].Type == csmModelAnimationCurve; ++c)
  {
    // Skip if no handler given.
    if (!handleModelCurve)
    {
      continue;
    }


    // Evaluate curve and call handler.
    value = EvaluateCurve(animation, c , time);


    handleModelCurve(model, curves[c].Id, value, userData);
  }


  // Evaluate parameter curves.
  parameterValues = csmGetParameterValues(model);


  for (; c < animation->CurveCount && curves[c].Type == csmParameterAnimationCurve; ++c)
  {
    // Find parameter index.
    p = csmFindParameterIndexByHashFAST(table, curves[c].Id);


    // Skip curve evaluation if no value in sink.
    if (p == -1)
    {
      continue;
    }


    // Evaluate curve and apply value.
    value = EvaluateCurve(animation, c , time);

    
    parameterValues[p] = blend(parameterValues[p], value, weight);
  }


  // Evaluate part curves.
  partOpacities = csmGetPartOpacities(model);


  for (; c < animation->CurveCount && curves[c].Type == csmPartOpacityAnimationCurve; ++c)
  {
    // Find parameter index.
    p = csmFindPartIndexByHashFAST(table, curves[c].Id);


    // Skip curve evaluation if no value in sink.
    if (p == -1)
    {
      continue;
    }


    // Evaluate curve and apply value.
    value = EvaluateCurve(animation, c , time);

    
    partOpacities[p] = blend(partOpacities[p], value, weight);
  }
}
