/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once


#include <Live2DCubismCore.h>


// ----- //
// TYPES //
// ----- //

/// 16-bit hash value.
typedef unsigned short csmHash;


/// JSON token type.
typedef enum csmJsonTokenType
{
  /// Begin of object.
  csmJsonObjectBegin,

  /// End of object.
  csmJsonObjectEnd,


  /// Begin of array.
  csmJsonArrayBegin,

  /// End of array.
  csmJsonArrayEnd,


  /// Key.
  csmJsonName,


  /// String value.
  csmJsonString,

  /// Number value.
  csmJsonNumber,

  /// 'true' value.
  csmJsonTrue,

  /// 'false' value.
  csmJsonFalse,

  /// 'null' value.
  csmJsonNull,


  /// Number of JSON token types.
  // (Make sure this value always is the last value of the enumeration).
  csmJsonTokenTypeCount
}
csmJsonTokenType;


/// JSON token handler when lexing JSONs.
///
/// @param  jsonString  Input JSON string.
/// @param  type        Token type.
/// @param  begin       Begin of token as offset into string (in chars).
/// @param  end         End of token as offset into string (in chars).
/// @param  userData    [Optional] User data.
///
/// @param Non-zero to continue lexing; '0' to stop lexing.
typedef int (*csmJsonTokenHandler)(const char* jsonString,
                                   const csmJsonTokenType type,
                                   const int begin,
                                   const int end,
                                   void* userData);


/// Single point making up an animation curve.
typedef struct csmAnimationPoint
{
  /// Timing of point.
  float Time;

  /// Value at time.
  float Value;
}
csmAnimationPoint;


/// Animation segment evaluation function.
///
/// @param  points  Points making up the segment.
/// @param  time    Time to evaluate at.
///
/// @return  Evaluation result.
typedef float (*csmAnimationSegmentEvaluationFunction)(const csmAnimationPoint* points, const float time);

/// Builtin linear animation segment evaluation.
///
/// First 2 points are evaluated.
float csmLinearAnimationSegmentEvaluationFunction(const csmAnimationPoint* points, const float time);

/// Builtin bezier animation segment evaluation.
///
/// First 4 points are evaluated.
float csmBezierAnimationSegmentEvaluationFunction(const csmAnimationPoint* points, const float time);

/// Builtin stepped animation segment evaluation.
///
/// First 2 points are evaluated.
float csmSteppedAnimationSegmentEvaluationFunction(const csmAnimationPoint* points, const float time);

/// Builtin inverse stepped animation segment evaluation.
///
/// First 2 points are evaluated.
float csmInverseSteppedAnimationSegmentEvaluationFunction(const csmAnimationPoint* points, const float time);


/// Animation curve types.
enum
{
  /// Model animation curve.
  csmModelAnimationCurve,

  /// Parameter animation curve.
  csmParameterAnimationCurve,

  /// Part opacity animation curve.
  csmPartOpacityAnimationCurve
};


/// Single animation curve segment.
typedef struct csmAnimationSegment
{
  /// Allows evaluating segment.
  csmAnimationSegmentEvaluationFunction Evaluate;

  /// Index of first segment point.
  int BasePointIndex;
}
csmAnimationSegment;


/// Animation curve.
typedef struct csmAnimationCurve
{
  /// Curve target type.
  short Type;

  /// ID of curve.
  csmHash Id;

  /// Number of segments the curve contains.
  int SegmentCount;

  /// Index of first segment in curve.
  int BaseSegmentIndex;
}
csmAnimationCurve;


/// Animation.
typedef struct csmAnimation
{
  /// Duration in seconds.
  float Duration;

  /// Non-zero if animation should loop.
  short Loop;


  /// Number of curves.
  short CurveCount;

  /// Curves.
  csmAnimationCurve* Curves;

  /// Curve segments.
  csmAnimationSegment* Segments;

  /// Curve points.
  csmAnimationPoint* Points;
}
csmAnimation;


// ------- //
// PHYSICS //
// ------- //

// TODO Document
enum
{
  csmParameterPhysics,
};

/// Component of source physical force.
enum
{
  /// Use X-axis position.
  csmSourceXPhysics,

  /// Use Y-axis position.
  csmSourceYPhysics,

  /// Use angle.
  csmSourceAnglePhysics,
};

/// Physics Input / Output parameter.
typedef struct csmPhysicsParameter
{
  /// Parameter id.
  csmHash Id;

  /// Type of target parameter.
  short TargetType;
}
csmPhysicsParameter;

/// Normalization values.
typedef struct csmPhysicsNormalization
{
  /// Normalized minimum value.
  float Minimum;

  /// Normalized maximum value.
  float Maximum;

  /// Normalized default value.
  float Default;
}
csmPhysicsNormalization;

/// Particle.
typedef struct csmPhysicsParticle
{
  /// Initial position.
  csmVector2 InitialPosition;

  /// Mobility ratio.
  float Mobility;

  /// Delay ratio.
  float Delay;

  /// Current acceleration.
  float Acceleration;

  /// Length of radius.
  float Radius;

  /// Current position.
  csmVector2 Position;

  /// Last position.
  csmVector2 LastPosition;

  /// Last gravity.
  csmVector2 LastGravity;

  /// Current force.
  csmVector2 Force;

  /// Current velocity.
  csmVector2 Velocity;
}
csmPhysicsParticle;

