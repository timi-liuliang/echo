/////////////////////////////////////////////////////////////////////////
//
//
//	Sampler Plug-Ins
//
//	Created 11/30/98	Kells Elmquist
//
#pragma once

#include <WTypes.h>
#include "sfx.h"
// forward declarations
class ShadeOutput;
class ShadeContext;

// Default Sampler ClassId
#define R25_SAMPLER_CLASS_ID			0x25773211
#define DEFAULT_SAMPLER_CLASS_ID		R25_SAMPLER_CLASS_ID

typedef	ULONG	MASK[2];
typedef SFXParamDlg SamplerParamDlg;

/*! \sa  Class Sampler, Class ShadeContext, Class Color,  Class Point2.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the callback object for the <b>DoSamples()</b> method of class
<b>Sampler</b>. The <b>SampleAtOffset()</b> method is the one that actually
computes the shading value for the Sampler.  */
class SamplingCallback : public InterfaceServer {
	public:
	/*! \remarks This is the method that integrates the sampler into the renderer.
	The plug-in Sampler calls this method to actually perform a sample at the
	specified 2D point. This method computes the output color and transparency.
	\par Parameters:
	<b>ShadeOutput* pOut</b>\n\n
	The output of the sampling\n\n
	<b>Point2\& sample</b>\n\n
	The 2D sample point.\n\n
	<b>float sampleScale</b>\n\n
	The scale of the sample. This parameter is the way a sampler tells the shader
	to use the whole pixel (sampleScale=1) size for texture samples or some
	fraction. This scale is an <b>edge</b> scale not an <b>area</b> scale, so if
	you want samples 1/4 pixel large the sampleScale should be 1/2.
	\return  TRUE if the sample was processed; FALSE if the clipped sample was	ignored. */
	virtual BOOL SampleAtOffset( ShadeOutput* pOut, Point2& sample, float sampleScale )=0;
};

class StdMat2;

/*! \sa  Class SpecialFX, Class SamplingCallback, Class SFXParamDlg, Class ShadeContext, Class Point3,  Class Point2,  Class ILoad, Class ISave.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class for the creation of Sampler plug-ins which work with the
Standard material. These appear in the Super Sampling rollout in the Sampler
dropdown. They have an Enable checkbox and a Quality spinner for their user
interface. An optional modal dialog may also be presented.\n\n
A Sampler is a plug-in that determines where inside a single pixel the shading
and texture samples are computed. For some Samplers this pattern is the same
for each pixel, for others a different pattern is chosen for each pixel. After
determining the sample locations, the sampler calls back to the renderer to
compute the shading values. It then averages the resluting shading values and
returns its estimate of the final color.\n\n
Some Samplers are adaptive. This means that the Sampler decides on-the-fly how
many samples to take to achieve its goal. There are many subtleties to adaptive
Samplers and many ways to define the adaptive mechanism. The adaptive mechanism
used by the R3 Samplers is very simple: take 4 samples, look for the maximum
change in any of the color channels, if it's greater than the threshold, then
sample the entire pixel according to the given quality. Threshold is an
optional parameter that may, but need not be used by adaptive Samplers.\n\n
The transfer of control from 3ds Max to the Sampler plug-in is as follows: A
Sampler is responsible for the sampling loop. It samples until it is done and
computes the sum of its samples upon completion. Once the Sampler's
<b>DoSample()</b> method is called 3ds Max no longer has control. This is how
adaptive samplers are handled. The <b>DoSample()</b> routine will determine how
often and where it samples, then it calls the provided
<b>SamplingCallback::SampleAtOffset()</b> method to have 3ds Max compute the
shading value.
\par Plug-In Information:
Class Defined In SAMPLER.H\n\n
Super Class ID SAMPLER_CLASS_ID\n\n
Standard File Name Extension DLH\n\n
Extra Include File Needed None  */

class Sampler : public SpecialFX {
	public:
		#pragma warning(push)
		#pragma warning(disable:4100)
		RefResult NotifyRefChanged(
			Interval changeInt, 
			RefTargetHandle hTarget, 
			PartID& partID, 
			RefMessage message ) { return REF_SUCCEED; }

		SClass_ID SuperClassID() { return SAMPLER_CLASS_ID; }
		
