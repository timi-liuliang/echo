<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="normalmap_actor_pbr.vs" ps="normalmap_actor_pbr.ps">
		<BlendState>
			<ColorWriteMask value="CMASK_ALL"/>
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
			<Macro value="ALPHA_TEST" default="false"/>
			<Macro value="NORMAL_MAP" default="false" />
			<Macro value="GLOSS_MAP" default="false" />
			<Macro value="ENV_MAP" default="false" />
			<Macro value="ENV_MAP_CUBE" default="false"/>
			<Macro value="DYNAMIC_SPECULAR" default="false"/>
			<Macro value="HSV_COLOR_MASK" default="false"/>
			<Macro value="EFFECT_COLOR" default="false" />
			<Macro value="SPECULAR_COLOR_MAP" default="false" />
			<Macro value="FLOW_MAP" default="false" />
			<Macro value="RIME_LIGHT" default="true" />
			<Macro value="SPRITE_ANIMATION" default="false"/>
		</Macros>
		<DefaultUniformValue>
			<Unifrom name="uFadeScale" type="SPT_FLOAT" count="1" value="0" />
		</DefaultUniformValue>		
	</ShaderPrograme>
</MaterialDesc>
