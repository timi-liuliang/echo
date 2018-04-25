/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include <Live2DCubismCore.h>

// -------- //
// REQUIRES //
// -------- //

// Cubism model.
typedef struct csmModel csmModel;


// ----- //
// TYPES //
// ----- //

/// 16-bit hash value.
typedef unsigned short csmHash;


/// Look up table.
typedef struct csmModelHashTable
{
  /// Parameters.
  struct
  {
    /// Parameter ID hashes.
    csmHash* IdHashes;

    /// Number of parameters.
    int Count;
  }
  Parameters;

  /// Parts.
  struct
  {
    /// Part ID hashes.
    csmHash* IdHashes;

    /// Number of parts.
    int Count;
  }
  Parts;

  // INV Would hashing drawables be helpful, too?
}
csmModelHashTable;


/// Float blend function.
///
/// @param  base    Current value.
/// @param  value   Value to blend in.
/// @param  weight  Blend weight to use.
///
/// @return Blend result.
typedef float (*csmFloatBlendFunction)(float base, float value, float weight);


/// Builtin override float blend function.
float csmOverrideFloatBlendFunction(float base, float value, float weight);

/// Builtin additive float blend function.
float csmAdditiveFloatBlendFunction(float base, float value, float weight);


/// Opaque Cubism animation.
typedef struct csmAnimation csmAnimation;


/// Play state of an animation.
typedef struct csmAnimationState
{
  /// Current time.
  float Time;
}
csmAnimationState;


/// Animation model curve type.
typedef enum csmModelAnimationCurveType
{
  /// Model opacity curve.
  csmOpacityAnimationCurve,

  /// Eye blink curve.
  csmEyeBlinkAnimationCurve,

  /// Lip-sync curve.
  csmLipSyncAnimationCurve
}
csmModelAnimationCurveType;


/// Animation model curve handler.
///
/// @param  model     Model animation being applied to.
/// @param  type      Model curve type.
/// @param  value     Evaluated value.
/// @param  userData  [Optional] User data.
typedef void csmModelAnimationCurveHandler(const csmModel* model, const csmModelAnimationCurveType type, const float value, void* userData);


// ------- //
// PHYSICS //
// ------- //

// TODO Document
typedef struct csmPhysicsRig csmPhysicsRig;

// TODO Document
typedef struct csmPhysicsOptions
{
  csmVector2 Gravity;

  csmVector2 Wind;
}
csmPhysicsOptions;


// ---------------- //
// MODEL EXTENSIONS //
// ---------------- //

/// Hashes an ID.
///
/// @param  id  ID to hash.
///
/// @return Non-zero hash on success; '0' otherwise.
csmHash csmHashId(const char* id);


/// Gets the size of a model hash table in bytes.
///
/// @param  model  Model to hash.
///
/// @return  Number of bytes necessary.
unsigned int csmGetSizeofModelHashTable(const csmModel* model);

/// Initializes model hash table.
///
/// @param  model    Model to hash.
/// @param  address  Address to place table into.
/// @param  size     Size of buffer in bytes.
///
/// @return  Valid pointer on success; '0' otherwise.
csmModelHashTable* csmInitializeModelHashTableInPlace(const csmModel* model, void* address, const unsigned int size);


/// Finds index of a parameter.
///
/// @param  model  Model to query against.
/// @param  hash   Parameter ID hash.
///
/// @return Valid index on success; '-1' otherwise.
int csmFindParameterIndexByHash(const csmModel* model, const csmHash hash);

/// Finds index of a parameter faster by comparing hashes.
///
/// @param  table  Table to compare against.
/// @param  hash   Parameter ID hash.
///
/// @return Valid index on success; '-1' otherwise.
int csmFindParameterIndexByHashFAST(const csmModelHashTable* table, const csmHash hash);

