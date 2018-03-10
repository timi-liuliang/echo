/*******************************************************************
 *
 *    DESCRIPTION: Standard materials, textures, and fog: generic interface
 *
 *    AUTHOR:  Dan Silva
 *
 *    HISTORY:   Created 3/5/96
 *					Modified for shiva 2/1/99, Kells Elmquist
 *
 *******************************************************************/

#pragma once
#include "buildver.h"
#include "imtl.h"
#include "TextureMapIndexConstants.h"
// forward declarations
class BitmapInfo;

// Shade values
#define NSHADES 	4
#define SHADE_CONST 	0
#define SHADE_PHONG 	1	
#define SHADE_METAL 	2
#define SHADE_BLINN 	3

// Transparency types
#define TRANSP_SUBTRACTIVE     	0
#define TRANSP_ADDITIVE     	1
#define TRANSP_FILTER     		2

// These queries used only in EvalStdChannel
#define ID_RAY_REFLECTION 12  // ray reflection: amt * spec color, 
#define ID_RAY_REFRACTION 13  // ray refraction: amt * transp * filter, 
#define ID_RAY_REFRACTION_IOR 14 // ray refraction: ior 
#define ID_TRANSLUCENT_CLR 15	// translucent color 

class Shader;
class Sampler;

namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
}

/*! \sa  Class Mtl, Class Color, <a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials</a>.\n\n
\par Description:
This class provides access to the properties of the 3ds Max Standard material.
All methods of this class are implemented by the system.\n\n
Note that some properties such as the texture maps used are accessed using
methods of the base class MtlBase. See that class, or the Advanced Topics section
<a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials</a>
for more details.
\par Method Groups:
See <a href="class_std_mat_groups.html">Method Groups for Class StdMat</a>.
*/
class StdMat: public Mtl {
	public:
		/*! \remarks Sets the 'Soften' setting on or off.
		\par Parameters:
		<b>BOOL onoff</b>\n\n
		TRUE to turn on; FALSE to turn off. */
		virtual void SetSoften(BOOL onoff)=0;
		/*! \remarks Sets the 'Face Map' setting on or off.
		\par Parameters:
		<b>BOOL onoff</b>\n\n
		TRUE to turn on; FALSE to turn off. */
		virtual void SetFaceMap(BOOL onoff)=0;
		/*! \remarks Sets the '2 Sided' setting on or off.
		\par Parameters:
		<b>BOOL onoff</b>\n\n
		TRUE to turn on; FALSE to turn off. */
		virtual void SetTwoSided(BOOL onoff)=0;
		/*! \remarks Sets the 'Wire' setting on or off.
		\par Parameters:
		<b>BOOL onoff</b>\n\n
		TRUE to turn on; FALSE to turn off. */
		virtual void SetWire(BOOL onoff)=0;
		/*! \remarks Sets the wire size to pixels or units.
		\par Parameters:
		<b>BOOL onoff</b>\n\n
		TRUE for units; FALSE for pixels. */
		virtual void SetWireUnits(BOOL onOff)=0;
		/*! \remarks Sets the opacity falloff setting to out or in.
		\par Parameters:
		<b>BOOL onoff</b>\n\n
		TRUE for Out; FALSE for In. */
		virtual void SetFalloffOut(BOOL onOff)=0;  // 1: out, 0: in
		/*! \remarks Sets the additive transparency setting.
		\par Parameters:
		<b>int type</b>\n\n
		One of the following values:\n\n
		<b>TRANSP_SUBTRACTIVE</b>\n\n
		<b>TRANSP_ADDITIVE</b>\n\n
		<b>TRANSP_FILTER</b> */
		virtual void SetTransparencyType(int type)=0;
		/*! \remarks Sets the ambient color to the specified value at the time
		passed.
		\par Parameters:
		<b>Color c</b>\n\n
		The color to set.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetAmbient(Color c, TimeValue t)=0;		
		/*! \remarks Sets the diffuse color to the specified value at the time
		passed.
		\par Parameters:
		<b>Color c</b>\n\n
		The color to set.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetDiffuse(Color c, TimeValue t)=0;		
		/*! \remarks Sets the specular color to the specified value at the
		time passed.
		\par Parameters:
		<b>Color c</b>\n\n
		The color to set.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetSpecular(Color c, TimeValue t)=0;
		/*! \remarks Sets the filter color to the specified value at the time
		passed.
		\par Parameters:
		<b>Color c</b>\n\n
		The color to set.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetFilter(Color c, TimeValue t)=0;
		/*! \remarks Sets the shininess to the specified value at the time
		passed.
		\par Parameters:
		<b>float v</b>\n\n
		The value to set in the range 0 - 1.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetShininess(float v, TimeValue t)=0;		
		/*! \remarks Sets the shininess strength to the specified value at the
		time passed.
		\par Parameters:
		<b>float v</b>\n\n
		The value to set in the range 0 - 1.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetShinStr(float v, TimeValue t)=0;		
		/*! \remarks Sets the self illumination to the specified value at the
		time passed.
		\par Parameters:
		<b>float v</b>\n\n
		The value to set in the range 0 - 1.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetSelfIllum(float v, TimeValue t)=0;		

		/*! \remarks Sets the opacity to the specified value at the time
		passed.
		\par Parameters:
		<b>float v</b>\n\n
		The value to set in the range 0 - 1.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetOpacity(float v, TimeValue t)=0;		
		/*! \remarks Sets the opacity falloff to the specified value at the
		time passed.
		\par Parameters:
		<b>float v</b>\n\n
		The value to set in the range 0 - 1.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetOpacFalloff(float v, TimeValue t)=0;		
		/*! \remarks Sets the wire size to the specified value at the time
		passed.
		\par Parameters:
		<b>float s</b>\n\n
		The value to set. This value should be \> 0.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetWireSize(float s, TimeValue t)=0;
		/*! \remarks Sets the index of refraction to the specified value at
		the time passed.
		\par Parameters:
		<b>float v</b>\n\n
		The value to set in the range 0 - 10.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the value. */
		virtual void SetIOR(float v, TimeValue t)=0;
		/*! \remarks Locks or unlocks the ambient/diffuse textures together.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to lock; FALSE to unlock. */
		virtual void LockAmbDiffTex(BOOL onOff)=0;

// begin - ke/mjm - 03.16.00 - merge reshading code
//		virtual BOOL SupportsShaders(){ return FALSE; } // moved to class Mtl
// end - ke/mjm - 03.16.00 - merge reshading code
		
		// >>>> Sampling
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Set super sampling on or off (enabled or disabled).
		\par Parameters:
		<b>BOOL on</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetSamplingOn( BOOL on )=0;	
		virtual BOOL GetSamplingOn()=0;	

		// Obsolete Calls, not used in StdMat2....see shaders.h
		/*! \remarks Sets the shading limit for the material.
		\par Parameters:
		<b>int s</b>\n\n
		One of the following values:\n\n
		<b>SHADE_CONST</b>\n\n
		<b>SHADE_PHONG</b>\n\n
		<b>SHADE_METAL</b>\n\n
		<b>SHADE_BLINN</b> */
		virtual void SetShading(int s)=0;
		/*! \remarks Returns the shading limit. One of the following
		values:\n\n
		<b>SHADE_CONST</b>\n\n
		<b>SHADE_PHONG</b>\n\n
		<b>SHADE_METAL</b>\n\n
		<b>SHADE_BLINN</b> */
		virtual int  GetShading()=0;

