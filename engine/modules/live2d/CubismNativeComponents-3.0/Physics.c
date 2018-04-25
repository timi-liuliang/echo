/*
* Copyright(c) Live2D Inc. All rights reserved.
*
* Use of this source code is governed by the Live2D Open Software license
* that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
*/


// -------- //
// REQUIRES //
// -------- //

#include "Live2DCubismFramework.h"
#include "Live2DCubismFrameworkINTERNAL.h"

#include <math.h>

#include "Local.h"

#include <Live2DCubismCore.h>


// --------- //
// CONSTANTS //
// --------- //

/// Constant of air resistance.
const float AirResistance = 5.0f;

/// Constant of maximum weight of input and output ratio.
const float MaximumWeight = 100.0f;

/// Constant of threshold of movement.
const float MovementThreshold = 0.001f;


// -------------- //
// IMPLEMENTATION //
// -------------- //

/// Initializes physics.
/// 
/// @param  physics  Target rig.
static void Initialize(csmPhysicsRig* physics)
{
  csmPhysicsParticle* strand;
  csmPhysicsSubRig* currentSetting;
  int i, settingIndex;
  csmVector2 radius;


  for (settingIndex = 0; settingIndex < physics->SubRigCount; ++settingIndex)
  {
    currentSetting = &physics->Settings[settingIndex];
    strand = &physics->Particles[currentSetting->BaseParticleIndex];

    // Initialize the top of particle.
    strand[0].InitialPosition = MakeVector2(0.0f, 0.0f);
    strand[0].LastPosition = strand[0].InitialPosition;
    strand[0].LastGravity = MakeVector2(0.0f, -1.0f);
    strand[0].LastGravity.Y *= -1.0f;
    strand[0].Velocity = MakeVector2(0.0f, 0.0f);
    strand[0].Force = MakeVector2(0.0f, 0.0f);


    // Initialize paritcles.
    for (i = 1; i < currentSetting->ParticleCount; ++i)
    {
      radius = MakeVector2(0.0f, 0.0f);
      radius.Y = strand[i].Radius;
      strand[i].InitialPosition = AddVector2(strand[i - 1].InitialPosition, radius);
      strand[i].Position = strand[i].InitialPosition;
      strand[i].LastPosition = strand[i].InitialPosition;
      strand[i].LastGravity = MakeVector2(0.0f, -1.0f);
      strand[i].LastGravity.Y *= -1.0f;
      strand[i].Velocity = MakeVector2(0.0f, 0.0f);
      strand[i].Force = MakeVector2(0.0f, 0.0f);
    }
  }
}

/// Updates output parameter value.
/// 
/// @param  parameterValue         Target parameter value.
/// @param  parameterValueMinimum  Minimum of parameter value.
/// @param  parameterValueMaximum  Maximum of parameter value.
/// @param  translation            Translation value.
static void UpdateOutputParameterValue(float* parameterValue, float parameterValueMinimum, float parameterValueMaximum, float translation, csmPhysicsOutput* output)
{
  float outputScale;
  float value;
  float weight;

  outputScale = 1.0f;

  outputScale = output->GetScale(output->TranslationScale, output->AngleScale);

  value = translation * outputScale;


  if (value < parameterValueMinimum)
  {
    if (value < output->ValueBelowMinimum)
    {
      output->ValueBelowMinimum = value;
    }


    value = parameterValueMinimum;
  }
  else if (value > parameterValueMaximum)
  {
    if (value > output->ValueExceededMaximum)
    {
      output->ValueExceededMaximum = value;
    }


    value = parameterValueMaximum;
  }


  weight = (output->Weight / MaximumWeight);

  if (weight >= 1.0f)
  {
    *parameterValue = value;
  }
  else
  {
    value = (*parameterValue * (1.0f - weight)) + (value * weight);
    *parameterValue = value;
  }
}

