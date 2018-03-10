<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="scene_object.vs" ps="scene_object.ps">
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
				<AddrUMode value="AM_MIRROR"/>
				<AddrVMode value="AM_MIRROR"/>
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
            <Macro value="ALPHA_TEST" default="false" />
            <Macro value="TRANSPARENT" default="false" />
            <Macro value="EMISSIVE" default="false" />
            <Macro value="EFFECT_COLOR" default="false" />
			<Macro value="RECEIVE_FOG" default="true" />
			<Macro value="ENV_MAP" default="false" />
            <Macro value="USE_SCENE_ENV_MAP" default="true" />
            <Macro value="LIGHT_FROM_SCENE" default="true" />
            <Macro value="LIGHT_FROM_LIGHTARRAY" default="false" />
            <Macro value="COLOR_BLEND" default="false" />
            <Macro value="ENVMAP_ORDER" default="false" />
		</Macros>
	</ShaderPrograme>
</MaterialDesc>