		// texmaps, these only work for translated ID's of map channels,
		// see stdMat2 for access to std ID channels translator
		/*! \remarks Enables or disables the specified map type.
		\par Parameters:
		<b>int i</b>\n\n
		See \ref Material_TextureMap_IDs "List of Material Texture Map Indices".
		<b>BOOL onoff</b>\n\n
		TRUE to enable; FALSE to disable. */
		virtual void EnableMap(int id, BOOL onoff)=0;
		/*! \remarks Returns TRUE if the specified map is enabled; otherwise
		FALSE.
		\par Parameters:
		<b>int i</b>\n\n
		See \ref Material_TextureMap_IDs "List of Material Texture Map Indices".
		*/
		virtual BOOL MapEnabled(int id)=0;
		/*! \remarks This method is used to change the 'Amount' setting of the
		specified map.
		\par Parameters:
		<b>int imap</b>\n\n
		See \ref Material_TextureMap_IDs "List of Material Texture Map Indices".
		<b>float amt</b>\n\n
		The amount to set in the range of 0-1.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to set the amount. */
		virtual void SetTexmapAmt(int id, float amt, TimeValue t)=0;
		/*! \remarks Returns the amount setting of the specified texture map
		at the time passed. The returned range is 0 to 1.
		\par Parameters:
		<b>int imap</b>\n\n
		See \ref Material_TextureMap_IDs "List of Material Texture Map Indices".
		<b>TimeValue t</b>\n\n
		The amount at this time is returned. */
		virtual float GetTexmapAmt(int id, TimeValue t)=0;

		/*! \remarks Returns TRUE if soften is on; otherwise FALSE. */
		virtual BOOL GetSoften()=0;
		/*! \remarks Returns TRUE if face mapping is on; otherwise FALSE. */
		virtual BOOL GetFaceMap()=0;
		/*! \remarks Returns TRUE if two sided is on; otherwise FALSE. */
		virtual BOOL GetTwoSided()=0;
		/*! \remarks Returns TRUE if wire is on; otherwise FALSE. */
		virtual BOOL GetWire()=0;
		/*! \remarks Returns TRUE if the wire size is in units; FALSE if the
		wire size is in pixels. */
		virtual BOOL GetWireUnits()=0;
		/*! \remarks Returns the opacity falloff setting: 1 = Out, 0 = In. */
		virtual BOOL GetFalloffOut()=0;  // 1: out, 0: in
		/*! \remarks Returns the transparency type.
		\return  <b>int type</b>\n\n
		One of the following values:\n\n
		<b>TRANSP_SUBTRACTIVE</b>\n\n
		<b>TRANSP_ADDITIVE</b>\n\n
		<b>TRANSP_FILTER</b> */
		virtual int GetTransparencyType()=0;

		/*! \remarks Returns the ambient color setting at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to return the color. */
		virtual Color GetAmbient(TimeValue t)=0;		
		/*! \remarks Returns the diffuse color setting at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to return the color. */
		virtual Color GetDiffuse(TimeValue t)=0;		
		/*! \remarks Returns the specular color setting at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to return the color. */
		virtual Color GetSpecular(TimeValue t)=0;
		/*! \remarks Returns the filter color setting at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to return the color. */
		virtual Color GetFilter(TimeValue t)=0;
		/*! \remarks Returns the shininess setting at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The value at this time is returned. */
		virtual float GetShininess( TimeValue t)=0;		
		/*! \remarks Returns the shininess strength setting at the specified
		time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The value at this time is returned. */
		virtual float GetShinStr(TimeValue t)=0;		
		/*! \remarks Returns the self illumination setting at the specified
		time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The value at this time is returned. */
		virtual float GetSelfIllum(TimeValue t)=0;		
		/*! \remarks Returns the opacity setting at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The value at this time is returned. */
		virtual float GetOpacity( TimeValue t)=0;		
		/*! \remarks Returns the opacity falloff setting at the specified
		time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The value at this time is returned. */
		virtual float GetOpacFalloff(TimeValue t)=0;		
		/*! \remarks Returns the wire size setting at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The value at this time is returned. */
		virtual float GetWireSize(TimeValue t)=0;
		/*! \remarks Returns the index of refraction setting at the specified
		time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The value at this time is returned. */
		virtual float GetIOR( TimeValue t)=0;
		/*! \remarks Returns TRUE if the ambient-diffuse texture lock is set;
		otherwise FALSE. */
		virtual BOOL GetAmbDiffTexLock()=0;
	};

//////////////////////////////////////////////////////////////////////////////
//
//	This is the base class for all materials that support plug-in shaders
//
/*! \sa  Class StdMat, Class Shader, Class Sampler, Class Class_ID.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class for all materials supporting the plug-in shader
mechanism. The 3ds Max Standard material is derived from this class.  */
class StdMat2 : public StdMat {
public:
	BOOL SupportsShaders(){ return TRUE; }

	/*! Checks if the material parameter has a key set at the specified time.
	\param id - The ID of the parameter to check.
	\param t - The time to check.
	\return TRUE if the specified parameter whose ID is passed
	has a key at the time passed; otherwise FALSE. */
	virtual BOOL  KeyAtTimeByID(ParamID id,TimeValue t) = 0;

	//! \deprecated Deprecated in 3ds Max 2012. Please use KeyAtTimeByID instead.
	MAX_DEPRECATED BOOL  KeyAtTime(int id,TimeValue t);
	/*! \remarks Returns a value to indicate the state of the specified map.
	One of the following values:\n\n
	<b>0</b>: No map present.\n\n
	<b>1</b>: Map present but disabled.\n\n
	<b>2</b>: Map present and on.
	\par Parameters:
	<b>int indx</b>\n\n
	The index of the map to check. See \ref Material_TextureMap_IDs "List of Material Texture Map Indices".
	*/
	virtual int   GetMapState( int indx ) = 0; //returns 0 = no map, 1 = disable, 2 = mapon
	/*! \remarks Returns the name of the map whose index is passed.
	\par Parameters:
	<b>int indx</b>\n\n
	The index of the map to check. See \ref Material_TextureMap_IDs "List of Material Texture Map Indices".
	*/
	virtual MSTR  GetMapName( int indx ) = 0;
	/*! \remarks This method is called when the state of the Ambient/Diffuse
	Texture lock is toggled. The material should store the setting and update
	the UI as required.
	\par Parameters:
	<b>BOOL lockOn</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void  SyncADTexLock( BOOL lockOn ) = 0;

	// Shaders
	/*! \remarks This method is called when a new Shader has been selected.
	\par Parameters:
	<b>Class_ID id</b>\n\n
	The Class_ID of the new shader to switch to. */
	virtual BOOL SwitchShader( Class_ID id )= 0;
	/*! \remarks Returns a pointer to the Shader in use. See
	Class Shader for details on this plug-in type. */
	virtual Shader* GetShader()= 0;
	/*! \remarks Returns TRUE if the shader is faceted; otherwise FALSE. The
	pre-R3 Constant shader is faceted. The other shaders are not. */
	virtual BOOL IsFaceted()= 0;
	/*! \remarks Sets the faceted setting of the Shader.
	\par Parameters:
	<b>BOOL on</b>\n\n
	TRUE if it is faceted; FALSE if not. */
	virtual void SetFaceted( BOOL on )= 0;

