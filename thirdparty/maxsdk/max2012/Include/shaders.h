//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "iparamb2.h"
#include "stdmat.h"
#include "buildver.h"

#define		N_ID_CHANNELS	16		//!< Number of IDs in stdMat
class Shader;
#define OPACITY_PARAM	0
#define DEFAULT_SOFTEN	0.1f

/**	A shader parameter dialog class. An instance of ShaderParamDlg 
 * is returned by a shader when it is asked by 3ds Max to display its rollup page.
 * \see ParamDlg, StdMat2, Shader
 */
class ShaderParamDlg : public ParamDlg {
	public:
		/** Returns the unique Class_ID of this object. */
		virtual Class_ID ClassID()=0;
		
		/** Sets the current shader (the one being edited) to the shader passed as a ReferenceTarget.
		 * \param m
		 * The pointer to the Shader to be set as the current shader. 
		 */
		virtual void SetThing(ReferenceTarget *m)=0;

		/** Sets the current Standard material and its shader (which are being
		 * edited) to those which are passed as inputs.
		 * \param pMtl The pointer to the standard material to be set as the current standard material.
		 * \param pShader The pointer to the shader to be set as the current shader. 
		 */
		virtual void SetThings( StdMat2* pMtl, Shader* pShader )=0;
		
		/** Returns the a pointer to the current material being edited. 
		 * Note that in most of the GetThing() or SetThing() methods in the
		 * SDK, the 'Thing' is referring to the actual plug-in. However, this does not hold true here. 
		 * This function returns the material using this Shader, not the actual plug-in.
		 */
		virtual ReferenceTarget* GetThing()=0;

		/** Returns a pointer to the current Shader. */
		virtual Shader* GetShader()=0;
		
		/** Called when the current time has changed.
		 * This gives the developer an opportunity to update any user interface data 
		 * that may need adjusting due to the change in time.
		 * \param t	The new current time.
		 */
		virtual void SetTime(TimeValue t) { UNUSED_PARAM(t); }	
		
		/** Called to delete this instance of the class.
		 * For dynamically created global utility plug-ins, this method has to be
		 * implemented and should have a implementation like 
		 \code
		 { delete this;	}
		 \endcode
		 */
		virtual void DeleteThis()=0;		
		
		/** The dialog procedure for the user interface controls of	the Shader.
		 * \param hwndDlg The window handle of the rollup page.
		 * \param msg The message to process.
		 * \param wParam The first dialog parameter.
		 * \param lParam The second dialog parameter.
		 * \return 
		 * In response to a WM_INITDIALOG message: Zero if the dialog box procedure 
		 * calls the SetFocus() function to set the focus to one of the controls
		 * in the dialog, Non-zero Otherwise. If non-zero, the system sets the focus to 
		 * the first control in the dialog that can be given the focus.
		 * \n In response to any message other than WM_INITDIALOG: Nonzero if the 
		 * procedure processes the message, zero otherwise.
		 */
		virtual INT_PTR PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )=0; 
		
		/** Loads the user interface controls with their current values.
		 * \param draw Not currently used.
		 */
		virtual void LoadDialog( int draw )=0;
		virtual void UpdateDialog( ParamID paramId )=0;

		/** Returns the window handle of the rollup	panel. */
		virtual HWND GetHWnd()=0;

		/** Returns the index of the sub-texture map corresponding to the window 
		 * whose handle is passed as the input. 
		 * \param hw The window handle to check. 
		 * \return The index of the sub-texmap corresponding to the window 
		 * whose handle is passed as the input. Returns -1 if the handle is not valid.
		 */
		virtual int  FindSubTexFromHWND(HWND hw)=0;
		
		/** Updates the opacity parameter of the plug-in in the user interface. */
		virtual void UpdateOpacity()=0;
		
		/** Updates the map buttons in the
		 * user interface. For example it can put a " " or "m" or "M" 
		 * on the button face based on the state of the map. 
		 */
		virtual void UpdateMapButtons()=0;
};
 
// Can be or'd w/ mtl, not sure it's necessary
#define SELFILLUM_CLR_ON	(1<<16) 

/** \defgroup Components_defines Components defines
 * These are the bits for the active components of bump, reflection, refraction and opacity mapping. 
 * If the bit is set that component is active. This provides a quick way for a Shader to check if 
 * they're used (as opposed to looking through the channels array searching for these channel types).
 */
//@{
#define HAS_BUMPS				0x01L		//!< Indicates that the bump mapping is present.
#define HAS_REFLECT				0x02L		//!< Indicates that there is any kind of reflection (raytraced, etc).
#define HAS_REFRACT				0x04L		//!< Indicates that there is any kind of refraction.
#define HAS_OPACITY				0x08L		//!< Indicates that opacity mapping is used.
/** Indicates that there is a reflection map only.
 * This is used by the Strauss shader for example. 
 * If it sees a reflection map present it dims the diffuse channel.
 */
#define HAS_REFLECT_MAP			0x10L
#define HAS_REFRACT_MAP			0x20L		//!< Indicates that there is a refraction map only.
#define HAS_MATTE_MTL			0x40L
//@}


/** \defgroup Texture_channel_type_flags Texture channel type flags
 * There are four channels which are part of the Material which are not specific to the Shader. 
 * All other channels are defined by the Shader (what they are and what they are called).
 * The four which are not the province of the Shader are Bump, Reflection, Refraction and Displacement. 
 * For example, Displacement mapping is really a geometry operation and not a shading one. 
 * The channel type returned from this method indicates if the specified channel is one of these, or 
 * if it is a monochrome channel, a color channel, or is not a supported channel. 
 * \see BaseShader::ChannelType() 
 */
//@{
#define UNSUPPORTED_CHANNEL		0x01L		//!< Indicates that the channel is not supported (is not used).
#define CLR_CHANNEL				0x02L		//!< A color channel. The Color.r, Color.g and Color.b parameters are used.
#define MONO_CHANNEL			0x04L		//!< A monochrome channel. Only the Color.r is used.
#define BUMP_CHANNEL			0x08L		//!< The bump mapping channel.
#define REFL_CHANNEL			0x10L		//!< The reflection channel.
#define REFR_CHANNEL			0x20L		//!< The refraction channel.
#define DISP_CHANNEL			0x40L		//!< The displacement channel.
#define SLEV_CHANNEL			0x80L		
/** Indicates that the channel is not supported. 
 * For example, a certain Shader might not support displacement mapping for some reason. 
 * If it didn't, it could use this channel type to eliminate the support of displacement 
 * mapping for itself. It would be as if displacement mapping was not included in the material. 
 * None of the 3ds Max shaders use this.
 */
