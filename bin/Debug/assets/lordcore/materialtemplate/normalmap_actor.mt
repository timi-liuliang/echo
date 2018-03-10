<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="normalmap_actor.vs" ps="normalmap_actor.ps">
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
			<Macro value="NORMAL_MAP" default="false" />
			<Macro value="GLOSS_MAP" default="false" />
			<Macro value="DYNAMIC_SPECULAR" default="false" />
		</Macros>
	</ShaderPrograme>
</MaterialDesc>
