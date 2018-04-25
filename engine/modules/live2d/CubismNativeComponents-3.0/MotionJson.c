/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Local.h"


// -------- //
// REQUIRES //
// -------- //

#include "Live2DCubismFramework.h"
#include "Live2DCubismFrameworkINTERNAL.h"


// ----- //
// TYPES //
// ----- //

/// Curve segment identifiers.
enum
{
  /// Linear segment identifier.
  LinearSegment = 0,

  /// Bézier segment identifier.
  BezierSegment = 1,

  /// Stepped segment identifier.
  SteppedSegment = 2,

  /// Inverse stepped segment identifier.
  InverseSteppedSegment = 3
};


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


  /// Flag for meta parser to read motion duration.
  ReadingDuration,

  /// Flag for meta parser to read motion duration.
  ReadingFps,

  /// Flag for meta parser to read motion duration.
  ReadingLoop,

  /// Flag for meta parser to read motion duration.
  ReadingCurveCount,
  
  /// Flag for meta parser to read motion duration.
  ReadingTotalSegmentCount,
  
  /// Flag for meta parser to read motion duration.
  ReadingTotalPointCount,

  /// Flag for meta parser to read bezier restriction.
  ReadingAreBeziersRestricted,


  // Flag for motion parser to read single curve data.
  ReadingCurve,

  // Flag for motion parser to read curve target data.
  ReadingTarget,

  // Flag for motion parser to read curve ID data.
  ReadingId,

  // Flag for motion parser to read curve segments data.
  ReadingSegments,
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
  MotionJsonMeta* Buffer;
}
MetaParserContext;


/// Context for motion data parser.
typedef struct MotionParserContext
{
  /// Parser state.
  ParserState State;

  /// Current offset into buffer curves array.
  int CurveIndex;

  /// Current offset into buffer segments array.
  int SegmentIndex;

  /// Current offset into buffer points array.
  int PointIndex;


  /// Position in segment being parsed.
  int SegmentValueIndex;

  /// Position of next segment type identifier in segments being parsed.
  int SegmentTypePosition;

  /// Non-zero if point time should be parsed. 
  int ReadPointTime;


  /// Motion meta data.
  MotionJsonMeta Meta;


  /// Buffer to write to.
  csmAnimation* Buffer;
}
MotionParserContext;


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


/// Parses version data from a serialized motion*.json.
///
/// @param  jsonString            Serialized motion.
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
static void InitializeMetaParserContext(MetaParserContext* context, MotionJsonMeta* buffer)
{
  context->State = Pending;
  context->Buffer = buffer;
  context->Buffer->AreBeziersRestricted = 0;
}


/// Initializes context.
///
/// @param  context  Context to initialize.
/// @param  buffer   Buffer to write to.
static void InitializeMotionParserContext(MotionParserContext* context, csmAnimation* buffer)
{
  context->State = Pending;
  context->CurveIndex = 0;
  context->SegmentIndex = 0;
  context->PointIndex = 0;
  context->SegmentValueIndex = 0;
  context->SegmentTypePosition = 0;
  context->ReadPointTime = 0;
  context->Buffer = buffer;


  // 'Flag' buffer as uninitialized...
  context->Buffer->Curves = 0;
}


// --------------------- //
// MOTION 3 JSON PARSING //
// --------------------- //