	// texture channels from stdmat id's
	/*! \remarks Returns the index of the mapping channels which corresponds
	to the specified Standard materials texture map ID.
	\par Parameters:
	<b>long id</b>\n\n
	The ID whose corresponding channel to return. See \ref Material_TextureMap_IDs "List of Material Texture Map Indices".
	\return  The zero based index of the channel. If there is not a
	corresponding channel return -1. */
	virtual long StdIDToChannel( long id )=0;
	#pragma warning(push)
	#pragma warning(disable:4100)
	// Obsolete Calls from StdMat, not used in StdMat2, except stdmtl2 provides 
	// support for translators: old shaders return correct id, all others return blinn
	/*! \remarks Sets the active shader to the one specified. The supported
	types are the pre-R3 shaders.
	\par Parameters:
	<b>int s</b>\n\n
	One of the following values (all other values are a NOOP):\n\n
	<b>SHADE_CONST</b> (Phong, faceted).\n\n
	<b>SHADE_PHONG</b>\n\n
	<b>SHADE_METAL</b>\n\n
	<b>SHADE_BLINN</b>
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetShading(int s){}
	/*! \remarks Returns one of the pre-R3 shader types. If an R3 shader type
	is active, SHADE_BLINN is returned.
	\return  One of the following values:\n\n
	<b>SHADE_CONST</b> (Phong, faceted).\n\n
	<b>SHADE_PHONG</b>\n\n
	<b>SHADE_METAL</b>\n\n
	<b>SHADE_BLINN</b>
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int GetShading(){ return -1; } 

	// Samplers
	/*! \remarks This method is called when the active Sampler is switched.
	\par Parameters:
	<b>Class_ID id</b>\n\n
	The Class_ID of the new Sampler. */
	virtual BOOL SwitchSampler( Class_ID id )=0;	
	/*! \remarks Returns a pointer to the sampler used. */
	virtual Sampler * GetPixelSampler(int mtlNum, BOOL backFace)=0;
	
	// these params extend the UI approximation set in stdMat
	/*! \remarks Returns the Self Illumination Color On setting. TRUE if on;
	FALSE if off.
	\par Parameters:
	These parameters are not used and may be ignored. */
	virtual BOOL  GetSelfIllumColorOn(int mtlNum=0, BOOL backFace=FALSE)=0;
	/*! \remarks Returns the Self Illumination Color setting.
	\par Parameters:
	These parameters are not used and may be ignored. */
	virtual Color GetSelfIllumColor(int mtlNum, BOOL backFace)=0;
	/*! \remarks Returns the Self Illumination Color setting at the specified
	time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the color. */
	virtual Color GetSelfIllumColor(TimeValue t)=0; 
	/*! \remarks Sets the Self Illumination Color On setting
	\par Parameters:
	<b>BOOL on</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetSelfIllumColorOn( BOOL on )=0;
	/*! \remarks Sets the Self Illumination Color setting at the specified
	time.
	\par Parameters:
	<b>Color c</b>\n\n
	The color to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the color. */
	virtual void SetSelfIllumColor(Color c, TimeValue t)=0;	
	
	// these are used to simulate traditional 3ds shading by the default handlers
	virtual float GetReflectionDim(float diffIllumIntensity ){ return 1.0f; }		
	virtual	Color TranspColor( float opac, Color filt, Color diff )=0;
	virtual float GetEffOpacity(ShadeContext& sc, float opac )=0;		
	#pragma warning(pop)
};


// Mapping types for SetCoordMapping
#define UVMAP_EXPLICIT   0
#define UVMAP_SPHERE_ENV 1
#define UVMAP_CYL_ENV  	 2
#define UVMAP_SHRINK_ENV 3
#define UVMAP_SCREEN_ENV 4

/*! \sa  Class UVGen.\n\n
\par Description:
This class provides access to the parameters of the 3ds Max UVGen class. These
are the settings in the 'Coordinates' and 'Noise' rollups such as UV offsets,
angle, blur, noise level, etc. All methods of this class are implemented by the
system.
\par Method Groups:
See <a href="class_std_u_v_gen_groups.html">Method Groups for Class StdUVGen</a>.
*/
class StdUVGen: public UVGen {
public:
	void* m_geoRefInfo; // used in the GeoReferencing system in VIZ

	BOOL IsStdUVGen() { return TRUE; }  // method inherited from UVGen

	/*! \remarks	Sets the mapping type to one of the specified values.
	\par Parameters:
	<b>int</b>\n\n
	The mapping type. One of the following values:\n\n
	<b>UVMAP_EXPLICIT</b>\n\n
	Explicit Texture mapping.\n\n
	<b>UVMAP_SPHERE_ENV</b>\n\n
	Spherical Environment mapping.\n\n
	<b>UVMAP_CYL_ENV</b>\n\n
	Cylindrical Environment mapping.\n\n
	<b>UVMAP_SHRINK_ENV</b>\n\n
	Shrink Wrap Environment mapping.\n\n
	<b>UVMAP_SCREEN_ENV</b>\n\n
	Screen Environment mapping. */
	virtual void SetCoordMapping(int)=0;
	/*! \remarks Sets the U Offset setting to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetUOffs(float f, TimeValue t)=0;
	/*! \remarks Sets the V Offset setting to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetVOffs(float f, TimeValue t)=0;
	/*! \remarks Sets the U tiling setting to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetUScl(float f,  TimeValue t)=0;
	/*! \remarks Sets the V tiling setting to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetVScl(float f,  TimeValue t)=0;
	/*! \remarks Sets the angle setting to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set in radians.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetAng(float f,   TimeValue t)=0; // angle in radians
	/*! \remarks	Sets the U Angle setting the specified value at the time passed.
	\par Parameters:
	<b>float f</b>\n\n
	The angle to set in radians.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the angle. */
	virtual void SetUAng(float f,   TimeValue t)=0; // angle in radians
	/*! \remarks	Sets the V Angle setting the specified value at the time passed.
	\par Parameters:
	<b>float f</b>\n\n
	The angle to set in radians.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the angle. */
	virtual void SetVAng(float f,   TimeValue t)=0; // angle in radians
	/*! \remarks	Sets the W Angle setting the specified value at the time passed.
	\par Parameters:
	<b>float f</b>\n\n
	The angle to set in radians.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the angle. */
	virtual void SetWAng(float f,   TimeValue t)=0; // angle in radians
	/*! \remarks Sets the blur to the specified value at the time passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetBlur(float f,  TimeValue t)=0;
	/*! \remarks Sets the blur offset to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetBlurOffs(float f,  TimeValue t)=0; 
	/*! \remarks Sets the noise amount to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetNoiseAmt(float f,  TimeValue t)=0; 
	/*! \remarks Sets the noise size to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetNoiseSize(float f,  TimeValue t)=0; 
	/*! \remarks Sets the noise level to the specified value at the time
	passed.
	\par Parameters:
	<b>int i</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetNoiseLev(int i,  TimeValue t)=0; 
	/*! \remarks Sets the noise phase to the specified value at the time
	passed.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetNoisePhs(float f,  TimeValue t)=0; 
	/*! \remarks Sets the texture tiling setting.
	\par Parameters:
	<b>int tiling</b>\n\n
	See \ref texture_symmetry_flags.html. */
	virtual void SetTextureTiling(int tiling)=0;
	/*! \remarks	Sets the mapping channel to the specified value.
	\par Parameters:
	<b>int i</b>\n\n
	The channel to set. */
	virtual void SetMapChannel(int i)=0;
	/*! \remarks	This method allows you to set the UVGen flags.
	\par Parameters:
	<b>ULONG f</b>\n\n
	See \ref texture_symmetry_flags.html.\n\n
	<b>ULONG val</b>\n\n
	The value to set. */
	virtual void SetFlag(ULONG f, ULONG val)=0;
	/*! \remarks	This method allows you to set the hide map back flag.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE to set the flag; FALSE to disable. */
	virtual void SetHideMapBackFlag(BOOL b)=0;

