<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="pbr.vs" ps="pbr.ps">
		<BlendState>
			<ColorWriteMask value="CMASK_ALL"/>
		</BlendState>
		<RasterizerState>
			<CullMode value="CULL_BACK"/>
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
			<stage no="5" sampler = "BiLinearMirror"/>
			<stage no="6" sampler = "BiLinearMirror"/>
		</Texture>
		<Macros>
			<Macro value="AMBIENT_OCCLUSION" default="false" />
			<Macro value="ALPHA_BLEND" default="false" />
			<Macro value="ALPHA_TEST" default="false" />
			<Macro value="TRANSPARENT" default="false" />
			<Macro value="TONE_MAPPING" default="false" />
			<Macro value="MERGE_GLOSS" default="true" />
			<Macro value="CONSERVE_ENERGY" default="true" />
		</Macros>
	</ShaderPrograme>
</MaterialDesc>