/// Parses meta data from a serialized motion3.json.
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
        if (DoesStringStartWith(jsonString + begin, "Duration"))
        {
          context->State = ReadingDuration;
        }
        else if (DoesStringStartWith(jsonString + begin, "Fps"))
        {
          context->State = ReadingFps;
        }
        else if (DoesStringStartWith(jsonString + begin, "Loop"))
        {
          context->State = ReadingLoop;
        }
        else if (DoesStringStartWith(jsonString + begin, "CurveCount"))
        {
          context->State = ReadingCurveCount;
        }
        else if (DoesStringStartWith(jsonString + begin, "TotalSegmentCount"))
        {
          context->State = ReadingTotalSegmentCount;
        }
        else if (DoesStringStartWith(jsonString + begin, "TotalPointCount"))
        {
          context->State = ReadingTotalPointCount;
        }
        else if (DoesStringStartWith(jsonString + begin, "AreBeziersRestricted"))
        {
          context->State = ReadingAreBeziersRestricted;
        }
      }


      break;
    }


    // Read duration.
    case ReadingDuration:
    {
      ReadFloatFromString(jsonString + begin, &context->Buffer->Duration);


      context->State = Waiting;


      break;
    }


    // Read fps.
    case ReadingFps:
    {
       ReadFloatFromString(jsonString + begin, &context->Buffer->Fps);


      context->State = Waiting;


      break;
    }


    // Read loop.
    case ReadingLoop:
    {
      context->Buffer->Loop = (type == csmJsonTrue);


      context->State = Waiting;


      break;
    }


    // Read curve count.
    case ReadingCurveCount:
    {
      ReadIntFromString(jsonString + begin, &context->Buffer->CurveCount);


      context->State = Waiting;


      break;
    }


    // Read segment count.
    case ReadingTotalSegmentCount:
    {
      ReadIntFromString(jsonString + begin, &context->Buffer->TotalSegmentCount);


      context->State = Waiting;


      break;
    }


    // Read point count.
    case ReadingTotalPointCount:
    {
      ReadIntFromString(jsonString + begin, &context->Buffer->TotalPointCount);


      context->State = Waiting;


      break;
    }


    // Read bézier restriction.
    case ReadingAreBeziersRestricted:
    {
      context->Buffer->AreBeziersRestricted = (type == csmJsonTrue);


      context->State = Waiting;


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


/// Parses motion data from a serialized motion3.json.
///
/// @param  jsonString         Serialized motion.
/// @param  type               Current JSON token.
/// @param  end                Begin of JSON token (in chars).
/// @param  begin              End of JSON token (in chars).
/// @param  metaParserContext  Parser context.
static int ParseMotion3(const char* jsonString, csmJsonTokenType type, int begin, int end, void* motionParserContext)
{
  MetaParserContext metaParserContext;
  MotionParserContext* context;
  int segmentType;


  // Recover context.
  context = motionParserContext;


  // Initialize meta related fields if necessary.
  if (!context->Buffer->Curves)
  {
    // Parse meta.
    InitializeMetaParserContext(&metaParserContext, &context->Meta);
    csmLexJson(jsonString, ParseMeta3, &metaParserContext);


    // Initialize data fields.
    context->Buffer->Duration = context->Meta.Duration;
    context->Buffer->Loop = (short)context->Meta.Loop;

    context->Buffer->CurveCount = (short)context->Meta.CurveCount;


    // Initialize pointer fields.
    context->Buffer->Curves = (csmAnimationCurve*)(context->Buffer + 1);
    context->Buffer->Segments = (csmAnimationSegment*)(context->Buffer->Curves + context->Meta.CurveCount);
    context->Buffer->Points = (csmAnimationPoint*)(context->Buffer->Segments + context->Meta.TotalSegmentCount);
  }


  // Handle token.
  switch (context->State)
  {
    // Wait for curves section.
    case Pending:
    {
      // Skip non-names.
      if (type != csmJsonName)
      {
        ;
      }


      else if (DoesStringStartWith(jsonString + begin, "Curves"))
      {
        context->State = Waiting;
      }


      break;
    }


    // Start reading curve or finalize parsing.
    case Waiting:
    {
      // Stop parsing at last curve.
      if (type == csmJsonArrayEnd)
      {
        context->State = FinishedParsing;
      }


      // Start parsing curve.
      else if (type == csmJsonObjectBegin)
      {
        // Initialize curve fields.
        context->Buffer->Curves[context->CurveIndex].BaseSegmentIndex = context->SegmentIndex;


        // Prepare context.
        context->State = ReadingCurve;
      }


      break;
    }


    // Handle parsing of a single curve.
    case ReadingCurve:
    {
      // End curve parsing.
      if (type == csmJsonObjectEnd)
      {
        // Finalize curve fields.
        context->Buffer->Curves[context->CurveIndex].SegmentCount = context->SegmentIndex - context->Buffer->Curves[context->CurveIndex].BaseSegmentIndex;


        // Update context.
        context->CurveIndex += 1;

        context->State = Waiting;
      }


      // Prepare context for reading curve data.
      else
      {
        if (DoesStringStartWith(jsonString + begin, "Target"))
        {
          context->State = ReadingTarget;
        }
        else if (DoesStringStartWith(jsonString + begin, "Id"))
        {
          context->State = ReadingId;
        }
        else if (DoesStringStartWith(jsonString + begin, "Segments"))
        {
          context->SegmentValueIndex = 0;
          context->SegmentTypePosition = 2;

          context->ReadPointTime = 1;

          context->State = ReadingSegments;
        }
      }


      break;
    }


    // Read curve target.
    case ReadingTarget:
    {
      if (DoesStringStartWith(jsonString + begin, "Model"))
      {
        context->Buffer->Curves[context->CurveIndex].Type = csmModelAnimationCurve;
      }
      else if (DoesStringStartWith(jsonString + begin, "Parameter"))
      {
        context->Buffer->Curves[context->CurveIndex].Type = csmParameterAnimationCurve;
      }
      else if (DoesStringStartWith(jsonString + begin, "PartOpacity"))
      {
        context->Buffer->Curves[context->CurveIndex].Type = csmPartOpacityAnimationCurve;
      }

      
      // Allow parsing of other curve data.
      context->State = ReadingCurve;


      break;
    }


    // Read curve id.
    case ReadingId:
    {
      // Hash ID..
      if (context->Buffer->Curves[context->CurveIndex].Type != csmModelAnimationCurve)
      {
        context->Buffer->Curves[context->CurveIndex].Id = csmHashIdFromSubString(jsonString, begin, end);
      }


      // ... Assign model curve target.
      else
      {
        if (DoesStringStartWith(jsonString + begin, "Opacity"))
        {
          context->Buffer->Curves[context->CurveIndex].Id = csmOpacityAnimationCurve;
        }
        else if (DoesStringStartWith(jsonString + begin, "EyeBlink"))
        {
          context->Buffer->Curves[context->CurveIndex].Id = csmEyeBlinkAnimationCurve;
        }
        else if (DoesStringStartWith(jsonString + begin, "LipSync"))
        {
          context->Buffer->Curves[context->CurveIndex].Id = csmLipSyncAnimationCurve;
        }
      }


      // Allow parsing of other curve data.
      context->State = ReadingCurve;


      break;
    }


    // Read curve segments.
    case ReadingSegments:
    {
      // Finalize segment parsing.
      if (type == csmJsonArrayEnd)
      {
        context->State = ReadingCurve;
      }


      else
      {
        // Skip array token.
        if (type == csmJsonArrayBegin)
        {
        }


        // Read segment type.
        else if (context->SegmentValueIndex == context->SegmentTypePosition)
        {
          // Read data.
          ReadIntFromString(jsonString + begin, &segmentType);


          switch (segmentType)
          {
            case LinearSegment:
            {
              context->Buffer->Segments[context->SegmentIndex].Evaluate = csmLinearAnimationSegmentEvaluationFunction;


              break;
            }
            case BezierSegment:
            {
              context->Buffer->Segments[context->SegmentIndex].Evaluate = csmBezierAnimationSegmentEvaluationFunction;


              break;
            }
            case SteppedSegment:
            {
              context->Buffer->Segments[context->SegmentIndex].Evaluate = csmSteppedAnimationSegmentEvaluationFunction;


              break;
            }
            case InverseSteppedSegment:
            {
              context->Buffer->Segments[context->SegmentIndex].Evaluate = csmInverseSteppedAnimationSegmentEvaluationFunction;


              break;
            }
          }


          context->Buffer->Segments[context->SegmentIndex].BasePointIndex = (context->PointIndex - 1);

          context->Buffer->Curves[context->CurveIndex].SegmentCount += 1;

          // Update context.
          context->SegmentValueIndex += 1;
          context->SegmentTypePosition += ((segmentType == BezierSegment)
            ? 7
            : 3);

          context->SegmentIndex += 1;
        }


        // Read point data.
        else
        {
          // Read time...
          if (context->ReadPointTime)
          {
            ReadFloatFromString(jsonString + begin, &context->Buffer->Points[context->PointIndex].Time);
          }


          // ... or value.
          else
          {
            ReadFloatFromString(jsonString + begin, &context->Buffer->Points[context->PointIndex].Value);


            // Update context.
            context->PointIndex += 1;
          }


          // Update context.
          context->SegmentValueIndex += 1;

          context->ReadPointTime = !context->ReadPointTime;
        }
      }


      break;
    }


    default:
    {
      break;
    }
  }


  return context->State != FinishedParsing;
}


// ------------------- //
// MOTION JSON PARSING //
// ------------------- //

/// Available motion meta readers.
static csmJsonTokenHandler MetaParsers[] =
{
  0,
  0,
  0,
  ParseMeta3
};

/// Available motion readers.
static csmJsonTokenHandler MotionParsers[] =
{
  0,
  0,
  0,
  ParseMotion3
};


// -------------- //
// IMPLEMENTATION //
// -------------- //

void ReadMotionJsonMeta(const char* motionJson, MotionJsonMeta* buffer)
{
  VersionParserContext versionParserContext;
  MetaParserContext context;
  int version;


  // Get version info.
  InitializeVersionParserContext(&versionParserContext, &version);
  csmLexJson(motionJson, ParseVersion, &versionParserContext);


  // Parse meta matching version.
  InitializeMetaParserContext(&context, buffer);
  csmLexJson(motionJson, MetaParsers[version], &context);
}

void ReadMotionJson(const char* motionJson, csmAnimation* buffer)
{
  VersionParserContext versionParserContext;
  MotionParserContext context;
  int version;


  // Get version info.
  InitializeVersionParserContext(&versionParserContext, &version);
  csmLexJson(motionJson, ParseVersion, &versionParserContext);


  // Parse matching version.
  InitializeMotionParserContext(&context, buffer);
  csmLexJson(motionJson, MotionParsers[version], &context);


  // TODO Log warning in case curves aren't restricted.
}
