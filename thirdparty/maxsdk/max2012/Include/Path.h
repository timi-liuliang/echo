//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Path class
// AUTHOR: Nicolas Desjardins
// DATE: 2006-02-27 
//***************************************************************************/

#ifndef _MAXSDK_UTIL_PATH_H_
#define _MAXSDK_UTIL_PATH_H_


#include "maxheap.h"
#include "utilexp.h"
#include "strclass.h"
#include "maxversion.h"
#include "autoptr.h"

namespace MaxSDK
{
namespace Util
{
/*!
 * \brief Path type - adds path-operation richness over symbol strings.
 *
 * The Path interface provides operations for path composition (Append and 
 * Prepend) and decomposition (StripToParent, StripToChild, RemoveParent, 
 * RemoveChild), query methods for determining information about a Path,
 * such as whether it's absolute, relative, a root or a UNC, and finally,
 * quick conversions from relative to absolute or absolute to relative.
 *
 * Note paths are automatically kept in a standard form, where:
 * \li all slashes '/' are converted to backslashes '\\'.
 * \li multiple backslashes in a row are collapsed down to one - '\\\\\\' becomes 
 *    '\\'
 *
 * Normalization is a separate, explicit method that must be called by the 
 * client.  Normalization removes null directories '\\.\\' and adds in any missing
 * backslash after the drive letter. Wherever possible, normalization will 
 * resolve out any ".." path components so: "r:\\dirA\\dirB\\..\\dirC" becomes 
 * "r:\\dirA\\dirC."  If these hit the root, they just get stripped off, so 
 * "r:\\.." normalizes to "r:\\" If the path is relative, and there are more ".."s
 *  than preceding directories, * the extra sets are kept at the start of the 
 * path: "b\\..\\..\\c" becomes "..\\c"  Any trailing backslash is kept on the path
 * for compatibility with many win32 API functions.  If a uniform 
 * trailing-backslash format is desired, for example, for display in the UI, it
 * is up to client-level code to implement this policy. Unresolved symbols are 
 * left intact during normalization.  Resolving the symbols in a string
 * normalizes automatically.
 *
 * To reiterate, normalization as performed by the Normalize() method is 
 * different from the basic standard form maintained by all members of Path.
 * The client must call Normalize() explicitly (or ResolveSymbols() which is 
 * the only method that calls Normalize() automatically).  The reason that these
 * are kept separate is to keep from radically modifying a path in a way that
 * may be surprising to a user.  If the user wishes to construt a path with 
 * many '..\\'s in it, the user will expect to see this result.  Normalize()
 * may then be called subsequently when it is deemed appropriate.
 * 
 * To illustrate the difference:
 * \li "/a/b/c/./../" is neither in standard form nor normalized.
 * \li "\\a\b\\c\\.." and "\\a\b\\c\\." are in standard form but not normalized.
 * \li "\\a\b" and "\\a\b\\c" are both in standard form and normalized.
 */
class Path : public MaxHeapOperators
{
private:
	class PathImpl;
	const AutoPtr<PathImpl> mImpl;

public:

	/*!
	 * \brief Default constructor - creates an empty path.
	 *
	 * \post IsEmpty() returns true.
	 */
	UtilExport Path();

	/*!
	 * \brief Construct a path from a path-filename string.
	 * 
	 * \param pathString path & filename as a string from which to construct
	 *     a path object.
	 */
	UtilExport Path(const MSTR &pathString);

	/*!
	 * \brief Construct from a null-terminated path-filename MCHAR array.
	 *
	 * \param pathString path & filename (must be null-terminated).
	 * \pre pathString is null terminated.
	 */
	UtilExport Path(const MCHAR *pathString);

	/*!
	 * \brief Copy construct a Path from another. 
	 *
	 * This performs a deep copy.  Subsequent operations on this path will not 
	 * affect the copied path and vice versa.
	 * \param other The path to copy.
	 */
	UtilExport Path(const Path &other);

	/*! 
	 * \brief Destructor.
	 */
	UtilExport ~Path();

	/*!
	 * \brief Set this Path from another.
	 *
	 * Equivalent to the assignment operator, but more verbose.
	 * \param other - path to copy.
	 */
	UtilExport Path& SetPath(const Path &other);

	/*!
	 * \brief Set this Path's internal string from a null terminated character 
	 * string.
	 *
	 * Equivalent to the assignment operator, but more verbose.
	 * \param pathString null-terminated character string to copy into this path. 
	 */
	UtilExport Path& SetPath(const MCHAR *pathString);

	/*!
	 * \brief Set this Path from a MSTR.
	 *
	 * Equivalent to the assignment operator, but more verbose.
	 * \param pathString MSTR to copy into this path.
	 */
	UtilExport Path& SetPath(const MSTR &pathString);

