<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="static_lightmap.vs" ps="static_lightmap.ps">
		<BlendState>
		</BlendState>
		<RasterizerState>
		</RasterizerState>
		<DepthStencilState>
		</DepthStencilState>
		<SamplerState>
			<BiLinearMirror>
				<MinFilter value="FO_LINEAR"/>
				<MagFilter value="FO_LINEAR"/>
				<MipFilter value="FO_NONE"/>
				<AddrUMode value="AM_WRAP"/>
				<AddrVMode value="AM_WRAP"/>
			</BiLinearMirror>
		</SamplerState>
		<Texture>
			<stage no="0" sampler = "BiLinearMirror"/>
			<stage no="1" sampler = "BiLinearMirror"/>
			<stage no="2" sampler = "BiLinearMirror"/>
			<stage no="3" sampler = "BiLinearMirror"/>
			<stage no="4" sampler = "BiLinearMirror"/>
		</Texture>
		<Macros>
			<Macro value="SURFACE_COLOR" default="false" />
			<Macro value="SURFACE_COLOR_MASK" default="false" />
			<Macro value="EDGE_SHINE" default="false" />
			<Macro value="NORMAL_MAP" default="false" />
			<Macro value="FLOW_MAP" default="false" />
			<Macro value="DYNAMIC_SPECULAR" default="false" />
			<Macro value="ALPHA_TEST" default="false" />
			<Macro value="TRANSPARENT" default="false" />
			<Macro value="USE_LIGHTMAP" default="true" />
			<Macro value="VS_ANIM" default="false" />
			<Macro value="ENV_MAP" default="false" />
			<Macro value="RECEIVE_FOG" default="true" />
			<Macro value="FILTER_COLOR" default="true" />
			<Macro value="GROW" default="false" />
			<Macro value="EFFECT_COLOR" default="false" />
			<Macro value="EMISSIVE" default="false" />
			<Macro value="SPECULAR_COLOR_MAP" default="false" />
			<Macro value="UV_ROLLING" default="false" />
		</Macros>
		<DefaultUniformValue>
			<Unifrom name="filterGray" type="SPT_FLOAT" count="1" value="0" />
			<Unifrom name="DiffuseIntensity" type="SPT_FLOAT" count="1" value="1" />
			<Unifrom name="HighLight" type="SPT_FLOAT" count="1" value="0" />
			<Uniform name="LM1ToLM2" type="SPT_FLOAT" count="1" value="0" />
		</DefaultUniformValue>		
	</ShaderPrograme>
</MaterialDesc>
