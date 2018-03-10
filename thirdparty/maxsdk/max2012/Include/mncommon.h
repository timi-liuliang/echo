// Common macros for all MNMath.
#pragma once

#include "maxheap.h"
#include <WTypes.h>
#include "maxtypes.h"
#include "ioapi.h"

// forward declarations



#ifndef MNEPS
#define MNEPS 1e-04f
#endif

// Selection levels for internal use:
#define MN_SEL_DEFAULT 0	// Default = use whatever's in mesh.
#define MN_SEL_OBJECT 1
#define MN_SEL_VERTEX 2
#define MN_SEL_EDGE 3
#define MN_SEL_FACE 4

// Got sick of redoing the following everywhere:
/*! \sa  Class MNMesh.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is a handy class to subclass off of when you're designing a class with
flags. It contains one private data member, a DWORD, with the flag info. It
then implements a bunch of handy flag-related functions. All methods of this
class are implemented by the system.
\par Data Members:
private:\n\n
<b>DWORD FlagUserFlags;</b>\n\n
Stores the flags.  */
class FlagUser: public MaxHeapOperators {
	DWORD FlagUserFlags;
public:
	/*! \remarks Constructor. Sets FlagUserFlags to 0, clearing all flag bits.
	*/
	FlagUser () { FlagUserFlags=0; }
	/*! \remarks Sets flags. Each bit that is set in <b>fl</b> is assigned the
	value <b>val</b>. */
	void SetFlag (DWORD fl, bool val=TRUE) { if (val) FlagUserFlags |= fl; else FlagUserFlags -= (FlagUserFlags & fl); }
	/*! \remarks Clears flags. Each bit that is set in fl is cleared. */
	void ClearFlag (DWORD fl) { FlagUserFlags -= (FlagUserFlags & fl); }
	/*! \remarks Checks if flags are set.
	\return  TRUE if any of the bits set in fl are set in this FlagUser. FALSE
	if none of them are. */
	bool GetFlag (DWORD fl) const { return (FlagUserFlags & fl) ? 1 : 0; }
	/*! \remarks Sets FlagUserFlags to 0, clearing all flag bits. */
	void ClearAllFlags () { FlagUserFlags = 0; }

	/*! \remarks	Copies all flag bits over from <b>fl</b>. */
	void CopyFlags (DWORD fl) { FlagUserFlags = fl; }
	/*! \remarks Copies all flag bits over from <b>fu</b>. */
	void CopyFlags (const FlagUser & fu) { FlagUserFlags = fu.FlagUserFlags; }
	/*! \remarks Copies all flag bits over from <b>*fu</b>. */
	void CopyFlags (const FlagUser * fu) { FlagUserFlags = fu->FlagUserFlags; }

	/*! \remarks Copies from <b>fl</b> only those bits set in <b>mask</b>. */
	void CopyFlags (DWORD fl, DWORD mask) { FlagUserFlags |= (fl & mask); }
	/*! \remarks Copies from <b>fu</b> only those bits set in <b>mask</b>. */
	void CopyFlags (const FlagUser &fu, DWORD mask) { FlagUserFlags |= (fu.FlagUserFlags & mask); }
	/*! \remarks Copies from <b>*fu</b> only those bits set in <b>mask</b>. */
	void CopyFlags (const FlagUser *fu, DWORD mask) { FlagUserFlags |= (fu->FlagUserFlags & mask); }

	/*! \remarks Sets all flags that are set in fu. */
	void OrFlags (const FlagUser & fu) { FlagUserFlags |= fu.FlagUserFlags; }
	/*! \remarks Sets all flags that are set in <b>*fu</b>. */
	void OrFlags (const FlagUser * fu) { FlagUserFlags |= fu->FlagUserFlags; }

	/*! \remarks Clears all flags that are clear in <b>fu</b>. */
	void AndFlags (const FlagUser & fu) { FlagUserFlags &= fu.FlagUserFlags; }
	/*! \remarks Clears all flags that are clear in <b>*fu</b>. */
	void AndFlags (const FlagUser * fu) { FlagUserFlags &= fu->FlagUserFlags; }

	/*! \remarks Checks whether all the bits that are set in fmask are the
	same in this FlagUser and in fl. */
	bool FlagMatch (DWORD fmask, DWORD fl) const {
		return ((FlagUserFlags & fmask) == (fl & fmask));
	}
	/*! \remarks Checks whether all the bits that are set in fmask are the
	same in this FlagUser and in fu. */
	bool FlagMatch (DWORD fmask, const FlagUser & fu) const {
		return ((FlagUserFlags & fmask) == (fu.FlagUserFlags & fmask));
	}
	/*! \remarks Checks whether all the bits that are set in fmask are the
	same in this FlagUser and in *fu. */
	bool FlagMatch (DWORD fmask, const FlagUser * fu) const {
		return ((FlagUserFlags & fmask) == (fu->FlagUserFlags & fmask));
	}

	/*! \remarks Returns a DWORD equal to FlagUserFlags. */
	DWORD ExportFlags () const { return FlagUserFlags; }
	/*! \remarks Sets FlagUserFlags equal to fl. Same as CopyFlags (fl), but
	it's included for "linguistic completeness". */
	void ImportFlags (DWORD fl) { FlagUserFlags = fl; }

	IOResult WriteFlags (ISave* isave, ULONG* nb) const {
		return isave->Write(&FlagUserFlags, sizeof(DWORD), nb);
	}
	IOResult ReadFlags (ILoad* iload, ULONG* nb) {
		return iload->Read (&FlagUserFlags, sizeof(DWORD), nb);
	}
};

