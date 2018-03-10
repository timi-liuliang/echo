//////////////////////////////////////////////////////////////////////////////
//
//		Composite Shader Interface
//
//		Created: 9/27/01 Cleve Ard
//
#pragma once

#include "iFnPub.h"
// forward declarations
class RenderGlobalContext;

// ISpecularCompositeShader is used to let a shader choose the manner
// that it combines specular and diffuse lighting. The shader can use
// any information in the RenderGlobalContext to make this decision.

// Several shaders, include anisotropic, translucent, and multilayer
// combine specular and diffuse lighting using this equation:
//   L = specular + (1 - specular) * diffuse
// This only works when 0 <= specular <= 1. These shaders use this
// interface to determine whether we are rendering with a tone operator.
// If the render is using a tone operator, then the specular and diffuse
// lighting is simply added for the total lighting.

// In order for a StdMtl2 to ask for and call this interface for a shader
// it must return MTLREQ_PREPRO in it's LocalRequirements method.

/*!  \n\n
class ISpecularCompositeShader : public BaseInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
This class is only used to communicate some information between the 3ds Max
Standard Material and the Shaders for the 3ds Max Standard Material. The
information that the Shaders require that wasn't supplied previous is the
RenderGlobalContext. This would be used by 3rd parties who want to write
shaders for the 3ds Max standard material. This is only required if the shader
performs operations that are not valid for arbitrary light values. For example:
Using the default values, the Physical Sun has light multiplier values that are
around 60.\n\n
Several standard 3ds Max shaders, like the Aniso and ONB shaders, composite
specular highlights over the diffuse light, and the calculation looks something
like:\n\n
<b>color = diffuse * (1 - specular) + specular;</b>\n\n
This works fine as long as specular is between 0 and 1. If specular goes above
1, you get all kinds of unusual artifacts. This calculation is done this way to
prevent color clipping because of bright specular highlights. With lighting
values at 60, the specular is frequently greater than 1.With an exposure
control, the exposure control manages the color clipping and we can simply add
the diffuse and specular components.\n\n
This interface gives the shader a chance to investigate the render parameters
and decide how the lighting is to be combined.\n\n
This is usually done by using multiple inheritances and deriving the Shader
Implementation from Shader and ISpecularCompositeShader.\n\n
The Shader needs to override GetInterface(Interface_ID id) and return the
address of the ISpecularCompositShader interface and GetRequirements also needs
to be overriden to return MTLREQ_PREPRO as one of the shader requirements.\n\n
Then the shader needs to implement ChooseSpecularMethod. The typical
implementation is:\n\n
void CombineComponentsCompShader::ChooseSpecularMethod(TimeValue t,
RenderGlobalContext* rgc)\n\n
<b>{</b>\n\n
<b>useComposite = true;</b>\n\n
<b>if (rgc == NULL) {</b>\n\n
<b>ToneOperatorInterface* tint = static_cast\<ToneOperatorInterface*\>(</b>\n\n
<b>GetCOREInterface(TONE_OPERATOR_INTERFACE));</b>\n\n
<b>if (tint != NULL) {</b>\n\n
<b>ToneOperator* top = tint-\>GetToneOperator();</b>\n\n
<b>if (top != NULL \&\& top-\>Active(t))</b>\n\n
<b>useComposite = false;</b>\n\n
<b>}</b>\n\n
<b>} else {</b>\n\n
<b>ToneOperator* top = rgc-\>pToneOp;</b>\n\n
<b>if (top != NULL \&\& top-\>Active(t))</b>\n\n
<b>useComposite = false;</b>\n\n
<b>}</b>\n\n
<b>}</b>\n\n
   */
class ISpecularCompositeShader : public BaseInterface {
public:

	// Choose specular method
	/*! \remarks To be implemented Material Shaders. Provides a selection
	mechanism for the specular methods provided by various tone operators
	either provided by the System or by the implementing Shader.\n\n

	\par Parameters:
	<b>TimeValue t</b>\n\n
	Present scene time value.\n\n
	<b>RenderGlobalContext *rgc</b>\n\n
	Pointer to a RenderGlobalContext that encapsulates the shared data between
	a Standard Material and the implementing Shader.\n\n

	\return  None */
	virtual void ChooseSpecularMethod(TimeValue t, RenderGlobalContext* rgc) = 0;
};


#define ISPECULAR_COMPOSITE_SHADER_ID Interface_ID(0x5e2117d0, 0x327e2f73)

// Get the ISpecularCompositeShader method, if there is one.
inline ISpecularCompositeShader* GetSpecularCompositeShader(InterfaceServer* s)
{
	return static_cast<ISpecularCompositeShader*>(s->GetInterface(
		ISPECULAR_COMPOSITE_SHADER_ID));
}

// Choose the method for combining specular and diffuse lighting.
inline void ChooseSpecularMethod(InterfaceServer* s, TimeValue t, RenderGlobalContext* rgc)
{
	ISpecularCompositeShader* scs = GetSpecularCompositeShader(s);
	if (scs != NULL)
		scs->ChooseSpecularMethod(t, rgc);
}