	/*! \remarks Retrieves the coordinate mapping type.
	\par Parameters:
	<b>int</b>\n\n
	This parameter is not used.
	\return  One of the following values:\n\n
	<b>UVMAP_EXPLICIT</b>\n\n
	<b>UVMAP_SPHERE_ENV</b>\n\n
	<b>UVMAP_CYL_ENV</b>\n\n
	<b>UVMAP_SHRINK_ENV</b>\n\n
	<b>UVMAP_SCREEN_ENV</b> */
	virtual int  GetCoordMapping(int)=0;
	/*! \remarks Retrieves the U Offset setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetUOffs( TimeValue t)=0;
	/*! \remarks Retrieves the V Offset setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetVOffs( TimeValue t)=0;
	/*! \remarks Retrieves the U Tiling setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetUScl(  TimeValue t)=0;
	/*! \remarks Retrieves the V Tiling setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetVScl(  TimeValue t)=0;
	/*! \remarks Retrieves the angle setting in radians.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetAng(   TimeValue t)=0; // angle in radians
	/*! \remarks	Returns the U Angle setting (in radians) at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the angle. */
	virtual float GetUAng(   TimeValue t)=0; // angle in radians
	/*! \remarks	Returns the V Angle setting (in radians) at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the angle. */
	virtual float GetVAng(   TimeValue t)=0; // angle in radians
	/*! \remarks	Returns the W Angle setting (in radians) at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the angle. */
	virtual float GetWAng(   TimeValue t)=0; // angle in radians
	/*! \remarks Retrieves the blur setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetBlur(  TimeValue t)=0;
	/*! \remarks Retrieves the blur offset setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetBlurOffs(  TimeValue t)=0; 
	/*! \remarks Retrieves the noise amount setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetNoiseAmt(  TimeValue t)=0; 
	/*! \remarks Retrieves the noise size setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetNoiseSize(  TimeValue t)=0; 
	/*! \remarks Retrieves the noise level setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual int GetNoiseLev( TimeValue t)=0; 
	/*! \remarks Retrieves the noise phase setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetNoisePhs(  TimeValue t)=0; 
	/*! \remarks Returns the texture tiling flags.
	\return  See \ref texture_symmetry_flags.html. */
	virtual int GetTextureTiling()=0;
	/*! \remarks	Returns the mapping channel. */
	virtual int GetMapChannel()=0;
	/*! \remarks	This method returns the flag status of the UVGen.
	\par Parameters:
	<b>ULONG f</b>\n\n
	See \ref texture_symmetry_flags.html. */
	virtual int GetFlag(ULONG f)=0;
	/*! \remarks	This method returns the state of the hide map back flag. */
	virtual	BOOL GetHideMapBackFlag()=0;

    /*! \remarks Sets whether to use real world mapping.\n\n
    */
    virtual void SetUseRealWorldScale(BOOL useRWS)=0;
    /*! \remarks Queries whether real world mapping is used. */
    virtual BOOL GetUseRealWorldScale()=0;
	};


// Values returned by GetCoordSystem, and passed into
// SetCoordSystem
#define XYZ_COORDS 0
#define UVW_COORDS 1
#define UVW2_COORDS 2
#define XYZ_WORLD_COORDS 3

/*! \sa  Class XYZGen.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides access to the parameters of the 3ds Max XYZGen class. The
XYZGen class puts up the 3D 'Coordinates' rollup.  */
class StdXYZGen: public XYZGen {
	public:

	/*! \remarks This is an implementation of a method of <b>XYZGen</b>. This
	returns TRUE to indicate this is a <b>StdXYZGen</b> instance. */
	BOOL IsStdXYZGen() { return TRUE; }
	/*! \remarks Sets the coordinate system used.
	\par Parameters:
	<b>int s</b>\n\n
	The coordinate system to set. One of the following values:\n\n
	<b>XYZ_COORDS</b> - Object XYZ\n\n
	<b>UVW_COORDS</b> - Explicit Map Channel\n\n
	<b>UVW2_COORDS</b> - Vertex Color Channel\n\n
	<b>XYZ_WORLD_COORDS</b> - World XYZ. This option is available in release
	3.0 and later only. */
	virtual	void SetCoordSystem(int s)=0;
	/*! \remarks Sets the Blur setting to the specified value at the specified
	time.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetBlur(float f,  TimeValue t)=0;
	/*! \remarks Sets the Blur Offset setting to the specified value at the
	specified time.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetBlurOffs(float f,  TimeValue t)=0; 
	/*! \remarks Sets the specified Offset setting to the specified value at
	the specified time.
	\par Parameters:
	<b>int axis</b>\n\n
	The axis to set. <b>0</b> for X, <b>1</b> for Y, <b>2</b> for Z.\n\n
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetOffs(int axis, float f, TimeValue t)=0;
	/*! \remarks Sets the specified Tiling (Scale) setting to the specified
	value at the specified time.
	\par Parameters:
	<b>int axis</b>\n\n
	The axis to set. <b>0</b> for X, <b>1</b> for Y, <b>2</b> for Z.\n\n
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetScl(int axis, float f, TimeValue t)=0;
	/*! \remarks Sets the specified Angle setting to the specified value at
	the specified time.
	\par Parameters:
	<b>int axis</b>\n\n
	The axis to set. <b>0</b> for X, <b>1</b> for Y, <b>2</b> for Z.\n\n
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetAng(int axis, float f, TimeValue t)=0;

	/*! \remarks Returns the coordinate system in use. One of the following
	values:\n\n
	<b>XYZ_COORDS</b> - Object XYZ\n\n
	<b>UVW_COORDS</b> - Explicit Map Channel\n\n
	<b>UVW2_COORDS</b> - Vertex Color Channel\n\n
	<b>XYZ_WORLD_COORDS</b> - World XYZ. This option is available in release
	3.0 and later only. */
	virtual	int GetCoordSystem()=0;
	/*! \remarks Returns the Blur setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the value. */
	virtual float GetBlur(TimeValue t)=0;
	/*! \remarks Returns the Blur Offset setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the value. */
	virtual float GetBlurOffs(TimeValue t)=0; 
	virtual float GetOffs(int axis, TimeValue t)=0;
	/*! \remarks Returns the Tiling (Scale) setting for the specified axis at
	the specified time.
	\par Parameters:
	<b>int axis</b>\n\n
	The axis to set. <b>0</b> for X, <b>1</b> for Y, <b>2</b> for Z.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the value. */
	virtual float GetScl(int axis, TimeValue t)=0;
	/*! \remarks Returns the Angle setting for the specified axis at the
	specified time.
	\par Parameters:
	<b>int axis</b>\n\n
	The axis to set. <b>0</b> for X, <b>1</b> for Y, <b>2</b> for Z.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the value. */
	virtual float GetAng(int axis, TimeValue t)=0;

	/*! \remarks	Sets the mapping channel to the specified value.
	\par Parameters:
	<b>int i</b>\n\n
	The mapping channel to set. */
	virtual void SetMapChannel(int i)=0;
	/*! \remarks	Returns the mapping channel. */
	virtual int GetMapChannel()=0;
	};

#define TEXOUT_XXXXX 		1
#define TEXOUT_INVERT		2
#define TEXOUT_CLAMP   		4
#define TEXOUT_ALPHA_RGB		8
#define TEXOUT_COLOR_MAP 		16
#define TEXOUT_COLOR_MAP_RGB	32

