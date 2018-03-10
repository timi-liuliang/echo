<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="sample_water_a.vs" ps="sample_water_a.ps">
		<BlendState>
			<BlendEnable value="true"/>
			<BlendOP value="BOP_ADD"/>
			<SrcBlend value="BF_SRC_ALPHA"/>
			<DstBlend value="BF_INV_SRC_ALPHA"/>
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
			<BiLinearBORDER>
				<MinFilter value="FO_LINEAR"/>
				<MagFilter value="FO_LINEAR"/>
				<MipFilter value="FO_NONE"/>
				<AddrUMode value="AM_BORDER"/>
				<AddrVMode value="AM_BORDER"/>
			</BiLinearBORDER>
		</SamplerState>
		<Texture>
			<stage no="0" sampler = "BiLinearMirror"/>
			<stage no="1" sampler = "BiLinearMirror"/>
			<stage no="2" sampler = "BiLinearMirror"/>
		</Texture>
		<Macros>
		</Macros>
	</ShaderPrograme>
</MaterialDesc>