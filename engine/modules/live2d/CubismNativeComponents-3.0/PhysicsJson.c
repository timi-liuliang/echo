/*
* Copyright(c) Live2D Inc. All rights reserved.
*
* Use of this source code is governed by the Live2D Open Software license
* that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
*/


// -------- //
// REQUIRES //
// -------- //

#include "Local.h"

#include "Live2DCubismFrameworkINTERNAL.h"


// ----- //
// TYPES //
// ----- //

/// Parser state.
typedef enum ParserState
{
  /// Common flag signalizing parsing hasn't actually started yet.
  Pending,

  /// Common flag signalizing parser is waiting for something...
  Waiting,

  /// Common flag signaling parsing has finished.
  FinishedParsing,


  /// Flag for version parser to read version.
  ReadingVersion,


  // TODO Document
  ReadingPhysicsSettingCount,

  // TODO Document
  ReadingTotalInputCount,

  // TODO Document
  ReadingTotalOutputCount,

  // TODO Document
  ReadingVertexCount,

  // TODO Document
  ReadingEffectiveForces,

  // TODO Document
  ReadingEffectiveForcesGravity,

  // TODO Document
  ReadingEffectiveForcesGravityX,

  // TODO Document
  ReadingEffectiveForcesGravityY,

  // TODO Document
  ReadingEffectiveForcesWind,

  // TODO Document
  ReadingEffectiveForcesWindX,

  // TODO Document
  ReadingEffectiveForcesWindY,


  // TODO Document
  ReadingPhysicsSetting,
  
  // TODO Document
  ReadingInput,

  // TODO Document
  ReadingOutput,

  // TODO Document
  ReadingVertices,

  // TODO Document
  ReadingNormalization,


  // TODO Document
  ReadingInputSource,

  // TODO Document
  ReadingInputSourceId,

  // TODO Document
  ReadingInputSourceTarget,

  // TODO Document
  ReadingInputWeight,

  // TODO Document
  ReadingInputType,

  // TODO Document
  ReadingInputReflect,


  // TODO Document
  ReadingOutputDestination,

  // TODO Document
  ReadingOutputDestinationId,

  // TODO Document
  ReadingOutputDestinationTarget,

  // TODO Document
  ReadingOutputVertexIndex,

  // TODO Document
  ReadingOutputScale,

  // TODO Document
  ReadingOutputWeight,

  // TODO Document
  ReadingOutputType,

  // TODO Document
  ReadingOutputReflect,


  // TODO Document
  ReadingVerticesPosition,

  // TODO Document
  ReadingVerticesPositionX,

  // TODO Document
  ReadingVerticesPositionY,
  
  // TODO Document
  ReadingVerticesMobility,

  // TODO Document
  ReadingVerticesDelay,

  // TODO Document
  ReadingVerticesAcceleration,

  // TODO Document
  ReadingVerticesRadius,


  // TODO Document
  ReadingNormalizationPosition,

  // TODO Document
  ReadingNormalizationPositionMinimum,

  // TODO Document
  ReadingNormalizationPositionMaximum,

  // TODO Document
  ReadingNormalizationPositionDefault,

  // TODO Document
  ReadingNormalizationAngle,

  // TODO Document
  ReadingNormalizationAngleMinimum,

  // TODO Document
  ReadingNormalizationAngleMaximum,

  // TODO Document
  ReadingNormalizationAngleDefault,
}
ParserState;


/// Context for version parser.
typedef struct VersionParserContext
{
  /// Parser state.
  ParserState State;

  /// Buffer to write to.
  int* Buffer;
}
VersionParserContext;


/// Context for meta data parser.
typedef struct MetaParserContext
{
  /// Parser state.
  ParserState State;

  /// Buffer to write to.
  PhysicsJsonMeta* Buffer;
}
MetaParserContext;


// TODO Document
typedef struct PhysicsParserContext
{
  /// Parser state.
  ParserState State;

  /// Buffer to write to.
  csmPhysicsRig* Buffer;


  // TODO Document
  PhysicsJsonMeta Meta;

  // TODO Document
  int SettingIndex;

  // TODO Document
  int InputIndex;

  // TODO Document
  int OutputIndex;

  // TODO Document
  int VertexIndex;
}
PhysicsParserContext;