#define ELIMINATE_CHANNEL		0x8000L
/** This is used internally to indicate that the channels to be skipped */
#define SKIP_CHANNELS	(UNSUPPORTED_CHANNEL+BUMP_CHANNEL+REFL_CHANNEL+REFR_CHANNEL) 
 //@}


#define  DEFAULT_SHADER_CLASS_ID BLINNClassID //!< Class Id upper half for loading the Pre 3.0 shaders

#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
#define  PHONGClassID (STDSHADERS_CLASS_ID+2)
#define  METALClassID (STDSHADERS_CLASS_ID+4)
#endif // USE_LIMITED_STDMTL


#define  BLINNClassID (STDSHADERS_CLASS_ID+3)				//!< For Blinn Shader
static Class_ID BlinnClassID(STDSHADERS_CLASS_ID + 3,0);	//!< For Blinn Shader

#define ANISOSHADER_CLASS_ID 0x2857f460							//!< For Anisotropic Shader
static Class_ID AnisoShaderClassID( ANISOSHADER_CLASS_ID, 0);	//!< For Anisotropic Shader
static Class_ID AnisoShaderDlgClassID( ANISOSHADER_CLASS_ID, 0);//!< For Anisotropic Shader

/** BlockID Anisotropic Shader */
enum Shader_Anisotropic_Shader_BlockID
{
	aniso_params
};

/** ParamID's for Anisotropic Shader */
enum Shader_Anisotropic_Shader_ParamIDs
{ 
	an_ambient, an_diffuse, an_specular, an_self_illum_color,
	an_diffuse_level, an_specular_level, an_self_illum_amnt, 
	an_glossiness, an_anisotropy, an_orientation, 
	an_map_channel, an_ad_texlock, an_ad_lock, an_ds_lock, an_use_self_illum_color, 
};


#define NEWSHADERS_CLASS_ID      0x2857f420									//!< For Oren/Nayar/Blinn Shader
static Class_ID OrenNayarBlinnShaderClassID( NEWSHADERS_CLASS_ID + 1, 0);	//!< For Oren/Nayar/Blinn Shader
static Class_ID OrenNayarShaderDlgClassID( NEWSHADERS_CLASS_ID + 2, 0);		//!< For Oren/Nayar/Blinn Shader

/** BlockID Oren Nayer Blinn Shader */
enum Shader_Oren_Nayar_Blinn_BlockID
{
	onb_params
};
/** ParamID's for Oren Nayer Blinn Shader */
enum Shader_Oren_Nayar_Blinn_ParamIDs
{ 
	onb_ambient, onb_diffuse, onb_specular,
	onb_self_illum_color, onb_self_illum_amnt,
	onb_glossiness, onb_specular_level, onb_soften,
	onb_diffuse_level, onb_roughness, 
	onb_ad_texlock, onb_ad_lock, onb_ds_lock, onb_use_self_illum_color, 
};



#define STRAUSS_SHADER_CLASS_ID		0x2857f450						//!< For Strauss Shader
static Class_ID StraussShaderClassID( STRAUSS_SHADER_CLASS_ID, 0);	//!< For Strauss Shader

/** BlockID for Strauss Shader */
enum Shader_Strauss_BlockID
{
	strauss_params
};

/** ParamID's for Strauss Shader */
enum Shader_Strauss_ParamIDs
{ 
	st_diffuse, st_glossiness, st_metalness, 
};

class ParamBlockDescID;
class IParamBlock;