		// Saves and loads name. These should be called at the start of
		// a plug-in's save and load methods.
		/*! \remarks Implemented by the System.\n\n
		This method saves the name of the sampler. This should be called at the
		start of a plug-in's save methods.
		\par Parameters:
		<b>ISave *isave</b>\n\n
		An interface for saving data. */
		IOResult Save(ISave *isave) { return SpecialFX::Save(isave); }
		/*! \remarks Implemented by the System.\n\n
		This method loads the name of the sampler. This should be called at the
		start of a plug-in's load methods.
		\par Parameters:
		<b>ILoad *iload</b>\n\n
		An interface for loading data. */
		IOResult Load(ILoad *iload) { return SpecialFX::Load(iload); }

		/*! \remarks This is the method where the Sampler plug-in does its sampling
		loop. Upon completion it returns the color and transparency back to 3ds Max in
		<b>c</b> and <b>t</b>.\n\n
		A sampler samples a range of 0.0 to 1.0. For a pixel sampler this range gets
		mapped to a single pixel. The sampler doesn't need to be concerned with this
		however. It just works within the 0.0 to 1.0 space determining where to put the
		samples. Essentially, this method generates a set of points and calls
		<b>SamplingCallback::SampleAtOffset()</b> for each one. Then it sums up the
		results of the values returned from <b>SampleAtOffset()</b>, divides by the
		number of samples, and stores the results in <b>c</b> and <b>t</b>.
		\par Parameters:
		<b>ShadeOutput* pOut</b>\n\n
		This is the output of the sampling.\n\n
		<b>SamplingCallback* cb</b>\n\n
		This is the callback provided by 3ds Max which the sampler uses to actually do the sampling.\n\n
		<b>ShadeContext* sc</b>\n\n
		The Shade Context which provides information about the pixel being sampled.\n\n
		<b>MASK mask=NULL</b>\n\n
		The 64 bit pixel mask. This mask coresponds to the 8x8 sub-pixel area grid. The
		actual geometry covers only some portion of these bits. This is essentially an
		8x8 raster for the inside of the pixel where bits are set over the polygon
		being rendered and bits are off for areas not over the polygon. Developers
		typically only want to sample where the geometry is and thus when the bits are
		on. If not the results are very poor visually.\n\n
		Note: Most polygons are quite small in a typically complex scene being
		rendered. In other words, most polygons that need to get sampled will only have
		a small number of these mask bits on since the polygons are very small relative
		to the pixel. For instance, edge on polygons may project down to only a few
		bits within the pixel. Consequently it is quite possible that there may be zero
		samples, i.e. no geometry in the mask. Developers need to check for this zero
		samples condition. If this is the case then a method of <b>ShadeContext</b>
		called <b>SurfacePtScreen()</b> is used. This method returns a point which is
		guaranteed to be on the fragment no matter how small it is. This point can then
		be used for at least a single sample.
		\par Sample Code:
		The following is a brief analysis of the <b>DoSamples()</b> method from the
		Uniform Sampler of 3ds Max. This sampler sub-divides the sample area into a
		grid and samples the pixel at the center point of each grid unit.\n\n
		This code is from the file
		<b>/MAXSDK/SAMPLES/RENDER/SAMPLERS/STDSAMPLERS.CPP</b>.\n\n
		The complete code is shown below and then a code fragment analysis follows:\n\n
		\code
		void UniformSampler::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, MASK mask ) {
			int sideSamples = GetSideSamples();
			DbgAssert( sideSamples > 0 );
			// we map 0...sideSz into 0..1
			float sideSzInv = 1.0f / float(sideSamples);
			float sampleScale = texSuperSampleOn ? sideSzInv : 1.0f;	
			Point2 samplePt;
			float nSamples = 0.0f;
			pOut->Reset();
			ShadeOutput sampOut( sc->NRenderElements() );

			// Sampling Loop
			for( int y = 0; y < sideSamples; ++y ) {
				samplePt.y = (float(y) + 0.5f) * sideSzInv;
				for( int x = 0; x < sideSamples; ++x ) {
					samplePt.x =  (float(x) + 0.5f) * sideSzInv;
					if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender ) ) {
						// returns true for unclipped samples
						if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
							(*pOut) += sampOut;
							nSamples += 1.0f;
						}
					}
				}
			} // end, sampling loop

			if ( nSamples == 0.0f ) {
				// gets center of frag in screen space
				samplePt = sc->SurfacePtScreen(); 
				cb->SampleAtOffset( pOut, samplePt, 1.0f );
			} 
			else {
				pOut->Scale( 1.0f / nSamples );
			}
		}
		\endcode
		The above code is broken into smaller fragments to look at below:\n\n
		\code 
			int sideSamples = GetSideSamples(); 
		\endcode
		Here the sampler is just getting the number of sides in the sampling grid. This
		is computed based on the Quality spinner in the user interface. In this sampler
		this results in a number between 2 and 6 (developers can look at the
		<b>UniformSampler::GetSideSamples()</b> method to see this). Thus the resulting
		sampling grid is 2x2 or 3x3, up to 6x6. Then the number of samples is computed
		by multiplying the number of sides times itself.\n\n
		\code 
			int numSamples = sideSamples * sideSamples;
		\endcode
		Next the side size inverse is computed to know how big the step size is. This
		is the amount to step along each time.\n\n
		The sample scale is how large is the piece that's being sampled. For example,
		if the grid is 2x2 then each sample is scaled by 1/2\n\n
		\code 
			float sideSzInv = 1.0f / float(sideSamples);
			float sampleScale = texSuperSampleOn ? sideSzInv : 1.0f;
		\endcode
		Next the number of samples, and the color and transparency are initialized to	zero:\n\n
		\code
			float nSamples = 0.0f;
			pOut->Reset();
			ShadeOutput sampOut( sc->NRenderElements() );
		\endcode
		Then the sampling loop begins. Here the positions of individual sampling points
		are computed. Each point is then checked to see if it corresponds to a point in
		the mask (is over a polygon). (The <b>sampleInMask</b> function is defined in
		<b>/MAXSDK/SAMPLES/RENDER/SAMPLERS/SAMPLERUTIL.CPP</b>). If it is a point
		that's over a polygon then <b>SampleAtOffset()</b> is called. What
		<b>SampleAtOffset()</b> does is turn the passed 2D sample into a 3D sample and
		fills out a ShadeOutput with a color and transparency. These returned values are 
		summed up over the sampling loop (<b>(*pOut) += sampOut;)</b>.\n\n
		\code
			for( int y = 0; y < sideSamples; ++y ) {
				samplePt.y = (float(y) + 0.5f) * sideSzInv;
				for( int x = 0; x < sideSamples; ++x ) {
					samplePt.x =  (float(x) + 0.5f) * sideSzInv;
					if ( sampleInMask( samplePt, mask, sc->globContext->fieldRender ) ) {
						// returns true for unclipped samples
						if (cb->SampleAtOffset( &sampOut, samplePt, sampleScale )) {
							(*pOut) += sampOut;
							nSamples += 1.0f;
						}
					}
				}
			}
		\endcode
		At the end of the sampling loop a check is done to see if there were zero
		samples. This is the case if the geometry is very small relative to the pixel.
		There are two approaches that one might take when there are zero samples. One
		is to simply return black. A strict 'jitter-type' sampler might do this since,
		in fact, no samples were hit. This will result in artifacts to the image
		however. A better approach is to use the <b>ShadeContext</b> method
		<b>SurfacePtScreen()</b> to return a point which is guaranteed to be at the
		center of the fragment. Then this point is passed to <b>SampleAtOffset()</b> so
		a single sample which is on the fragment is used.\n\n
		If a single sample point was used, <b>DoSamples()</b> is finished. The reults
		are in <b>pOut</b> as returned from	<b>SampleAtOffset()</b>.\n\n
		If a number of samples was taken, the shade info is scaled by the inverse of the 
		number of samples (<b>nSamples</b>) to get the final colors.\n\n
		\code
			if ( nSamples == 0.0f ) {
				// gets center of frag in screen space
				samplePt = sc->SurfacePtScreen(); 
				cb->SampleAtOffset( pOut, samplePt, 1.0f );
			} 
			else {
				pOut->Scale( 1.0f / nSamples );
			}
		\endcode 
		*/
		virtual void DoSamples(
			ShadeOutput* pOut, 
			SamplingCallback* cb, 
			ShadeContext* sc, 
			MASK mask = NULL) = 0;
	
