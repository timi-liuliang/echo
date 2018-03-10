<?xml version = "1.0" encoding = "GB2312"?>
<MaterialDesc>
	<ShaderPrograme vs="sky.vs" ps="sky2.ps">
		<BlendState>
		</BlendState>
		<RasterizerState>
		</RasterizerState>
		<DepthStencilState>
			<DepthEnable value="true"/>
			<WriteDepth value="false"/>
		</DepthStencilState>
		<SamplerState>
			<BiLinearWrap>
				<MinFilter value="FO_LINEAR"/>
				<MagFilter value="FO_LINEAR"/>
				<MipFilter value="FO_NONE"/>
				<AddrUMode value="AM_WRAP"/>
				<AddrVMode value="AM_WRAP"/>
			</BiLinearWrap>
		</SamplerState>
		<Texture>
			<stage no="0" sampler = "BiLinearWrap"/>
			<stage no="1" sampler = "BiLinearWrap"/>
			<stage no="2" sampler = "BiLinearWrap"/>
			<stage no="3" sampler = "BiLinearWrap"/>
			<stage no="4" sampler = "BiLinearWrap"/>
		</Texture>
		<Macros>
			<Macro value="DOUBLE_SKY" default="false" />
			<Macro value="RECEIVE_FOG" default="false" />
			<Macro value="HEIGHT_FOG" default="true" />
		</Macros>
	</ShaderPrograme>
</MaterialDesc>
