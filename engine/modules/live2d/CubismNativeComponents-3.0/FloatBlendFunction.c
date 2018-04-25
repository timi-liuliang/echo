/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Live2DCubismFramework.h"


// -------------- //
// IMPLEMENTATION //
// -------------- //

float csmOverrideFloatBlendFunction(float base, float value, float weight)
{
  return value * weight;
}

float csmAdditiveFloatBlendFunction(float base, float value, float weight)
{
  return base + (value * weight);
}