/// Updates particles.
/// 
/// @param  strand            Target array of particle.
/// @param  strandCount       Count of particle.
/// @param  totalTranslation  Total translation value.
/// @param  totalAngle        Total angle.
/// @param  wind              Direction of wind.
/// @param  thresholdValue    Threshold of movement.
/// @param  deltaTime         Delta time.
static void UpdateParticles(
  csmPhysicsParticle* strand,
  int strandCount,
  csmVector2 totalTranslation,
  float totalAngle,
  csmVector2 wind,
  float thresholdValue,
  float deltaTime
)
{
  int i;
  float totalRadian;
  float delay;
  float distance;
  float angle;
  float radian;
  csmVector2 currentGravity;
  csmVector2 direction;
  csmVector2 velocity;
  csmVector2 force;
  csmVector2 newDirection;

  strand[0].Position = totalTranslation;

  totalRadian = DegreesToRadian(totalAngle);
  currentGravity = RadianToDirection(totalRadian);
  Normalize(&currentGravity);

  for (i = 1; i < strandCount; ++i)
  {
    strand[i].Force = AddVector2(MultiplyVectoy2ByScalar(currentGravity, strand[i].Acceleration), wind);

    strand[i].LastPosition = strand[i].Position;

    delay = strand[i].Delay * deltaTime * 30.0f;


    direction =  SubVector2(strand[i].Position, strand[i - 1].Position);
    distance = Distance(MakeVector2(0.0f, 0.0f), direction);
    angle = DirectionToDegrees(strand[i].LastGravity, currentGravity);
    radian = DegreesToRadian(angle);


    radian /= AirResistance;


    direction.X = (((float)cos(radian) * direction.X) - (direction.Y * (float)sin(radian)));
    direction.Y = (((float)sin(radian) * direction.X) + (direction.Y * (float)cos(radian)));
    Normalize(&direction);


    strand[i].Position = AddVector2(strand[i - 1].Position, MultiplyVectoy2ByScalar(direction, distance));


    velocity = MultiplyVectoy2ByScalar(strand[i].Velocity, delay);
    force = MultiplyVectoy2ByScalar(MultiplyVectoy2ByScalar(strand[i].Force, delay), delay);


    strand[i].Position = AddVector2(AddVector2(strand[i].Position, velocity), force);


    newDirection = SubVector2(strand[i].Position, strand[i - 1].Position);

    Normalize(&newDirection);


    strand[i].Position = AddVector2(strand[i - 1].Position, MultiplyVectoy2ByScalar(newDirection, strand[i].Radius));

    if (fabs(strand[i].Position.X) < thresholdValue)
    {
      strand[i].Position.X = 0.0f;
    }


    if (delay != 0.0f)
    {
      strand[i].Velocity =
        MultiplyVectoy2ByScalar(DivideVector2ByScalar(SubVector2(strand[i].Position, strand[i].LastPosition), delay), strand[i].Mobility);
    }
    else
    {
      strand[i].Velocity = MakeVector2(0.0f, 0.0f);
    }


    strand[i].Force = MakeVector2(0.0f, 0.0f);
    strand[i].LastGravity = currentGravity;
  }
}

unsigned int csmGetDeserializedSizeofPhysics(const char *physicsJson)
{
  PhysicsJsonMeta meta;

  ReadPhysicsJsonMeta(physicsJson, &meta);

  return sizeof(csmPhysicsRig) +
    (sizeof(csmPhysicsSubRig) * meta.SubRigCount) +
    (sizeof(csmPhysicsInput) * meta.TotalInputCount) +
    (sizeof(csmPhysicsOutput) * meta.TotalOutputCount) +
    (sizeof(csmPhysicsParticle) * meta.ParticleCount);
}

csmPhysicsRig* csmDeserializePhysicsInPlace(const char *physicsJson, void* address, const unsigned int size)
{
  csmPhysicsRig* physics;


  // 'Patch' pointer.
  physics = (csmPhysicsRig*)address;


  // Deserialize physics.
  ReadPhysicsJson(physicsJson, physics);

  Initialize(physics);

  return physics;
}