/*! \sa  Class TextureOutput.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class provides access to the Texture Output object. These are the
parameters available in the Output rollup in the Materials Editor for a 2D map.
 */
class StdTexoutGen: public TextureOutput {
	public:

	/*! \remarks Returns TRUE if this is the standard 3ds Max Texout object;
	FALSE if it's a 3rd party Texout.
	\par Default Implementation:
	<b>{ return TRUE; }</b> */
	BOOL IsStdTexoutGen() { return TRUE; }
	/*! \remarks Returns the Output Level at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the output level. */
	virtual float GetOutputLevel(TimeValue t)=0;
	/*! \remarks Returns the Invert on / off state. TRUE is on; FALSE is off.
	*/
	virtual BOOL GetInvert()=0;
	/*! \remarks Returns the Clamp on / off state. TRUE is on; FALSE is off.
	*/
	virtual BOOL GetClamp()=0;
	/*! \remarks Returns the Alpha from RGB Intensity on / off state. TRUE is
	on; FALSE is off. */
	virtual BOOL GetAlphaFromRGB()=0;
	/*! \remarks Returns the RGB Level at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the value. */
	virtual float GetRGBAmt( TimeValue t)=0;
	/*! \remarks Returns the RGB Offset at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the value. */
	virtual float GetRGBOff( TimeValue t)=0; 
	/*! \remarks Returns the Output Amount at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the value. */
	virtual float GetOutAmt( TimeValue t)=0;
	/*! \remarks Returns the Bump Amount at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the value. */
	virtual float GetBumpAmt( TimeValue t)=0;
	/*! \remarks Returns TRUE if the specified flag(s) are set; otherwise
	FALSE. These flags reflect the same setings as the individual methods.
	\par Parameters:
	<b>ULONG f</b>\n\n
	One or more of the following values:\n\n
	<b>TEXOUT_INVERT</b>\n\n
	<b>TEXOUT_CLAMP</b>\n\n
	<b>TEXOUT_ALPHA_RGB</b>\n\n
	<b>TEXOUT_COLOR_MAP</b>\n\n
	<b>TEXOUT_COLOR_MAP_RGB</b> */
	virtual BOOL GetFlag(ULONG f)=0;

	/*! \remarks Sets the Output Amount at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>float v</b>\n\n
	The value to set. */
	virtual void SetOutputLevel(TimeValue t, float v)=0;
	/*! \remarks Sets the Invert state to on or off.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetInvert(BOOL onoff)=0;
	/*! \remarks Sets the Clamp state to on or off.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetClamp(BOOL onoff)=0;
	/*! \remarks Sets the Alpha from RGB Intensity to on or off.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetAlphaFromRGB(BOOL onoff)=0;
	/*! \remarks Sets the RGB Level at the specified time.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetRGBAmt( float f, TimeValue t)=0;
	/*! \remarks Sets the RGB Offset parameter at the specified time.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetRGBOff(float f, TimeValue t)=0; 
	/*! \remarks Sets the Output Amount at the specified time.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetOutAmt(float f, TimeValue t)=0; 
	/*! \remarks Sets the Bump Amount at the specified time.
	\par Parameters:
	<b>float f</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the value. */
	virtual void SetBumpAmt(float f, TimeValue t)=0; 
	/*! \remarks Sets the state of the flags.
	\par Parameters:
	<b>ULONG f</b>\n\n
	One or more of the following values:\n\n
	<b>TEXOUT_INVERT</b>\n\n
	<b>TEXOUT_CLAMP</b>\n\n
	<b>TEXOUT_ALPHA_RGB</b>\n\n
	<b>TEXOUT_COLOR_MAP</b>\n\n
	<b>TEXOUT_COLOR_MAP_RGB</b>\n\n
	<b>ULONG val</b>\n\n
	Non-zero to set the specified flags; zero to clear the flags. */
	virtual void SetFlag(ULONG f, ULONG val)=0;
};

// Image filtering types
#define FILTER_PYR     0
#define FILTER_SAT     1
#define FILTER_NADA	   2

// Alpha source types
#define ALPHA_FILE 	0
#define ALPHA_RGB	2
#define ALPHA_NONE	3

// End conditions:
#define END_LOOP     0
#define END_PINGPONG 1
#define END_HOLD     2


//***************************************************************
//Function Publishing System stuff   
//****************************************************************
#define BITMAPTEX_INTERFACE Interface_ID(0x55b4400e, 0x29ff7cc9)

#define GetIBitmapTextInterface(cd) \
			(BitmapTex *)(cd)->GetInterface(BITMAPTEX_INTERFACE)


enum {  bitmaptex_reload, bitmaptex_crop };

//****************************************************************


/*! \sa  Class Texmap, Class StdUVGen, Class TextureOutput.\n\n
\par Description:
This class is an interface into the Bitmap texture. All methods of this class
are implemented by the system.  */
class BitmapTex: public Texmap, public FPMixinInterface {
	public:
	#pragma warning(push)
	#pragma warning(disable:4100)
	//Function Publishing System
	//Function Map For Mixin Interface
	//*************************************************
	BEGIN_FUNCTION_MAP
		VFN_0(bitmaptex_reload, fnReload);
		VFN_0(bitmaptex_crop, fnViewImage);

	END_FUNCTION_MAP

	/*! \remarks Sets the bitmap filtering method used.
	\par Parameters:
	<b>int ft</b>\n\n
	Image filtering types. One of the following values:\n\n
	<b>FILTER_PYR</b>\n\n
	Pyramidal.\n\n
	<b>FILTER_SAT</b>\n\n
	Summed Area.\n\n
	<b>FILTER_NADA</b>\n\n
	None. */
	virtual void SetFilterType(int ft)=0;
	/*! \remarks This method may be used to set the alpha source for the
	bitmap.
	\par Parameters:
	<b>int as</b>\n\n
	Alpha source types. One of the following values:\n\n
	<b>ALPHA_FILE</b>\n\n
	Image alpha (if present).\n\n
	<b>ALPHA_RGB</b>\n\n
	RGB Intensity.\n\n
	<b>ALPHA_NONE</b>\n\n
	None (opaque). */
	virtual void SetAlphaSource(int as)=0;  
	/*! \remarks Sets the end condition setting.
	\par Parameters:
	<b>int endcond</b>\n\n
	The end condition. One of the following values:\n\n
	<b>END_LOOP</b>\n\n
	<b>END_PINGPONG</b>\n\n
	<b>END_HOLD</b> */
	virtual void SetEndCondition(int endcond)=0;
	/*! \remarks The Mono Channel Intensity may be either RGB Intensity or
	Alpha.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for Alpha; FALSE for RGB Intensity. */
	virtual void SetAlphaAsMono(BOOL onoff)=0;
	/*! \remarks	The Alpha Source may be either from the RGB channels or Image Alpha
	channel.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for RGB; FALSE for Alpha channel. */
	virtual	void SetAlphaAsRGB(BOOL onoff)=0;
	virtual void SetPremultAlpha(BOOL onoff)=0;

	//! \brief Changes the name of the bitmap file.
	/*!	Changes the file name of the image to be used.
		\param[in] name - The new file name.
		\param[in] isUIAction - Should be set to true if and only if the renaming
		of the file is the result of a UI action from the user. When this is set
		to true, the bitmap texture may, for example, display a dialog asking the
		user whether the map should be cached by the bitmap proxy system.
	*/
	virtual void SetMapName(const MCHAR *name, bool isUIAction = false)=0;