/// Children of rig.
typedef struct csmPhysicsSubRig
{
  /// Number of input.
  int InputCount;

  /// Number of output.
  int OutputCount;

  /// Number of particle.
  int ParticleCount;

  /// Index of first input in this rig.
  int BaseInputIndex;

  /// Index of first output in this rig.
  int BaseOutputIndex;

  /// Index of first particle in this rig.
  int BaseParticleIndex;

  /// Normalized position values.
  csmPhysicsNormalization NormalizationPosition;

  /// Normalized angle values.
  csmPhysicsNormalization NormalizationAngle;
}
csmPhysicsSubRig;

/// Delegation of function of getting normalized parameter value.
///
/// @param  targetTranslation      Result of translation.
/// @param  targetAngle            Result of rotation.
/// @param  value                  Current parameter value.
/// @param  parameterMinimumValue  Minimum parameter value.
/// @param  parameterMaximumValue  Maximum parameter value.
/// @param  parameterDefaultValue  Default parameter value.
/// @param  normalizationPosition  Normalized position value.
/// @param  normalizationAngle     Normalized angle value.
/// @param  isInverted             True if value is inverted; othewise.
/// @param  weight                 Weight.
typedef void(*NormalizedPhysicsParameterValueGetter)(
  csmVector2* targetTranslation,
  float* targetAngle,
  float value,
  float parameterMinimumValue,
  float parameterMaximumValue,
  float parameterDefaultValue,
  csmPhysicsNormalization* normalizationPosition,
  csmPhysicsNormalization* normalizationAngle,
  int isInverted,
  float weight
  );

/// Input data of physics.
typedef struct csmPhysicsInput
{
  /// Source of parameter.
  csmPhysicsParameter Source;

  /// Index of source parameter.
  int SourceParameterIndex;

  /// Weight.
  float Weight;

  /// Component of source.
  short Type;

  /// True if value is inverted; othewise.
  short Reflect;

  /// Function of getting normalized parameter value.
  NormalizedPhysicsParameterValueGetter GetNormalizedParameterValue;
}
csmPhysicsInput;

/// Delegation of function of getting output value.
///
/// @param  translation    Translation.
/// @param  particles      Particles.
/// @param  particleIndex  Index of particle.
/// @param  isInverted     True if value is inverted; othewise.
/// @param  parentGravity  Gravity.
typedef float(*PhysicsValueGetter)(
  csmVector2 translation,
  csmPhysicsParticle* particles,
  int particleIndex,
  int isInverted,
  csmVector2 parentGravity
  );

/// Delegation of function of getting output scale.
///
/// @param  translationScale  Scale of transition.
/// @param  angleScale        Scale of angle.
typedef float(*PhysicsScaleGetter)(csmVector2 translationScale, float angleScale);

/// Output data of physics.
typedef struct csmPhysicsOutput
{
  /// Destination of parameter.
  csmPhysicsParameter Destination;

  /// Index of destination parameter.
  int DestinationParameterIndex;

  /// Index of particle.
  int VertexIndex;

  /// Scale of translation.
  csmVector2 TranslationScale;

  /// Scale of angle.
  float AngleScale;

  /// Weight.
  float Weight;

  /// Component of source.
  short Type;

  /// True if value is inverted; othewise.
  short Reflect;

  /// The value that below minimum.
  float ValueBelowMinimum;

  /// The value that exceeds maximum.
  float ValueExceededMaximum;

  /// Function of getting output value.
  PhysicsValueGetter GetValue;

  /// Function of getting output scale.
  PhysicsScaleGetter GetScale;
}
csmPhysicsOutput;

/// Physics rig.
typedef struct csmPhysicsRig
{
  /// Count of children rig.
  int SubRigCount;

  /// Array of sub rig.
  csmPhysicsSubRig* Settings;

  /// Array of input.
  csmPhysicsInput* Inputs;

  /// Array of output.
  csmPhysicsOutput* Outputs;

  /// Array of particle.
  csmPhysicsParticle* Particles;

  /// Gravity.
  csmVector2 Gravity;

  /// Direction of wind.
  csmVector2 Wind;
}
csmPhysicsRig;


// ---------------- //
// MODEL EXTENSIONS //
// ---------------- //

/// Hashes part of a string.
///
/// @param  string    String to hash from.
/// @param  idBegin   Inclusive offset into string to start hashing at.
/// @param  idEnd     Exclusive offset into string to stop hashing at.
///
/// @return  Non-zero hash value on success; '0' otherwise.
csmHash csmHashIdFromSubString(const char* string, const int idBegin, const int idEnd);


// ---- //
// JSON //
// ---- //

/// Lexes a JSON string.
///
/// @param  jsonString  JSON string to lex.
/// @param  onToken     Token handler.
/// @param  userData    [Optional] Data to pass to token handler.
void csmLexJson(const char* jsonString, csmJsonTokenHandler onToken, void* userData);


// --------- //
// ANIMATION //
// --------- //

/// Initializes an animation.
///
/// @param  animation     Animation to reset.
/// @param  duration      Duration in seconds.
/// @param  loop          Loop flag.
/// @param  curves        Curve data.
/// @param  curveCount    Number of curves.
/// @param  segments      Segment data.
/// @param  points        Point data.
void csmInitializesAnimation(csmAnimation* animation,
                             float duration,
                             short loop,
                             csmAnimationCurve* curves, const short curveCount,
                             csmAnimationSegment* segments,
                             csmAnimationPoint* points);