/** One of the base classes for the creation of the Shaders, which plug-in to
 * the Standard material[?]. 
 * This class is available in release 3.0 and later only. Note that developers should derive 
 * their plug-in Shader from Class Shader and not from this class directly. If not, the 
 * interactive renderer will not know how to render the Shader in the viewports.
 * Developers of this plug-in type also need to have an understanding of how the Standard 
 * material and the Shader work together.
 * Every material has a Shader. The Shader is the piece of code which controls how
 * light is reflected off the surface. The Standard material is the mapping mechanism which 
 * handles all the texturing for the material. Managing the user interface is also done by 
 * the standard material. This way the Shader plug-in needs only to worry about the interaction 
 * of light on the surface.\n\n
 * Prior to release 3, developers could write Material plug-ins that performed
 * their own shading. However, this was usually a major programming task. Release 3
 * provides the simpler Shader plug-in that would benefit from sharing all the
 * common capabilities. The Standard material, with its 'blind' texturing
 * mechanism, makes this possible. It doesn't know what it is texturing - it
 * simply textures 'stuff'. The shader names the channels (map), fills in the
 * initial values, and specifies if they are a single channel (mono) or a triple
 * channel (color). The Standard material handles the rest including managing the
 * user interface.
 * The main part of the code in a Shader is responsible for supplying this information to a
 * Standard material. The values are passed and received back in class IllumParams.
 * There is a specific method named Illum() in a shader, which actually does the shading. 
 * \par Plug-In Information:
 * Class defined in: SHADER.H\n
 * Super class ID: SHADER_CLASS_ID\n
 * Standard file name extension: DLB\n
 * Extra include file needed: SHADERS.H
 * \see SpecialFX, ShaderParamDlg, ShadeContext, IllumParams, IMtlParams, StdMat2
 * Mtl, Color, ILoad, ISave
*/
class BaseShader : public SpecialFX {
	public:
		#pragma warning(push)
		#pragma warning(disable:4100)
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message) {return REF_SUCCEED;}

		SClass_ID SuperClassID() {return SHADER_CLASS_ID;}

		/** Wants to promote the shader properties to the material level. */
		BOOL BypassPropertyLevel() { return TRUE; }  

		/** Returns the requirements of the Shader for the specified
		 * sub-material. Many objects in the rendering pipeline use the
		 * requirements to tell the renderer what data needs to be available. The
		 * shader's requirements are OR'd with the combined map requirements and
		 * returned to the renderer via the Stdmtl2's GetRequirements() function.
		 * \param subMtlNum This parameter is not used.
		 * \return  One or more flag from \ref materialRequirementsFlags
		 * \see materialRequirementsFlags (defined in imtl.h)
		 */
		virtual ULONG GetRequirements(int subMtlNum)=0;

		/** Put up a dialog that lets the user edit the plug-ins parameters.
		 * This method creates and returns a pointer to a ShaderParamDlg 
		 * object and puts up the dialog which lets the user edit the Shader's parameters.
		 * \param hOldRollup The window handle of the old rollup. If non-NULL, 
		 * the IMtlParams::ReplaceRollupPage() method is usually used instead of IMtlParams::AddRollupPage() 
		 * to present the rollup. 
		 * \param hwMtlEdit	The window handle of the material editor.
		 * \param imp The interface pointer for calling methods in 3ds Max.
		 * \param theMtl Pointer to the Standard material being edited.
		 * \param rollupOpen True to have the UI rollup open; false to have it closed.
		 * \param n	This parameter is available in release 4.0 and later only. Specifies the 
		 * number of the rollup to create. Reserved for future use with multiple rollups. 
		 */
		virtual ShaderParamDlg* CreateParamDialog(
			HWND hOldRollup, 
			HWND hwMtlEdit, 
			IMtlParams *imp, 
			StdMat2* theMtl, 
			int rollupOpen, 
			int n=0) = 0;

		/** Returns the number of rollups this shader is requesting.*/
		virtual int NParamDlgs(){ return 1; }

		/** Returns a pointer to the ShaderParamDlg object
		 * which manages the user interface.
		 * \param n	Specifies the rollup to get ShaderParamDlg for. Reserved for
		 * future use with multiple rollups. 
		 */
		virtual ShaderParamDlg* GetParamDlg(int n=0 )=0;
		
		/** Sets the ShaderParamDlg object which manages the
		 * user interface to the one passed.
		 * \param newDlg Points to the new ShaderParamDlg object.
		 * \param n	Specifies the rollup to set ShaderParamDlg for. Reserved for
		 * future use with multiple rollups.
		 */
		virtual void SetParamDlg( ShaderParamDlg* newDlg, int n=0 )=0;

		/** Saves and loads the plug-in's name. These should be called at the start of
		 * a plug-in's Save() method.
		 * \param *isave An interface for saving data. 
		 */
		IOResult Save(ISave *isave) { return SpecialFX::Save(isave); }
		
		/** Loads the plug-in's name. This should be called at the
		 * start of a plug-in's Load() method.
		 * \param *iload An interface for loading data. */
		IOResult Load(ILoad *iload) { return SpecialFX::Load(iload); }

		/** Std parameter support. Returns a value which indicates which of the standard
		 * parameters are supported.
		 * \return A Shader Standard Parameter Flags. 
		 * \see Standard_Parameter_flags
		 */
		virtual ULONG SupportStdParams()=0;

		/** Required for R2.5 shaders to convert
		 * the previous Standard material parameter blocks to the current version.
		 * \param descOld Pointer to the old parameter block descriptor.
		 * \param oldCount The number in the array of parameters above.
		 * \param oldPB	Pointer to the old parameter block. 
		 */
		virtual void ConvertParamBlk( ParamBlockDescID *descOld, int oldCount, IParamBlock *oldPB ){};

		/** Updates the "channels"(as well as other) data
		 * member of the "IllumParams" object passed to it with the "local"
		 * variables of the material for possible mapping prior to being given to the
		 * Shader's Illum() method. The shader plug-in copies the state of all its
		 * parameters (at their current animation state) into the data members of the
		 * "IllumParams" passed.
		 * \param ip Pointer to the IllumParams to update. 
		 */
		virtual void GetIllumParams( ShadeContext &sc, IllumParams& ip )=0;

		/** This is the illumination function for the Shader.
		 * Developers will find it very helpful to review the Mtl::Shade()
		 * method of the Standard material. This is the main method of the
		 * material which computes the color of the point being rendered. This
		 * code is available in /MAXSDK/SAMPLES/MATERIALS/STDMTL2.CPP. This
		 * code shows how the Standard calls Shader::GetIllumParams(), sets
		 * up mapping channels, calls this Illum() method, and calls the
		 * Shader::CombineComponents() method when all done.
		 * \param sc The ShadeContext which provides information on the pixel being
		 * shaded.
		 * \param ip The object whose data members provide communication between 3ds Max and
		 * the shader. Input values are read from here and output values are
		 * also stored here. Note that the "XOut" (ambIllumout, etc) data
		 * members of this class are initialized to 0 before the first call to
		 * this method.
		 * The input to this method has the textured illumination parameters, the
		 * bump perturbed normal, the view vector, the raw (unattenuated) colors
		 * in the reflection and refraction directions, etc.
		 * \par Sample Code:
		 * Below is a brief analysis of the standard Blinn shader Illum() method.
		 * This is the standard 'computer graphics look' type shader supplied with
		 * 3ds Max. The entire method follows:\n\n
		 * \code
		
		void Blinn2::Illum(ShadeContext &sc, IllumParams &ip)
		{
			LightDesc *l;
			Color lightCol;
		
		// Blinn style phong
			BOOL is_shiny= (ip.channels[ID_SS].r > 0.0f) ? 1:0;
			double phExp = pow(2.0, ip.channels[ID_SH].r * 10.0) *	4.0;
		
			for (int i=0; i<sc.nLights; i++) {
				l = sc.Light(i);
				register float NL, diffCoef;
				Point3 L;
				if (l->Illuminate(sc,ip.N,lightCol,L,NL,diffCoef)) {
					if (l->ambientOnly) {
						ip.ambIllumOut += lightCol;
						continue;
					}
					if (NL<=0.0f)
						continue;
		
		// diffuse
					if (l->affectDiffuse)
						ip.diffIllumOut += diffCoef * lightCol;
		
		// specular (Phong2)
					if (is_shiny&&l->affectSpecular) {
						Point3 H = Normalize(L-ip.V);
						float c = DotProd(ip.N,H);
						if (c>0.0f) {
							if (softThresh!=0.0 && diffCoef<softThresh) {
								c *= Soften(diffCoef/softThresh);
							}
							c = (float)pow((double)c, phExp);
							ip.specIllumOut += c * ip.channels[ID_SS].r * lightCol;
						}
					}
				}
			}
		
		// Apply mono self illumination
			if ( ! selfIllumClrOn ) {
				float si = ip.channels[ID_SI].r;
				ip.diffIllumOut = (si>=1.0f) ? Color(1.0f,1.0f,1.0f):ip.diffIllumOut * (1.0f-si) + si;
			}
			else {
		// colored self illum,
				ip.selfIllumOut += ip.channels[ID_SI];
			}
		// now we can multiply by the clrs,
			ip.ambIllumOut *= ip.channels[ID_AM];
			ip.diffIllumOut *= ip.channels[ID_DI];
			ip.specIllumOut *= ip.channels[ID_SP];
		
		// the following is applicable only in R4
			int chan = ip.stdIDToChannel[ ID_RL ];
			ShadeTransmission(sc, ip, ip.channels[chan], ip.refractAmt);
			chan = ip.stdIDToChannel[ ID_RR ];
			ShadeReflection( sc, ip, ip.channels[chan] );
			CombineComponents( sc, ip );
		}
		\endcode 

		* Some of the key parts of this method are discussed below:\n\n
 		* The <b>is_shiny</b> line sets a boolean based on if the Shader has a
		* shininess setting \> 0. Note that the Blinn shader uses the same
		* channel ordering as the original Standard material so it can index its
		* channels using the standard ID <b>ID_SS</b>.\n\n
		* <b>BOOL is_shiny= (ip.channels[ID_SS].r \> 0.0f) ? 1:0;</b>\n\n
		* Next the 'Phong Exponent' is computed. This is just a function that is
		* used to give a certain look. It uses 2^(Shinniness *10) * 4. This
		* calculation simply 'feels right' and gives a good look.\n\n
		* <b>double phExp = pow(2.0, ip.channels[ID_SH].r * 10.0) * 4.0;</b>\n\n
		* The following loop sums up the effect of each light on this point on
		* surface.\n\n
		* <b>for (int i=0; i\<sc.nLights; i++) {</b>\n\n
		* Inside the loop, the light descriptor is grabbed from the
		* ShadeContext:\n\n
		* <b>  l = sc.Light(i);</b>\n\n
		* The <b>LightDesc::Illuminate()</b> method is then called to compute
		* some data:\n\n
		* <b>  if (l-\>Illuminate(sc,ip.N,lightCol,L,NL,diffCoef))
		* {</b>\n\n
		* To <b>Illuminate()</b> is passed the ShadeContext (<b>sc</b>), and the
		* normal to the surface (<b>ip.N</b>) (pointing away from the surface
		* point).\n\n
		* The method returns the light color (<b>lightCol</b>), light vector
		* (<b>L</b>) (which points from the surface point to the light), the dot
		* product of N and L (<b>NL</b>) and the diffuse coefficient
		* (<b>diffCoef</b>). The diffuse coefficient is similar to NL but has the
		* atmosphere between the light and surface point taken into account as
		* well.\n\n
		* The next piece of code checks if the light figures into the
		* computations:\n\n
		* <b>   if (NL\<=0.0f)</b>\n\n
		* <b>    continue;</b>\n\n
		* If NL\<0 then the cosine of the vectors is greater than 90 degrees.
		* This means the light is looking at the back of the surface and is
		* therefore not to be considered.\n\n
		* The next statement checks if the light affect the diffuse channel
		* (lights may toggle on or off their ability to affect the diffuse and
		* specular channels.)\n\n
		* <b>   if (l-\>affectDiffuse)</b>\n\n
		* <b>    ip.diffIllumOut += diffCoef *
		* lightCol;</b>\n\n
		* If the light affects the diffuse channel then the diffuse illumination
		* output component of the <b>IllumParams</b> is added to. This is done by
		* multiplying the diffuse coefficient (returned by
		* <b>LightDesc::Illuminate()</b>) times the light color (also returned by
		* <b>LightDesc::Illuminate()</b>). Notice that the <b>diffIllumOut</b> is
		* being accumulated with each pass of the light loop.\n\n
		* The next section of code involves the specular component. If the light
		* is shiny, and it affects the specular channel a vector <b>H</b> is
		* computed.\n\n
		* <b>   if (is_shiny\&\&l-\>affectSpecular) {</b>\n\n
		* Note the following about this <b>H</b> vector computation. Most vectors
		* are considered pointing <b>from</b> the point on the surface. The view
		* vector (<b>IllumParams::V</b>) does not follow this convention. It
		* points from the 'eye' towards the surface. Thus it's reversed from the
		* usual convention.\n\n
		* <b>H</b> is computed to be the average of <b>L</b> and <b>V</b>. This
		* is <b>(L+V)/2</b>. Since we normalize this we don't have to divide by
		* the 2. So, if <b>V</b> followed the standard convention this would be
		* simply <b>L+V</b>. Since it doesn't it is <b>L+(-ip.V)</b> or
		* <b>L-ip.V</b>.\n\n
		* <b>    Point3 H = Normalize(L-ip.V);</b>\n\n
		* Next the dot product of <b>N</b> and <b>H</b> is computed and stored in
		* <b>c</b>. When you take the dot product of two normalized vectors what
		* is returned is the cosine of the angle between the vectors.\n\n
		* <b>    float c = DotProd(ip.N,H); </b>\n\n
		* If c\>0 and the diffuse coefficient is less than the soften threshold
		* then <b>c</b> is modified by a 'softening' curve.
		* \code
		if (c>0.0f)
		{
			if (softThresh!=0.0 &&
			diffCoef<softThresh) {
				c *=
					Soften(diffCoef/softThresh);
			}
		}
		\endcode  
		Note that the <b>Soften()</b> function is defined in
		<b>/MAXSDK/SAMPLES/MATERIALS/SHADERUTIL.CPP</b> and developers can copy
		this code.
		\code
		c = (float)pow((double)c,phExp);
		\endcode 
		Next, <b>c</b> is raised to the power of the Phong exponent. This is
		effectively taking a cosine (a smooth S curve) and raising it to a
		power. As it is raised to a power it becomes a sharper and sharper S
		curve. This is where the shape of the highlight curve in the Materials
		Editor UI comes from.\n\n
		That completes the pre computations for the specular function. Then
		<b>c</b> is multiplied by the specular strength
		(<b>ip.channels[ID_SS].r</b>) times the light color (<b>lightCol</b>).
		The result is summed in specular illumination out
		(<b>ip.specIllumOut</b>).\n\n
		\code
		ip.specIllumOut += c * ip.channels[ID_SS].r * lightCol;
		\endcode 
		That completes the light loop. It happens over and over for each
		light.\n\n
		Next the self illumunation is computed. If the Self Illumination Color
		is not on, then the original code for doing mono self illumination is
		used.
		\code
		// Apply mono self illumination
		if ( ! selfIllumClrOn )
		{
			float si = ip.channels[ID_SI].r;
			ip.diffIllumOut = (si>=1.0f) ? Color(1.0f,1.0f,1.0f)
				:
			ip.diffIllumOut * (1.0f-si) + si;
		}
		else
		{
		// Otherwise the self illumination color is summed in to the Self Illumination 
			Out (ip.selfIllumOut).
		// colored self illum,
			ip.selfIllumOut += ip.channels[ID_SI];
		}
		\endcode 
		* Then, we multiply by the colors for ambient, diffuse and specular.\n\n
		\code
		ip.ambIllumOut *= ip.channels[ID_AM];
		ip.diffIllumOut *= ip.channels[ID_DI];
		ip.specIllumOut *= ip.channels[ID_SP];
		\endcode 
		* The results are <b>ambIllumOut</b>, <b>diffIllumOut</b>, and
		* <b>specIllumOut</b>. Note that these components are not summed. In R3
		* and earlier these results would be returned to the Standard material.
		* However, R4 introduces a couple extra steps.\n\n
		* Finally, we call <b>ShadeTransmission()</b> and
		* <b>ShadeReflection()</b> to apply the transmission/refraction and
		* reflection models. The core implementation of 3ds Max provides the
		* standard models, but the shader can override these methods to compute
		* its own models.\n\n
		\code
		int chan = ip.stdIDToChannel[ ID_RL ];
		ShadeTransmission(sc, ip, ip.channels[chan],ip.refractAmt);
		chan = ip.stdIDToChannel[ ID_RR ];
		ShadeReflection( sc, ip, ip.channels[chan] );
		\endcode 
		* In R4, It is a shader's responsibility to combine the components of the
		* shading process prior to exiting <b>Illum()</b> (in R3, this was the
		* responsibility of the Standard material). In order to combine these
		* values together to produce the final color for that point on the
		* surface (<b>IllumParams.finalC</b>), the shader should call
		* <b>CombineComponents()</b> method. The Shader base class provides a
		* default implementation which simply sums everything together and
		* multiplies by the opacity.\n\n
		\code
		virtual void CombineComponents( IllumParams& ip )
		{
			ip.finalC = ip.finalOpac * (ip.ambIllumOut + ip.diffIllumOut + 	ip.selfIllumOut)
				+ ip.specIllumOut + ip.reflIllumOut + ip.transIllumOut ;
		}
		\endcode */
		virtual void Illum(ShadeContext &sc, IllumParams &ip)=0;

