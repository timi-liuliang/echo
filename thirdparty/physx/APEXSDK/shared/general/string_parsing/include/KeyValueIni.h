/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef KEY_VALUE_INI_H
#define KEY_VALUE_INI_H

#include "Ps.h"

namespace physx
{
	namespace general_string_parsing2
	{

class KeyValueIni;
class KeyValueSection;


/*!
 * \brief
 * Write loadKeyValueIni loads an INI file by filename and returns an opaque handle to the interface.  If null, then the file was not found or was empty.
 *
 * \param fname
 * The name of the file on disk to load.
 *
 * \param sections
 * A reference to an unsigned integer that will report the number of init sections in the file.  A special init section called '@HEADER' contains values that were found before any section block.
 *
 * \returns
 * Returns an opaque pointer to the KeyValueIni interface.
 *
 * Write detailed description for loadKeyValueIni here.
 *
 * Loads .INI files that can be separated into sections using the bracket keys.  Example [RENDER] [PHYSICS] etc.  Supports key value pairs in the form
 * of KEY=VALUE or it will support a single KEY entry with no accompanying value.  Keys encountered prior to encountering any section are placed in the
 * default section of '@HEADER'
 *
 * Note that if you have a section called [RENDER] and then a section called [MESH] but later refer to [RENDER] again, this does *not* create a new section, it
 * adds the keys to the previously defined section.
 *
 * Note also that all returned pointers are persistent up until the KeyValueIni file is released.  That means you can avoid string copies and instead only cache the
 * pointers internally so long as you keep the INI file loaded.
 */
KeyValueIni *     loadKeyValueIni(const char *fname,PxU32 &sections);

/*!
 * \brief
 * Loads an INI file from a block of memory rather than a file on disk.
 *
 * \param mem
 * An address in memory pointing to the text file containing initialization data.
 *
 * \param len
 * The length of the input data.
 *
 * \param sections
 * A reference that is filled with the number of sections that were encountered.
 *
 * \returns
 * Returns an opaque pointer to the KeyValueIni interface.
 *
 * Write detailed description for loadKeyValueIni here.
 *
 * The memory passed in is copied internally so the caller does not need to keep it around.  There are no memory copies in this system, the initialization data
 * points back to the original source content in memory.  This system is extremely fast.
 *
 * Comment symbols for INI files are '#' '!' and ';'
 */
KeyValueIni *     loadKeyValueIni(const char *mem,PxU32 len,PxU32 &sections);

/*!
 * \brief
 * Locates a section by name.
 *
 * \param ini
 * The KeyValueIni pointer from a previously loaded INI file.
 *
 * \param section
 * The name of the initialization section you want to located.
 *
 * \param keycount
 * A reference that will be assigned the number of keys in this section.
 *
 * \param lineno
 * A reference to the line number in the original INI file where this key is found.
 *
 * \returns
 * Returns an opaque pointer to the corresponding section.
 *
 */
const KeyValueSection * locateSection(const KeyValueIni *ini,const char *section,PxU32 &keycount,PxU32 &lineno);

/*!
 * \brief
 * gets a specific section by array index.  If the index is out of range it will return a null.
 *
 * \param ini
 * The opaque pointer to a previously loaded INI file
 *
 * \param index
 * The array index (zero based) for the corresponding section.
 *
 * \param keycount
 * A reference that will be assigned the number of keys in this section.
 *
 * \param lineno
 * A reference that will be assigned the line number where this section was defined in the original source data.
 * 
 * \returns
 * Returns an opaque pointer to the corresponding section, or null if the array index is out of ragne.
 */
const KeyValueSection * getSection(const KeyValueIni *ini,PxU32 index,PxU32 &keycount,PxU32 &lineno);

/*!
 * \brief
 * Returns the ASCIIZ name field for a particular section.
 * 
 * \param section
 * The previously obtained opaque KeyValueSection pointer.
 * 
 * \returns
 * Returns the ASCIIZ name of the section.
 * 
 */
const char *            getSectionName(const KeyValueSection *section);

/*!
 * \brief
 * locates the value for a particular Key/Value pair (i.e. KEY=VALUE)
 * 
 * \param section
 * The opaque pointer to the KeyValueSection to query.
 * 
 * \param key
 * The name of the key we are searching for.
 * 
 * \param lineno
 * A reference that will be assigned to the line number in the original INI file that this key was found.
 * 
 * \returns
 * Returns a pointer to the value component.  If this is null it means only a key was found when the file was parsed, no corresponding value was encountered.
 * 
 */
const char *      locateValue(const KeyValueSection *section,const char *key,PxU32 &lineno);

/*!
 * \brief
 * Gets a key by array index.
 * 
 * \param section
 * The pointer to the KeyValueSection we are indexing.
 * 
 * \param keyindex
 * The array index for the key we are requesting.
 * 
 * \param lineno
 * A reference that will be assigned the linenumber in the original text file where the key was found.
 *
 * \returns
 * Returns the key found at this location.
 */
const char *      getKey(const KeyValueSection *section,PxU32 keyindex,PxU32 &lineno);

/*!
 * \brief
 * Returns the value field based on an array index.
 *
 * \param section
 * A pointer to the KeyValueSection we are searching.
 *
 * \param keyindex
 * The array index to look the value up against.
 *
 * \param lineno
 * A reference to the line number in the original file where this key was located.
 *
 * \returns
 * Returns a pointer to the value at this entry or a null if there was only a key but no value.
 *
 */
const char *      getValue(const KeyValueSection *section,PxU32 keyindex,PxU32 &lineno);

/*!
 * \brief
 * This method releases the previously loaded and parsed INI file.  This frees up memory and any previously cached pointers
 * will no longer be valid.
 *
 * \param ini
 * The pointer to the previously loaded INI file.
 *
 */
void              releaseKeyValueIni(const KeyValueIni *ini);

bool              saveKeyValueIni(const KeyValueIni *ini,const char *fname);
void *            saveKeyValueIniMem(const KeyValueIni *ini,PxU32 &len); // save it to a buffer in memory..
bool              releaseIniMem(void *mem);

KeyValueIni      *createKeyValueIni(void); // create an empty .INI file in memory for editing.

KeyValueSection  *createKeyValueSection(KeyValueIni *ini,const char *section_name,bool reset);  // creates, or locates and existing section for editing.  If reset it true, will erase previous contents of the section.
bool              addKeyValue(KeyValueSection *section,const char *key,const char *value); // adds a key-value pair.  These pointers *must* be persistent for the lifetime of the INI file!


}; // end of namespace
using namespace general_string_parsing2;
};

#endif // KEY_VALUE_INI_H