// ------------------ //
// INTERNAL FUNCTIONS //
// ------------------ //

// TODO Document
static void GetInputTranslationXFromNormalizedParameterValue(
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
)
{
  targetTranslation->X += NormalizeParameterValue(
    value,
    parameterMinimumValue,
    parameterMaximumValue,
    parameterDefaultValue,
    normalizationPosition->Minimum,
    normalizationPosition->Maximum,
    normalizationPosition->Default,
    isInverted
  ) * weight;
}

// TODO Document
static void GetInputTranslationYFromNormalizedParameterValue(
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
)
{
  targetTranslation->Y += NormalizeParameterValue(
    value,
    parameterMinimumValue,
    parameterMaximumValue,
    parameterDefaultValue,
    normalizationPosition->Minimum,
    normalizationPosition->Maximum,
    normalizationPosition->Default,
    isInverted
  ) * weight;
}

// TODO Document
static void GetInputAngleFromNormalizedParameterValue(
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
)
{
  *targetAngle += NormalizeParameterValue(
    value,
    parameterMinimumValue,
    parameterMaximumValue,
    parameterDefaultValue,
    normalizationAngle->Minimum,
    normalizationAngle->Maximum,
    normalizationAngle->Default,
    isInverted
  ) * weight;
}


// TODO Document
static float GetOutputTranslationX(
  csmVector2 translation,
  csmPhysicsParticle* particles,
  int particleIndex,
  int isInverted,
  csmVector2 parentGravity
)
{
  float outputValue = translation.X;

  if (isInverted)
  {
    outputValue *= -1.0f;
  }

  return outputValue;
}

// TODO Document
static float GetOutputTranslationY(
  csmVector2 translation,
  csmPhysicsParticle* particles,
  int particleIndex,
  int isInverted,
  csmVector2 parentGravity
)
{
  float outputValue = translation.Y;

  if (isInverted)
  {
    outputValue *= -1.0f;
  }

  return outputValue;
}

// TODO Document
static float GetOutputAngle(
  csmVector2 translation,
  csmPhysicsParticle* particles,
  int particleIndex,
  int isInverted,
  csmVector2 parentGravity
)
{
  parentGravity = MultiplyVectoy2ByScalar(parentGravity, -1.0f);

  translation.Y *= -1.0f;

  float outputValue = DirectionToRadian(MultiplyVectoy2ByScalar(parentGravity, -1.0f), MultiplyVectoy2ByScalar(translation, -1.0f));

  outputValue = (((-translation.X) - (-parentGravity.X)) > 0.0f)
    ? -outputValue
    : outputValue;


  if (isInverted)
  {
    outputValue *= -1.0f;
  }

  return outputValue;
}


// TODO Document
static float GetOutputScaleTranslationX(csmVector2 translationScale, float angleScale)
{
  return translationScale.X;
}

// TODO Document
static float GetOutputScaleTranslationY(csmVector2 translationScale, float angleScale)
{
  return translationScale.Y;
}

// TODO Document
static float GetOutputScaleAngle(csmVector2 translationScale, float angleScale)
{
  return angleScale;
}

// --------------------------- //
// VERSION INDEPENDENT PARSERS //
// --------------------------- //

/// Initializes context.
///
/// @param  context  Context to initialize.
/// @param  buffer   Buffer to write to.
static void InitializeVersionParserContext(VersionParserContext* context, int* buffer)
{
  context->State = Pending;
  context->Buffer = buffer;
}


