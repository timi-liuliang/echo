/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Live2DCubismFramework.h"


// -------- //
// REQUIRES //
// -------- //

#include "Local.h"


// -------------- //
// IMPLEMENTATION //
// -------------- //

void csmInitializeAnimationState(csmAnimationState* state)
{
  // Validate argument.
  Ensure(state, "\"state\" is invalid.", return);


  csmResetAnimationState(state);
}

void csmResetAnimationState(csmAnimationState* state)
{
  // Validate argument.
  Ensure(state, "\"state\" is invalid.", return);


  state->Time = 0.0f;
}

void csmUpdateAnimationState(csmAnimationState* state, const float timeDelta)
{
  // Validate argument.
  Ensure(state, "\"state\" is invalid.", return);


  state->Time += timeDelta;
}