// begin - ke/mjm - 03.16.00 - merge reshading code
		// these support the pre-shade/reshade protocol
//		virtual void PreIllum(ShadeContext &sc, IReshadeFragment* pFrag){}
//		virtual void PostIllum(ShadeContext &sc, IllumParams &ip, IReshadeFragment* pFrag ){ Illum(sc,ip); }

		// >>>> new for V4, one call superceded, 2 new ones added
		/**	Compute the reflected color from the <b>sc</b>, <b>ip</b>, and
		 * reflection map (or ray) color. The core implementation of this provides
		 * the standard 3ds Max reflection model. To support the standard
		 * reflection model, a shader may call this default implementation.
		 * \param sc The context which provides information on the pixel being shaded.
		 * \param ip The object whose data members provide communication between 3ds Max and
		 * the shader.
		 * \param mapClr The input reflection (or ray) color is passed in here and the resulting
		 * 'affected' color is stored here. 
		 */
		virtual void ShadeReflection(ShadeContext &sc, IllumParams &ip, Color &mapClr){}
		
		/**	Compute the transmission/refraction color for the sample. The core
		 * implementation of this provides the standard 3ds Max reflection model.
		 * To support the standard transmission/refraction model, a shader may
		 * call this default implementation.
		 * \param sc The context which provides information on the pixel being shaded.
		 * \param ip The object whose data members provide communication between 3ds Max and
		 * the shader.
		 * \param mapClr The input refraction (or ray) color is passed in here and the resulting
		 * 'affected' color is stored here.
		 * \param amount The level of the amount spinner for the refraction channel. */
		virtual void ShadeTransmission(ShadeContext &sc, IllumParams &ip, Color &mapClr, float amount){}
			
		/**
		 * \deprecated 
		 * This method was superceded by ShadeReflection() and is obsolete in 
		 * release 4.0 and later.
		 * This method provides the shader with an opportunity to affect the
		 * reflection code.
		 * \param sc The ShadeContext which provides information on the pixel being	shaded.
		 * \param ip The object whose data members provide communication between 3ds Max and
		 * the shader.
		 * \param rcol The input reflection color is passed in here, and the resulting 
		 * 'affected' color is stored here too.
		 * \par Sample Code:
		 * A simple example like Phong does the following:
		 * \code
		 * void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
		 * {
		 *		rcol *= ip.channels[ID_SP];
		 * };
		 * \endcode 
		 * If a color can affect the reflection of light off a surface than it can
		 * usually affect the reflection of other things off a surface. Thus some
		 * shaders influence the reflection color using the specular color and
		 * specular level. For instance the Multi Layer Shader does the following:
		 * \code
		 * #define DEFAULT_GLOSS2 0.03f
		 * void MultiLayerShader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
		 * {
		 * 	float axy = DEFAULT_GLOSS2;
		 * 	float norm = 1.0f / (4.0f * PI * axy );
		 * 	rcol *= ip.channels[_SPECLEV1].r * ip.channels[_SPECCLR1] * norm;
		 * }
		 * \endcode  */
		virtual void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol){}

		/** Finalizes the compositing of the various
		 * illumination components. A default implementation is provided which
		 * simply adds the components together. Developers who want to do other
		 * more specialized composition can override this method. For example, a
		 * certain Shader might want to composited highlights over the underlying
		 * diffuse component since the light is reflected and the diffuse color
		 * wouldn't fully show through. Such a Shader would provide its own
		 * version of this method.
		 * \param sc The ShadeContext which provides information on the pixel being
		 * shaded.
		 * \param ip The illumination parameters to composite and store.
		 */
		virtual void CombineComponents( ShadeContext &sc, IllumParams& ip ){};
		#pragma warning(pop)
		
		/** Returns the number of texture map map channels supported
		by this Shader. */
		virtual long nTexChannelsSupported()=0;
		
		/** Returns the name of the specified texture map channel.
		 * \param nTextureChan The zero based index of the texture map channel whose name is returned.
		 */
		virtual MSTR GetTexChannelName( long nTextureChan )=0;
		
		/** Returns the internal name of the specified texture map.
		 * The Standard material uses this to get the fixed, parsable internal
		 * name for each texture channel it defines.
		 * \param nTextureChan The zero based index of the texture map whose name is returned.
		 */
		virtual MSTR GetTexChannelInternalName( long nTextureChan ) { return GetTexChannelName(nTextureChan); }
		
		/** Returns the channel type for the specified texture map
		 * channel. There are four channels which are part of the Material which
		 * are not specific to the Shader. All other channels are defined by the
		 * Shader (what they are and what they are called.) The four which are not
		 * the province of the Shader are Bump, Reflection, Refraction and
		 * Displacement. For example, Displacement mapping is really a geometry
		 * operation and not a shading one. The channel type returned from this
		 * method indicates if the specified channel is one of these, or if it is
		 * a monochrome channel, a color channel, or is not a supported channel.
		 * \param nTextureChan The zero based index of the texture map whose name is returned.
		 * \return  Texture channel type flags. \see Texture_channel_type_flags
		 */
		virtual long ChannelType( long nTextureChan )=0;
		
		/** Returns the index of this Shader's channels which
		 * corresponds to the specified Standard materials texture map ID. This
		 * allows the Shader to arrange its channels in any order it wants in the
		 * IllumParams::channels array but enables the Standard material to
		 * access specific ones it needs (for instance the Bump channel or
		 * Reflection channel).
		 * \param stdID The ID whose corresponding channel to return. \see Material_TextureMap_IDs 
		 * "List of Material Texture Map Indices".
		 * \return  The zero based index of the channel. -1 if there is not a
		 * corresponding channel.
		 * \par Sample Code:
		 * This can be handled similar to below where an array is initialized with
		 * the values of this plug-in shader's channels that correspond to each of
		 * the standard channels. Then this method just returns the correspond
		 * index from the array.
		 * \code
		 * static int stdIDToChannel[N_ID_CHANNELS] =
		 * {
		 * 0, 1, 2, 5, 4, -1, 7, 8, 9, 10, 11, 12
		 * };
		 * long StdIDToChannel(long stdID){ return stdIDToChannel[stdID]; }
		 * \endcode 
		 */
		virtual long StdIDToChannel( long stdID )=0;

		/** Indicates the number of UserIllum output channels used by the shader. */
		virtual long nUserIllumOut(){ return 0; } 

		/** Returns a static array of channel names for matching by render elements. */
		virtual MCHAR** UserIllumNameArray(){ return NULL; } 

		/** Called when the Shader is first activated
		 * in the dropdown list of Shader choices. The Shader should reset itself
		 * to its default values. 
		 */
		virtual void Reset()=0;	

	};