/// Parses version data from a serialized physics*.json.
///
/// @param  jsonString            Serialized physics.
/// @param  tokenType             Current JSON token.
/// @param  tokenEnd              Begin of JSON token (in chars).
/// @param  tokenBegin            End of JSON token (in chars).
/// @param  versionParserContext  Parser context.
static int ParseVersion(const char* jsonString, csmJsonTokenType type, int begin, int end, void* versionParserContext)
{
  VersionParserContext* context;


  // Recover context.
  context = versionParserContext;


  // Handle token.
  switch (context->State)
  {
    // Wait for version section...
    case Pending:
    {
      // Skip non-names.
      if (type != csmJsonName)
      {
        ;
      }


      else if (DoesStringStartWith(jsonString + begin, "Version"))
      {
        context->State = ReadingVersion;
      }


      break;
    }


    // ... then read version.
    case ReadingVersion:
    {
      ReadIntFromString(jsonString + begin, context->Buffer);


      context->State = FinishedParsing;


      break;
    }


    default:
    {
      break;
    }
  }


  // Stop lexing as soon as version read.
  return context->State != FinishedParsing;
}


/// Initializes context.
///
/// @param  context  Context to initialize.
/// @param  buffer   Buffer to write to.
static void InitializeMetaParserContext(MetaParserContext* context, PhysicsJsonMeta* buffer)
{
  context->State = Pending;
  context->Buffer = buffer;
}


// TODO Document
static void InitializePhysicsParserContext(PhysicsParserContext* context, csmPhysicsRig* buffer)
{
  context->State = Pending;
  context->Buffer = buffer;
  context->InputIndex = 0;
  context->OutputIndex = 0;
  context->SettingIndex = 0;
  context->VertexIndex = 0;


  // 'Flag' buffer as uninitialized...
  context->Buffer->Settings = 0;
}



// ---------------------- //
// PHYSICS 3 JSON PARSING //
// ---------------------- //

/// Parses meta data from a serialized physics.json.
///
/// @param  jsonString         Serialized motion.
/// @param  type               Current JSON token.
/// @param  end                Begin of JSON token (in chars).
/// @param  begin              End of JSON token (in chars).
/// @param  metaParserContext  Parser context.
static int ParseMeta3(const char* jsonString, csmJsonTokenType type, int begin, int end, void* metaParserContext)
{
  MetaParserContext* context;


  // Recover context.
  context = metaParserContext;


  // Parse data.
  switch (context->State)
  {
    // Wait for meta section.
    case Pending:
    {
      // Skip non-names.
      if (type != csmJsonName)
      {
        ;
      }


      else if (DoesStringStartWith(jsonString + begin, "Meta"))
      {
        context->State = Waiting;
      }


      break;
    }


    // Look for data to read.
    case Waiting:
    {
      // Stop parsing if end of meta section reached.
      if (type == csmJsonObjectEnd)
      {
        context->State = FinishedParsing;
      }


      // Select data to read.
      else
      {
        if (DoesStringStartWith(jsonString + begin, "PhysicsSettingCount"))
        {
          context->State = ReadingPhysicsSettingCount;
        }
        else if (DoesStringStartWith(jsonString + begin, "TotalInputCount"))
        {
          context->State = ReadingTotalInputCount;
        }
        else if (DoesStringStartWith(jsonString + begin, "TotalOutputCount"))
        {
          context->State = ReadingTotalOutputCount;
        }
        else if (DoesStringStartWith(jsonString + begin, "VertexCount"))
        {
          context->State = ReadingVertexCount;
        }
        else if (DoesStringStartWith(jsonString + begin, "EffectiveForces"))
        {
          context->State = ReadingEffectiveForces;
        }
      }


      break;
    }


    // TODO Document
    case ReadingPhysicsSettingCount:
    {
      ReadIntFromString(jsonString + begin, &context->Buffer->SubRigCount);


      context->State = Waiting;


      break;
    }


    // TODO Document
    case ReadingTotalInputCount:
    {
      ReadIntFromString(jsonString + begin, &context->Buffer->TotalInputCount);


      context->State = Waiting;


      break;
    }


    // TODO Document
    case ReadingTotalOutputCount:
    {
      ReadIntFromString(jsonString + begin, &context->Buffer->TotalOutputCount);


      context->State = Waiting;


      break;
    }


    // TODO Document
    case ReadingVertexCount:
    {
      ReadIntFromString(jsonString + begin, &context->Buffer->ParticleCount);


      context->State = Waiting;


      break;
    }


    // TODO Document
    case ReadingEffectiveForces:
    {
      if (type == csmJsonObjectEnd)
      {
        context->State = Waiting;
      }
      else
      {
        if (DoesStringStartWith(jsonString + begin, "Gravity"))
        {
          context->State = ReadingEffectiveForcesGravity;
        }
        else if (DoesStringStartWith(jsonString + begin, "Wind"))
        {
          context->State = ReadingEffectiveForcesWind;
        }
      }


      break;
    }


    // TODO Document
    case ReadingEffectiveForcesGravity:
    {
      if (type == csmJsonObjectEnd)
      {
        context->State = ReadingEffectiveForces;
      }
      else
      {
        if (DoesStringStartWith(jsonString + begin, "X"))
        {
          context->State = ReadingEffectiveForcesGravityX;
        }
        else if (DoesStringStartWith(jsonString + begin, "Y"))
        {
          context->State = ReadingEffectiveForcesGravityY;
        }
      }


      break;
    }


    // TODO Document
    case ReadingEffectiveForcesGravityX:
    {
      ReadFloatFromString(jsonString + begin, &context->Buffer->EffectiveForces.Gravity.X);


      context->State = ReadingEffectiveForcesGravity;


      break;
    }


    // TODO Document
    case ReadingEffectiveForcesGravityY:
    {
      ReadFloatFromString(jsonString + begin, &context->Buffer->EffectiveForces.Gravity.Y);


      context->State = ReadingEffectiveForcesGravity;


      break;
    }


    // TODO Document
    case ReadingEffectiveForcesWind:
    {
      if (type == csmJsonObjectEnd)
      {
        context->State = ReadingEffectiveForces;
      }
      else
      {
        if (DoesStringStartWith(jsonString + begin, "X"))
        {
          context->State = ReadingEffectiveForcesWindX;
        }
        else if (DoesStringStartWith(jsonString + begin, "Y"))
        {
          context->State = ReadingEffectiveForcesWindY;
        }
      }


      break;
    }


    // TODO Document
    case ReadingEffectiveForcesWindX:
    {
      ReadFloatFromString(jsonString + begin, &context->Buffer->EffectiveForces.Wind.X);


      context->State = ReadingEffectiveForcesWind;


      break;
    }


    // TODO Document
    case ReadingEffectiveForcesWindY:
    {
      ReadFloatFromString(jsonString + begin, &context->Buffer->EffectiveForces.Wind.Y);


      context->State = ReadingEffectiveForcesWind;


      break;
    }


    default:
    {
      break;
    }
  }


  // Stop lexing as soon as version read.
  return context->State != FinishedParsing;
}