	/*!
	 * \brief Assignment operator.
	 *
	 * \param other - path to copy
	 */
	UtilExport Path& operator=(const Path &other);

	/*!
	 * \brief Assignment operator taking a null terminated character 
	 * string.
	 *
	 * \param pathString null-terminated character string to copy into this path.
	 */
	UtilExport Path& operator=(const MCHAR *pathString);

	/*!
	 * \brief Assignment operator.
	 *
	 * \param pathString MSTR to copy into this path.
	 */
	UtilExport Path& operator=(const MSTR &pathString);
	

	/*!
	 * \brief Get whether this is an absolutely specified path.
	 *
	 * An absolute path is rooted from a starting backslash (\\), drive letter or
	 * UNC hostname.
	 *
	 * \return Returns true if this is an absolute path.
	 */
	UtilExport bool IsAbsolute() const;

	/*!
	 * \brief Get whether this is an empty path.
	 *
	 * An empty path has a backing string representation of "".
	 * \return true if this is an empty path. Returns false otherwise.
	 */
	UtilExport bool IsEmpty() const;

	/*!
	 * \brief Query whether this path doesn't have any illegal characters
	 * according to the Windows platform rules.
	 *
	 * Illegal characters are ':' for any character but the second and any 
	 * occurrence of * ? " < > or |.
	 *
	 * \return true if the path appears to be legal, false if an illegal 
	 * character was found.
	 */
	UtilExport bool IsLegal() const;

	/*!
	 * \brief Check if this path is just a root.
	 *
	 * Possible roots are:
	 * \li just backslash "\\"
	 * \li drive letter "c:\\"
	 * \li unc host "\\\\host"
	 * \li unc share "\\\\host\\share"
	 *
	 * Note that all roots are absolute paths, but not all absolute paths are 
	 * roots.
	 *
	 * The host or drive letter does not have to exist.  This path just has
	 * to fit the pattern.
	 *
	 * \return true if this path fits one of the patterns for a root outlined
	 * above, false other wise.
	 */
	UtilExport bool IsRoot() const;

	/*!
	 * \brief Query whether this path is a UNC path.
	 *
	 * A UNC path fits \\\\host or \\\\host\\share or \\\\host\\share\\subdirectory\\...
	 * The host doesn't necessarily have to exist.
	 *
	 * \return true if this path is a UNC, false otherwise.
	 */
	UtilExport bool IsUnc() const;

	/*!
	 * \brief Query whether this path fits the pattern for a UNC share.
	 *
	 * The UNC share pattern is \\\\host\\share.
	 * \return true if this path is a UNC share.  False, otherwise.
	 */
	UtilExport bool IsUncShare() const;

	/*!
	 * \brief Query whether this path is rooted at backslash, that is at the
	 * current drive with no drive letter.
	 *
    * e.g. "\\abc\\def" is rooted at backslash.
	 *
	 * \return true if the first character is '\\' and this is not a UNC.
	 */
	UtilExport bool IsRootedAtBackslash() const;

	/*!
	 * \brief Query whether this path is rooted at a drive letter.
	 *
	 * \return true if this path is rooted at a drive letter like "c:\\dir\\dir"
	 */
	UtilExport bool IsRootedAtDriveLetter() const;

	/*!
	 * \brief Query whether unresolved symbols remain in this string.
	 *
	 * \return true if there are symbols yet to be resolved, false if all 
	 * symbols have been resolved.
	 */
	UtilExport bool HasUnresolvedSymbols() const;

	/*!
	 * \brief Query with this path, when normalized, starts with ".."
	 *
	 * \return true if normalizing the path causes it to be relative to 
	 * an ancestor path - ie, it starts with ".."  False, otherwise.
	 */
	UtilExport bool StartsWithUpDirectory() const;

	/*!
	 * \brief Query whether this and the passed-in path are exactly the same.
	 *
	 * This does not perform any path resolution to try to determine if the paths
	 * point to the same file.  To be equal, the two paths must have exactly the
	 * same current representation.
	 *
	 * \param other a path to check for equality with this one.
	 * \return true if both paths are exactly the same.
	 */
	UtilExport bool IsEqual(const Path &other) const;

	/*!
	 * \brief Query whether this and the passed-in path are exactly the same.
	 *
	 * This is a convenience operator overload. It calls IsEqual() under the hood.
	 *
	 * \param other a path to check for equality with this one.
	 * \return true if both paths are exactly the same.
	 */
	UtilExport bool operator==(const Path &other) const;

	/*!
	 * \brief Query whether this and the passed-in path currently point to the 
	 * same location.
	 *
	 * This may not remain true (or false) if the paths contain symbols that may 
	 * resolve differently later on.
	 * 
	 * \param other a path to check for equivalence with this one
	 * \return true if both paths resolve out to pointing to the same location,
	 * false otherwise.
	 */
	UtilExport bool ResolvesEquivalent(const Path &other) const;