		// integer number of samples for current quality
		/*! \remarks This methods returns the integer number of samples given
		the current quality setting. If doing adaptive sampling (where the
		number of samples may vary) return the maximum number of samples
		possible. */
		virtual int GetNSamples()=0;	//what return when adaptive? n for max quality.

		// This is the one default parameter
		// Quality is nominal, 0...1, 
		// 0 is one sample, high about .75, 1.0 shd be awesome
		// for adaptive samplers, this sets the maximum quality
		/*! \remarks Sets the sampling quality. This is the one default
		parameter.
		\par Parameters:
		<b>float value</b>\n\n
		Quality is nominal with a range of 0.0 to 1.0. */
		virtual void SetQuality( float value )=0;
		/*! \remarks Returns the sampling quality in the range of 0.0 to 1.0.
		Quality means how many samples are taken to compute the shade in a
		pixel. Higher quality is of course achieved by more samples. Quality
		0.0 means "minimal", Quality 1.0 means "best", and Quality 0.5 means
		"good, the default ". Some samplers do not have adjustable quality
		(like 3ds Max 2.5 Star), in which case the quality spinner is disabled
		and this method is ignored. */
		virtual float GetQuality()=0;
		// returns 0 on "unchangeable", otherwise n quality levels
		/*! \remarks This method returns 0 on "unchangeable", otherwise the
		number of quality levels. */
		virtual int SupportsQualityLevels()=0;