static int ParsePhysics3(const char* jsonString, csmJsonTokenType type, int begin, int end, void* physicsParserContext)
{
  MetaParserContext metaParserContext;
  PhysicsParserContext* context;
  int segmentType;


  // Recover context.
  context = physicsParserContext;


  // Initialize meta related fields if necessary.
  if (!context->Buffer->Settings)
  {
    // Parse meta.
    InitializeMetaParserContext(&metaParserContext, &context->Meta);
    csmLexJson(jsonString, ParseMeta3, &metaParserContext);


    // Initialize data fields.
    context->Buffer->Gravity = context->Meta.EffectiveForces.Gravity;

    context->Buffer->Wind = context->Meta.EffectiveForces.Wind;

    context->Buffer->SubRigCount = context->Meta.SubRigCount;


    // Initialize pointer fields.
    context->Buffer->Settings = (csmPhysicsSubRig*)(context->Buffer + 1);
    context->Buffer->Inputs = (csmPhysicsInput*)(context->Buffer->Settings + context->Meta.SubRigCount);
    context->Buffer->Outputs = (csmPhysicsOutput*)(context->Buffer->Inputs + context->Meta.TotalInputCount);
    context->Buffer->Particles = (csmPhysicsParticle*)(context->Buffer->Outputs + context->Meta.TotalOutputCount);
  }

  // Handle token.
  switch (context->State)
  {
    // Wait for setting section.
  case Pending:
  {
    // Skip non-names.
    if (type != csmJsonName)
    {
      ;
    }


    else if (DoesStringStartWith(jsonString + begin, "PhysicsSettings"))
    {
      context->State = Waiting;
    }


    break;
  }


  // Start reading curve or finalize parsing.
  case Waiting:
  {
    // Stop parsing at last setting.
    if (type == csmJsonArrayEnd)
    {
      context->State = FinishedParsing;
    }


    // Start parsing setting.
    else if (type == csmJsonObjectBegin)
    {
      // Initialize curve fields.
      context->Buffer->Settings[context->SettingIndex].BaseInputIndex = context->InputIndex;
      context->Buffer->Settings[context->SettingIndex].BaseOutputIndex = context->OutputIndex;
      context->Buffer->Settings[context->SettingIndex].BaseParticleIndex = context->VertexIndex;


      // Prepare context.
      context->State = ReadingPhysicsSetting;
    }


    break;
  }


  // Handle parsing of a setting.
  case ReadingPhysicsSetting:
  {
    // End curve parsing.
    if (type == csmJsonObjectEnd)
    {
      // Finalize curve fields.
      context->Buffer->Settings[context->SettingIndex].InputCount = context->InputIndex - context->Buffer->Settings[context->SettingIndex].BaseInputIndex;
      context->Buffer->Settings[context->SettingIndex].OutputCount = context->OutputIndex - context->Buffer->Settings[context->SettingIndex].BaseOutputIndex;
      context->Buffer->Settings[context->SettingIndex].ParticleCount = context->VertexIndex - context->Buffer->Settings[context->SettingIndex].BaseParticleIndex;


      // Update context.
      context->SettingIndex += 1;

      context->State = Waiting;
    }


    // Prepare context for reading setting data.
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Input"))
      {
        context->State = ReadingInput;
      }
      else if (DoesStringStartWith(jsonString + begin, "Output"))
      {
        context->State = ReadingOutput;
      }
      else if (DoesStringStartWith(jsonString + begin, "Vertices"))
      {
        context->State = ReadingVertices;
      }
      else if (DoesStringStartWith(jsonString + begin, "Normalization"))
      {
        context->State = ReadingNormalization;
      }
    }


    break;
  }


  // TODO Document
  case ReadingInput:
  {
    if (type == csmJsonObjectEnd)
    {
      context->InputIndex += 1;
    }
    else if (type == csmJsonArrayEnd)
    {
      context->State = ReadingPhysicsSetting;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Source"))
      {
        context->State = ReadingInputSource;
      }
      else if (DoesStringStartWith(jsonString + begin, "Weight"))
      {
        context->State = ReadingInputWeight;
      }
      else if (DoesStringStartWith(jsonString + begin, "Type"))
      {
        context->State = ReadingInputType;
      }
      else if (DoesStringStartWith(jsonString + begin, "Reflect"))
      {
        context->State = ReadingInputReflect;
      }
    }


    break;
  }


  // TODO Document
  case ReadingInputSource:
  {
    if (type == csmJsonObjectEnd)
    {
      context->State = ReadingInput;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Target"))
      {
        context->State = ReadingInputSourceTarget;
      }
      else if (DoesStringStartWith(jsonString + begin, "Id"))
      {
        context->State = ReadingInputSourceId;
      }
    }


    break;
  }

  
  // TODO Document
  case ReadingInputSourceId:
  {
    context->Buffer->Inputs[context->InputIndex].Source.Id = csmHashIdFromSubString(jsonString, begin, end);
    context->Buffer->Inputs[context->InputIndex].SourceParameterIndex = -1;

    context->State = ReadingInputSource;


    break;
  }


  // TODO Document
  case ReadingInputSourceTarget:
  {
    if (DoesStringStartWith(jsonString + begin, "Parameter"))
    {
      context->Buffer->Inputs[context->InputIndex].Source.TargetType = csmParameterPhysics;
    }
    

    context->State = ReadingInputSource;


    break;
  }


  // TODO Document
  case ReadingInputWeight:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Inputs[context->InputIndex].Weight);


    context->State = ReadingInput;


    break;
  }


  // TODO Document
  case ReadingInputType:
  {
    if (DoesStringStartWith(jsonString + begin, "X"))
    {
      context->Buffer->Inputs[context->InputIndex].Type = csmSourceXPhysics;
      context->Buffer->Inputs[context->InputIndex].GetNormalizedParameterValue = GetInputTranslationXFromNormalizedParameterValue;
    }
    else if (DoesStringStartWith(jsonString + begin, "Y"))
    {
      context->Buffer->Inputs[context->InputIndex].Type = csmSourceYPhysics;
      context->Buffer->Inputs[context->InputIndex].GetNormalizedParameterValue = GetInputTranslationYFromNormalizedParameterValue;
    }
    else if (DoesStringStartWith(jsonString + begin, "Angle"))
    {
      context->Buffer->Inputs[context->InputIndex].Type = csmSourceAnglePhysics;
      context->Buffer->Inputs[context->InputIndex].GetNormalizedParameterValue = GetInputAngleFromNormalizedParameterValue;
    }

    context->State = ReadingInput;


    break;
  }


  // TODO Document
  case ReadingInputReflect:
  {
    context->Buffer->Inputs[context->InputIndex].Reflect = (type == csmJsonTrue);


    context->State = ReadingInput;


    break;
  }


  // TODO Document
  case ReadingOutput:
  {
    if (type == csmJsonObjectEnd)
    {
      context->OutputIndex += 1;
    }
    else if (type == csmJsonArrayEnd)
    {
      context->State = ReadingPhysicsSetting;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Destination"))
      {
        context->State = ReadingOutputDestination;
      }
      else if (DoesStringStartWith(jsonString + begin, "VertexIndex"))
      {
        context->State = ReadingOutputVertexIndex;
      }
      else if (DoesStringStartWith(jsonString + begin, "Scale"))
      {
        context->State = ReadingOutputScale;
      }
      else if (DoesStringStartWith(jsonString + begin, "Weight"))
      {
        context->State = ReadingOutputWeight;
      }
      else if (DoesStringStartWith(jsonString + begin, "Type"))
      {
        context->State = ReadingOutputType;
      }
      else if (DoesStringStartWith(jsonString + begin, "Reflect"))
      {
        context->State = ReadingOutputReflect;
      }
    }


    break;
  }


  // TODO Document
  case ReadingOutputDestination:
  {
    if (type == csmJsonObjectEnd)
    {
      context->State = ReadingOutput;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Target"))
      {
        context->State = ReadingOutputDestinationTarget;
      }
      else if (DoesStringStartWith(jsonString + begin, "Id"))
      {
        context->State = ReadingOutputDestinationId;
      }
    }


    break;
  }


  // TODO Document
  case ReadingOutputDestinationId:
  {
    context->Buffer->Outputs[context->OutputIndex].Destination.Id = csmHashIdFromSubString(jsonString, begin, end);
    context->Buffer->Outputs[context->OutputIndex].DestinationParameterIndex = -1;

    context->State = ReadingOutputDestination;


    break;
  }


  // TODO Document
  case ReadingOutputDestinationTarget:
  {
    if (DoesStringStartWith(jsonString + begin, "Parameter"))
    {
      context->Buffer->Outputs[context->OutputIndex].Destination.TargetType = csmParameterPhysics;
    }


    context->State = ReadingOutputDestination;


    break;
  }


  // TODO Document
  case ReadingOutputVertexIndex:
  {
    ReadIntFromString(jsonString + begin, &context->Buffer->Outputs[context->OutputIndex].VertexIndex);


    context->State = ReadingOutput;


    break;
  }


  // TODO Document
  case ReadingOutputScale:
  {
    context->Buffer->Outputs[context->OutputIndex].TranslationScale = MakeVector2(0.0f, 0.0f);
    ReadFloatFromString(jsonString + begin, &context->Buffer->Outputs[context->OutputIndex].AngleScale);


    context->State = ReadingOutput;


    break;
  }


  // TODO Document
  case ReadingOutputWeight:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Outputs[context->OutputIndex].Weight);


    context->State = ReadingOutput;


    break;
  }


  // TODO Document
  case ReadingOutputType:
  {
    if (DoesStringStartWith(jsonString + begin, "X"))
    {
      context->Buffer->Outputs[context->OutputIndex].Type = csmSourceXPhysics;
      context->Buffer->Outputs[context->OutputIndex].GetValue = GetOutputTranslationX;
      context->Buffer->Outputs[context->OutputIndex].GetScale = GetOutputScaleTranslationX;
    }
    else if (DoesStringStartWith(jsonString + begin, "Y"))
    {
      context->Buffer->Outputs[context->OutputIndex].Type = csmSourceYPhysics;
      context->Buffer->Outputs[context->OutputIndex].GetValue = GetOutputTranslationY;
      context->Buffer->Outputs[context->OutputIndex].GetScale = GetOutputScaleTranslationY;
    }
    else if (DoesStringStartWith(jsonString + begin, "Angle"))
    {
      context->Buffer->Outputs[context->OutputIndex].Type = csmSourceAnglePhysics;
      context->Buffer->Outputs[context->OutputIndex].GetValue = GetOutputAngle;
      context->Buffer->Outputs[context->OutputIndex].GetScale = GetOutputScaleAngle;
    }

    context->State = ReadingOutput;


    break;
  }


  // TODO Document
  case ReadingOutputReflect:
  {
    context->Buffer->Outputs[context->OutputIndex].Reflect = (type == csmJsonTrue);


    context->State = ReadingOutput;


    break;
  }


  // TODO Document
  case ReadingVertices:
  {
    if (type == csmJsonObjectEnd)
    {
      context->VertexIndex += 1;
    }
    else if (type == csmJsonArrayEnd)
    {
      context->State = ReadingPhysicsSetting;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Position"))
      {
        context->State = ReadingVerticesPosition;
      }
      else if (DoesStringStartWith(jsonString + begin, "Mobility"))
      {
        context->State = ReadingVerticesMobility;
      }
      else if (DoesStringStartWith(jsonString + begin, "Delay"))
      {
        context->State = ReadingVerticesDelay;
      }
      else if (DoesStringStartWith(jsonString + begin, "Acceleration"))
      {
        context->State = ReadingVerticesAcceleration;
      }
      else if (DoesStringStartWith(jsonString + begin, "Radius"))
      {
        context->State = ReadingVerticesRadius;
      }
    }


    break;
  }


  // TODO Document
  case ReadingVerticesPosition:
  {
    if (type == csmJsonObjectEnd)
    {
      context->State = ReadingVertices;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "X"))
      {
        context->State = ReadingVerticesPositionX;
      }
      else if (DoesStringStartWith(jsonString + begin, "Y"))
      {
        context->State = ReadingVerticesPositionY;
      }
    }


    break;
  }


  // TODO Document
  case ReadingVerticesPositionX:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Particles[context->VertexIndex].InitialPosition.X);

    context->State = ReadingVerticesPosition;

    break;
  }


  // TODO Document
  case ReadingVerticesPositionY:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Particles[context->VertexIndex].InitialPosition.Y);

    context->State = ReadingVerticesPosition;

    break;
  }


  // TODO Document
  case ReadingVerticesMobility:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Particles[context->VertexIndex].Mobility);

    context->State = ReadingVertices;

    break;
  }


  // TODO Document
  case ReadingVerticesDelay:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Particles[context->VertexIndex].Delay);

    context->State = ReadingVertices;

    break;
  }


  // TODO Document
  case ReadingVerticesAcceleration:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Particles[context->VertexIndex].Acceleration);

    context->State = ReadingVertices;

    break;
  }


  // TODO Document
  case ReadingVerticesRadius:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Particles[context->VertexIndex].Radius);

    context->State = ReadingVertices;

    break;
  }


  // TODO Document
  case ReadingNormalization:
  {
    if (type == csmJsonObjectEnd)
    {
      context->State = ReadingPhysicsSetting;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Position"))
      {
        context->State = ReadingNormalizationPosition;
      }
      else if (DoesStringStartWith(jsonString + begin, "Angle"))
      {
        context->State = ReadingNormalizationAngle;
      }
    }


    break;
  }


  // TODO Document
  case ReadingNormalizationPosition:
  {
    if (type == csmJsonObjectEnd)
    {
      context->State = ReadingNormalization;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Minimum"))
      {
        context->State = ReadingNormalizationPositionMinimum;
      }
      else if (DoesStringStartWith(jsonString + begin, "Default"))
      {
        context->State = ReadingNormalizationPositionDefault;
      }
      else if (DoesStringStartWith(jsonString + begin, "Maximum"))
      {
        context->State = ReadingNormalizationPositionMaximum;
      }
    }


    break;
  }


  // TODO Document
  case ReadingNormalizationPositionMinimum:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Settings[context->SettingIndex].NormalizationPosition.Minimum);

    context->State = ReadingNormalizationPosition;

    break;
  }


  // TODO Document
  case ReadingNormalizationPositionDefault:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Settings[context->SettingIndex].NormalizationPosition.Default);

    context->State = ReadingNormalizationPosition;

    break;
  }


  // TODO Document
  case ReadingNormalizationPositionMaximum:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Settings[context->SettingIndex].NormalizationPosition.Maximum);

    context->State = ReadingNormalizationPosition;

    break;
  }


  // TODO Document
  case ReadingNormalizationAngle:
  {
    if (type == csmJsonObjectEnd)
    {
      context->State = ReadingNormalization;
    }
    else
    {
      if (DoesStringStartWith(jsonString + begin, "Minimum"))
      {
        context->State = ReadingNormalizationAngleMinimum;
      }
      else if (DoesStringStartWith(jsonString + begin, "Default"))
      {
        context->State = ReadingNormalizationAngleDefault;
      }
      else if (DoesStringStartWith(jsonString + begin, "Maximum"))
      {
        context->State = ReadingNormalizationAngleMaximum;
      }
    }


    break;
  }


  // TODO Document
  case ReadingNormalizationAngleMinimum:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Settings[context->SettingIndex].NormalizationAngle.Minimum);

    context->State = ReadingNormalizationAngle;

    break;
  }


  // TODO Document
  case ReadingNormalizationAngleDefault:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Settings[context->SettingIndex].NormalizationAngle.Default);

    context->State = ReadingNormalizationAngle;

    break;
  }


  // TODO Document
  case ReadingNormalizationAngleMaximum:
  {
    ReadFloatFromString(jsonString + begin, &context->Buffer->Settings[context->SettingIndex].NormalizationAngle.Maximum);

    context->State = ReadingNormalizationAngle;

    break;
  }


  default:
  {
    break;
  }
  }


  return context->State != FinishedParsing;
}