void csmPhysicsEvaluate(csmModel* model, csmPhysicsRig* physics, csmPhysicsOptions* options, float deltaTime)
{
  float totalAngle;
  float weight;
  float radAngle;
  float translationX, translationY;
  float outputValue;
  csmVector2 totalTranslation;
  int i, settingIndex, particleIndex;
  csmPhysicsSubRig* currentSetting;
  csmPhysicsInput* currentInput;
  csmPhysicsOutput* currentOutput;
  csmPhysicsParticle* currentParticles;

  float* parameterValue;
  const float* parameterMaximumValue;
  const float* parameterMinimumValue;
  const float* parameterDefaultValue;


  parameterValue = csmGetParameterValues(model);
  parameterMaximumValue = csmGetParameterMaximumValues(model);
  parameterMinimumValue = csmGetParameterMinimumValues(model);
  parameterDefaultValue = csmGetParameterDefaultValues(model);

  for (settingIndex = 0; settingIndex < physics->SubRigCount; ++settingIndex)
  {
    totalAngle = 0.0f;
    totalTranslation.X = 0.0f;
    totalTranslation.Y = 0.0f;
    currentSetting = &physics->Settings[settingIndex];
    currentInput = &physics->Inputs[currentSetting->BaseInputIndex];
    currentOutput = &physics->Outputs[currentSetting->BaseOutputIndex];
    currentParticles = &physics->Particles[currentSetting->BaseParticleIndex];

    
    // Load input parameters.
    for (i = 0; i < currentSetting->InputCount; ++i)
    {
      weight = currentInput[i].Weight / MaximumWeight;
      

      if (currentInput[i].SourceParameterIndex == -1)
      {
        currentInput[i].SourceParameterIndex = csmFindParameterIndexByHash(model, currentInput[i].Source.Id);
      }


      currentInput[i].GetNormalizedParameterValue(
        &totalTranslation,
        &totalAngle,
        parameterValue[currentInput[i].SourceParameterIndex],
        parameterMinimumValue[currentInput[i].SourceParameterIndex],
        parameterMaximumValue[currentInput[i].SourceParameterIndex],
        parameterDefaultValue[currentInput[i].SourceParameterIndex],
        &currentSetting->NormalizationPosition,
        &currentSetting->NormalizationAngle,
        currentInput->Reflect,
        weight
      );

    }


    radAngle = DegreesToRadian(-totalAngle);

    translationX = totalTranslation.X;
    translationY = totalTranslation.Y;

    totalTranslation.X = (translationX * (float)cos(radAngle) - translationY * (float)sin(radAngle));
    totalTranslation.Y = (translationX * (float)sin(radAngle) + translationY * (float)cos(radAngle));


    // Calculate particles position.
    UpdateParticles(
      currentParticles,
      currentSetting->ParticleCount,
      totalTranslation,
      totalAngle,
      options->Wind,
      MovementThreshold * currentSetting->NormalizationPosition.Maximum,
      deltaTime
    );


    // Update output parameters.
    for (i = 0; i < currentSetting->OutputCount; ++i)
    {
      particleIndex = currentOutput[i].VertexIndex;

      if (particleIndex < 1 || particleIndex >= currentSetting->ParticleCount)
      {
        break;
      }

      if (currentOutput[i].DestinationParameterIndex == -1)
      {
        currentOutput[i].DestinationParameterIndex = csmFindParameterIndexByHash(model, currentOutput[i].Destination.Id);
      }

      csmVector2 translation = SubVector2(currentParticles[particleIndex - 1].Position, currentParticles[particleIndex].Position);

      outputValue = currentOutput[i].GetValue(
        translation,
        currentParticles,
        particleIndex,
        currentOutput[i].Reflect,
        options->Gravity
      );


      UpdateOutputParameterValue(
        &parameterValue[currentOutput[i].DestinationParameterIndex],
        parameterMinimumValue[currentOutput[i].DestinationParameterIndex],
        parameterMaximumValue[currentOutput[i].DestinationParameterIndex],
        outputValue,
        &currentOutput[i]);
    }
  }
}
