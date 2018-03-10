/////////////////////////////////////////////////////////////////////////
//
//
//	Targeted IO Utility
//
//	Created 3-13-03: Tom Burke
//

#pragma once

// includes
#include "maxheap.h"
#include "sfx.h"

//==============================================================================
// ITargetedIO
/*! 
\sa Class Renderer
\par Description:
The TargetedIO system is useful for renderers that wish to implement custom 
Render Preset categories. The Mental Ray categories "Processing" and "Illumination"
are an example. TargetedIO is NOT important to developers who wish only to load and 
save render presets, and are not implementing their own renderer.\n\n
Abstractly, ITargetedIO represents a subset of the current scene, a "targeted" set 
of parameters, to be saved as a preset. Render Preset files are stored in the same 
format as .max files, but contain only the target parts of the scene, namely the renderer's parameters.\n\n
In practice, an ITargetedIO instance is passed to the renderer when presets are saved and 
loaded. The renderer filters what is saved and loaded by interacting with this IO object.
*/
//==============================================================================
class ITargetedIO: public MaxHeapOperators
{
public:

	//! \brief This is used when a preset is saved. 
	/*! It is possible for the user to choose some categories
		for the preset without selecting the renderer itself. 
		In this case the renderer is NOT part of the preset and its Load() method is 
		not called when the preset is loaded. Other categories defined by the renderer
		might still be saved in the preset. For this reason, it is important that
		each category be separate a ReferenceTarget with its own Load() and Save().
		\param targetIndex - a category ID number as determined by the renderer. 
		\param rt - the object which embodies this category of parameters. */
	virtual void AddSaveTarget( int targetIndex, ReferenceTarget * rt ) = 0;

	//! \brief This is used when a preset is loaded. 
	/*! The renderer can 
		then copy this loaded set of parameters into its own active set. As described 
		above, the renderer might not have its Load() method called when the preset 
		is loaded, so so the renderer must take responsibility to call %GetSaveTarget() 
		and manually copy its parameters out of this object.
		\param targetIndex - A category ID number as determined by the renderer. 
		\return the ReferenceTarget that was loaded under that category. */
	virtual ReferenceTarget * GetSaveTarget( int targetIndex ) = 0;

	//! This is used internally and should not be called by developers
	virtual int SaveToFile( const MCHAR * fileName, FileIOType context ) = 0;

	//! \deprecated Implement SaveToFile( const MCHAR * fileName, FileIOType context )
	virtual int SaveToFile( MCHAR * fileName, FileIOType context ) { return SaveToFile(const_cast<const MCHAR*>(fileName), context); }

	//! This is used internally and should not be called by developers
	virtual int LoadFromFile( const MCHAR * fileName, FileIOType context ) = 0;

	//! \deprecated Implement LoadFromFile( const MCHAR * fileName, FileIOType context )
	virtual int LoadFromFile( MCHAR * fileName, FileIOType context ) { return LoadFromFile(const_cast<const MCHAR*>(fileName), context); }

	//! \brief Allows the renderer to preserve a ReferenceTarget object before a preset is loaded.  
	/*! these utility methods can be used by renderers to hang on to reference targets between
		the RenderPresetsPreLoad and RenderPresetsPostLoad calls.
		This is needed when the user has chosen to save the renderer in 
		the preset, but NOT all of its parameters. In this case the renderer's Load() will 
		be called which may result in loading parameters that the user did not want as part 
		of the preset. If the renderer sees that a certain category is not being loaded, it 
		can Store() that object before the load and Retrieve() it afterwards, restoring its 
		active set of parameters to their correct value.
		\param targetIndex - A category ID number as determined by the renderer.
		\param rt - the object which embodies this category of parameters. */
	virtual void Store( int targetIndex, ReferenceTarget * rt ) = 0;

	//! \brief returns a referenceTarget that was previously saved with Store().
	/*! these utility methods can be used by renderers to hang on to reference targets between
		the RenderPresetsPreLoad and RenderPresetsPostLoad calls.
		\param targetIndex - A category ID number as determined by the renderer.
		\return the object which embodies this category of parameters. */
	virtual ReferenceTarget * Retrieve( int targetIndex ) = 0;

};

