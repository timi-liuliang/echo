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

/// Computes length of a string by looking for its null-terminator.
///
/// @param  string      Null-terminated string to measure.
/// @param  iterations  Maximum number of iterations.
///
/// @return  Length of string (or iterations).
static int GetStringLength(const char* string, const int iterations)
{
  int c;


  for (c = 0; c < iterations && string[c] != '\0'; ++c)
  {
    ;
  }


  return c;
}


// -------------- //
// IMPLEMENTATION //
// -------------- //

csmHash csmHashId(const char* id)
{
  // Validate argument.
  Ensure(id, "\"id\" is invalid.", return 0);


  return csmHashIdFromSubString(id, 0, GetStringLength(id, 64));
}

// INV  Is algorithm sufficient for its purpose?
csmHash csmHashIdFromSubString(const char* string, const int idBegin, const int idEnd)
{
  csmHash hash;
  int c;


  // Validate arguments.
  Ensure(string, "\"string\" is invalid.", return 0);
  Ensure((idEnd > idBegin), "\"idBegin\" is bigger than \"idEnd\".", return 0);


  for (hash = 0, c = idBegin; c < idEnd; ++c)
  {
    hash = (hash * 13) + (unsigned char)string[c];
  }


  return hash;
}


unsigned int csmGetSizeofModelHashTable(const csmModel* model)
{
  int valueCount;


  // Validate argument.
  Ensure(model, "\"model\" is invalid.", return 0);


  valueCount  = csmGetParameterCount(model);
  valueCount += csmGetPartCount(model);


  return (unsigned int)(sizeof(csmModelHashTable) + (sizeof(csmHash) * valueCount));
}

csmModelHashTable* csmInitializeModelHashTableInPlace(const csmModel* model, void* address, const unsigned int size)
{
  csmModelHashTable* table;
  int i;


  // Validate arguments.
  Ensure(model, "\"model\" is invalid.", return 0);
  Ensure(address, "\"address\" is invalid.", return 0);
  Ensure((size >= csmGetSizeofModelHashTable(model)), "\"size\" is invalid.", return 0);


  table = address;


  // Initialize parameters table.
  table->Parameters.Count = csmGetParameterCount(model);
  table->Parameters.IdHashes = (csmHash*)(table + 1);


  for (i = 0; i < table->Parameters.Count; ++i)
  {
    table->Parameters.IdHashes[i] = csmHashId(csmGetParameterIds(model)[i]);
  }


  // Initialize parts table.
  table->Parts.Count = csmGetPartCount(model);
  table->Parts.IdHashes = table->Parameters.IdHashes + table->Parameters.Count;


  for (i = 0; i < table->Parts.Count; ++i)
  {
    table->Parts.IdHashes[i] = csmHashId(csmGetPartIds(model)[i]);
  }


  return table;
}


int csmFindParameterIndexByHash(const csmModel* model, const csmHash hash)
{
  csmHash c;
  int i;


  // Validate argument.
  Ensure(model, "\"model\" is invalid.", return 0);


  for (i = 0; i < csmGetParameterCount(model); ++i)
  {
    c = csmHashId(csmGetParameterIds(model)[i]);


    if (hash != c)
    {
      continue;
    }


    return i;
  }


  return -1;
}

int csmFindParameterIndexByHashFAST(const csmModelHashTable* table, const csmHash hash)
{
  int h;


  // Validate argument.
  Ensure(table, "\"table\" is invalid.", return 0);


  for (h = 0; h < table->Parameters.Count; ++h)
  {
    if (hash != table->Parameters.IdHashes[h])
    {
      continue;
    }


    return h;
  }


  return -1;
}

int csmFindPartIndexByHash(const csmModel* model, const csmHash hash)
{
  csmHash c;
  int i;


  // Validate argument.
  Ensure(model, "\"model\" is invalid.", return 0);


  for (i = 0; i < csmGetPartCount(model); ++i)
  {
    c = csmHashId(csmGetPartIds(model)[i]);


    if (hash != c)
    {
      continue;
    }


    return i;
  }


  return -1;
}

int csmFindPartIndexByHashFAST(const csmModelHashTable* table, const csmHash hash)
{
  int h;


  // Validate argument.
  Ensure(table, "\"table\" is invalid.", return 0);


  for (h = 0; h < table->Parts.Count; ++h)
  {
    if (hash != table->Parts.IdHashes[h])
    {
      continue;
    }


    return h;
  }


  return -1;
}

int csmFindDrawableIndexByHash(const csmModel* model, const csmHash hash)
{
  csmHash c;
  int i;


  // Validate argument.
  Ensure(model, "\"model\" is invalid.", return 0);


  for (i = 0; i < csmGetDrawableCount(model); ++i)
  {
    c = csmHashId(csmGetDrawableIds(model)[i]);


    if (hash != c)
    {
      continue;
    }


    return i;
  }


  return -1;
}


int csmDoesModelUseMasks(const csmModel* model)
{
  int d;


  // Validate argument.
  Ensure(model, "\"model\" is invalid.", return 0);


  for (d = 0; d < csmGetDrawableCount(model); ++d)
  {
    if (csmGetDrawableMaskCounts(model)[d] <= 0)
    {
      continue;
    }


    return 1;
  }


  return 0;
}