/// \defgroup Chunk_IDs Chunk IDs saved by base class
//@{
#define SHADERBASE_CHUNK	0x39bf
#define SHADERNAME_CHUNK	0x0100
//@}

/** \defgroup Standard_Parameter_flags Standard Parameter flags
* Standard parameter flags for shaders. A combination of these flags is returned by 
* Shader::SupportStdParams()
 */
//@{
#define STD_PARAM_NONE			(0)				//!< Indicates that none of the flags in the Standard_Parameters is used
#define STD_PARAM_ALL			(0xffffffffL)	//!< Indicates that all of the flags in the Standard_Parameters are used
#define STD_PARAM_METAL			(1)				//!< Only used by the Metal shader
#define STD_PARAM_LOCKDS		(1<<1)			//!< Indicates support for the Diffuse/Specular lock.
#define STD_PARAM_LOCKAD		(1<<2)			//!< Indicates support for the Ambient/Diffuse lock.
#define STD_PARAM_LOCKADTEX		(1<<3)			//!< Indicates support for the Ambient/Diffuse texture lock.
#define STD_PARAM_SELFILLUM		(1<<4)			//!< Indicates support for the Self Illumination parameter.
#define STD_PARAM_SELFILLUM_CLR	(1<<5)			//!< Indicates support for the Self Illumination color parameter.
#define STD_PARAM_AMBIENT_CLR	(1<<6)			//!< Indicates support for the Ambient color parameter.
#define STD_PARAM_DIFFUSE_CLR	(1<<7)			//!< Indicates support for the Diffuse color parameter.
#define STD_PARAM_SPECULAR_CLR	(1<<8)			//!< Indicates support for the Specular color parameter.
#define STD_PARAM_FILTER_CLR	(1<<9)			//!< Indicates support for the Filter color parameter.
#define STD_PARAM_GLOSSINESS	(1<<10)			//!< Indicates support for the Glossiness parameter.
#define STD_PARAM_SOFTEN_LEV	(1<<11)			//!< Indicates support for the Soften Level parameter.
#define STD_PARAM_SPECULAR_LEV	(1<<12)			//!< Indicates support for the Specular Level parameter.
#define STD_PARAM_DIFFUSE_LEV	(1<<13)			//!< Indicates support for the Diffuse Level parameter.
#define STD_PARAM_DIFFUSE_RHO	(1<<14)			//!< Indicates support for the Roughness parameter.
#define STD_PARAM_ANISO			(1<<15)			//!< Indicates support for the Specular Highlight Anisotropy parameter.
#define STD_PARAM_ORIENTATION	(1<<16)			//!< Indicates support for the Specular Highlight Orientation parameter.
#define STD_PARAM_REFL_LEV		(1<<17)			//!< This is reserved for future use.
#define STD_PARAM_SELFILLUM_CLR_ON		(1<<18)	//!< Indicates support for the Self Illumination Color On/Off checkbox.
/** This bit is only set by the three pre-R3 shaders (Phong, Blinn, and Metal).
 * If this bit is not set then the Basic Parameters dialog is replaced by the one
 * provided by the plug-in shader. 
 */