		/*! \remarks Sets the Enable Sampler state to on or off.
		\par Parameters:
		<b>BOOL samplingOn</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetEnable( BOOL samplingOn )=0;
		/*! \remarks Returns TRUE if sampling is enabled; otherwise FALSE. */
		virtual BOOL GetEnable()=0;

		/*! \remarks Returns a comment string for the Sampler which appears in
		the Materials Editor user inteface. */
		virtual MCHAR* GetDefaultComment()=0;

		// Adaptive Sampling, non-reqd methods
		// there are various optional params, this defines which ones to show/enable
		/*! \remarks This method determines which of the various optional
		parameters are displayed. Zero or more of the following values (which
		may be added together):\n\n
		<b>IS_ADAPTIVE</b> -- Samples is adaptive in some way.\n\n
		<b>ADAPTIVE_CHECK_BOX</b> -- Enable the Adaptive check box.\n\n
		<b>ADAPTIVE_THRESHOLD</b> -- Enable the adaptive Threshold spinner.\n\n
		<b>SUPER_SAMPLE_TEX_CHECK_BOX</b> -- Enable the texture Super Sampling
		check box.\n\n
		<b>ADVANCED_DLG_BUTTON</b> -- Enable the Advanced button. This allows
		an additional popup dialog to be presented to the user. See the method
		<b>ExecuteParamDialog()</b>.\n\n
		<b>OPTIONAL_PARAM_0</b> -- Enable optional spinner 0. See the methods
		<b>GetOptionalParamName()</b>, <b>GetOptionalParamMax()</b>, etc.\n\n
		<b>OPTIONAL_PARAM_1</b> -- Enable optional spinner 1.\n\n
		The following option is simply a set of these:\n\n
		<b>R3_ADAPTIVE</b> =
		(<b>IS_ADAPTIVE+ADAPTIVE_CHECK_BOX+ADAPTIVE_THRESHOLD</b>)
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual ULONG SupportsStdParams(){ return 0; }

		// this determines whether to cut down the texture sample size of each sample, 
		// or whether to always use 1 pixel texture sample size
		/*! \remarks This method is called on the Sampler to reflect the
		change in the 'Supersamp. Tex.' checkbox state. This determines whether
		to cut down the texture sample size of each sample, or whether to
		always use 1 pixel texture sample size.
		\par Parameters:
		<b>BOOL on</b>\n\n
		TRUE for on; FALSE for off.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetTextureSuperSampleOn( BOOL on ){}
		/*! \remarks Returns TRUE if Super Sampling is on; otherwise FALSE.
		See <b>SetTextureSuperSampleOn()</b> above.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL GetTextureSuperSampleOn(){ return FALSE; }

		/*! \remarks This method is called on the Sampler to reflect the
		change in the 'Adaptive' checkbox state.
		\par Parameters:
		<b>BOOL on</b>\n\n
		TRUE for on; FALSE for off.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetAdaptiveOn( BOOL on ){}
		/*! \remarks Returns TRUE if Adaptive is on (cheched in the user
		interface); otherwise FALSE.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL IsAdaptiveOn(){ return FALSE; }

		/*! \remarks This method is called on the Sampler to reflect the
		change in the 'Threshold' spinner.
		\par Parameters:
		<b>float value</b>\n\n
		The value to set. Range 0-1.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetAdaptiveThreshold( float value ){}
		/*! \remarks Returns the adaptive threshold setting.
		\par Default Implementation:
		<b>{ return 0.0f; }</b> */
		virtual float GetAdaptiveThreshold(){ return 0.0f; }