	//! \brief Change the bitmap file.
	/*!	Changes the asset to be used.
	\param[in] asset - The new asset.
	\param[in] isUIAction - Should be set to true if and only if the renaming
	of the file is the result of a UI action from the user. When this is set
	to true, the bitmap texture may, for example, display a dialog asking the
	user whether the map should be cached by the bitmap proxy system.
	*/
	virtual void SetMap(const MaxSDK::AssetManagement::AssetUser& asset, bool isUIAction=false)=0;

	/*! \remarks Sets the start time setting.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The new start time. */
	virtual void SetStartTime(TimeValue t)=0;
	/*! \remarks Sets the playback rate setting.
	\par Parameters:
	<b>float r</b>\n\n
	The new playback rate. This is frames of the bitmap per frame of rendering
	time. If the value is 1 then you are playing 1 frame for every render
	frame. If it is 0.5 then the bitmap frame is held for 2 rendering frames.
	*/
	virtual void SetPlaybackRate(float r)=0;

	/*! \remarks Returns the filter type.
	\return  One of the following values:\n\n
	<b>FILTER_PYR</b>\n\n
	Pyramidal.\n\n
	<b>FILTER_SAT</b>\n\n
	Summed Area Table.\n\n
	<b>FILTER_NADA</b>\n\n
	None. */
	virtual int GetFilterType()=0;
	/*! \remarks Returns the alpha source.
	\return  One of the following values:\n\n
	<b>ALPHA_FILE</b>\n\n
	<b>ALPHA_RGB</b>\n\n
	<b>ALPHA_NONE</b> */
	virtual int GetAlphaSource()=0;
	/*! \remarks Returns the end condition setting.
	\return  One of the following values:\n\n
	<b>END_LOOP</b>\n\n
	<b>END_PINGPONG</b>\n\n
	<b>END_HOLD</b> */
	virtual int GetEndCondition()=0;
	/*! \remarks The Mono Channel Intensity may be either RGB Intensity or
	Alpha.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	This parameter is ignored.
	\return  TRUE if Alpha; FALSE if RGB Intensity. */
	virtual BOOL GetAlphaAsMono(BOOL onoff)=0;
	/*! \remarks	The Alpha Channel may be either RGB or Image Alpha.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	This parameter is ignored.
	\return  TRUE for RGB; FALSE for Image Alpha. */
	virtual	BOOL GetAlphaAsRGB(BOOL onoff)=0;
	virtual	BOOL GetPremultAlpha(BOOL onoff)=0;
	/*! \remarks Returns the name of the bitmap file. */
	virtual MCHAR *GetMapName()=0;

	/*! \remarks Returns an AssetUser of the bitmap file. */
	virtual const MaxSDK::AssetManagement::AssetUser& GetMap() = 0;
	/*! \remarks Returns the start frame setting as a TimeValue. */
	virtual TimeValue GetStartTime()=0;
	/*! \remarks Returns the playback rate setting. */
	virtual float GetPlaybackRate()=0;

	/*! \remarks Retrieves a pointer to the <b>StdUVGen</b> interface for this
	bitmap. This allows access to the mapping parameters such as UV offsets,
	blur, angle, noise level, etc. */
	/*! \remarks Retrieves a pointer to the <b>StdUVGen</b> interface for this
	bitmap. This allows access to the mapping parameters such as UV offsets,
	blur, angle, noise level, etc. */
	virtual StdUVGen* GetUVGen()=0;
	/*! \remarks Returns a pointer to a class to access TextureOutput
	properties of this texture. */
	virtual TextureOutput* GetTexout()=0;

	/*! \remarks	This will swap the bitmap pointer without updating BitmapInfo.
	\par Parameters:
	<b>Bitmap *bm</b>\n\n
	A pointer to the bitmap.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetBitmap(Bitmap *bm) {}
	/*! \remarks	Returns a pointer to the Bitmap associated with this Bitmap Texture.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the bitmap. */
	virtual Bitmap *GetBitmap(TimeValue t) { return NULL; }
	#pragma warning(pop)
	/*! \remarks	This method brings up a bitmap loader dialog.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual void BitmapLoadDlg() { }
	/*! \remarks	This method forces the bitmap to reload and the view to be redrawn.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual void ReloadBitmapAndUpdate() { }


//published functions		 

	FPInterfaceDesc* GetDesc();    // <-- must implement 

	/*! \remarks	This method reloads the bitmap texture and operates as if the user pressed
	the reload button. */
	virtual void	fnReload()=0;
	/*! \remarks	This method will view the bitmap texture image and operates as if the user
	pressed the view image button. */
	virtual void	fnViewImage()=0;

	/*! \remarks	This method will update the bitmap name and layer information of the texture
	with the information from the BitmapInfo structure*/
	virtual void	SetBitmapInfo(const BitmapInfo& /*bi*/) { }
	};

/*! \sa  Class Mtl.\n\n
\par Description:
This class provides access to the developer alterable properties of the 3ds Max
Multi/Sub-Object material.  */
class MultiMtl: public Mtl {
	public:
	/*! \remarks Sets the number of sub-materials for the multi-material.
	\par Parameters:
	<b>int n</b>\n\n
	The number of sub-materials.\n\n
	  */
	virtual void SetNumSubMtls(int n)=0;
	/*! \remarks	Retrieves the name of the sub-material whose ID is passed.
	\par Parameters:
	<b>int mtlid</b>\n\n
	The zero based index of the sub-material.\n\n
	<b>MSTR \&s</b>\n\n
	The name is returned here. */
	virtual void GetSubMtlName(int mtlid, MSTR &s)=0;
	/*! \remarks	Retrieves the name and pointer to the material for the specified
	sub-material.
	\par Parameters:
	<b>int mtlid</b>\n\n
	The zero based index of the sub-material.\n\n
	<b>Mtl *m</b>\n\n
	Points to the sub-material.\n\n
	<b>MSTR \&subMtlName</b>\n\n
	The name is returned here. */
	virtual void SetSubMtlAndName(int mtlid, Mtl *m, MSTR &subMtlName)=0;

	/*! \remarks Add a sub material with the specified material ID
	\param rt - The material to add as our sub-material
	\param mtlid - The ID to add rt at
	\param name - The name of the new sub material. */
	virtual void AddMtl(ReferenceTarget *rt, int mtlid, MCHAR *name)=0;

	/*! \remarks Remove the sub-material at the specified ID
	\param mtlid - the ID of the material to remove */
	virtual void RemoveMtl(int mtlid)=0;
	};

/*! \sa  Class Texmap, <a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials and Textures</a>.\n\n
\par Description:
Developers that have created a 3D Studio/DOS SXP and a corresponding 3ds Max
texture plug-in may want to subclass from this class. It provides a way to have
an instance of your 3ds Max texture plug-in created automatically when the
corresponding SXP is found in a 3DS file being imported.\n\n
This works as follows:\n\n
In the 3ds Max texture plug-in's implementation of <b>DllMain()</b> the
following function is called:\n\n
<b>void RegisterSXPReader(MCHAR *sxpName, Class_ID cid);</b>\n\n
The plug-in passes its own SXP name (i.e. "MARBLE_I.SXP") and its own
Class_ID.\n\n
The system then remembers this. When the 3DStudio import plug-ins is loading a
.3DS file and it encounters an SXP with this name, it will create an instance
of the plug-in class (using the Class_ID) and call the method of this class
<b>ReadSXPData()</b>. The plug-in can then initialize itself with proper values
by reading the old SXP data.\n\n
A sample plug-in that uses this technique is the 3ds Max Marble texture. It
imports the settings from the 3D Studio Marble SXP. See the sample code in
<b>/MAXSDK/SAMPLES/MATERIALS/MARBLE.CPP</b>.  */
class Tex3D: public Texmap {
	public:
	/*! \remarks This method is called when the 3D Studio/DOS import plug-in
	encounters an SXP with the name registered by <b>RegisterSXPReader()</b>.
	\par Parameters:
	<b>MCHAR *name</b>\n\n
	The name of the SXP.\n\n
	<b>void *sxpdata</b>\n\n
	This is the SXP's initialization data. The plug-in can look at this data to
	see what numbers it was initialized to in the .3DS file being imported. It
	can then set its initial value to match the SXP settings. */
	virtual void ReadSXPData(MCHAR *name, void *sxpdata)=0;
	};