#define STD_BASIC2_DLG			(1<<20)			 
/** Indicates support for the Extended Parameters rollout. If this bit is not set 
 * one provided by the plug-in shader will be used instead. The following three flags,
 * when set, enable the specified controls in the Extended Parameters rollout.
 */
#define STD_EXTRA_DLG			(1<<21)			
/** Indicates support for Reflection Dimming parameters (Apply, Dim Level, Refl Level). 
 * Not including this in the param string disables the relevant params in extra params 
 * dialog. */
#define STD_EXTRA_REFLECTION	(1<<22)			
/** Indicates support for Index of Refraction parameter. Not including this 
 * in the param string disables the relevant params in extra params dialog.
 */
#define STD_EXTRA_REFRACTION	(1<<23)			
/** Indicates support for Opacity parameters (Amount, In/Out, Type). 
 * Not including this in the param string disables the relevant params 
 * in extra params dialog.
 */
#define STD_EXTRA_OPACITY		(1<<24)			
//@}



#define STD_EXTRA	(STD_EXTRA_DLG \
					+STD_EXTRA_REFLECTION+STD_EXTRA_REFRACTION \
					+STD_EXTRA_OPACITY )

#define STD_BASIC	(0x00021ffeL | STD_BASIC2_DLG)

#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
#define STD_BASIC_METAL	(0x00021fffL | STD_BASIC2_DLG)
#define STD_ANISO	(0x0002cffe)
#define STD_MULTILAYER	(0x0002fffe)
#define STD_ONB		(0x00023ffe)
#define STD_WARD	(0x00000bce)
#endif // USE_LIMITED_STDMTL



