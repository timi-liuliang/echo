//**************************************************************************/
// Copyright (c) 2010 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/

#pragma once

#include "..\ScripterExport.h"

// forward declarations
#include "..\..\strclass.h"
class HashTable;

/** 
 * \brief Locates the resource file for the specified script file name.
 * The filename portion of the resource file name is the script file name plus a ".res"
 * extension ([script.res] = <scriptname>.res). 
 * The resource file is looked for in the following locations, in the order given:
 * \list
 * \li The directory the script file is in ([scriptdir]\[script.res])
 * \li A subdirectory of the directory the script file is in, where the subdirectory name
 *     is the localization name (en-US, fr-FR, ja-JP, etc.) ([scriptdir]\<lang>\[script.res])
 * \li IFF script is in install tree, the localized shadow directory under the install 
 *     directory ([installdir]\<lang>\[shadow tree]\[script.res])
 * \li The en-US subdirectory of the directory the script file is in ([scriptdir]\en-US\[script.res])
 * \li IFF script is in install tree, the english shadow directory under the install 
 *     directory ([installdir]\en-US\[shadow tree]\[script.res])
 * \endlist
 * \param[in] scriptFileName - The fully qualified script file name
 * \param[out] resourceFileName - The resource file name if found
 * \return True if the resource file name was located.
*/
ScripterExport bool LocateScriptResourceFile(const MCHAR* scriptFileName, MSTR& resourceFileName);

/** 
 * \brief Loads the specified resource file.
 * Each line in the resource file consists of the resource id and the resource value. This 
 * creates and returns a HashTable where the resource id is the key and the resource value
 * is the value. If the resource file does not exist, NULL will be returned.
 * \param resourceFileName - The fully qualified resource file name
 * \return Pointer to HashTable containing the resource id/resource value pairs, or NULL if 
 * the resource file doesn't exist.
 Note: this function will throw a MAXScriptException if an error occurs while processing the
 resource file
*/
ScripterExport HashTable* LoadScriptResourceFile(const MCHAR* resourceFileName);

/** 
 * \brief Returns the system's localization language and country name.
 * The system's localization language and country name are determined by querying the system for
 * its ISO languange and country locale names.
 * \param[in] user_default_locale - If true, default locale for the user is returned. If false, default locale
 * for the system is returned.
 * \return The system's 2-letter localization language and country names in the form <language>-<country>, for
 * example, 'en-US'.
*/
ScripterExport TSTR GetSystemLocaleName(bool user_default_locale);

/** 
 * \brief Returns Max's localization language and country name.
 * Max's localization language and country name are determined by querying Max for its language id and returning
 * the appropriate ISO language and country locale name.
 * \return Max's 2-letter localization language and country names in the form <language>-<country>, for
 * example, 'en-US'.
 Note: the return value will be one of the following: "ja-JP", "zh-CN", "ko-KR", "de-DE", "fr-FR", "en-US".
*/
ScripterExport TSTR GetMaxLocaleName();
