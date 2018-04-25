/*
* Copyright(c) Live2D Inc. All rights reserved.
*
* Use of this source code is governed by the Live2D Open Software license
* that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
*/


// -------- //
// REQUIRES //
// -------- //

#include <Live2DCubismCore.h>
#include <math.h>

#include "Local.h"


// --------- //
// CONSTANTS //
// --------- //

/// Constant of Pi.
const float PI = 3.14159f;


// -------------- //
// IMPLEMENTATION //
// -------------- //

/// Gets length of vector.
///
/// @param  vector  Target vector.
///
/// @return  Length of vector.
static float Length(csmVector2 vector)
{
  return (float)sqrt((vector.X * vector.X) + (vector.Y * vector.Y));
}

/// Calculates dot product.
///
/// @param  vector1  Vector1.
/// @param  vector2  Vector2.
///
/// @return  Dot product for vector1 and vector2.
static float Dot(csmVector2 vector1, csmVector2 vector2)
{
  return (vector1.X * vector2.X) + (vector1.Y * vector2.Y);
}

/// Gets maximum.
///
/// @param  l  Left-hand value.
/// @param  r  Right-hand value.
///
/// @return  Maximum value.
static float Max(float l, float r)
{
  return (l > r) ? l : r;
}

/// Gets minimum.
///
/// @param  l  Left-hand value.
/// @param  r  Right-hand value.
///
/// @return  Minimum value.
static float Min(float l, float r)
{
  return (l > r) ? r : l;
}

/// Gets sign.
///
/// @param  value  Evaluation target value.
///
/// @return  Sign of value.
static int Sign(float value)
{
  int ret;

  ret = 0;

  if (value > 0.0f)
  {
    ret = 1;
  }
  else if (value < 0.0f)
  {
    ret = -1;
  }

  return ret;
}

csmVector2 MakeVector2(float x, float y)
{
  csmVector2 ret;

  ret.X = x;
  ret.Y = y;

  return ret;
}

csmVector2 AddVector2(csmVector2 a, csmVector2 b)
{
  return MakeVector2(a.X + b.X, a.Y + b.Y);
}

csmVector2 SubVector2(csmVector2 a, csmVector2 b)
{
  return MakeVector2(a.X - b.X, a.Y - b.Y);
}

csmVector2 MultiplyVector2(csmVector2 a, csmVector2 b)
{
  return MakeVector2(a.X * b.X, a.Y * b.Y);
}

csmVector2 MultiplyVectoy2ByScalar(csmVector2 v, float s)
{
  return MakeVector2(v.X * s, v.Y * s);
}

csmVector2 DivideVector2(csmVector2 a, csmVector2 b)
{
  return MakeVector2(a.X / b.X, a.Y / b.Y);
}

csmVector2 DivideVector2ByScalar(csmVector2 v, float s)
{
  return MakeVector2(v.X / s, v.Y / s);
}

float Distance(csmVector2 a, csmVector2 b)
{
  return Length(SubVector2(a, b));
}

void Normalize(csmVector2* target)
{
  float length;

  length = powf((target->X * target->X) + (target->Y * target->Y), 0.5f);

  target->X = target->X / length;
  target->Y = target->Y / length;
}

float DegreesToRadian(float degrees)
{
  return (degrees / 180.0f) * PI;
}

float RadianToDegrees(float radian)
{
  return (radian * 180.0f) / PI;
}

float DirectionToRadian(csmVector2 from, csmVector2 to)
{
  float dotProduct;
  float magnitude;


  dotProduct = Dot(from, to);
  magnitude = Length(from) * Length(to);

  if (magnitude == 0.0f)
  {
    return 0.0f;
  }


  float cosTheta = (dotProduct / magnitude);

  if (fabs(cosTheta) > 1.0)
  {
    return 0.0f;
  }


  float theta = (float)acos(cosTheta);

  return theta;
}

float DirectionToDegrees(csmVector2 from, csmVector2 to)
{
  float radian;
  float degree;


  radian = DirectionToRadian(from, to);
  degree = (float)RadianToDegrees(radian);

  if ((to.X - from.X) > 0.0f)
  {
    degree = -degree;
  }


  return degree;
}

csmVector2 RadianToDirection(float totalAngle)
{
  csmVector2 ret;


  ret.X = (float)sin(totalAngle);
  ret.Y = (float)cos(totalAngle);


  return ret;
}

float GetRangeValue(float min, float max)
{
  float maxValue = Max(min, max);
  float minValue = Min(min, max);
  return (float)fabs(maxValue - minValue);
}

float GetDefaultValue(float min, float max)
{
  float minValue = Min(min, max);
  return minValue + (GetRangeValue(min, max) / 2.0f);
}

float NormalizeParameterValue(
  float value,
  float parameterMinimum,
  float parameterMaximum,
  float parameterDefault,
  float normalizedMinimum,
  float normalizedMaximum,
  float normalizedDefault,
  int isInverted)
{
  float result = 0.0f;

  float maxValue = Max(parameterMaximum, parameterMinimum);

  if (maxValue < value)
  {
    return result;
  }

  float minValue = Min(parameterMaximum, parameterMinimum);

  if (minValue > value)
  {
    return result;
  }

  float minNormValue = Min(normalizedMinimum, normalizedMaximum);
  float maxNormValue = Max(normalizedMinimum, normalizedMaximum);
  float middleNormValue = normalizedDefault;

  float middleValue = GetDefaultValue(minValue, maxValue);
  float paramValue = value - middleValue;

  switch (Sign(paramValue))
  {
    case 1:
    {
      float nLength = maxNormValue - middleNormValue;
      float pLength = maxValue - middleValue;
      if (pLength != 0.0f)
      {
        result = paramValue*(nLength / pLength);
        result += middleNormValue;
      }


      break;
    }
    case -1:
    {
      float nLength = minNormValue - middleNormValue;
      float pLength = minValue - middleValue;
      if (pLength != 0.0f)
      {
        result = paramValue*(nLength / pLength);
        result += middleNormValue;
      }


      break;
    }
    case 0:
    {
      result = middleNormValue;


      break;
    }
  }

  return (isInverted)
          ? result
          : (result * -1.0f);
}

