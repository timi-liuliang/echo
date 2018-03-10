<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="catui_ui.vs" ps="catui_ui.ps">
		<BlendState>
			<BlendEnable value="true"/> 
			<SrcBlend value="BF_SRC_ALPHA"/>
			<DstBlend value="BF_INV_SRC_ALPHA"/>
			</BlendState>
		<RasterizerState>
		<CullMode value="CULL_NONE"/>	
		</RasterizerState>
		<DepthStencilState>
			<DepthEnable value="false"/>
			<WriteDepth value="false"/>
		</DepthStencilState>
		<SamplerState>
			<BiLinearMirror>
				<MinFilter value="FO_LINEAR"/>
				<MagFilter value="FO_LINEAR"/>
				<MipFilter value="FO_NONE"/>
				<AddrUMode value="AM_CLAMP"/>
				<AddrVMode value="AM_CLAMP"/>
			</BiLinearMirror>
		</SamplerState>
		<Texture>
			<stage no="0" sampler = "BiLinearMirror"/>
		</Texture>
		<VertexFormats>
			<VertexFormat>
				<VertexSemantic value="VS_POSITION"/>
				<PixelFormat value="PF_RGB32_FLOAT"/>
			</VertexFormat>
			<VertexFormat>
				<VertexSemantic value="VS_COLOR"/>
				<PixelFormat value="PF_RGBA8_UNORM"/>
			</VertexFormat>
			<VertexFormat>
				<VertexSemantic value="VS_TEXCOORD"/>
				<PixelFormat value="PF_RG32_FLOAT"/>
			</VertexFormat>
		</VertexFormats>
	</ShaderPrograme>
</MaterialDesc>
