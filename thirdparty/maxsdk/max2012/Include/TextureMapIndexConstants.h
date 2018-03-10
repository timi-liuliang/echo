

/*! \defgroup Material_TextureMap_IDs Texture Map Indices
The following are the texture map indices used by shader based materials. 
All materials that derive from class StdMat2 are shader based. Examples of shaders 
include Blinn, Phong, Constant, Metal, etc.
Since the shaders can arrange their texture map channels in any order, the standard 
texture map channels listed below need to be converted to the actual channel id 
of the shader via StdMat2::StdIDToChannel or BaseShader::StdIDToChannel.
For material plugins that are not shader based (not derived from StdMat2), the
standard texture map channels do not need to be converted.
\see class BaseShader, class Shader, class MtlBase, class StdMat2
*/
//@{
#define ID_AM 0   //!< Ambient 
#define ID_DI 1   //!< Diffuse
#define ID_SP 2   //!< Specular
#define ID_SH 3   //!< Glossiness (Shininess in 3ds Max release 2.0 and earlier)
#define ID_SS 4   //!< Specular Level (Shininess strength in 3ds Max release 2.0 and earlier)
#define ID_SI 5   //!< Self-illumination
#define ID_OP 6   //!< Opacity
#define ID_FI 7   //!< Filter color
#define ID_BU 8   //!< Bump 
#define ID_RL 9   //!< Reflection
#define ID_RR 10  //!< Refraction 
#define ID_DP 11  //!< Displacement 
#define NTEXMAPS 12
//@}
