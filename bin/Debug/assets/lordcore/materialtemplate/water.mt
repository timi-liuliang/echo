<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="water.vs" ps="water.ps">
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
			<stage no="2" sampler = "BiLinearBORDER"/>
			<stage no="3" sampler = "BiLinearBORDER"/>
			<stage no="4" sampler = "BiLinearBORDER"/>
		</Texture>
		<Macros>
		</Macros>
	</ShaderPrograme>
</MaterialDesc>