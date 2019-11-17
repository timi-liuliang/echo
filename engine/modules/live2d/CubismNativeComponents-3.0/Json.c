/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * 
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at http://live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "Live2DCubismFrameworkINTERNAL.h"


// ------- //
// HELPERS //
// ------- //

/// JSON 'true' token as string.
static const char TrueStr[] = "true";

/// JSON 'false' token as string.
static const char FalseStr[] = "false";

/// JSON 'null' token as string.
static const char NullStr[] = "null";


/// Value identifying invalid token.
static const csmJsonTokenType InvalidToken = csmJsonTokenTypeCount;


// -------------- //
// IMPLEMENTATION //
// -------------- //

void csmLexJson(const char* jsonString, csmJsonTokenHandler onToken, void* userData)
{
  int tokenBegin, tokenEnd, lex;
  csmJsonTokenType tokenType;
  const char* string, * base;


  // Initialize locals for lexing.
  tokenType = InvalidToken;
  string = jsonString;
  base = jsonString;


  // Lex.
  for (lex = 1; *string != '\0' && lex; ++string)
  {
    switch (*string)
    {
      case '{':
      {
        tokenType = csmJsonObjectBegin;
        tokenBegin = (int)(string - base);
        tokenEnd = tokenBegin + 1;


        break;
      }
      case '}':
      {
        tokenType = csmJsonObjectEnd;
        tokenBegin = (int)(string - base);
        tokenEnd = tokenBegin + 1;


        break;
      }
      case '[':
      {
        tokenType = csmJsonArrayBegin;
        tokenBegin = (int)(string - base);
        tokenEnd = tokenBegin + 1;


        break;
      }
      case ']':
      {
        tokenType = csmJsonArrayEnd;
        tokenBegin = (int)(string - base);
        tokenEnd = tokenBegin + 1;


        break;
      }
      case '"':
      {
        tokenBegin = (int)(string - base) + 1;
        

        for (++string; *string != '"'; ++string)
        {
          ;
        }


        tokenEnd = (int)(string - base);


        tokenType = (*(string + 1) == ':')
          ? csmJsonName
          : csmJsonString;


        break;
      }
      case 't':
      {
        tokenType = csmJsonTrue;
        tokenBegin = (int)(string - base);
        tokenEnd = tokenBegin + (int)sizeof(TrueStr);


        string += (tokenEnd - tokenBegin);


        break;
      }
      case 'f':
      {
        tokenType = csmJsonFalse;
        tokenBegin = (int)(string - base);
        tokenEnd = tokenBegin + (int)sizeof(FalseStr);


        string += (tokenEnd - tokenBegin);


        break;
      }
      case 'n':
      {
        tokenType = csmJsonNull;
        tokenBegin = (int)(string - base);
        tokenEnd = tokenBegin + (int)sizeof(NullStr);


        string += (tokenEnd - tokenBegin);


        break;
      }
      default:
      {
        if ((*string >= '0' && *string <= '9') || *string == '-')
        {
          tokenType = csmJsonNumber;
          tokenBegin = (int)(string - base);


          for (; (*string >= '0' && *string <= '9') || *string == '-' || *string == '.'; ++string)
          {
            ;
          }


          tokenEnd = (int)(string - base);
        }


        break;
      }
    }


    // Do callback if token type is valid.
    if (tokenType != InvalidToken)
    {
      lex = onToken(jsonString, tokenType, tokenBegin, tokenEnd, userData);
    }


    // Reset condition for callback.
    tokenType = InvalidToken;
  }
}