		// there are 2 optional 0.0...max parameters, for whatever
		/*! \remarks Samplers plug-ins support two optional parameter which
		may be used by the plug-in for its own needs. This methods returns the
		number of parameters it supports. Note that the max value is 2.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual long GetNOptionalParams(){ return 0; }
		/*! \remarks Returns the name of the specified parameter.
		\par Parameters:
		<b>long nParam</b>\n\n
		The zero based index of the optional parameter: 0 for the first one, 1
		for the second.
		\par Default Implementation:
		<b>{ return _M(""); }</b> */
		virtual MCHAR * GetOptionalParamName( long nParam ){ return _M(""); }
		/*! \remarks Returns the maximum value of the specified optional
		parameter.
		\par Parameters:
		<b>long nParam</b>\n\n
		The zero based index of the optional parameter: 0 for the first one, 1
		for the second.
		\par Default Implementation:
		<b>{ return 1.0f; }</b> */
		virtual float GetOptionalParamMax( long nParam ){ return 1.0f; }
		/*! \remarks Returns the value of the specified optional parameter.
		\par Parameters:
		<b>long nParam</b>\n\n
		The zero based index of the optional parameter: 0 for the first one, 1
		for the second.
		\par Default Implementation:
		<b>{ return 0.0f; }</b> */
		virtual float GetOptionalParam( long nParam ){ return 0.0f; };
		/*! \remarks Sets the value of the specified optional parameter.
		\par Parameters:
		<b>long nParam</b>\n\n
		The zero based index of the optional parameter: 0 for the first one, 1
		for the second.\n\n
		<b>float val</b>\n\n
		The value to set.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetOptionalParam( long nParam, float val ){};

		// Put up a modal pop-up dialog that allows editing the sampler extended
		/*! \remarks This method is called to put up a modal dialog which
		allows editing of the extended parameters. The rest of the operation of
		3ds Max should be disalbed by this modal dialog (which is why you
		should use <b>GetMAXHWnd()</b>). This method is called when the
		Advanced button is pressed (which is enabled by using the
		<b>ADVANCED_DLG_BUTTON</b> flag returned from
		<b>SupportsStdParams()</b>.
		\par Parameters:
		<b>HWND hWndParent</b>\n\n
		The parent window handle. Use <b>Interface::GetMAXHWnd()</b>.\n\n
		<b>StdMat2* mtl</b>\n\n
		Points to the owning Standard material.
		\par Default Implementation:
		<b>{}</b> */
		virtual void ExecuteParamDialog(HWND hWndParent, StdMat2* mtl ){}

		// Implement this if you are using the ParamMap2 AUTO_UI system and the 
		// effect has secondary dialogs that don't have the sampler as their 'thing'.
		// Called once for each secondary dialog for you to install the correct thing.
		// Return TRUE if you process the dialog, false otherwise.
		/*! \remarks You should implement this method if you are using the
		ParamMap2 AUTO_UI system and the sampler has secondary dialogs that
		have something other than the incoming effect as their 'thing'. Called
		once for each secondary dialog for you to install the correct thing.
		Return TRUE if you process the dialog, FALSE otherwise, in which case
		the incoming effect will be set into the dialog.\n\n
		Note: Developers needing more information on this method can see the
		remarks for
		<a href="class_mtl_base.html#A_GM_mtlbase_ui">MtlBase::CreateParamDlg()</a>
		which describes a similar example of this method in use (in that case
		it's for use by a texture map plug-in).
		\par Parameters:
		<b>EffectParamDlg* dlg</b>\n\n
		Points to the ParamDlg.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL SetDlgThing(EffectParamDlg* dlg) { return FALSE; }
		#pragma warning(pop)
	};

// There are the standard parameters for samplers
#define	IS_ADAPTIVE					0x1		// adaptive in some way
#define	ADAPTIVE_CHECK_BOX			0x2		// enable adaptive check box
#define	ADAPTIVE_THRESHOLD			0x4		// enable adaptive threshold spinner
#define	SUPER_SAMPLE_TEX_CHECK_BOX	0x8		// enable texture subsampling check box
#define	ADVANCED_DLG_BUTTON			0x10	// enable advanced button
#define	OPTIONAL_PARAM_0			0x20	// enable optional spinner
#define	OPTIONAL_PARAM_1			0x40	// enable optional spinner

#define	R3_ADAPTIVE					(IS_ADAPTIVE+ADAPTIVE_CHECK_BOX+ADAPTIVE_THRESHOLD)		

// Chunk IDs saved by base class
#define SAMPLERBASE_CHUNK	0x39bf
#define SAMPLERNAME_CHUNK	0x0100
#define SAMPLER_VERS_CHUNK  0x0200