/** Used by developers to create Shader plug-ins. The methods of this class must be implemented
 * to provide data to the 3ds Max interactive renderer. This will let that
 * renderer to properly reflect the look of the shader in the
 * viewports. The methods associated with the actual Shader illumination code are
 * from the base class BaseShader.
 * There are various Get and Set methods defined in this class. Plug-in developers
 * provide implementations for the 'Get' methods which are used by the interactive
 * renderer. The implementations of the 'Set' methods are used when switching
 * between shaders types in the Materials Editor. This is used to transfer the
 * corresponding colors between the old Shader and the new one.
 * Note that some shaders may not have the exact parameters as called for in the
 * methods. In those case an approximate value may be returned from the 'Get'
 * methods. For example, the Strauss Shader doesn't have an Ambient channel. In
 * that case the Diffuse color is taken and divided by 2 and returned as the
 * Ambient color. This gives the interactive renderer something to work with that
 * might not be exact but is somewhat representative.  
 * \see BaseShader, MacroRecorder
 */
class Shader : public BaseShader, public IReshading
{
	public:
	#pragma warning(push)
	#pragma warning(disable:4100)
	/** Copies the standard shader parameters from
	 * pFrom to this object. Note that plug-ins typically disable the macro
	 * recorder during this operation as the Get and Set methods are called. See
	 * the sample code for examples.
	 * \param pFrom The pointer to the source parameters. 
	 */
	virtual void CopyStdParams( Shader* pFrom )=0;
	
	// these are the standard shader params

	/** Sets the state of the Diffuse/Specular lock to on or off.
	 * \param lock true for on, false for off. 
	 */
	virtual void SetLockDS(BOOL lock)=0;
	
	/** Returns the Diffuse/Specular lock'
	 * \return true if the Diffuse/Specular lock is on, false otherwise. */
	virtual BOOL GetLockDS()=0;

	/** Sets the state of the Ambient/Diffuse lock to on or off.
	 * \param lock	true for on, false for off. 
	 */
	virtual void SetLockAD(BOOL lock)=0;

	/** Returns the Ambient/Diffuse lock.
	 * \returns true if the Ambient/Diffuse lock is on, false otherwise.*/
	virtual BOOL GetLockAD()=0;
	
	/** Sets the state of the Ambient/Diffuse Texture lock to on or	off.
	 * \param lock true for on; false for off. 
	 */
	virtual void SetLockADTex(BOOL lock)=0;

	/** Returns the Ambient/Diffuse Texture lock.
	 * \returns true if the Ambient/Diffuse Texture lock is on, false otherwise.*/
	virtual BOOL GetLockADTex()=0;

	/** Sets the Self Illumination parameter to the specified value
	 * at the time passed as TimeValue.
	 * \param v The value to set.
	 * \param t The time to set the value. 
	 */
	virtual void SetSelfIllum(float v, TimeValue t)=0;		

	/** Sets the Self Illumination Color On/Off state.
	 * \param on true for on, false for off. 
	 */
	virtual void SetSelfIllumClrOn( BOOL on )=0;

	/** Sets the Self Illumination Color at the specified time.
	 * \param c The color for the self illumination to set to.
	 * \param t The time to set the color. 
	 */
	virtual void SetSelfIllumClr(Color c, TimeValue t)=0;		

	/** Sets the Ambient Color at the specified time.
	 * \param c The color for the ambient to set to.
	 * \param t The time to set the color. 
	 */
	virtual void SetAmbientClr(Color c, TimeValue t)=0;		

	/** Sets the Diffuse Color at the specified time.
	 * \param c The color for the diffuse color to set to.
	 * \param t The time to set the color. 
	 */
	virtual void SetDiffuseClr(Color c, TimeValue t)=0;		

