<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="general.vs" ps="general.ps">
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
			<Macro value="SURFACE_COLOR" default="false" />
			<Macro value="SURFACE_COLOR_MASK" default="false" />
			<Macro value="EDGE_SHINE" default="false" />
			<Macro value="NORMAL_MAP" default="false" />
			<Macro value="MASK_MAP" default="false" />
			<Macro value="FLOW_MAP" default="false" />
			<Macro value="ALPHA_TEST" default="false" />
			<Macro value="TRANSPARENT" default="false" />
			<Macro value="COVERAGE_ALPHA" default="false" />
			<Macro value="USE_LIGHTMAP" default="false" />
			<Macro value="VS_ANIM" default="false" />
			<Macro value="EFFECT_COLOR" default="false" />
		</Macros>
	</ShaderPrograme>
</MaterialDesc>