	/*!
	 * \brief Query whether this path has a trailing backslash.
	 * 
	 * \return true if this path has a trailing backslash.
	 */
	UtilExport bool HasTrailingBackslash() const;

	/*!
	 * \brief Add a backslash to the end of this path if there isn't one already.
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& AddTrailingBackslash();

	/*!
	 * \brief Remove any trailing backslash from this path.  Does nothing if 
	 * there is no trailing backslash.
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& RemoveTrailingBackslash();
	
	/*!
	 * \brief Add a new parent path to the start of this path.
	 *
	 * While the usual cleaning up is done, Normalization is not performed.
	 * No checking is performed to verify that the resulting path will be 
	 * legal.  Generally, this Path should be relative while the new parent
	 * path may be absolute or relative.
	 *
	 * Examples:
	 * \li \c Path("a\\b\\c").Prepend(Path("e\\f\\g")) produces \c Path("e\\f\\g\\a\\b\\c")
	 * \li \c Path("c:\\").Prepend(Path("e\\f\\g")) produces \c Path("e\\f\\g\\c:")
	 * \li \c Path("a\\b\\c").Prepend(Path("c:\\")) produces \c Path("c:\\a\\b\\c")
	 * \li \c Path("").Prepend(Path("e\\f\\g")) produces \c Path("e\\f\\g")
	 * \li \c Path("..\\d").Prepend(Path("e\\f\\g")) produces \c Path("e\\f\\g\\..\\d")
	 *     and normalizes to \c Path("e\\f\\d") after a call to \c Normalize()
	 *
	 * To convert a relative path to an absolute path with an arbitrary base,
	 * simply Prepend that base to this relative path.
	 *
	 * \param newParent The new parent path to place at the start of this path.
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& Prepend(const Path &newParent);

	/*!
	 * \brief Add a new child path at the end of this path.
	 *
	 * While the usual cleaning up is done, Normalization is not performed.
	 * No checking is performed to verify that the resulting path will be 
	 * legal.  Generally, the new child should be relative should be relative,
	 * while this base path may be absolute or relative.
	 *
	 * Examples:
	 * \li \c Path("a\\b\\c").Append(Path("e\\f\\g")) produces \c Path("a\\b\\c\\e\\f\\g")
	 * \li \c Path("c:\\").Append(Path("e\\f\\g")) produces \c Path("c:\\e\\f\\g")
	 * \li \c Path("a\\b\\c").Append(Path("c:\\")) produces \c Path("a\\b\\c\\c:")
	 * \li \c Path("").Append(Path("e\\f\\g")) produces \c Path("e\\f\\g")
	 * \li \c Path("a\\b\\c").Append(Path("..\\d")) produces \c Path("a\\b\\c\\..\\d")
	 *     and normalizes to \c Path("a\\b\\d") after a call to \c Normalize()
	 *
	 * \param relativeChild the new relative child component for this path.
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& Append(const Path &relativeChild);
	
	/*!
	 * \brief Strip all but the topmost parent from this path.
	 *
	 * Examples:
	 * \li \c Path("a\\b\\c").StripToTopParent() produces \c Path("a")
	 * \li \c Path("c:\\dir").StripToTopParent() produces \c Path("c:\\")
	 * \li \c Path("a").StripToTopParent() produces \c Path("a")
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& StripToTopParent();

	/*!
	 * \brief Remove the topmost parent from this path.
	 *
	 * Examples:
	 * \li \c Path("a\\b\\c").RemoveTopParent() produces \c Path("b\\c")
	 * \li \c Path("c:\\dir").RemoveTopParent() produces \c Path("dir")
	 * \li \c Path("a").RemoveTopParent() produces \c Path("")
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& RemoveTopParent();

	/*!
	 * \brief Strip all but the very last child from this path.
	 *
	 * Examples:
	 * \li \c Path("a\\b\\c").StripToLeaf() produces \c Path("c")
	 * \li \c Path("c:\\dir").StripToLeaf() produces \c Path("dir")
	 * \li \c Path("a").StripToLeaf() produces \c Path("a")
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& StripToLeaf();

	/*!
	 * \brief Remove the very last child from this path.
	 *
	 * Examples:
	 * \li \c Path("a\\b\\c").RemoveLeaf() produces \c Path("a\\b")
	 * \li \c Path("c:\\dir").RemoveLeaf() produces \c Path("c:\\")
	 * \li \c Path("a").RemoveLeaf() produces \c Path("")
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& RemoveLeaf();

	/*!
	 * \brief Resolve out any symbols in the path string.
	 *
	 * After resolving the symbols, normalization is performed automatically.
	 * At the moment, this is an internal feature to 3ds Max, but is exposed 
	 * to the SDK to support future functionality.
	 * 
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& ResolveSymbols();

	/*!
	 * \brief Transform this path into its normal form.
	 *
	 * This will clean up pieces not following the backslash rules outlined in 
	 * the class documentation, and will convert ".."s as much as possible.
	 * "r:\\dirA\\dirB\\..\\dirC" becomes "r:\\dirA\\dirC."  If these hit
	 * the root, they just get stripped off, so "r:\\.." normalizes to "r:\\"
	 * If the path is relative, and there are more ".."s than preceding 
	 * directories, the extra sets are kept at the start of the path: 
	 * "b\\..\\..\\c" becomes "..\\c"
	 * 
	 * Any symbols in the path will be maintained since a symbol may resolve
	 * out to more than one directory.
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& Normalize();

	/*!
	 * \brief Convert all the characters in this path to lower case.
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& ConvertToLowerCase();

	/*!
	 * \brief Convert this absolute Path so that it points to the same location, 
	 * but relative to the new base path.
	 *
	 * The following attempted conversions are invalid and will leave the path
	 * unchanged:
	 * \li this path is already relative
	 * \li newBase is a relative path
	 * \li this path and newBase are both absolute but on different drives or 
	 * hosts.
	 *
	 * Example: \c Path("c:\\a\\b\\c").ConvertToRelativeTo(Path("c:\\a\\b\\d") produces
	 * \c Path("..\\c")
	 *
	 * \param newBase absolute base path from which this Path must later be 
	 * rooted to point to its current location again.
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& ConvertToRelativeTo(const Path &newBase);

	/*!
	 * \brief Convert this mapped drive Path to a UNC path with a host and
	 * share name.
	 *
	 * This operation only modifies the path if it is rooted at a valid mapped 
	 * drive.
	 * 
	 *	\return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& ConvertToUNC();

	/*!
	 * \brief Convert this relative path to absolute rooted at the current 
	 * 3ds Max Project Folder.
	 *
	 * This will automatically resolve any symbols and normalize this path.  If
	 * this Path already resolves to an absolute Path, it will ignore the current
	 * working directory.  This does not resolve the path using the 3ds Max
	 * file search mechanism.
	 *
	 * If conversion to an arbitrarily-rooted absolute path is desired, 
	 * \c Prepend should be used instead.
	 *
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& ConvertToAbsolute();

	/*!
	 * \brief Get the fully-resolved, lower case, path converted to absolute 
	 * rooted at the current 3ds Max Project Folder.
	 *
	 * This is a performance-oriented convenience method for clients that wish
	 * to hold on to the original, unresolved, relative path yet often need to 
	 * access the absolute path.
	 *
	 * \return This path after it's been fully resolved, converted to absolute
	 * (rooted at the Project Folder), and converted to lower case.
	 */
	UtilExport const Path& GetResolvedAbsolutePath() const;