	/** Sets the Specular Color at the specified time.
	 * \param c The color to set to.
	 * \param t The time to set the color. */
	virtual void SetSpecularClr(Color c, TimeValue t)=0;

	/** Sets the Glossiness parameter to the specified value at the
	 * time passed.
	 * \param v	The value to set to.
	 * \param t	The time to set the value. */
	virtual void SetGlossiness(float v, TimeValue t)=0;		

	/** Sets the Specular Level parameter to the specified value at
	 * the time passed.
	 * \param v The value to set.
	 * \param t	The time to set the value. */
	virtual void SetSpecularLevel(float v, TimeValue t)=0;		

	/** Sets the Soften Specular Highlights Level to the specified
	 * value at the time passed.
	 * \param v The value to set.
	 * \param t The time to set the value. */
	virtual void SetSoftenLevel(float v, TimeValue t)=0;
		
	/** Returns the The Self Illumination Color setting.
	 * parameters to this method are not applicable and may safely be ignored.
	 * \return true if the Self Illumination Color setting is on
	 *(checked), false otherwise.
	 */
	virtual BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace)=0;

	/** Returns the Ambient Color.
	 * The parameters to this method are not applicable and may safely be ignored.
	 */
	virtual Color GetAmbientClr(int mtlNum, BOOL backFace)=0;		
    
	/** Returns the Diffuse Color.
     * The parameters to this method are not applicable and may safely be ignored.
     */
    virtual Color GetDiffuseClr(int mtlNum, BOOL backFace)=0;		

	/** Returns the Specular Color.
	 * The parameters to this method are not applicable and may safely be ignored.
	 */
	virtual Color GetSpecularClr(int mtlNum, BOOL backFace)=0;
	
	/** Returns the Self Illumination Color.
	 * The parameters to this method are not applicable and may safely be ignored.
	*/
	virtual Color GetSelfIllumClr(int mtlNum, BOOL backFace)=0;
	
	/** Returns the Self Illumination Amount.
	* The parameters to this method are not applicable and may safely be ignored.
	*/
	virtual float GetSelfIllum(int mtlNum, BOOL backFace)=0;
	
	/** Returns the Glossiness Level.
	 * The parameters to this method are not applicable and may safely be ignored.
	 */
	virtual float GetGlossiness(int mtlNum, BOOL backFace)=0;	
	
	/** Returns the Specular Level.
	 * The parameters to this method are not applicable and may safely be ignored.
	 */
	virtual float GetSpecularLevel(int mtlNum, BOOL backFace)=0;
	
	/** Returns the Soften Level as a float.
	 * The parameters to this method are not applicable and may safely be ignored.
	 */
	virtual float GetSoftenLevel(int mtlNum, BOOL backFace)=0;

	/** Returns the Self Illumination Color setting.
	 * \return true if the Self Illumination Color setting is on
	 * (checked), false if it is off */
	virtual BOOL IsSelfIllumClrOn()=0;
	
	/** Returns the Ambient Color at the specified time.
	 * \param t	The time at which to return the color. */
	virtual Color GetAmbientClr(TimeValue t)=0;		
	
	/** Returns the Diffuse Color at the specified time.
	 * \param t	The time at which to return the color. */
	virtual Color GetDiffuseClr(TimeValue t)=0;		
	
	/** Returns the Specular Color at the specified time.
	 * \param t	The time at which to return the color. */
	virtual Color GetSpecularClr(TimeValue t)=0;
	
	/** Returns the Glossiness value at the specified time.
	 * \param t	The time at which to return the value. */

	virtual float GetGlossiness( TimeValue t)=0;		
	/** Returns the Specular Level at the specified time.
	 * \param t	The time at which to return the value. */
	virtual float GetSpecularLevel(TimeValue t)=0;
	
	/** Returns the Soften Specular Highlights setting at the
	 * specified time.
	 * \param t	The time at which to return the value. */
	virtual float GetSoftenLevel(TimeValue t)=0;
	
	/** Returns the Self Illumination Amount at the specified time.
	 * \param t	The time at which to return the value. */
	virtual float GetSelfIllum(TimeValue t)=0;		
	
	/** Returns the Self Illumination Color at the specified time.
	 * \param t	The time at which to return the color. */
	virtual Color GetSelfIllumClr(TimeValue t)=0;		

	/** Evaluates the hightlight curve that	appears in the Shader user interface.
	 * This gets called from the DrawHilite() function which is available to 
	 * developers in /MAXSDK/SAMPLES/MATERIALS/SHADER/SHADERUTIL.CPP 
	 * \param x The input value.
	 * \return  The output value on the curve. A value of 1.0 represents the top
	 * of the curve as it appears in the UI. Values greater than 1.0 are okay and
	 * simply appear off the top of the graph.
	 */
	virtual float EvalHiliteCurve(float x){ return 0.0f; }

	/** This is the highlight curve function for the two highlight
	 * curves which intersect and appear in the UI, for instance in the Anistropic
	 * shader.
	 * \param x The x input value.
	 * \param y The y input value.
	 * \param level This is used by multi-layer shaders to indicate which layer to draw. The
	 * draw highlight curve routines use this when redrawing the graph.
	 * \return The output value of the curve. */
	virtual float EvalHiliteCurve2(float x, float y, int level = 0 ){ return 0.0f; }

	/** Implements the Max std way of handling reflection to provide the default implementation.*/
	CoreExport void ShadeReflection(ShadeContext &sc, IllumParams &ip, Color &mapClr);
	/** Implements the Max std way of handling transmission to provide the default implementation.*/
	CoreExport void ShadeTransmission(ShadeContext &sc, IllumParams &ip, Color &mapClr, float amount);

	// Reshading
	void PreShade(ShadeContext& sc, IReshadeFragment* pFrag){}
	void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip){ Illum( sc, *ip ); }
	#pragma warning(pop)
	// [dl | 13march2003] Adding this inlined definition to resolve compile errors
    BaseInterface* GetInterface(Interface_ID id) { return BaseShader::GetInterface(id); }
	void* GetInterface(ULONG id){
		if( id == IID_IReshading )
			return (IReshading*)( this );
	//	else if ( id == IID_IValidityToken )
	//		return (IValidityToken*)( this );
		else
			return BaseShader::GetInterface(id);
	}
};