/*! \sa  Class Texmap, Class Color, <a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials and
Textures</a>.\n\n
\par Description:
This class provides access to the developer settable properties of the 3ds Max
multi-textures such as Composite, Tint and Mix. All methods of this class are
implemented by the system.  */
class MultiTex: public Texmap {
	public:
	#pragma warning(push)
	#pragma warning(disable:4100)
	/*! \remarks Sets the number of sub-texmaps for this texmap.
	\par Parameters:
	<b>int n</b>\n\n
	The number of sub-texmaps. */
	virtual void SetNumSubTexmaps(int n) {}
	/*! \remarks Sets the color of the 'i-th' sub-texmap to the specified
	color at the time passed.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the sub-texmap to set.\n\n
	<b>Color c</b>\n\n
	The color to set.\n\n
	<b>TimeValue t=0</b>\n\n
	The time at which to set the color. */
	virtual void SetColor(int i, Color c, TimeValue t=0){}
	#pragma warning(pop)
};

/*! \sa  Class MultiTex, Class TextureOutput.\n\n
\par Description:
This class provides access to the 3ds Max Gradient texture. All methods of this
class are implemented by the system.  */
class GradTex: public MultiTex {
	public:
	/*! \remarks Retrieves a pointer to the <b>StdUVGen</b> interface for this
	texture. This allows access to the mapping parameters such as UV offsets,
	blur, angle, noise level, etc. */
	virtual StdUVGen* GetUVGen()=0;
	/*! \remarks Retrieves a pointer to the <b>TextureOutput</b> for this
	texture. This allows access to the RGB level and output and the output
	amount. */
	virtual TextureOutput* GetTexout()=0;
	/*! \remarks Sets the 'Color 2 Position' setting at the time passed.
	\par Parameters:
	<b>float m</b>\n\n
	The value to set in the range of 0.0 to 1.0.\n\n
	<b>TimeValue t=0</b>\n\n
	The time to set the value. */
	virtual void SetMidPoint(float m, TimeValue t=0) { UNUSED_PARAM(m); UNUSED_PARAM(t); }
};


//===============================================================================
// StdCubic
//===============================================================================
/*! \sa  Class Texmap.\n\n
\par Description:
This class provides access to the parameters of the standard 3ds Max
Reflect/Refract texture. All methods of this class are implemented by the
system.  */
class StdCubic: public Texmap {
	public:
	/*! \remarks Sets the map size parameter.
	\par Parameters:
	<b>int n</b>\n\n
	The size in pixels.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetSize(int n, TimeValue t)=0;
	/*! \remarks Sets the 'Every Nth Frame' or 'First Frame Only' toggle.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for 'Every Nth Frame'; FALSE for 'First Frame Only'. */
	virtual void SetDoNth(BOOL onoff)=0;
	/*! \remarks Sets the 'Nth Frame' parameter to the specified value.
	\par Parameters:
	<b>int n</b>\n\n
	The Nth Frame setting. */
	virtual void SetNth(int n)=0;
	/*! \remarks Sets or clears the 'Apply blur' checkbox.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetApplyBlur(BOOL onoff)=0;
	/*! \remarks Sets the blur setting to the specified value at the specified
	time.
	\par Parameters:
	<b>float b</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetBlur(float b, TimeValue t)=0;
	/*! \remarks Sets the blur offset setting to the specified value at the
	specified time.
	\par Parameters:
	<b>float b</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetBlurOffset(float b, TimeValue t)=0;
	/*! \remarks	Sets if the reflect / refract texture uses high dynamic range bitmaps or
	not. See <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working With
	Bitmaps</a> for details on high dynamic range bitmaps.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	Pass TRUE for on; FALSE for off. */
	virtual void UseHighDynamicRange(BOOL onoff)=0;
	/*! \remarks Returns the size setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual int GetSize(TimeValue t)=0;
	/*! \remarks Returns the state of the 'Every Nth Frame' or 'First Frame
	Only' toggle.
	\return  <b>BOOL onoff</b>\n\n
	TRUE is 'Every Nth Frame'; FALSE is 'First Frame Only'. */
	virtual BOOL GetDoNth()=0;
	/*! \remarks Returns the Nth Frame setting. */
	virtual int GetNth()=0;
	/*! \remarks Returns the state of the 'Apply blur' checkbox.
	\return  TRUE is on; FALSE is off. */
	virtual BOOL GetApplyBlur()=0;
	/*! \remarks Returns the blur setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetBlur(TimeValue t)=0;
	/*! \remarks Returns the blur offset setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the value. */
	virtual float GetBlurOffset(TimeValue t)=0;
	};

//===============================================================================
// StdMirror
//===============================================================================
/*! \sa  Class Texmap.\n\n
\par Description:
This class provides access to the 3ds Max Flat Mirror material. All methods of
this class are implemented by the system.  */
class StdMirror: public Texmap {
	public:
	/*! \remarks This method determines if 'Every Nth Frame' or 'First Frame
	Only' is used.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for Every Nth Frame; FALSE for First Frame Only. */
	virtual void SetDoNth(BOOL onoff)=0;
	/*! \remarks This methods controls the 'Nth Frame' value.
	\par Parameters:
	<b>int n</b>\n\n
	The number of frames. */
	virtual void SetNth(int n)=0;
	/*! \remarks This method controls the 'Apply Blur' check box setting.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE to toggle on; FALSE to toggle off. */
	virtual void SetApplyBlur(BOOL onoff)=0;
	/*! \remarks Sets the specified blur value at the specified time.
	\par Parameters:
	<b>float b</b>\n\n
	The blur value to set in the range 0.0 to 100.0\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the blur value. */
	virtual void SetBlur(float b, TimeValue t)=0;
	/*! \remarks	Sets if the mirror texture uses high dynamic range bitmaps or not. See
	<a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working With Bitmaps</a>
	for details on high dynamic range bitmaps.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	Pass TRUE for on; FALSE for off. */
	virtual void UseHighDynamicRange(BOOL onoff)=0;
	/*! \remarks Determines if 'Every Nth Frame' or 'First Frame Only' is
	used.
	\return  TRUE if Every Nth Frame is in use; FALSE if First Frame Only is in
	use. */
	virtual BOOL GetDoNth()=0;
	/*! \remarks Returns the Nth Frame setting. */
	virtual int GetNth()=0;
	/*! \remarks Returns TRUE if the Apply Blur check box is on; otherwise
	FALSE. */
	virtual BOOL GetApplyBlur()=0;
	/*! \remarks Returns the blur setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the blur setting. */
	virtual float GetBlur(TimeValue t)=0;
	};

//===============================================================================
// StdFog
//===============================================================================

// Fallof Types
#define FALLOFF_TOP		0
#define FALLOFF_BOTTOM	1
#define FALLOFF_NONE	2