// -------------------- //
// PHYSICS JSON PARSING //
// -------------------- //

/// Available physics meta readers.
static csmJsonTokenHandler MetaParsers[] =
{
  0,
  0,
  0,
  ParseMeta3
};

/// Available physics readers.
static csmJsonTokenHandler PhysicsParsers[] =
{
  0,
  0,
  0,
  ParsePhysics3,
};


void ReadPhysicsJsonMeta(const char* physicsJson, PhysicsJsonMeta* buffer)
{
  VersionParserContext versionParserContext;
  MetaParserContext context;
  int version;


  // Get version info.
  InitializeVersionParserContext(&versionParserContext, &version);
  csmLexJson(physicsJson, ParseVersion, &versionParserContext);


  // Parse matching version.
  InitializeMetaParserContext(&context, buffer);
  csmLexJson(physicsJson, MetaParsers[version], &context);
}

void ReadPhysicsJson(const char* physicsJson, csmPhysicsRig* buffer)
{
  VersionParserContext versionParserContext;
  PhysicsParserContext context;
  int version;


  // Get version info.
  InitializeVersionParserContext(&versionParserContext, &version);
  csmLexJson(physicsJson, ParseVersion, &versionParserContext);


  // Parse matching version.
  InitializePhysicsParserContext(&context, buffer);
  csmLexJson(physicsJson, PhysicsParsers[version], &context);
}