/// Finds index of a part.
///
/// @param  model  Model to query against.
/// @param  hash   Part ID hash.
///
/// @return Valid index on success; '-1' otherwise.
int csmFindPartIndexByHash(const csmModel* model, const csmHash hash);

/// Finds index of a part faster by comparing hashes.
///
/// @param  table  Table to compare against.
/// @param  hash   Part ID hash.
///
/// @return Valid index on success; '-1' otherwise.
int csmFindPartIndexByHashFAST(const csmModelHashTable* table, const csmHash hash);

/// Finds index of a drawable.
///
/// @param  model  Model to query against.
/// @param  hash   Drawable ID hash.
///
/// @return Valid index on success; '-1' otherwise.
int csmFindDrawableIndexByHash(const csmModel* model, const csmHash hash);


/// Queries whether a model uses clipping masks.
///
/// @param  model  Model to query.
///
/// @return  Non-zero if model uses clipping masks; '0'otherwise.
int csmDoesModelUseMasks(const csmModel* model);


// --------------- //
// ANIMATION STATE //
// --------------- //

/// Initializes an animation state.
///
/// @param  state  State to initialize.
void csmInitializeAnimationState(csmAnimationState* state);

/// Resets an animation state.
///
/// @param  state  State to reset.
void csmResetAnimationState(csmAnimationState* state);

/// Ticks an animation state.
///
/// @param  state      State to tick.
/// @param  deltaTime  Time passed since last tick.
void csmUpdateAnimationState(csmAnimationState* state, const float deltaTime);


// --------- //
// ANIMATION //
// --------- //

/// Gets the deserialized size of a serialized animation in bytes.
///
/// @param  motionJson  Serialized animation to query for.
///
/// @return  Number of bytes necessary.
unsigned int csmGetDeserializedSizeofAnimation(const char* motionJsonString);


/// Deserializes an animotion.
///
/// @param[in]  motionJson  Serialized animation.
/// @param[in]  address     Address to place deserialized animation at.
/// @param[in]  size        Size of passed memory block (in bytes).
///
/// @return  Valid pointer on success; '0' otherwise.
csmAnimation *csmDeserializeAnimationInPlace(const char *motionJson, void* address, const unsigned int size);


/// Evaluates an animation fast by using a hash table for look-ups.
///
/// @param  animation         Animation to evaluate.
/// @param  state             Animation state.
/// @param  blend             Blend function to use for filling sink.
/// @param  weight            Blend weight factor.
/// @param  model             Model to apply results to.
/// @param  table             Model table to user for look-ups.
/// @param  handleModelCurve  [Optional] Model curve handler.
/// @param  userData          [Optional] Data to pass to model curve handler.
void csmEvaluateAnimationFAST(const csmAnimation *animation,
                              const csmAnimationState *state,
                              const csmFloatBlendFunction blend,
                              const float weight,
                              csmModel* model,
                              const csmModelHashTable* table,
                              csmModelAnimationCurveHandler handleModelCurve,
                              void* userData);

// ------- //
// PHYSICS //
// ------- //

/// Gets the deserialized size of a serialized physics in bytes.
///
/// @param  physicsJson  Serialized physics to query for.
///
/// @return  Number of bytes necessary.
unsigned int csmGetDeserializedSizeofPhysics(const char *physicsJson);


/// Deserializes physics.
///
/// @param  physicsJson  Serialized physics.
/// @param  address      Address to place deserialized animation at.
/// @param  size         Size of passed memory block (in bytes).
///
/// @return  Valid pointer on success; '0' otherwise.
csmPhysicsRig *csmDeserializePhysicsInPlace(const char *physicsJson, void* address, const unsigned int size);


/// Evaluates physics.
///
/// @param  model  Model to apply result to.
/// @param  physics  Physics to evaluate.
/// @param  options  Options of evaluation.
void csmPhysicsEvaluate(csmModel* model, csmPhysicsRig* physics, csmPhysicsOptions* options, float deltaTime);