/*! \sa  Class Atmospheric, Class Texmap.\n\n
\par Description:
This class provides access to the settings of the Standard Fog Atmospheric
plug-in of 3ds Max. All methods of this class are implemented by the system.
 */
class StdFog : public Atmospheric {
	public:
	/*! \remarks Sets the fog color.
	\par Parameters:
	<b>Color c</b>\n\n
	The color to set.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the color. */
	virtual void SetColor(Color c, TimeValue t)=0;
	/*! \remarks Sets the state of the 'Use Map' toggle.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetUseMap(BOOL onoff)=0;
	/*! \remarks Sets the state of the use opacity map toggle.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetUseOpac(BOOL onoff)=0;
	/*! \remarks Set the color map used.
	\par Parameters:
	<b>Texmap *tex</b>\n\n
	The map to set. */
	virtual void SetColorMap(Texmap *tex)=0;
	/*! \remarks Set the opacity map used.
	\par Parameters:
	<b>Texmap *tex</b>\n\n
	The map to set. */
	virtual void SetOpacMap(Texmap *tex)=0;
	/*! \remarks Sets the state of the fog background toggle.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetFogBackground(BOOL onoff)=0;
	/*! \remarks Set the type of fog, layered or standard.
	\par Parameters:
	<b>int type</b>\n\n
	The type of fog: 0 = Standard; 1 = Layered. */
	virtual void SetType(int type)=0;  // 0:Regular, 1:Layered
	/*! \remarks Sets the standard fog near percentage.
	\par Parameters:
	<b>float v</b>\n\n
	The value to set in the range 0 to 1.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetNear(float v, TimeValue t)=0;
	/*! \remarks Sets the standard fog far percentage.
	\par Parameters:
	<b>float v</b>\n\n
	The value to set in the range 0 to 1.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetFar(float v, TimeValue t)=0;
	/*! \remarks Sets the layered fog top value.
	\par Parameters:
	<b>float v</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetTop(float v, TimeValue t)=0;
	/*! \remarks Sets the layered fog bottom value.
	\par Parameters:
	<b>float v</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetBottom(float v, TimeValue t)=0;
	/*! \remarks Sets the layered fog density setting.
	\par Parameters:
	<b>float v</b>\n\n
	The value to set (\> 0).\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetDensity(float v, TimeValue t)=0;
	/*! \remarks Sets the falloff type to top, bottom or none.
	\par Parameters:
	<b>int tv</b>\n\n
	One of the following values:\n\n
	<b>FALLOFF_TOP</b>\n\n
	<b>FALLOFF_BOTTOM</b>\n\n
	<b>FALLOFF_NONE</b> */
	virtual void SetFalloffType(int tv)=0;
	/*! \remarks Sets the state of the 'Horizon Noise' toggle.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetUseNoise(BOOL onoff)=0;
	/*! \remarks Set the 'Horizon Noise Size' setting.
	\par Parameters:
	<b>float v</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetNoiseScale(float v, TimeValue t)=0;
	/*! \remarks Set the 'Horizon Noise Angle' setting.
	\par Parameters:
	<b>float v</b>\n\n
	The value to set in radians.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetNoiseAngle(float v, TimeValue t)=0;
	/*! \remarks Set the 'Horizon Noise Phase' setting.
	\par Parameters:
	<b>float v</b>\n\n
	The value to set.\n\n
	<b>TimeValue t</b>\n\n
	The time to set the value. */
	virtual void SetNoisePhase(float v, TimeValue t)=0;

	/*! \remarks Returns the fog color at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the color. */
	virtual Color GetColor(TimeValue t)=0;
	/*! \remarks Returns the state of the 'Use Map' toggle.
	\return  TRUE is on; FALSE is off. */
	virtual BOOL GetUseMap()=0;
	/*! \remarks Returns the state of the use opacity map toggle.
	\return  TRUE is on; FALSE is off. */
	virtual BOOL GetUseOpac()=0;
	/*! \remarks Returns the color map used. */
	virtual Texmap *GetColorMap()=0;
	/*! \remarks Returns the opacity map used. */
	virtual Texmap *GetOpacMap()=0;
	/*! \remarks Returns the state of the fog background toggle.
	\return  TRUE is on; FALSE is off. */
	virtual BOOL GetFogBackground()=0;
	/*! \remarks Returns the type of fog, layered or standard.
	\return  The type of fog: 0 = Standard; 1 = Layered. */
	virtual int GetType()=0;  // 0:Regular, 1:Layered
	/*! \remarks Returns the standard fog near percentage.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value. */
	virtual float GetNear(TimeValue t)=0;
	/*! \remarks Returns the standard fog far percentage.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value. */
	virtual float GetFar(TimeValue t)=0;
	/*! \remarks Returns the layered fog top value.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value. */
	virtual float GetTop(TimeValue t)=0;
	/*! \remarks Returns the layered fog bottom value.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value. */
	virtual float GetBottom(TimeValue t)=0;
	/*! \remarks Returns the layered fog density setting.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value. */
	virtual float GetDensity(TimeValue t)=0;
	/*! \remarks Returns the falloff type to top, bottom or none.
	\return  One of the following values:\n\n
	<b>FALLOFF_TOP</b>\n\n
	<b>FALLOFF_BOTTOM</b>\n\n
	<b>FALLOFF_NONE</b> */
	virtual int GetFalloffType()=0;
	/*! \remarks Returns the state of the 'Horizon Noise' toggle.
	\return  TRUE is on; FALSE is off. */
	virtual BOOL GetUseNoise()=0;
	/*! \remarks Returns the 'Horizon Noise Size' setting.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value. */
	virtual float GetNoiseScale( TimeValue t)=0;
	/*! \remarks Returns the 'Horizon Noise Angle' setting.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value. */
	virtual float GetNoiseAngle( TimeValue t)=0;
	/*! \remarks Returns the 'Horizon Noise Phase' setting.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value. */
	virtual float GetNoisePhase( TimeValue t)=0;

	};


// Subclasses of Tex3D call this on loading to register themselves
// as being able to read sxpdata for sxpName.  (name includes ".SXP")
CoreExport void RegisterSXPReader(MCHAR *sxpName, Class_ID cid);

// When importing, this is called to get a "reader" for the sxp being loaded.
CoreExport Tex3D *GetSXPReaderClass(MCHAR *sxpName);

//==========================================================================
// Create new instances of the standard materials, textures, and atmosphere
//==========================================================================
CoreExport StdMat2 *NewDefaultStdMat();
CoreExport BitmapTex *NewDefaultBitmapTex();
CoreExport MultiMtl *NewDefaultMultiMtl();
CoreExport MultiTex *NewDefaultCompositeTex();
CoreExport MultiTex *NewDefaultMixTex();

#ifndef NO_MAPTYPE_RGBTINT // orb 01-07-2001
CoreExport MultiTex *NewDefaultTintTex();
#endif // NO_MAPTYPE_RGBTINT

#ifndef NO_MAPTYPE_GRADIENT // orb 01-07-2001
CoreExport GradTex *NewDefaultGradTex();
#endif // NO_MAPTYPE_GRADIENT

#ifndef NO_MAPTYPE_REFLECTREFRACT // orb 01-07-2001
CoreExport StdCubic *NewDefaultStdCubic();
#endif // NO_MAPTYPE_REFLECTREFRACT

#ifndef NO_MAPTYPE_FLATMIRROR // orb 01-07-2001
CoreExport StdMirror *NewDefaultStdMirror();
#endif // NO_MAPTYPE_FLATMIRROR

CoreExport StdFog *NewDefaultStdFog();

