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

#include <stdlib.h>


// -------------- //
// IMPLEMENTATION //
// -------------- //

void ReadFloatFromString(const char* string, float* buffer)
{
  *buffer = (float)atof(string);
}

void ReadIntFromString(const char* string, int* buffer)
{
  *buffer = atoi(string);
}


int DoesStringStartWith(const char* string, const char* expected)
{
  for (; *string != '\0' && *expected != '\0'; ++string, ++expected)
  {
    if (*string != *expected)
    {
      return 0;
    }
  }


  return 1;
}