	/*!
	 * \brief Converts an absolute path into a path that is relative to the 
	 * current Project Folder.
	 *
	 * Converts an absolute path into a path that is 
	 * relative to the current Project Folder.  If the path cannot be made 
	 * relative (if it is rooted on a different drive, for instance), then
	 * it is left untouched. If the path is already relative, it is also left 
	 * untouched.
	 * \return A reference to this Path.  (This allows for operation chaining.)
	 */
	UtilExport Path& ConvertToRelativeToProjectFolder();

	/*!
	 * \brief Get the current string representation of this path.
	 *
	 * Note that this does not perform any symbol resolution or normalization.
	 * \return A copy of this Path's current string representation.
	 */
	UtilExport MSTR GetString() const;

	/*! 
	 * \brief Get the internal c-string formatted data representation for this path.
	 * 
	 * Note that this c-string is not guaranteed to be consistent after any 
	 * modifications are made to the Path object.  If the data is needed 
	 * persistently, consider either copying the contents of this pointer, or 
	 * calling GetString() instead.
	 * \return A null-terminated c-string representation of the internals of 
	 * this object.
	 */
	UtilExport const MCHAR* GetCStr() const;

	/*!
	 * \brief Not exported outside the DLL - for use by PathImpl only.
	 *
	 * \return The internal implementation of this Path.
	 */
	PathImpl& GetImpl();

	/*!
	* \brief Not exported outside the DLL - for use by PathImpl only.
	*
	* Same as the other version, but this one's for const-context.
	* \return The internal implementation of this Path.
	*/
	const PathImpl& GetImpl() const;
};

}
}

#if !defined(NDEBUG) && defined(BUILD_WITH_IMPL)
// Debug and Hybrid builds can pull in implementation details
#include "..\..\include\SymbolString\TSymbolString.h"
#include "..\..\util\pathImpl.h"
#endif


#endif