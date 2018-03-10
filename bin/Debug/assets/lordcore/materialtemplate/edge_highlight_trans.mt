<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="edge_highlight_trans.vs" ps="edge_highlight_trans.ps">
		<BlendState>
		</BlendState>
		<RasterizerState>
			<CullMode value="CULL_BACK"/>
		</RasterizerState>
		<DepthStencilState>
			<WriteDepth value="false"/>
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
		</Macros>
	</ShaderPrograme>
</MaterialDesc>
